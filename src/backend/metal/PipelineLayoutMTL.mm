// Copyright 2017 The NXT Authors
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

#include "backend/metal/PipelineLayoutMTL.h"

#include "backend/metal/MetalBackend.h"

namespace backend {
namespace metal {

    PipelineLayout::PipelineLayout(PipelineLayoutBuilder* builder)
        : PipelineLayoutBase(builder) {
        // Each stage has its own numbering namespace in CompilerMSL.
        for (auto stage : IterateStages(kAllStages)) {
            uint32_t bufferIndex = 0;
            uint32_t samplerIndex = 0;
            uint32_t textureIndex = 0;

            for (size_t group = 0; group < kMaxBindGroups; ++group) {
                const auto& groupInfo = GetBindGroupLayout(group)->GetBindingInfo();
                for (size_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                    if (!(groupInfo.visibilities[binding] & StageBit(stage))) {
                        continue;
                    }
                    if (!groupInfo.mask[binding]) {
                        continue;
                    }

                    switch (groupInfo.types[binding]) {
                        case nxt::BindingType::UniformBuffer:
                        case nxt::BindingType::StorageBuffer:
                            indexInfo[stage][group][binding] = bufferIndex;
                            bufferIndex++;
                            break;
                        case nxt::BindingType::Sampler:
                            indexInfo[stage][group][binding] = samplerIndex;
                            samplerIndex++;
                            break;
                        case nxt::BindingType::SampledTexture:
                            indexInfo[stage][group][binding] = textureIndex;
                            textureIndex++;
                            break;
                    }
                }
            }
        }
    }

    const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo(nxt::ShaderStage stage) const {
        return indexInfo[stage];
    }

}
}
