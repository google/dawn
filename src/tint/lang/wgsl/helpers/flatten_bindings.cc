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

#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"

#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"

namespace tint::wgsl {

std::optional<Program> FlattenBindings(const Program& program) {
    // TODO(crbug.com/tint/1101): Make this more robust for multiple entry points.
    tint::ast::transform::BindingRemapper::BindingPoints binding_points;
    uint32_t next_buffer_idx = 0;
    uint32_t next_sampler_idx = 0;
    uint32_t next_texture_idx = 0;

    tint::inspector::Inspector inspector(program);
    auto entry_points = inspector.GetEntryPoints();
    for (auto& entry_point : entry_points) {
        auto bindings = inspector.GetResourceBindings(entry_point.name);

        for (auto& binding : bindings) {
            BindingPoint src = {binding.bind_group, binding.binding};
            if (binding_points.count(src)) {
                continue;
            }
            switch (binding.resource_type) {
                case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
                    binding_points.emplace(src, BindingPoint{0, next_buffer_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampler:
                case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                    binding_points.emplace(src, BindingPoint{0, next_sampler_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
                    binding_points.emplace(src, BindingPoint{0, next_texture_idx++});
                    break;
            }
        }
    }

    // Run the binding remapper transform.
    if (!binding_points.empty()) {
        tint::ast::transform::Manager manager;
        tint::ast::transform::DataMap inputs;
        tint::ast::transform::DataMap outputs;
        inputs.Add<tint::ast::transform::BindingRemapper::Remappings>(
            std::move(binding_points), tint::ast::transform::BindingRemapper::AccessControls{},
            /* mayCollide */ true);
        manager.Add<tint::ast::transform::BindingRemapper>();
        return manager.Run(program, inputs, outputs);
    }

    return {};
}

}  // namespace tint::wgsl
