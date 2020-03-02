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

#ifndef DAWNNATIVE_VULKAN_TEXTUREVK_H_
#define DAWNNATIVE_VULKAN_TEXTUREVK_H_

#include "dawn_native/Texture.h"

#include "common/vulkan_platform.h"
#include "dawn_native/ResourceMemoryAllocation.h"
#include "dawn_native/vulkan/ExternalHandle.h"
#include "dawn_native/vulkan/external_memory/MemoryService.h"

namespace dawn_native { namespace vulkan {

    struct CommandRecordingContext;
    class Device;

    VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format);
    VkImageUsageFlags VulkanImageUsage(wgpu::TextureUsage usage, const Format& format);
    VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount);

    MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase* device,
                                               const TextureDescriptor* descriptor);

    bool IsSampleCountSupported(const dawn_native::vulkan::Device* device,
                                const VkImageCreateInfo& imageCreateInfo);

    class Texture : public TextureBase {
      public:
        // Used to create a regular texture from a descriptor.
        static ResultOrError<Texture*> Create(Device* device, const TextureDescriptor* descriptor);

        // Creates a texture and initializes it with a VkImage that references an external memory
        // object. Before the texture can be used, the VkDeviceMemory associated with the external
        // image must be bound via Texture::BindExternalMemory.
        static ResultOrError<Texture*> CreateFromExternal(
            Device* device,
            const ExternalImageDescriptor* descriptor,
            const TextureDescriptor* textureDescriptor,
            external_memory::Service* externalMemoryService);

        Texture(Device* device, const TextureDescriptor* descriptor, VkImage nativeImage);
        ~Texture();

        VkImage GetHandle() const;
        VkImageAspectFlags GetVkAspectMask() const;

        // Transitions the texture to be used as `usage`, recording any necessary barrier in
        // `commands`.
        // TODO(cwallez@chromium.org): coalesce barriers and do them early when possible.
        void TransitionUsageNow(CommandRecordingContext* recordingContext,
                                wgpu::TextureUsage usage);
        void EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                 uint32_t baseMipLevel,
                                                 uint32_t levelCount,
                                                 uint32_t baseArrayLayer,
                                                 uint32_t layerCount);

        MaybeError SignalAndDestroy(VkSemaphore* outSignalSemaphore);
        // Binds externally allocated memory to the VkImage and on success, takes ownership of
        // semaphores.
        MaybeError BindExternalMemory(const ExternalImageDescriptor* descriptor,
                                      VkSemaphore signalSemaphore,
                                      VkDeviceMemory externalMemoryAllocation,
                                      std::vector<VkSemaphore> waitSemaphores);

      private:
        using TextureBase::TextureBase;
        MaybeError InitializeAsInternalTexture();

        MaybeError InitializeFromExternal(const ExternalImageDescriptor* descriptor,
                                          external_memory::Service* externalMemoryService);

        void DestroyImpl() override;
        MaybeError ClearTexture(CommandRecordingContext* recordingContext,
                                uint32_t baseMipLevel,
                                uint32_t levelCount,
                                uint32_t baseArrayLayer,
                                uint32_t layerCount,
                                TextureBase::ClearValue);

        VkImage mHandle = VK_NULL_HANDLE;
        ResourceMemoryAllocation mMemoryAllocation;
        VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;

        enum class ExternalState {
            InternalOnly,
            PendingAcquire,
            Acquired,
            PendingRelease,
            Released
        };
        ExternalState mExternalState = ExternalState::InternalOnly;
        ExternalState mLastExternalState = ExternalState::InternalOnly;

        VkSemaphore mSignalSemaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> mWaitRequirements;

        // A usage of none will make sure the texture is transitioned before its first use as
        // required by the Vulkan spec.
        wgpu::TextureUsage mLastUsage = wgpu::TextureUsage::None;
    };

    class TextureView : public TextureViewBase {
      public:
        static ResultOrError<TextureView*> Create(TextureBase* texture,
                                                  const TextureViewDescriptor* descriptor);
        ~TextureView();

        VkImageView GetHandle() const;

      private:
        using TextureViewBase::TextureViewBase;
        MaybeError Initialize(const TextureViewDescriptor* descriptor);

        VkImageView mHandle = VK_NULL_HANDLE;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_TEXTUREVK_H_
