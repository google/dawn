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

#ifndef SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
#define SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_

#include <memory>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "dawn/native/Texture.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
class Texture;

VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format);
VkImageUsageFlags VulkanImageUsage(wgpu::TextureUsage usage, const Format& format);
VkImageLayout VulkanImageLayout(const Texture* texture, wgpu::TextureUsage usage);
VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount);

MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase* device,
                                           const TextureDescriptor* descriptor);

bool IsSampleCountSupported(const dawn::native::vulkan::Device* device,
                            const VkImageCreateInfo& imageCreateInfo);

class Texture final : public TextureBase {
  public:
    // Used to create a regular texture from a descriptor.
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const TextureDescriptor* descriptor,
                                              VkImageUsageFlags extraUsages = 0);

    // Creates a texture and initializes it with a VkImage that references an external memory
    // object. Before the texture can be used, the VkDeviceMemory associated with the external
    // image must be bound via Texture::BindExternalMemory.
    static ResultOrError<Texture*> CreateFromExternal(
        Device* device,
        const ExternalImageDescriptorVk* descriptor,
        const TextureDescriptor* textureDescriptor,
        external_memory::Service* externalMemoryService);

    // Creates a texture that wraps a swapchain-allocated VkImage.
    static Ref<Texture> CreateForSwapChain(Device* device,
                                           const TextureDescriptor* descriptor,
                                           VkImage nativeImage);

    VkImage GetHandle() const;

    // Transitions the texture to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext,
                            wgpu::TextureUsage usage,
                            const SubresourceRange& range);
    void TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                const TextureSubresourceUsage& textureUsages,
                                std::vector<VkImageMemoryBarrier>* imageBarriers,
                                VkPipelineStageFlags* srcStages,
                                VkPipelineStageFlags* dstStages);

    void EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                             const SubresourceRange& range);

    VkImageLayout GetCurrentLayoutForSwapChain() const;

    // Binds externally allocated memory to the VkImage and on success, takes ownership of
    // semaphores.
    MaybeError BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
                                  VkSemaphore signalSemaphore,
                                  VkDeviceMemory externalMemoryAllocation,
                                  std::vector<VkSemaphore> waitSemaphores);

    MaybeError ExportExternalTexture(VkImageLayout desiredLayout,
                                     VkSemaphore* signalSemaphore,
                                     VkImageLayout* releasedOldLayout,
                                     VkImageLayout* releasedNewLayout);

    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;

  private:
    ~Texture() override;
    Texture(Device* device, const TextureDescriptor* descriptor, TextureState state);

    MaybeError InitializeAsInternalTexture(VkImageUsageFlags extraUsages);
    MaybeError InitializeFromExternal(const ExternalImageDescriptorVk* descriptor,
                                      external_memory::Service* externalMemoryService);
    void InitializeForSwapChain(VkImage nativeImage);

    void DestroyImpl() override;
    MaybeError ClearTexture(CommandRecordingContext* recordingContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue);

    // Implementation details of the barrier computations for the texture.
    void TransitionUsageAndGetResourceBarrier(wgpu::TextureUsage usage,
                                              const SubresourceRange& range,
                                              std::vector<VkImageMemoryBarrier>* imageBarriers,
                                              VkPipelineStageFlags* srcStages,
                                              VkPipelineStageFlags* dstStages);
    void TransitionUsageForPassImpl(CommandRecordingContext* recordingContext,
                                    const SubresourceStorage<wgpu::TextureUsage>& subresourceUsages,
                                    std::vector<VkImageMemoryBarrier>* imageBarriers,
                                    VkPipelineStageFlags* srcStages,
                                    VkPipelineStageFlags* dstStages);
    void TransitionUsageAndGetResourceBarrierImpl(wgpu::TextureUsage usage,
                                                  const SubresourceRange& range,
                                                  std::vector<VkImageMemoryBarrier>* imageBarriers,
                                                  VkPipelineStageFlags* srcStages,
                                                  VkPipelineStageFlags* dstStages);
    void TweakTransitionForExternalUsage(CommandRecordingContext* recordingContext,
                                         std::vector<VkImageMemoryBarrier>* barriers,
                                         size_t transitionBarrierStart);
    bool CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage, wgpu::TextureUsage usage);

    // In base Vulkan, Depth and stencil can only be transitioned together. This function
    // indicates whether we should combine depth and stencil barriers to accommodate this
    // limitation.
    bool ShouldCombineDepthStencilBarriers() const;

    // This indicates whether the VK_IMAGE_ASPECT_COLOR_BIT instead of
    // VK_IMAGE_ASPECT_PLANE_n_BIT must be used.
    bool ShouldCombineMultiPlaneBarriers() const;

    bool ShouldCombineBarriers() const {
        return ShouldCombineDepthStencilBarriers() || ShouldCombineMultiPlaneBarriers();
    }

    // Compute the Aspects of the SubresourceStoage for this texture depending on whether we're
    // doing the workaround for combined depth and stencil barriers, or combining multi-plane
    // barriers.
    Aspect ComputeAspectsForSubresourceStorage() const;

    VkImage mHandle = VK_NULL_HANDLE;
    ResourceMemoryAllocation mMemoryAllocation;
    VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;

    enum class ExternalState { InternalOnly, PendingAcquire, Acquired, Released };
    ExternalState mExternalState = ExternalState::InternalOnly;
    ExternalState mLastExternalState = ExternalState::InternalOnly;

    VkImageLayout mPendingAcquireOldLayout;
    VkImageLayout mPendingAcquireNewLayout;

    VkSemaphore mSignalSemaphore = VK_NULL_HANDLE;
    std::vector<VkSemaphore> mWaitRequirements;

    // Note that in early Vulkan versions it is not possible to transition depth and stencil
    // separately so textures with Depth|Stencil aspects will have a single Depth aspect in the
    // storage.
    std::unique_ptr<SubresourceStorage<wgpu::TextureUsage>> mSubresourceLastUsages;

    bool mSupportsDisjointVkImage = false;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(TextureBase* texture,
                                                  const TextureViewDescriptor* descriptor);
    VkImageView GetHandle() const;

  private:
    ~TextureView() override;
    void DestroyImpl() override;
    using TextureViewBase::TextureViewBase;
    MaybeError Initialize(const TextureViewDescriptor* descriptor);

    // Dawn API
    void SetLabelImpl() override;

    VkImageView mHandle = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
