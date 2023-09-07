// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    core::type::Manager& ty{ir->Types()};

    /// Process the module.
    void Process() {
        // Find module-scope variables that need to be replaced.
        if (ir->root_block) {
            Vector<Instruction*, 4> to_remove;
            for (auto inst : *ir->root_block) {
                auto* var = inst->As<Var>();
                if (!var) {
                    continue;
                }
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (!ptr) {
                    continue;
                }
                auto* storage_texture = ptr->StoreType()->As<core::type::StorageTexture>();
                if (storage_texture &&
                    storage_texture->texel_format() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceVar(var, storage_texture);
                    to_remove.Push(var);
                }
            }
            for (auto* remove : to_remove) {
                remove->Destroy();
            }
        }

        // Find function parameters that need to be replaced.
        for (auto* func : ir->functions) {
            for (uint32_t index = 0; index < func->Params().Length(); index++) {
                auto* param = func->Params()[index];
                auto* storage_texture = param->Type()->As<core::type::StorageTexture>();
                if (storage_texture &&
                    storage_texture->texel_format() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceParameter(func, param, index, storage_texture);
                }
            }
        }
    }

    /// Replace a variable declaration with one that uses rgba8unorm instead of bgra8unorm.
    /// @param old_var the variable declaration to replace
    /// @param bgra8 the bgra8unorm texture type
    void ReplaceVar(Var* old_var, const core::type::StorageTexture* bgra8) {
        // Redeclare the variable with a rgba8unorm texel format.
        auto* rgba8 = ty.Get<core::type::StorageTexture>(
            bgra8->dim(), core::TexelFormat::kRgba8Unorm, bgra8->access(), bgra8->type());
        auto* new_var = b.Var(ty.ptr(handle, rgba8));
        auto bp = old_var->BindingPoint();
        new_var->SetBindingPoint(bp->group, bp->binding);
        new_var->InsertBefore(old_var);
        if (auto name = ir->NameOf(old_var)) {
            ir->SetName(new_var, name.NameView());
        }

        // Replace all uses of the old variable with the new one.
        ReplaceUses(old_var->Result(), new_var->Result());
    }

    /// Replace a function parameter with one that uses rgba8unorm instead of bgra8unorm.
    /// @param func the function
    /// @param old_param the function parameter to replace
    /// @param index the index of the function parameter
    /// @param bgra8 the bgra8unorm texture type
    void ReplaceParameter(Function* func,
                          FunctionParam* old_param,
                          uint32_t index,
                          const core::type::StorageTexture* bgra8) {
        // Redeclare the parameter with a rgba8unorm texel format.
        auto* rgba8 = ty.Get<core::type::StorageTexture>(
            bgra8->dim(), core::TexelFormat::kRgba8Unorm, bgra8->access(), bgra8->type());
        auto* new_param = b.FunctionParam(rgba8);
        if (auto name = ir->NameOf(old_param)) {
            ir->SetName(new_param, name.NameView());
        }

        Vector<FunctionParam*, 4> new_params = func->Params();
        new_params[index] = new_param;
        func->SetParams(std::move(new_params));

        // Replace all uses of the old parameter with the new one.
        ReplaceUses(old_param, new_param);
    }

    /// Recursively replace the uses of @p value with @p new_value.
    /// @param old_value the value whose usages should be replaced
    /// @param new_value the value to use instead
    void ReplaceUses(Value* old_value, Value* new_value) {
        old_value->ForEachUse([&](Usage use) {
            tint::Switch(
                use.instruction,
                [&](Load* load) {
                    // Replace load instructions with new ones that have the updated type.
                    auto* new_load = b.Load(new_value);
                    new_load->InsertBefore(load);
                    ReplaceUses(load->Result(), new_load->Result());
                    load->Destroy();
                },
                [&](CoreBuiltinCall* call) {
                    // Replace arguments to builtin functions and add swizzles if necessary.
                    call->SetOperand(use.operand_index, new_value);
                    if (call->Func() == core::Function::kTextureStore) {
                        // Swizzle the value argument of a `textureStore()` builtin.
                        auto* tex = old_value->Type()->As<core::type::StorageTexture>();
                        auto index = core::type::IsTextureArray(tex->dim()) ? 3u : 2u;
                        auto* value = call->Args()[index];
                        auto* swizzle = b.Swizzle(value->Type(), value, Vector{2u, 1u, 0u, 3u});
                        swizzle->InsertBefore(call);
                        call->SetOperand(index, swizzle->Result());
                    } else if (call->Func() == core::Function::kTextureLoad) {
                        // Swizzle the result of a `textureLoad()` builtin.
                        auto* swizzle =
                            b.Swizzle(call->Result()->Type(), nullptr, Vector{2u, 1u, 0u, 3u});
                        call->Result()->ReplaceAllUsesWith(swizzle->Result());
                        swizzle->InsertAfter(call);
                        swizzle->SetOperand(Swizzle::kObjectOperandOffset, call->Result());
                    }
                },
                [&](UserCall* call) {
                    // Just replace arguments to user functions and then stop.
                    call->SetOperand(use.operand_index, new_value);
                },
                [&](Default) {
                    TINT_ICE() << "unhandled instruction " << use.instruction->FriendlyName();
                });
        });
    }
};

}  // namespace

Result<SuccessType, std::string> Bgra8UnormPolyfill(Module* ir) {
    auto result = ValidateAndDumpIfNeeded(*ir, "Bgra8UnormPolyfill transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
