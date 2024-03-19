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

#ifndef SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
#define SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_

#include <memory>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "dawn/native/Texture.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/SharedTextureMemoryVk.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
class Texture;

VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format);
// This version does not support depth stencil formats which can depend on
// properties of the Device.
VkFormat ColorVulkanImageFormat(wgpu::TextureFormat format);
ResultOrError<wgpu::TextureFormat> FormatFromVkFormat(const Device* device, VkFormat vkFormat);
VkImageUsageFlags VulkanImageUsage(wgpu::TextureUsage usage, const Format& format);
VkImageLayout VulkanImageLayout(const Format& format, wgpu::TextureUsage usage);
VkImageLayout VulkanImageLayoutForDepthStencilAttachment(const Format& format,
                                                         bool depthReadOnly,
                                                         bool stencilReadOnly);
VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount);

MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase* device,
                                           const UnpackedPtr<TextureDescriptor>& descriptor);

bool IsSampleCountSupported(const dawn::native::vulkan::Device* device,
                            const VkImageCreateInfo& imageCreateInfo);

class Texture final : public TextureBase {
  public:
    // Used to create a regular texture from a descriptor.
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor,
                                              VkImageUsageFlags extraUsages = 0);

    // Creates a texture and initializes it with a VkImage that references an external memory
    // object. Before the texture can be used, the VkDeviceMemory associated with the external
    // image must be bound via Texture::BindExternalMemory.
    static ResultOrError<Ref<Texture>> CreateFromExternal(
        Device* device,
        const ExternalImageDescriptorVk* descriptor,
        const UnpackedPtr<TextureDescriptor>& textureDescriptor,
        external_memory::Service* externalMemoryService);

    // Create a texture from contents of a SharedTextureMemory.
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& textureDescriptor);

    // Creates a texture that wraps a swapchain-allocated VkImage.
    static Ref<Texture> CreateForSwapChain(Device* device,
                                           const UnpackedPtr<TextureDescriptor>& descriptor,
                                           VkImage nativeImage);

    VkImage GetHandle() const;
    // Returns the aspects used for tracking of Vulkan state. These can be the combined aspects.
    Aspect GetDisjointVulkanAspects() const;

    // Transitions the texture to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext,
                            wgpu::TextureUsage usage,
                            wgpu::ShaderStage shaderStages,
                            const SubresourceRange& range);
    void TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                const TextureSubresourceSyncInfo& textureSyncInfos,
                                std::vector<VkImageMemoryBarrier>* imageBarriers,
                                VkPipelineStageFlags* srcStages,
                                VkPipelineStageFlags* dstStages);

    // Eagerly transition the texture for export.
    void TransitionEagerlyForExport(CommandRecordingContext* recordingContext);
    std::vector<VkSemaphore> AcquireWaitRequirements();

    MaybeError EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
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

    void SetPendingAcquire(VkImageLayout pendingAcquireOldLayout,
                           VkImageLayout pendingAcquireNewLayout);
    MaybeError EndAccess(ExternalSemaphoreHandle* handle,
                         VkImageLayout* releasedOldLayout,
                         VkImageLayout* releasedNewLayout);

    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;

  private:
    ~Texture() override;
    Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor);

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
                                              wgpu::ShaderStage shaderStages,
                                              const SubresourceRange& range,
                                              std::vector<VkImageMemoryBarrier>* imageBarriers,
                                              VkPipelineStageFlags* srcStages,
                                              VkPipelineStageFlags* dstStages);
    void TransitionUsageForPassImpl(CommandRecordingContext* recordingContext,
                                    const SubresourceStorage<TextureSyncInfo>& subresourceSyncInfos,
                                    std::vector<VkImageMemoryBarrier>* imageBarriers,
                                    VkPipelineStageFlags* srcStages,
                                    VkPipelineStageFlags* dstStages);
    void TransitionUsageAndGetResourceBarrierImpl(wgpu::TextureUsage usage,
                                                  wgpu::ShaderStage shaderStages,
                                                  const SubresourceRange& range,
                                                  std::vector<VkImageMemoryBarrier>* imageBarriers,
                                                  VkPipelineStageFlags* srcStages,
                                                  VkPipelineStageFlags* dstStages);
    void TweakTransitionForExternalUsage(CommandRecordingContext* recordingContext,
                                         std::vector<VkImageMemoryBarrier>* barriers,
                                         size_t transitionBarrierStart);
    bool CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage,
                                wgpu::TextureUsage usage,
                                wgpu::ShaderStage lastShaderStage,
                                wgpu::ShaderStage shaderStage);

    VkImage mHandle = VK_NULL_HANDLE;
    bool mOwnsHandle = false;
    ResourceMemoryAllocation mMemoryAllocation;
    VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;
    struct SharedTextureMemoryObjects {
        Ref<RefCountedVkHandle<VkImage>> vkImage;
        Ref<RefCountedVkHandle<VkDeviceMemory>> vkDeviceMemory;
    };
    SharedTextureMemoryObjects mSharedTextureMemoryObjects;

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
    SubresourceStorage<TextureSyncInfo> mSubresourceLastSyncInfos;

    bool UseCombinedAspects() const;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(TextureBase* texture,
                                                  const TextureViewDescriptor* descriptor);
    VkImageView GetHandle() const;
    VkImageView GetHandleForBGRA8UnormStorage() const;

    ResultOrError<VkImageView> GetOrCreate2DViewOn3D(uint32_t depthSlice = 0u);

  private:
    ~TextureView() override;
    void DestroyImpl() override;
    using TextureViewBase::TextureViewBase;
    MaybeError Initialize(const TextureViewDescriptor* descriptor);

    VkImageViewCreateInfo GetCreateInfo(wgpu::TextureFormat format,
                                        wgpu::TextureViewDimension dimension,
                                        uint32_t depthSlice = 0u) const;

    // Dawn API
    void SetLabelImpl() override;

    VkImageView mHandle = VK_NULL_HANDLE;
    VkImageView mHandleForBGRA8UnormStorage = VK_NULL_HANDLE;
    std::vector<VkImageView> mHandlesFor2DViewOn3D;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
