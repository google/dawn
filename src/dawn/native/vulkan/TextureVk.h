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
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

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
    // Returns the aspects used for tracking of Vulkan state. These can be the combined aspects.
    Aspect GetDisjointVulkanAspects() const;

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

    // Eagerly transition the texture for export.
    void TransitionEagerlyForExport(CommandRecordingContext* recordingContext);
    std::vector<VkSemaphore> AcquireWaitRequirements();

    void EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                             const SubresourceRange& range);

    VkImageLayout GetCurrentLayoutForSwapChain() const;

    // Binds externally allocated memory to the VkImage and on success, takes ownership of
    // semaphores.
    MaybeError BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
                                  VkDeviceMemory externalMemoryAllocation,
                                  std::vector<VkSemaphore> waitSemaphores);
    // Update the 'ExternalSemaphoreHandle' to be used for export with the newly submitted one.
    void UpdateExternalSemaphoreHandle(ExternalSemaphoreHandle handle);
    MaybeError ExportExternalTexture(VkImageLayout desiredLayout,
                                     ExternalSemaphoreHandle* handle,
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

    VkImage mHandle = VK_NULL_HANDLE;
    ResourceMemoryAllocation mMemoryAllocation;
    VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;

    // The states of an external texture:
    //   InternalOnly: Not initialized as an external texture yet.
    //   PendingAcquire: Intialized as an external texture already, but unavailable for access yet.
    //   Acquired: Ready for access.
    //   EagerlyTransitioned: The texture has ever been used, and eagerly transitioned for export.
    //   Now it can be acquired for access again, or directly exported. Released: The texture has
    //   been destoried, and should no longer be used.
    enum class ExternalState {
        InternalOnly,
        PendingAcquire,
        Acquired,
        EagerlyTransitioned,
        Released
    };
    ExternalState mExternalState = ExternalState::InternalOnly;
    ExternalState mLastExternalState = ExternalState::InternalOnly;
    uint32_t mExportQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;

    VkImageLayout mPendingAcquireOldLayout;
    VkImageLayout mPendingAcquireNewLayout;

    VkImageLayout mDesiredExportLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    ExternalSemaphoreHandle mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;

    std::vector<VkSemaphore> mWaitRequirements;

    // Sometimes the WebGPU aspects don't directly map to Vulkan aspects:
    //
    //  - In early Vulkan versions it is not possible to transition depth and stencil separetely so
    //    textures with Depth|Stencil will be promoted to a single CombinedDepthStencil aspect
    //    internally.
    //  - Some multiplanar images cannot have planes transitioned separately and instead Vulkan
    //    requires that the "Color" aspect be used for barriers, so Plane0|Plane1 is promoted to
    //    just Color.
    //
    // This variable, if not Aspect::None, is the combined aspect to use for all transitions.
    const Aspect mCombinedAspect;
    SubresourceStorage<wgpu::TextureUsage> mSubresourceLastUsages;

    bool UseCombinedAspects() const;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(TextureBase* texture,
                                                  const TextureViewDescriptor* descriptor);
    VkImageView GetHandle() const;
    VkImageView GetHandleForBGRA8UnormStorage() const;

  private:
    ~TextureView() override;
    void DestroyImpl() override;
    using TextureViewBase::TextureViewBase;
    MaybeError Initialize(const TextureViewDescriptor* descriptor);

    // Dawn API
    void SetLabelImpl() override;

    VkImageView mHandle = VK_NULL_HANDLE;
    VkImageView mHandleForBGRA8UnormStorage = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
