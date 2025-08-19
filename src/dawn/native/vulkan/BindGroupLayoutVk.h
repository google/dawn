// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_

#include <vector>

#include "dawn/common/MutexProtected.h"
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

// Backend BindGroupLayout implementation for Vulkan. In addition to containing a BindGroupAllocator
// for the CPU-side tracking data, it has a DescriptorSetAllocator that handles efficient allocation
// of the corresponding VkDescriptorSets.
class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    static ResultOrError<Ref<BindGroupLayout>> Create(
        Device* device,
        const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);

    BindGroupLayout(DeviceBase* device, const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);

    VkDescriptorSetLayout GetHandle() const;

    ResultOrError<Ref<BindGroup>> AllocateBindGroup(
        Device* device,
        const UnpackedPtr<BindGroupDescriptor>& descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);
    void DeallocateDescriptorSet(DescriptorSetAllocation* descriptorSetAllocation);
    void ReduceMemoryUsage() override;

    // If the client specified that the texture at `textureBinding` should be
    // combined with a static sampler, returns the binding index of the static
    // sampler that is sampling this texture.
    std::optional<BindingIndex> GetStaticSamplerIndexForTexture(BindingIndex textureBinding) const;

  private:
    ~BindGroupLayout() override;
    MaybeError Initialize();
    void DestroyImpl() override;

    // Dawn API
    void SetLabelImpl() override;

    // Maps from indices of texture entries that are paired with static samplers
    // to indices of the entries of their respective samplers.
    absl::flat_hash_map<BindingIndex, BindingIndex> mTextureToStaticSamplerIndices;

    VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;

    MutexProtected<SlabAllocator<BindGroup>> mBindGroupAllocator;
    Ref<DescriptorSetAllocator> mDescriptorSetAllocator;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
