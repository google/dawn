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

#include "backend/vulkan/NativeSwapChainImplVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {

        bool ChooseSurfaceConfig(const VulkanSurfaceInfo& info,
                                 NativeSwapChainImpl::ChosenConfig* config) {
            // TODO(cwallez@chromium.org): For now this is hardcoded to what works with one NVIDIA
            // driver. Need to generalize
            config->nativeFormat = VK_FORMAT_B8G8R8A8_UNORM;
            config->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            config->format = nxt::TextureFormat::B8G8R8A8Unorm;
            config->minImageCount = 3;
            // TODO(cwallez@chromium.org): This is upside down compared to what we want, at least
            // on Linux
            config->preTransform = info.capabilities.currentTransform;
            config->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            config->compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            return true;
        }

    }  // anonymous namespace

    NativeSwapChainImpl::NativeSwapChainImpl(Device* device, VkSurfaceKHR surface)
        : mSurface(surface), mDevice(device) {
    }

    NativeSwapChainImpl::~NativeSwapChainImpl() {
        if (mSwapChain != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mSwapChain);
            mSwapChain = VK_NULL_HANDLE;
        }
        if (mSurface != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mSurface);
            mSurface = VK_NULL_HANDLE;
        }
    }

    void NativeSwapChainImpl::Init(nxtWSIContextVulkan* /*context*/) {
        if (!GatherSurfaceInfo(*mDevice, mSurface, &mInfo)) {
            ASSERT(false);
        }

        if (!ChooseSurfaceConfig(mInfo, &mConfig)) {
            ASSERT(false);
        }
    }

    nxtSwapChainError NativeSwapChainImpl::Configure(nxtTextureFormat format,
                                                     nxtTextureUsageBit usage,
                                                     uint32_t width,
                                                     uint32_t height) {
        ASSERT(mInfo.capabilities.minImageExtent.width <= width);
        ASSERT(mInfo.capabilities.maxImageExtent.width >= width);
        ASSERT(mInfo.capabilities.minImageExtent.height <= height);
        ASSERT(mInfo.capabilities.maxImageExtent.height >= height);

        ASSERT(format == static_cast<nxtTextureFormat>(GetPreferredFormat()));
        // TODO(cwallez@chromium.org): need to check usage works too

        // Create the swapchain with the configuration we chose
        VkSwapchainCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = mSurface;
        createInfo.minImageCount = mConfig.minImageCount;
        createInfo.imageFormat = mConfig.nativeFormat;
        createInfo.imageColorSpace = mConfig.colorSpace;
        createInfo.imageExtent.width = width;
        createInfo.imageExtent.height = height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage =
            VulkanImageUsage(static_cast<nxt::TextureUsageBit>(usage), mConfig.format);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = mConfig.preTransform;
        createInfo.compositeAlpha = mConfig.compositeAlpha;
        createInfo.presentMode = mConfig.presentMode;
        createInfo.clipped = false;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (mDevice->fn.CreateSwapchainKHR(mDevice->GetVkDevice(), &createInfo, nullptr,
                                           &mSwapChain) != VK_SUCCESS) {
            ASSERT(false);
        }

        // Gather the swapchain's images. Implementations are allowed to return more images than the
        // number we asked for.
        uint32_t count = 0;
        if (mDevice->fn.GetSwapchainImagesKHR(mDevice->GetVkDevice(), mSwapChain, &count,
                                              nullptr) != VK_SUCCESS) {
            ASSERT(false);
        }

        ASSERT(count >= mConfig.minImageCount);
        mSwapChainImages.resize(count);
        if (mDevice->fn.GetSwapchainImagesKHR(mDevice->GetVkDevice(), mSwapChain, &count,
                                              mSwapChainImages.data()) != VK_SUCCESS) {
            ASSERT(false);
        }

        // Do the initial layout transition for all these images from an undefined layout to
        // present so that it matches the "present" usage after the first GetNextTexture.
        VkCommandBuffer commands = mDevice->GetPendingCommandBuffer();
        for (VkImage image : mSwapChainImages) {
            VkImageMemoryBarrier barrier;
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            mDevice->fn.CmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0,
                                           nullptr, 1, &barrier);
        }

        return NXT_SWAP_CHAIN_NO_ERROR;
    }

    nxtSwapChainError NativeSwapChainImpl::GetNextTexture(nxtSwapChainNextTexture* nextTexture) {
        // Transiently create a semaphore that will be signaled when the presentation engine is done
        // with the swapchain image. Further operations on the image will wait for this semaphore.
        VkSemaphore semaphore = VK_NULL_HANDLE;
        {
            VkSemaphoreCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            if (mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &createInfo, nullptr,
                                            &semaphore) != VK_SUCCESS) {
                ASSERT(false);
            }
        }

        if (mDevice->fn.AcquireNextImageKHR(mDevice->GetVkDevice(), mSwapChain,
                                            std::numeric_limits<uint64_t>::max(), semaphore,
                                            VK_NULL_HANDLE, &mLastImageIndex) != VK_SUCCESS) {
            ASSERT(false);
        }

        nextTexture->texture.u64 = mSwapChainImages[mLastImageIndex].GetHandle();
        mDevice->AddWaitSemaphore(semaphore);

        return NXT_SWAP_CHAIN_NO_ERROR;
    }

    nxtSwapChainError NativeSwapChainImpl::Present() {
        // Since we're going to do a queue operations we need to flush pending commands such as
        // layout transitions of the swapchain images to the PRESENT layout.
        mDevice->SubmitPendingCommands();

        // Assuming that the present queue is the same as the graphics queue, the proper
        // synchronization has already been done by the usage transition to present so we don't
        // need to wait on any semaphores.
        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mSwapChain;
        presentInfo.pImageIndices = &mLastImageIndex;
        presentInfo.pResults = nullptr;

        VkQueue queue = mDevice->GetQueue();
        if (mDevice->fn.QueuePresentKHR(queue, &presentInfo) != VK_SUCCESS) {
            ASSERT(false);
        }

        return NXT_SWAP_CHAIN_NO_ERROR;
    }

    nxt::TextureFormat NativeSwapChainImpl::GetPreferredFormat() const {
        return mConfig.format;
    }

}}  // namespace backend::vulkan
