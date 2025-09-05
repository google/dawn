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

#include "src/tint/lang/core/ir/transform/resource_binding_helper.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/resource_type.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::transform {

std::optional<ResourceBindingConfig> GenerateResourceBindingConfig(Module& mod) {
    ResourceBindingConfig cfg;

    uint32_t max_group = 0;
    std::vector<Var*> resource_bindings;

    for (auto* inst : *mod.root_block) {
        auto* var = inst->As<Var>();
        if (!var) {
            continue;
        }
        if (!var->BindingPoint().has_value()) {
            continue;
        }

        auto* ty = var->Result()->Type()->UnwrapPtr()->As<type::ResourceBinding>();
        if (!ty) {
            continue;
        }
        resource_bindings.push_back(var);

        max_group = std::max(max_group, var->BindingPoint()->group);
    }
    if (resource_bindings.empty()) {
        return std::nullopt;
    }

    max_group += 1;
    uint32_t binding = 0;
    for (auto* var : resource_bindings) {
        ResourceBindingInfo info{
            .storage_buffer_binding = BindingPoint{.group = max_group, .binding = binding++},
            .default_binding_type_order = {},
        };

        std::vector<Value*> to_check = {var->Result()};
        while (!to_check.empty()) {
            auto* next = to_check.back();
            to_check.pop_back();

            for (auto& usage : next->UsagesUnsorted()) {
                Switch(
                    usage->instruction, [&](Load* l) { to_check.push_back(l->Result()); },
                    [&](CoreBuiltinCall* call) {
                        if (call->Func() != BuiltinFn::kHasBinding &&
                            call->Func() != BuiltinFn::kGetBinding) {
                            return;
                        }

                        auto exp = call->ExplicitTemplateParams();
                        TINT_ASSERT(exp.Length() == 1);
                        info.default_binding_type_order.push_back(type::TypeToResourceType(exp[0]));
                    },
                    TINT_ICE_ON_NO_MATCH);
            }
        }
        // Sort so we get stable generated results
        std::sort(info.default_binding_type_order.begin(), info.default_binding_type_order.end());

        cfg.bindings.insert({var->BindingPoint().value(), std::move(info)});
    }

    return cfg;
}

}  // namespace tint::core::ir::transform
