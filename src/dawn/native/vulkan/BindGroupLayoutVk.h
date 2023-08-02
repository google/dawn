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

#ifndef SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_

#include <vector>

#include "dawn/common/SlabAllocator.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/vulkan/BindGroupVk.h"

namespace dawn::native {
class CacheKey;
}  // namespace dawn::native

namespace dawn::native::vulkan {

struct DescriptorSetAllocation;
class DescriptorSetAllocator;
class Device;

VkDescriptorType VulkanDescriptorType(const BindingInfo& bindingInfo);

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
class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    static ResultOrError<Ref<BindGroupLayout>> Create(Device* device,
                                                      const BindGroupLayoutDescriptor* descriptor);

    BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);

    VkDescriptorSetLayout GetHandle() const;

    ResultOrError<Ref<BindGroup>> AllocateBindGroup(Device* device,
                                                    const BindGroupDescriptor* descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup,
                             DescriptorSetAllocation* descriptorSetAllocation);

  private:
    ~BindGroupLayout() override;
    MaybeError Initialize();
    void DestroyImpl() override;

    // Dawn API
    void SetLabelImpl() override;

    VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;

    SlabAllocator<BindGroup> mBindGroupAllocator;
    Ref<DescriptorSetAllocator> mDescriptorSetAllocator;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
