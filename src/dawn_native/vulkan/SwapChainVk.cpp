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

#include "dawn_native/vulkan/SwapChainVk.h"

#include "dawn_native/Surface.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/BackendVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/TextureVk.h"
#include "dawn_native/vulkan/VulkanError.h"

namespace dawn_native { namespace vulkan {

    // OldSwapChain

    // static
    OldSwapChain* OldSwapChain::Create(Device* device, const SwapChainDescriptor* descriptor) {
        return new OldSwapChain(device, descriptor);
    }

    OldSwapChain::OldSwapChain(Device* device, const SwapChainDescriptor* descriptor)
        : OldSwapChainBase(device, descriptor) {
        const auto& im = GetImplementation();
        DawnWSIContextVulkan wsiContext = {};
        im.Init(im.userData, &wsiContext);

        ASSERT(im.textureUsage != WGPUTextureUsage_None);
        mTextureUsage = static_cast<wgpu::TextureUsage>(im.textureUsage);
    }

    OldSwapChain::~OldSwapChain() {
    }

    TextureBase* OldSwapChain::GetNextTextureImpl(const TextureDescriptor* descriptor) {
        const auto& im = GetImplementation();
        DawnSwapChainNextTexture next = {};
        DawnSwapChainError error = im.GetNextTexture(im.userData, &next);

        if (error) {
            GetDevice()->HandleError(InternalErrorType::Internal, error);
            return nullptr;
        }

        ::VkImage image = NativeNonDispatachableHandleFromU64<::VkImage>(next.texture.u64);
        VkImage nativeTexture = VkImage::CreateFromHandle(image);
        return Texture::CreateForSwapChain(ToBackend(GetDevice()), descriptor, nativeTexture)
            .Detach();
    }

    MaybeError OldSwapChain::OnBeforePresent(TextureViewBase* view) {
        Device* device = ToBackend(GetDevice());

        // Perform the necessary pipeline barriers for the texture to be used with the usage
        // requested by the implementation.
        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        ToBackend(view->GetTexture())
            ->TransitionUsageNow(recordingContext, mTextureUsage, view->GetSubresourceRange());

        DAWN_TRY(device->SubmitPendingCommands());

        return {};
    }

    // SwapChain

    namespace {

        ResultOrError<VkSurfaceKHR> CreateVulkanSurface(Backend* backend, Surface* surface) {
            const VulkanGlobalInfo& info = backend->GetGlobalInfo();
            const VulkanFunctions& fn = backend->GetFunctions();
            VkInstance instance = backend->GetVkInstance();

            switch (surface->GetType()) {
#if defined(DAWN_ENABLE_BACKEND_METAL)
                case Surface::Type::MetalLayer:
                    if (info.HasExt(InstanceExt::MetalSurface)) {
                        VkMetalSurfaceCreateInfoEXT createInfo;
                        createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
                        createInfo.pNext = nullptr;
                        createInfo.flags = 0;
                        createInfo.pLayer = surface->GetMetalLayer();

                        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                        DAWN_TRY(CheckVkSuccess(
                            fn.CreateMetalSurfaceEXT(instance, &createInfo, nullptr, &*vkSurface),
                            "CreateMetalSurface"));
                        return vkSurface;
                    }
                    break;
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_PLATFORM_WINDOWS)
                case Surface::Type::WindowsHWND:
                    if (info.HasExt(InstanceExt::Win32Surface)) {
                        VkWin32SurfaceCreateInfoKHR createInfo;
                        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                        createInfo.pNext = nullptr;
                        createInfo.flags = 0;
                        createInfo.hinstance = static_cast<HINSTANCE>(surface->GetHInstance());
                        createInfo.hwnd = static_cast<HWND>(surface->GetHWND());

                        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                        DAWN_TRY(CheckVkSuccess(
                            fn.CreateWin32SurfaceKHR(instance, &createInfo, nullptr, &*vkSurface),
                            "CreateWin32Surface"));
                        return vkSurface;
                    }
                    break;
#endif  // defined(DAWN_PLATFORM_WINDOWS)

#if defined(DAWN_USE_X11)
                case Surface::Type::Xlib:
                    if (info.HasExt(InstanceExt::XlibSurface)) {
                        VkXlibSurfaceCreateInfoKHR createInfo;
                        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
                        createInfo.pNext = nullptr;
                        createInfo.flags = 0;
                        createInfo.dpy = static_cast<Display*>(surface->GetXDisplay());
                        createInfo.window = surface->GetXWindow();

                        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                        DAWN_TRY(CheckVkSuccess(
                            fn.CreateXlibSurfaceKHR(instance, &createInfo, nullptr, &*vkSurface),
                            "CreateWin32Surface"));
                        return vkSurface;
                    }
                    break;
#endif  // defined(DAWN_USE_X11)

                default:
                    break;
            }

            return DAWN_VALIDATION_ERROR("Unsupported surface type for Vulkan");
        }

