// Copyright 2019 The Dawn Authors
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

#ifndef DAWNNATIVE_BINDGROUPANDSTORAGEBARRIERTRACKER_H_
#define DAWNNATIVE_BINDGROUPANDSTORAGEBARRIERTRACKER_H_

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupTracker.h"

namespace dawn_native {

    // Extends BindGroupTrackerBase to also keep track of resources that need a usage transition.
    template <bool CanInheritBindGroups, typename DynamicOffset>
    class BindGroupAndStorageBarrierTrackerBase
        : public BindGroupTrackerBase<CanInheritBindGroups, DynamicOffset> {
        using Base = BindGroupTrackerBase<CanInheritBindGroups, DynamicOffset>;

      public:
        BindGroupAndStorageBarrierTrackerBase() = default;

        void OnSetBindGroup(uint32_t index,
                            BindGroupBase* bindGroup,
                            uint32_t dynamicOffsetCount,
                            uint32_t* dynamicOffsets) {
            if (this->mBindGroups[index] != bindGroup) {
                mBuffers[index] = {};
                mBuffersNeedingBarrier[index] = {};

                const BindGroupLayoutBase* layout = bindGroup->GetLayout();

                for (BindingIndex bindingIndex = 0; bindingIndex < layout->GetBindingCount();
                     ++bindingIndex) {
                    const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);

                    if ((bindingInfo.visibility & wgpu::ShaderStage::Compute) == 0) {
                        continue;
                    }

                    mBindingTypes[index][bindingIndex] = bindingInfo.type;
                    switch (bindingInfo.type) {
                        case wgpu::BindingType::UniformBuffer:
                        case wgpu::BindingType::ReadonlyStorageBuffer:
                        case wgpu::BindingType::Sampler:
                        case wgpu::BindingType::SampledTexture:
                            // Don't require barriers.
                            break;

                        case wgpu::BindingType::StorageBuffer:
                            mBuffersNeedingBarrier[index].set(bindingIndex);
                            mBuffers[index][bindingIndex] =
                                bindGroup->GetBindingAsBufferBinding(bindingIndex).buffer;
                            break;

                        case wgpu::BindingType::StorageTexture:
                        case wgpu::BindingType::ReadonlyStorageTexture:
                        case wgpu::BindingType::WriteonlyStorageTexture:
                            // Not implemented.

                        default:
                            UNREACHABLE();
                            break;
                    }
                }
            }

            Base::OnSetBindGroup(index, bindGroup, dynamicOffsetCount, dynamicOffsets);
        }

      protected:
        std::array<std::bitset<kMaxBindingsPerGroup>, kMaxBindGroups> mBuffersNeedingBarrier = {};
        std::array<std::array<wgpu::BindingType, kMaxBindingsPerGroup>, kMaxBindGroups>
            mBindingTypes = {};
        std::array<std::array<BufferBase*, kMaxBindingsPerGroup>, kMaxBindGroups> mBuffers = {};
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BINDGROUPANDSTORAGEBARRIERTRACKER_H_
