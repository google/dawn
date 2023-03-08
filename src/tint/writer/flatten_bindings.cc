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

#include "src/tint/writer/flatten_bindings.h"

#include <utility>

#include "src/tint/inspector/inspector.h"
#include "src/tint/transform/binding_remapper.h"
#include "src/tint/transform/manager.h"
#include "src/tint/writer/binding_point.h"

namespace tint::writer {

std::optional<Program> FlattenBindings(const Program* program) {
    // TODO(crbug.com/tint/1101): Make this more robust for multiple entry points.
    tint::transform::BindingRemapper::BindingPoints binding_points;
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
                case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
                    binding_points.emplace(src, BindingPoint{0, next_texture_idx++});
                    break;
            }
        }
    }

    // Run the binding remapper transform.
    tint::transform::Output transform_output;
    if (!binding_points.empty()) {
        tint::transform::Manager manager;
        tint::transform::DataMap inputs;
        inputs.Add<tint::transform::BindingRemapper::Remappings>(
            std::move(binding_points), tint::transform::BindingRemapper::AccessControls{},
            /* mayCollide */ true);
        manager.Add<tint::transform::BindingRemapper>();
        transform_output = manager.Run(program, inputs);
        return std::move(transform_output.program);
    }

    return {};
}

}  // namespace tint::writer