        VkPresentModeKHR ToVulkanPresentMode(wgpu::PresentMode mode) {
            switch (mode) {
                case wgpu::PresentMode::Fifo:
                    return VK_PRESENT_MODE_FIFO_KHR;
                case wgpu::PresentMode::Immediate:
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case wgpu::PresentMode::Mailbox:
                    return VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }

    }  // anonymous namespace

    // static
    ResultOrError<SwapChain*> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                NewSwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor) {
        std::unique_ptr<SwapChain> swapchain =
            std::make_unique<SwapChain>(device, surface, descriptor);
        DAWN_TRY(swapchain->Initialize(previousSwapChain));
        return swapchain.release();
    }

    SwapChain::~SwapChain() {
        DetachFromSurface();
    }

    MaybeError SwapChain::Initialize(NewSwapChainBase* previousSwapChain) {
        Device* device = ToBackend(GetDevice());
        Adapter* adapter = ToBackend(GetDevice()->GetAdapter());

        VkSwapchainKHR oldVkSwapChain = VK_NULL_HANDLE;

        if (previousSwapChain != nullptr) {
            // TODO(cwallez@chromium.org): The first time a surface is used with a Device, check
            // it is supported with vkGetPhysicalDeviceSurfaceSupportKHR.

            // TODO(cwallez@chromium.org): figure out what should happen when surfaces are used by
            // multiple backends one after the other. It probably needs to block until the backend
            // and GPU are completely finished with the previous swapchain.
            ASSERT(previousSwapChain->GetBackendType() == wgpu::BackendType::Vulkan);

            // The previous swapchain is a dawn_native::vulkan::SwapChain so we can reuse its
            // VkSurfaceKHR provided they are on the same instance.
            // TODO(cwallez@chromium.org): check they are the same instance.
            // TODO(cwallez@chromium.org): use ToBackend once OldSwapChainBase is removed.
            SwapChain* previousVulkanSwapChain = static_cast<SwapChain*>(previousSwapChain);
            std::swap(previousVulkanSwapChain->mVkSurface, mVkSurface);

            // TODO(cwallez@chromium.org): Figure out switching a single surface between multiple
            // Vulkan devices. Probably needs to block too, but could reuse the surface!
            ASSERT(previousSwapChain->GetDevice() == GetDevice());

            // The previous swapchain was on the same Vulkan device so we can use Vulkan's
            // "oldSwapchain" mechanism to ensure a seamless transition. We track the old swapchain
            // for release immediately so it is not leaked in case of an error. (Vulkan allows
            // destroying it immediately after the call to vkCreateSwapChainKHR but tracking
            // using the fenced deleter makes the code simpler).
            std::swap(previousVulkanSwapChain->mSwapChain, oldVkSwapChain);
            device->GetFencedDeleter()->DeleteWhenUnused(oldVkSwapChain);

            previousSwapChain->DetachFromSurface();
        }

        if (mVkSurface == VK_NULL_HANDLE) {
            DAWN_TRY_ASSIGN(mVkSurface, CreateVulkanSurface(adapter->GetBackend(), GetSurface()));
        }

        VulkanSurfaceInfo surfaceInfo;
        DAWN_TRY_ASSIGN(surfaceInfo, GatherSurfaceInfo(*adapter, mVkSurface));

        DAWN_TRY_ASSIGN(mConfig, ChooseConfig(surfaceInfo));

        // TODO Choose config instead of hardcoding
        VkSwapchainCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = mVkSurface;
        createInfo.minImageCount = 3;                                    // TODO
        createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;               // TODO
        createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // TODO?
        createInfo.imageExtent.width = GetWidth();                       // TODO
        createInfo.imageExtent.height = GetHeight();                     // TODO
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;  // TODO
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;    // TODO
        createInfo.presentMode = mConfig.presentMode;
        createInfo.clipped = false;
        createInfo.oldSwapchain = oldVkSwapChain;

        DAWN_TRY(CheckVkSuccess(device->fn.CreateSwapchainKHR(device->GetVkDevice(), &createInfo,
                                                              nullptr, &*mSwapChain),
                                "CreateSwapChain"));

        // Gather the swapchain's images. Implementations are allowed to return more images than the
        // number we asked for.
        uint32_t count = 0;
        DAWN_TRY(CheckVkSuccess(
            device->fn.GetSwapchainImagesKHR(device->GetVkDevice(), mSwapChain, &count, nullptr),
            "GetSwapChainImages1"));

        // TODO(cwallez@chromium.org): Figure out if we can only have more swapchain images, or also
        // less than requested (and what should happen in that case).
        ASSERT(count >= 3);
        mSwapChainImages.resize(count);
        DAWN_TRY(CheckVkSuccess(
            device->fn.GetSwapchainImagesKHR(device->GetVkDevice(), mSwapChain, &count,
                                             AsVkArray(mSwapChainImages.data())),
            "GetSwapChainImages2"));

        return {};
    }

