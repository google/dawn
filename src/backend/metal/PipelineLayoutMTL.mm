// Copyright 2017 The Dawn Authors
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

#include "backend/BindGroupLayout.h"
#include "backend/metal/DeviceMTL.h"
#include "common/BitSetIterator.h"

namespace backend { namespace metal {

    PipelineLayout::PipelineLayout(Device* device, const nxt::PipelineLayoutDescriptor* descriptor)
        : PipelineLayoutBase(device, descriptor) {
        // Each stage has its own numbering namespace in CompilerMSL.
        for (auto stage : IterateStages(kAllStages)) {
            // Buffer number 0 is reserved for push constants
            uint32_t bufferIndex = 1;
            uint32_t samplerIndex = 0;
            uint32_t textureIndex = 0;

            for (uint32_t group : IterateBitSet(GetBindGroupLayoutsMask())) {
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
                            mIndexInfo[stage][group][binding] = bufferIndex;
                            bufferIndex++;
                            break;
                        case nxt::BindingType::Sampler:
                            mIndexInfo[stage][group][binding] = samplerIndex;
                            samplerIndex++;
                            break;
                        case nxt::BindingType::SampledTexture:
                            mIndexInfo[stage][group][binding] = textureIndex;
                            textureIndex++;
                            break;
                    }
                }
            }
        }
    }

    const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo(
        nxt::ShaderStage stage) const {
        return mIndexInfo[stage];
    }

}}  // namespace backend::metal
