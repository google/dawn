// Copyright 2022 The Tint Authors.
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

#include "src/tint/cmd/generate_external_texture_bindings.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "src/tint/ast/module.h"
#include "src/tint/program.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/external_texture.h"

namespace tint::cmd {

writer::ExternalTextureOptions::BindingsMap GenerateExternalTextureBindings(
    const Program* program) {
    // TODO(tint:1491): Use Inspector once we can get binding info for all
    // variables, not just those referenced by entry points.

    // Collect next valid binding number per group
    std::unordered_map<uint32_t, uint32_t> group_to_next_binding_number;
    std::vector<sem::BindingPoint> ext_tex_bps;
    for (auto* var : program->AST().GlobalVariables()) {
        if (auto* sem_var = program->Sem().Get(var)->As<sem::GlobalVariable>()) {
            if (auto bp = sem_var->BindingPoint()) {
                auto& n = group_to_next_binding_number[bp->group];
                n = std::max(n, bp->binding + 1);

                if (sem_var->Type()->UnwrapRef()->Is<type::ExternalTexture>()) {
                    ext_tex_bps.emplace_back(*bp);
                }
            }
        }
    }

    writer::ExternalTextureOptions::BindingsMap new_bindings_map;
    for (auto bp : ext_tex_bps) {
        uint32_t g = bp.group;
        uint32_t& next_num = group_to_next_binding_number[g];
        auto new_bps =
            writer::ExternalTextureOptions::BindingPoints{{g, next_num++}, {g, next_num++}};

        new_bindings_map[bp] = new_bps;
    }

    return new_bindings_map;
}

}  // namespace tint::cmd
