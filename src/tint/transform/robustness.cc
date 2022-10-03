// Copyright 2020 The Tint Authors.
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

#include "src/tint/transform/robustness.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Robustness::Config);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

/// State holds the current transform state
struct Robustness::State {
    /// The clone context
    CloneContext& ctx;

    /// Set of address spacees to not apply the transform to
    std::unordered_set<ast::AddressSpace> omitted_classes;

    /// Applies the transformation state to `ctx`.
    void Transform() {
        ctx.ReplaceAll([&](const ast::IndexAccessorExpression* expr) { return Transform(expr); });
        ctx.ReplaceAll([&](const ast::CallExpression* expr) { return Transform(expr); });
    }

    /// Apply bounds clamping to array, vector and matrix indexing
    /// @param expr the array, vector or matrix index expression
    /// @return the clamped replacement expression, or nullptr if `expr` should be cloned without
    /// changes.
    const ast::IndexAccessorExpression* Transform(const ast::IndexAccessorExpression* expr) {
        auto* sem =
            ctx.src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::IndexAccessorExpression>();
        auto* ret_type = sem->Type();

        auto* ref = ret_type->As<sem::Reference>();
        if (ref && omitted_classes.count(ref->AddressSpace()) != 0) {
            return nullptr;
        }

        ProgramBuilder& b = *ctx.dst;

        // idx return the cloned index expression, as a u32.
        auto idx = [&]() -> const ast::Expression* {
            auto* i = ctx.Clone(expr->index);
            if (sem->Index()->Type()->UnwrapRef()->is_signed_integer_scalar()) {
                return b.Construct(b.ty.u32(), i);  // u32(idx)
            }
            return i;
        };

        auto* clamped_idx = Switch(
            sem->Object()->Type()->UnwrapRef(),  //
            [&](const sem::Vector* vec) -> const ast::Expression* {
                if (sem->Index()->ConstantValue()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }

                return b.Call("min", idx(), u32(vec->Width() - 1u));
            },
            [&](const sem::Matrix* mat) -> const ast::Expression* {
                if (sem->Index()->ConstantValue()) {
                    // Index and size is constant.
                    // Validation will have rejected any OOB accesses.
                    return nullptr;
                }

                return b.Call("min", idx(), u32(mat->columns() - 1u));
            },
            [&](const sem::Array* arr) -> const ast::Expression* {
                const ast::Expression* max = nullptr;
                if (arr->IsRuntimeSized()) {
                    // Size is unknown until runtime.
                    // Must clamp, even if the index is constant.
                    auto* arr_ptr = b.AddressOf(ctx.Clone(expr->object));
                    max = b.Sub(b.Call("arrayLength", arr_ptr), 1_u);
                } else if (auto count = arr->ConstantCount()) {
                    if (sem->Index()->ConstantValue()) {
                        // Index and size is constant.
                        // Validation will have rejected any OOB accesses.
                        return nullptr;
                    }
                    max = b.Expr(u32(count.value() - 1u));
                } else {
                    // Note: Don't be tempted to use the array override variable as an expression
                    // here, the name might be shadowed!
                    ctx.dst->Diagnostics().add_error(diag::System::Transform,
                                                     sem::Array::kErrExpectedConstantCount);
                    return nullptr;
                }

                return b.Call("min", idx(), max);
            },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled object type in robustness of array index: "
                    << ctx.src->FriendlyName(ret_type->UnwrapRef());
                return nullptr;
            });

        if (!clamped_idx) {
            return nullptr;  // Clamping not needed
        }

