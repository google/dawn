// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/transform/resource_binding.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The configuration.
    const ResourceBindingConfig& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The helper
    ResourceBindingHelper* helper;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        std::vector<core::ir::Var*> resource_bindings;

        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* type = var->Result()->Type()->UnwrapPtr()->As<core::type::ResourceBinding>();
            if (!type) {
                continue;
            }
            TINT_ASSERT(var->BindingPoint().has_value());

            resource_bindings.push_back(var);
        }

        std::vector<core::ir::Instruction*> to_delete;
        for (auto* var : resource_bindings) {
            auto iter = config.bindings.find(var->BindingPoint().value());
            TINT_ASSERT(iter != config.bindings.end());

            auto alias_for_type =
                helper->GenerateAliases(b, var, iter->second.default_binding_type_order);
            auto* sb = InjectStorageBuffer(var, iter->second);

            std::vector<core::ir::Value*> to_fix;
            to_fix.push_back(var->Result());

            while (!to_fix.empty()) {
                auto* val = to_fix.back();
                to_fix.pop_back();

                for (auto& usage : val->UsagesUnsorted()) {
                    tint::Switch(
                        usage->instruction,
                        [&](core::ir::Load* l) {
                            to_fix.push_back(l->Result());
                            to_delete.push_back(l);
                        },
                        [&](core::ir::CoreBuiltinCall* call) {
                            switch (call->Func()) {
                                case core::BuiltinFn::kArrayLength:
                                    b.InsertBefore(call, [&] {
                                        auto* access =
                                            b.Access(ty.ptr<storage, u32, read>(), sb, 0_u);
                                        b.LoadWithResult(call->DetachResult(), access);
                                    });
                                    to_delete.push_back(call);
                                    break;
                                default:
                                    TINT_UNREACHABLE();
                            }
                        },
                        TINT_ICE_ON_NO_MATCH);
                }
            }

            if (helper->DeleteSourceVar()) {
                to_delete.push_back(var);
            }
        }

        for (auto* inst : to_delete) {
            inst->Destroy();
        }
    }

    core::ir::Var* InjectStorageBuffer(core::ir::Var* var, const ResourceBindingInfo& info) {
        core::ir::Var* sb = nullptr;
        b.InsertBefore(var, [&] {
            auto* str = ty.Struct(ir.symbols.New("tint_resource_binding_buffer"),
                                  Vector<core::type::Manager::StructMemberDesc, 2>{
                                      {ir.symbols.New("array_length"), ty.u32()},
                                      {ir.symbols.New("bindings"), ty.array<u32>()},
                                  });

            auto* sb_ty = ty.ptr(storage, str, read);
            sb = b.Var(sb_ty);
            sb->SetBindingPoint(info.storage_buffer_binding.group,
                                info.storage_buffer_binding.binding);
        });

        return sb;
    }
};

}  // namespace

ResourceBindingHelper::~ResourceBindingHelper() = default;

Result<SuccessType> ResourceBinding(core::ir::Module& ir,
                                    const ResourceBindingConfig& config,
                                    ResourceBindingHelper* helper) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.ResourceBinding");
    if (result != Success) {
        return result.Failure();
    }

    State{config, ir, helper}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