    ResultOrError<SwapChain::Config> SwapChain::ChooseConfig(
        const VulkanSurfaceInfo& surfaceInfo) const {
        Config config;

        // Choose the present mode. The only guaranteed one is FIFO so it has to be the fallback for
        // all other present modes. IMMEDIATE has tearing which is generally undesirable so it can't
        // be the fallback for MAILBOX. So the fallback order is always IMMEDIATE -> MAILBOX ->
        // FIFO.
        {
            auto HasPresentMode = [](const std::vector<VkPresentModeKHR>& modes,
                                     VkPresentModeKHR target) -> bool {
                return std::find(modes.begin(), modes.end(), target) != modes.end();
            };

            VkPresentModeKHR targetMode = ToVulkanPresentMode(GetPresentMode());
            const std::array<VkPresentModeKHR, 3> kPresentModeFallbacks = {
                VK_PRESENT_MODE_IMMEDIATE_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_FIFO_KHR,
            };

            // Go to the target mode.
            size_t modeIndex = 0;
            while (kPresentModeFallbacks[modeIndex] != targetMode) {
                modeIndex++;
            }

            // Find the first available fallback.
            while (!HasPresentMode(surfaceInfo.presentModes, kPresentModeFallbacks[modeIndex])) {
                modeIndex++;
            }

            ASSERT(modeIndex < kPresentModeFallbacks.size());
            config.presentMode = kPresentModeFallbacks[modeIndex];
        }

        return config;
    }

    MaybeError SwapChain::PresentImpl() {
        Device* device = ToBackend(GetDevice());

        // Transition the texture to the present usage.
        // TODO(cwallez@chromium.org): Remove the need for this by eagerly transitioning the
        // presentable texture to present at the end of submits that use them and ideally even
        // folding that in the free layout transition at the end of render passes.
        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        mTexture->TransitionUsageNow(recordingContext, kPresentTextureUsage,
                                     mTexture->GetAllSubresources());
        DAWN_TRY(device->SubmitPendingCommands());

        // Assuming that the present queue is the same as the graphics queue, the proper
        // synchronization has already been done on the queue so we don't need to wait on any
        // semaphores.
        // TODO(cwallez@chromium.org): Support the present queue not being the main queue.
        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &*mSwapChain;
        presentInfo.pImageIndices = &mLastImageIndex;
        presentInfo.pResults = nullptr;

        // Free the texture before present so error handling doesn't skip that step.
        mTexture->Destroy();
        mTexture = nullptr;

        VkResult result =
            VkResult::WrapUnsafe(device->fn.QueuePresentKHR(device->GetQueue(), &presentInfo));
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // TODO reinitialize?
        } else if (result == VK_ERROR_SURFACE_LOST_KHR) {
            // TODO IDK what to do here, just lose the device?
        } else {
            DAWN_TRY(CheckVkSuccess(::VkResult(result), "QueuePresent"));
        }

        return {};
    }

    ResultOrError<TextureViewBase*> SwapChain::GetCurrentTextureViewImpl() {
        Device* device = ToBackend(GetDevice());

        // Transiently create a semaphore that will be signaled when the presentation engine is done
        // with the swapchain image. Further operations on the image will wait for this semaphore.
        VkSemaphoreCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        VkSemaphore semaphore = VK_NULL_HANDLE;
        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateSemaphore(device->GetVkDevice(), &createInfo, nullptr, &*semaphore),
            "CreateSemaphore"));

        // TODO(cwallez@chromium.org) put the semaphore on the texture so it is waited on when used
        // instead of directly on the recording context?
        device->GetPendingRecordingContext()->waitSemaphores.push_back(semaphore);

        VkResult result = VkResult::WrapUnsafe(device->fn.AcquireNextImageKHR(
            device->GetVkDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), semaphore,
            VkFence{}, &mLastImageIndex));
        if (result == VK_SUBOPTIMAL_KHR) {
            // TODO reinitialize?
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // TODO reinitialize?
        } else if (result == VK_ERROR_SURFACE_LOST_KHR) {
            // TODO IDK what to do here, just lose the device?
        } else {
            DAWN_TRY(CheckVkSuccess(::VkResult(result), "AcquireNextImage"));
        }

        VkImage currentImage = mSwapChainImages[mLastImageIndex];

        TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(this);
        mTexture = Texture::CreateForSwapChain(device, &textureDesc, currentImage);
        return mTexture->CreateView(nullptr);
    }

    void SwapChain::DetachFromSurfaceImpl() {
        if (mTexture.Get() != nullptr) {
            mTexture->Destroy();
            mTexture = nullptr;
        }

        // The swapchain images are destroyed with the swapchain.
        if (mSwapChain != VK_NULL_HANDLE) {
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mSwapChain);
            mSwapChain = VK_NULL_HANDLE;
        }

        if (mVkSurface != VK_NULL_HANDLE) {
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mVkSurface);
            mVkSurface = VK_NULL_HANDLE;
        }
    }

}}  // namespace dawn_native::vulkan
