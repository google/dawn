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

#ifndef DAWNNATIVE_VULKAN_SWAPCHAINVK_H_
#define DAWNNATIVE_VULKAN_SWAPCHAINVK_H_

#include "dawn_native/SwapChain.h"

#include "common/vulkan_platform.h"

#include <vector>

namespace dawn_native { namespace vulkan {

    class Device;
    class Texture;
    struct VulkanSurfaceInfo;

    class OldSwapChain : public OldSwapChainBase {
      public:
        static OldSwapChain* Create(Device* device, const SwapChainDescriptor* descriptor);

      protected:
        OldSwapChain(Device* device, const SwapChainDescriptor* descriptor);
        ~OldSwapChain() override;

        TextureBase* GetNextTextureImpl(const TextureDescriptor* descriptor) override;
        MaybeError OnBeforePresent(TextureViewBase* texture) override;

      private:
        wgpu::TextureUsage mTextureUsage;
    };

    class SwapChain : public NewSwapChainBase {
      public:
        static ResultOrError<SwapChain*> Create(Device* device,
                                                Surface* surface,
                                                NewSwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor);
        ~SwapChain() override;

      private:
        using NewSwapChainBase::NewSwapChainBase;
        MaybeError Initialize(NewSwapChainBase* previousSwapChain);

        struct Config {
            // Information that's passed to vulkan swapchain creation.
            VkPresentModeKHR presentMode;
            VkExtent2D extent;
            VkImageUsageFlags usage;
            VkFormat format;
            VkColorSpaceKHR colorSpace;
            uint32_t targetImageCount;
            VkSurfaceTransformFlagBitsKHR transform;
            VkCompositeAlphaFlagBitsKHR alphaMode;

            // Redundant information but as WebGPU enums to create the wgpu::Texture that
            // encapsulates the native swapchain texture.
            wgpu::TextureUsage wgpuUsage;
            wgpu::TextureFormat wgpuFormat;

            // Information about the blit workarounds we need to do (if any)
            bool needsBlit = false;
        };
        ResultOrError<Config> ChooseConfig(const VulkanSurfaceInfo& surfaceInfo) const;
        ResultOrError<TextureViewBase*> GetCurrentTextureViewInternal(bool isReentrant = false);

        // NewSwapChainBase implementation
        MaybeError PresentImpl() override;
        ResultOrError<TextureViewBase*> GetCurrentTextureViewImpl() override;
        void DetachFromSurfaceImpl() override;

        Config mConfig;

        VkSurfaceKHR mVkSurface = VK_NULL_HANDLE;
        VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> mSwapChainImages;
        uint32_t mLastImageIndex = 0;

        Ref<Texture> mBlitTexture;
        Ref<Texture> mTexture;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_SWAPCHAINVK_H_
