// Copyright 2018 The NXT Authors
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

#ifndef BACKEND_VULKAN_TEXTUREVK_H_
#define BACKEND_VULKAN_TEXTUREVK_H_

#include "backend/Texture.h"

#include "backend/vulkan/MemoryAllocator.h"
#include "common/vulkan_platform.h"

namespace backend { namespace vulkan {

    VkFormat VulkanImageFormat(nxt::TextureFormat format);
    VkImageUsageFlags VulkanImageUsage(nxt::TextureUsageBit usage, nxt::TextureFormat format);

    class Texture : public TextureBase {
      public:
        Texture(TextureBuilder* builder);
        Texture(TextureBuilder* builder, VkImage nativeImage);
        ~Texture();

        VkImage GetHandle() const;
        VkImageAspectFlags GetVkAspectMask() const;

        // Transitions the texture to be used as `usage`, recording any necessary barrier in
        // `commands`.
        // TODO(cwallez@chromium.org): coalesce barriers and do them early when possible.
        void TransitionUsageNow(VkCommandBuffer commands, nxt::TextureUsageBit usage);

      private:
        VkImage mHandle = VK_NULL_HANDLE;
        DeviceMemoryAllocation mMemoryAllocation;

        // A usage of none will make sure the texture is transitioned before its first use as
        // required by the spec.
        nxt::TextureUsageBit mLastUsage = nxt::TextureUsageBit::None;
    };

    class TextureView : public TextureViewBase {
      public:
        TextureView(TextureViewBuilder* builder);
        ~TextureView();

        VkImageView GetHandle() const;

      private:
        VkImageView mHandle = VK_NULL_HANDLE;
    };

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_TEXTUREVK_H_
