// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
#define DAWNNATIVE_VULKAN_BINDGROUPLAYOUTVK_H_

#include "dawn_native/BindGroupLayout.h"

#include "common/SlabAllocator.h"
#include "common/vulkan_platform.h"

#include <vector>

namespace dawn_native { namespace vulkan {

    class BindGroup;
    class Device;

    VkDescriptorType VulkanDescriptorType(wgpu::BindingType type, bool isDynamic);

    // Contains a descriptor set along with data necessary to track its allocation.
    struct DescriptorSetAllocation {
        size_t index = 0;
        VkDescriptorSet set = VK_NULL_HANDLE;
    };

    // In Vulkan descriptor pools have to be sized to an exact number of descriptors. This means
    // it's hard to have something where we can mix different types of descriptor sets because
    // we don't know if their vector of number of descriptors will be similar.
    //
    // That's why that in addition to containing the VkDescriptorSetLayout to create
    // VkDescriptorSets for its bindgroups, the layout also acts as an allocator for the descriptor
    // sets.
    //
    // The allocations is done with one pool per descriptor set, which is inefficient, but at least
    // the pools are reused when no longer used. Minimizing the number of descriptor pool allocation
    // is important because creating them can incur GPU memory allocation which is usually an
    // expensive syscall.
    class BindGroupLayout : public BindGroupLayoutBase {
      public:
        static ResultOrError<BindGroupLayout*> Create(Device* device,
                                                      const BindGroupLayoutDescriptor* descriptor);

        BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);
        ~BindGroupLayout();

        VkDescriptorSetLayout GetHandle() const;

        ResultOrError<BindGroup*> AllocateBindGroup(Device* device,
                                                    const BindGroupDescriptor* descriptor);
        void DeallocateBindGroup(BindGroup* bindGroup);

        ResultOrError<DescriptorSetAllocation> AllocateOneDescriptorSet();
        void DeallocateDescriptorSet(DescriptorSetAllocation* descriptorSetAllocation);

        // Interaction with the DescriptorSetService.
        void FinishDeallocation(size_t index);

      private:
        MaybeError Initialize();

        std::vector<VkDescriptorPoolSize> mPoolSizes;

        struct SingleDescriptorSetAllocation {
            VkDescriptorPool pool = VK_NULL_HANDLE;
            // Descriptor sets are freed when the pool is destroyed.
            VkDescriptorSet set = VK_NULL_HANDLE;
        };
        std::vector<SingleDescriptorSetAllocation> mAllocations;
        std::vector<size_t> mAvailableAllocations;

        VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;

        SlabAllocator<BindGroup> mBindGroupAllocator;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