        auto src = ctx.Clone(expr->source);
        auto* obj = ctx.Clone(expr->object);
        return b.IndexAccessor(src, obj, clamped_idx);
    }

    /// @param type builtin type
    /// @returns true if the given builtin is a texture function that requires
    /// argument clamping,
    bool TextureBuiltinNeedsClamping(sem::BuiltinType type) {
        return type == sem::BuiltinType::kTextureLoad || type == sem::BuiltinType::kTextureStore;
    }

    /// Apply bounds clamping to the coordinates, array index and level arguments
    /// of the `textureLoad()` and `textureStore()` builtins.
    /// @param expr the builtin call expression
    /// @return the clamped replacement call expression, or nullptr if `expr`
    /// should be cloned without changes.
    const ast::CallExpression* Transform(const ast::CallExpression* expr) {
        auto* call = ctx.src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* call_target = call->Target();
        auto* builtin = call_target->As<sem::Builtin>();
        if (!builtin || !TextureBuiltinNeedsClamping(builtin->Type())) {
            return nullptr;  // No transform, just clone.
        }

        ProgramBuilder& b = *ctx.dst;

        // Indices of the mandatory texture and coords parameters, and the optional
        // array and level parameters.
        auto& signature = builtin->Signature();
        auto texture_idx = signature.IndexOf(sem::ParameterUsage::kTexture);
        auto coords_idx = signature.IndexOf(sem::ParameterUsage::kCoords);
        auto array_idx = signature.IndexOf(sem::ParameterUsage::kArrayIndex);
        auto level_idx = signature.IndexOf(sem::ParameterUsage::kLevel);

        auto* texture_arg = expr->args[static_cast<size_t>(texture_idx)];
        auto* coords_arg = expr->args[static_cast<size_t>(coords_idx)];
        auto* coords_ty = builtin->Parameters()[static_cast<size_t>(coords_idx)]->Type();

        // If the level is provided, then we need to clamp this. As the level is
        // used by textureDimensions() and the texture[Load|Store]() calls, we need
        // to clamp both usages.
        // TODO(bclayton): We probably want to place this into a let so that the
        // calculation can be reused. This is fiddly to get right.
        std::function<const ast::Expression*()> level_arg;
        if (level_idx >= 0) {
            level_arg = [&] {
                auto* arg = expr->args[static_cast<size_t>(level_idx)];
                auto* num_levels = b.Call("textureNumLevels", ctx.Clone(texture_arg));
                auto* zero = b.Expr(0_i);
                auto* max = ctx.dst->Sub(num_levels, 1_i);
                auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
                return clamped;
            };
        }

        // Clamp the coordinates argument
        {
            auto* texture_dims =
                level_arg ? b.Call("textureDimensions", ctx.Clone(texture_arg), level_arg())
                          : b.Call("textureDimensions", ctx.Clone(texture_arg));
            auto* zero = b.Construct(CreateASTTypeFor(ctx, coords_ty));
            auto* max =
                ctx.dst->Sub(texture_dims, b.Construct(CreateASTTypeFor(ctx, coords_ty), 1_i));
            auto* clamped_coords = b.Call("clamp", ctx.Clone(coords_arg), zero, max);
            ctx.Replace(coords_arg, clamped_coords);
        }

        // Clamp the array_index argument, if provided
        if (array_idx >= 0) {
            auto* arg = expr->args[static_cast<size_t>(array_idx)];
            auto* num_layers = b.Call("textureNumLayers", ctx.Clone(texture_arg));
            auto* zero = b.Expr(0_i);
            auto* max = ctx.dst->Sub(num_layers, 1_i);
            auto* clamped = b.Call("clamp", ctx.Clone(arg), zero, max);
            ctx.Replace(arg, clamped);
        }

        // Clamp the level argument, if provided
        if (level_idx >= 0) {
            auto* arg = expr->args[static_cast<size_t>(level_idx)];
            ctx.Replace(arg, level_arg ? level_arg() : ctx.dst->Expr(0_i));
        }

        return nullptr;  // Clone, which will use the argument replacements above.
    }
};

Robustness::Config::Config() = default;
Robustness::Config::Config(const Config&) = default;
Robustness::Config::~Config() = default;
Robustness::Config& Robustness::Config::operator=(const Config&) = default;

Robustness::Robustness() = default;
Robustness::~Robustness() = default;

void Robustness::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    Config cfg;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    std::unordered_set<ast::AddressSpace> omitted_classes;
    for (auto sc : cfg.omitted_classes) {
        switch (sc) {
            case AddressSpace::kUniform:
                omitted_classes.insert(ast::AddressSpace::kUniform);
                break;
            case AddressSpace::kStorage:
                omitted_classes.insert(ast::AddressSpace::kStorage);
                break;
        }
    }

    State state{ctx, std::move(omitted_classes)};

    state.Transform();
    ctx.Clone();
}

}  // namespace tint::transform
