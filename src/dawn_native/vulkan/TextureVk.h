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
#include "dawn_native/vulkan/ExternalHandle.h"
#include "dawn_native/vulkan/MemoryAllocator.h"

namespace dawn_native { namespace vulkan {

    struct CommandRecordingContext;
    struct ExternalImageDescriptor;

    VkFormat VulkanImageFormat(dawn::TextureFormat format);
    VkImageUsageFlags VulkanImageUsage(dawn::TextureUsageBit usage, const Format& format);
    VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount);

    MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase* device,
                                               const TextureDescriptor* descriptor);

    class Texture : public TextureBase {
      public:
        enum class ExternalState {
            InternalOnly,
            PendingAcquire,
            Acquired,
            PendingRelease,
            Released
        };

        Texture(Device* device, const TextureDescriptor* descriptor);
        Texture(Device* device, const TextureDescriptor* descriptor, VkImage nativeImage);
        Texture(Device* device,
                const ExternalImageDescriptor* descriptor,
                const TextureDescriptor* textureDescriptor,
                VkSemaphore signalSemaphore,
                VkDeviceMemory externalMemoryAllocation,
                std::vector<VkSemaphore> waitSemaphores);
        ~Texture();

        VkImage GetHandle() const;
        VkImageAspectFlags GetVkAspectMask() const;

        // Transitions the texture to be used as `usage`, recording any necessary barrier in
        // `commands`.
        // TODO(cwallez@chromium.org): coalesce barriers and do them early when possible.
        void TransitionUsageNow(CommandRecordingContext* recordingContext,
                                dawn::TextureUsageBit usage);
        void EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                 uint32_t baseMipLevel,
                                                 uint32_t levelCount,
                                                 uint32_t baseArrayLayer,
                                                 uint32_t layerCount);

        MaybeError SignalAndDestroy(VkSemaphore* outSignalSemaphore);

      private:
        void DestroyImpl() override;
        void ClearTexture(CommandRecordingContext* recordingContext,
                          uint32_t baseMipLevel,
                          uint32_t levelCount,
                          uint32_t baseArrayLayer,
                          uint32_t layerCount);

        VkImage mHandle = VK_NULL_HANDLE;
        DeviceMemoryAllocation mMemoryAllocation;
        VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;

        ExternalState mExternalState = ExternalState::InternalOnly;
        ExternalState mLastExternalState = ExternalState::InternalOnly;
        VkSemaphore mSignalSemaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> mWaitRequirements;

        // A usage of none will make sure the texture is transitioned before its first use as
        // required by the spec.
        dawn::TextureUsageBit mLastUsage = dawn::TextureUsageBit::None;
    };

    class TextureView : public TextureViewBase {
      public:
        TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor);
        ~TextureView();

        VkImageView GetHandle() const;

      private:
        VkImageView mHandle = VK_NULL_HANDLE;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_TEXTUREVK_H_
