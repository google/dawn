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

#include "src/tint/lang/spirv/writer/helpers/generate_bindings.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::spirv::writer {

Bindings GenerateBindings(const Program& program) {
    // TODO(tint:1491): Use Inspector once we can get binding info for all
    // variables, not just those referenced by entry points.

    Bindings bindings{};

    std::unordered_set<tint::BindingPoint> seen_binding_points;

    // Collect next valid binding number per group
    Hashmap<uint32_t, uint32_t, 4> group_to_next_binding_number;
    Vector<tint::BindingPoint, 4> ext_tex_bps;
    for (auto* var : program.AST().GlobalVariables()) {
        if (auto* sem_var = program.Sem().Get(var)->As<sem::GlobalVariable>()) {
            if (auto bp = sem_var->BindingPoint()) {
                // This is a bit of a hack. The binding points must be unique over all the `binding`
                // entries. But, this is looking at _all_ entry points where bindings can overlap.
                // In the case where both entry points used the same type (uniform, sampler, etc)
                // then it would be fine as it just overwrites with itself. But, if one entry point
                // has a uniform and the other a sampler at the same (group,binding) pair then we'll
                // get a validation error due to duplicate WGSL bindings.
                //
                // For generating bindings we don't really care as we always map to itself, so if it
                // exists anywhere, we just pretend that's the only one.
                if (seen_binding_points.find(*bp) != seen_binding_points.end()) {
                    continue;
                }
                seen_binding_points.emplace(*bp);

                if (auto val = group_to_next_binding_number.Find(bp->group)) {
                    *val = std::max(*val, bp->binding + 1);
                } else {
                    group_to_next_binding_number.Add(bp->group, bp->binding + 1);
                }

                // Store up the external textures, we'll add them in the next step
                if (sem_var->Type()->UnwrapRef()->Is<core::type::ExternalTexture>()) {
                    ext_tex_bps.Push(*bp);
                    continue;
                }

                binding::BindingInfo info{bp->group, bp->binding};
                switch (sem_var->AddressSpace()) {
                    case core::AddressSpace::kHandle:
                        Switch(
                            sem_var->Type()->UnwrapRef(),  //
                            [&](const core::type::Sampler*) {
                                bindings.sampler.emplace(*bp, info);
                            },
                            [&](const core::type::StorageTexture*) {
                                bindings.storage_texture.emplace(*bp, info);
                            },
                            [&](const core::type::Texture*) {
                                bindings.texture.emplace(*bp, info);
                            });
                        break;
                    case core::AddressSpace::kStorage:
                        bindings.storage.emplace(*bp, info);
                        break;
                    case core::AddressSpace::kUniform:
                        bindings.uniform.emplace(*bp, info);
                        break;

                    case core::AddressSpace::kUndefined:
                    case core::AddressSpace::kPixelLocal:
                    case core::AddressSpace::kPrivate:
                    case core::AddressSpace::kPushConstant:
                    case core::AddressSpace::kIn:
                    case core::AddressSpace::kOut:
                    case core::AddressSpace::kFunction:
                    case core::AddressSpace::kWorkgroup:
                        break;
                }
            }
        }
    }

    for (auto bp : ext_tex_bps) {
        uint32_t g = bp.group;
        uint32_t next_num = *(group_to_next_binding_number.GetOrZero(g));

        binding::BindingInfo plane0{bp.group, bp.binding};
        binding::BindingInfo plane1{g, next_num++};
        binding::BindingInfo metadata{g, next_num++};

        group_to_next_binding_number.Replace(g, next_num);

        bindings.external_texture.emplace(bp, binding::ExternalTexture{metadata, plane0, plane1});
    }

    return bindings;
}

}  // namespace tint::spirv::writer
