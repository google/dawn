// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_
#define SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_

#include <map>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/vulkan/DescriptorSetAllocation.h"

namespace dawn::native::vulkan {

class BindGroupLayout;

class DescriptorSetAllocator : public ObjectBase {
    using PoolIndex = uint32_t;
    using SetIndex = uint16_t;

  public:
    static Ref<DescriptorSetAllocator> Create(
        BindGroupLayout* layout,
        std::map<VkDescriptorType, uint32_t> descriptorCountPerType);

    ResultOrError<DescriptorSetAllocation> Allocate();
    void Deallocate(DescriptorSetAllocation* allocationInfo);
    void FinishDeallocation(ExecutionSerial completedSerial);

  private:
    DescriptorSetAllocator(BindGroupLayout* layout,
                           std::map<VkDescriptorType, uint32_t> descriptorCountPerType);
    ~DescriptorSetAllocator() override;

    MaybeError AllocateDescriptorPool();

    BindGroupLayout* mLayout;

    std::vector<VkDescriptorPoolSize> mPoolSizes;
    SetIndex mMaxSets;

    struct DescriptorPool {
        VkDescriptorPool vkPool;
        std::vector<VkDescriptorSet> sets;
        std::vector<SetIndex> freeSetIndices;
    };

    std::vector<PoolIndex> mAvailableDescriptorPoolIndices;
    std::vector<DescriptorPool> mDescriptorPools;

    struct Deallocation {
        PoolIndex poolIndex;
        SetIndex setIndex;
    };
    SerialQueue<ExecutionSerial, Deallocation> mPendingDeallocations;
    ExecutionSerial mLastDeallocationSerial = ExecutionSerial(0);
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_DESCRIPTORSETALLOCATOR_H_
