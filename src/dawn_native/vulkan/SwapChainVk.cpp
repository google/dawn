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

#include "common/Compiler.h"
#include "dawn_native/Surface.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/BackendVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/TextureVk.h"
#include "dawn_native/vulkan/VulkanError.h"

#include <algorithm>

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

            // May not be used in the platform-specific switches below.
            DAWN_UNUSED(info);
            DAWN_UNUSED(fn);
            DAWN_UNUSED(instance);

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
                            "CreateXlibSurface"));
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

        uint32_t MinImageCountForPresentMode(VkPresentModeKHR mode) {
            switch (mode) {
                case VK_PRESENT_MODE_FIFO_KHR:
                case VK_PRESENT_MODE_IMMEDIATE_KHR:
                    return 2;
                case VK_PRESENT_MODE_MAILBOX_KHR:
                    return 3;
                default:
                    UNREACHABLE();
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

    // Note that when we need to re-create the swapchain because it is out of date,
    // previousSwapChain can be set to `this`.
    MaybeError SwapChain::Initialize(NewSwapChainBase* previousSwapChain) {
        Device* device = ToBackend(GetDevice());
        Adapter* adapter = ToBackend(GetDevice()->GetAdapter());

        VkSwapchainKHR previousVkSwapChain = VK_NULL_HANDLE;

        if (previousSwapChain != nullptr) {
            // TODO(cwallez@chromium.org): The first time a surface is used with a Device, check
            // it is supported with vkGetPhysicalDeviceSurfaceSupportKHR.

            // TODO(cwallez@chromium.org): figure out what should happen when surfaces are used by
            // multiple backends one after the other. It probably needs to block until the backend
            // and GPU are completely finished with the previous swapchain.
            if (previousSwapChain->GetBackendType() != wgpu::BackendType::Vulkan) {
                return DAWN_VALIDATION_ERROR("vulkan::SwapChain cannot switch between APIs");
            }
            // TODO(cwallez@chromium.org): use ToBackend once OldSwapChainBase is removed.
            SwapChain* previousVulkanSwapChain = static_cast<SwapChain*>(previousSwapChain);

            // TODO(cwallez@chromium.org): Figure out switching a single surface between multiple
            // Vulkan devices on different VkInstances. Probably needs to block too!
            VkInstance previousInstance =
                ToBackend(previousSwapChain->GetDevice())->GetVkInstance();
            if (previousInstance != ToBackend(GetDevice())->GetVkInstance()) {
                return DAWN_VALIDATION_ERROR("vulkan::SwapChain cannot switch between instances");
            }

            // The previous swapchain is a dawn_native::vulkan::SwapChain so we can reuse its
            // VkSurfaceKHR provided since they are on the same instance.
            std::swap(previousVulkanSwapChain->mVkSurface, mVkSurface);

            // The previous swapchain was on the same Vulkan instance so we can use Vulkan's
            // "oldSwapchain" mechanism to ensure a seamless transition. We track the previous
            // swapchain for release immediately so it is not leaked in case of an error. (Vulkan
            // allows destroying it immediately after the call to vkCreateSwapChainKHR but tracking
            // using the fenced deleter makes the code simpler).
            std::swap(previousVulkanSwapChain->mSwapChain, previousVkSwapChain);
            ToBackend(previousSwapChain->GetDevice())
                ->GetFencedDeleter()
                ->DeleteWhenUnused(previousVkSwapChain);
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
        createInfo.minImageCount = mConfig.targetImageCount;
        createInfo.imageFormat = mConfig.format;
        createInfo.imageColorSpace = mConfig.colorSpace;
        createInfo.imageExtent = mConfig.extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = mConfig.usage;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = mConfig.transform;
        createInfo.compositeAlpha = mConfig.alphaMode;
        createInfo.presentMode = mConfig.presentMode;
        createInfo.clipped = false;
        createInfo.oldSwapchain = previousVkSwapChain;

        DAWN_TRY(CheckVkSuccess(device->fn.CreateSwapchainKHR(device->GetVkDevice(), &createInfo,
                                                              nullptr, &*mSwapChain),
                                "CreateSwapChain"));

        // Gather the swapchain's images. Implementations are allowed to return more images than the
        // number we asked for.
        uint32_t count = 0;
        DAWN_TRY(CheckVkSuccess(
            device->fn.GetSwapchainImagesKHR(device->GetVkDevice(), mSwapChain, &count, nullptr),
            "GetSwapChainImages1"));

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

        // Choose the target width or do a blit.
        if (GetWidth() < surfaceInfo.capabilities.minImageExtent.width ||
            GetWidth() > surfaceInfo.capabilities.maxImageExtent.width ||
            GetHeight() < surfaceInfo.capabilities.minImageExtent.height ||
            GetHeight() > surfaceInfo.capabilities.maxImageExtent.height) {
            config.needsBlit = true;
        } else {
            config.extent.width = GetWidth();
            config.extent.height = GetHeight();
        }

        // Choose the target usage or do a blit.
        VkImageUsageFlags targetUsages =
            VulkanImageUsage(GetUsage(), GetDevice()->GetValidInternalFormat(GetFormat()));
        VkImageUsageFlags supportedUsages = surfaceInfo.capabilities.supportedUsageFlags;
        if (!IsSubset(targetUsages, supportedUsages)) {
            config.needsBlit = true;
        } else {
            config.usage = targetUsages;
            config.wgpuUsage = GetUsage();
        }

        // Only support BGRA8Unorm with SRGB color space for now.
        bool hasBGRA8Unorm = false;
        for (const VkSurfaceFormatKHR& format : surfaceInfo.formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                hasBGRA8Unorm = true;
                break;
            }
        }
        if (!hasBGRA8Unorm) {
            return DAWN_INTERNAL_ERROR(
                "Vulkan swapchain must support BGRA8Unorm with SRGB colorspace");
        }
        config.format = VK_FORMAT_B8G8R8A8_UNORM;
        config.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        config.wgpuFormat = wgpu::TextureFormat::BGRA8Unorm;

        // Only the identity transform with opaque alpha is supported for now.
        if ((surfaceInfo.capabilities.supportedTransforms &
             VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0) {
            return DAWN_VALIDATION_ERROR("Vulkan swapchain must support the identity transform");
        }
        config.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

        if ((surfaceInfo.capabilities.supportedCompositeAlpha &
             VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0) {
            return DAWN_VALIDATION_ERROR("Vulkan swapchain must support opaque alpha");
        }
        config.alphaMode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        // Choose the number of images for the swapchain= and clamp it to the min and max from the
        // surface capabilities. maxImageCount = 0 means there is no limit.
        ASSERT(surfaceInfo.capabilities.maxImageCount == 0 ||
               surfaceInfo.capabilities.minImageCount <= surfaceInfo.capabilities.maxImageCount);
        uint32_t targetCount = MinImageCountForPresentMode(config.presentMode);

        targetCount = std::max(targetCount, surfaceInfo.capabilities.minImageCount);
        if (surfaceInfo.capabilities.maxImageCount != 0) {
            targetCount = std::min(targetCount, surfaceInfo.capabilities.maxImageCount);
        }

        config.targetImageCount = targetCount;

        // Choose a valid config for the swapchain texture that will receive the blit.
        if (config.needsBlit) {
            // Vulkan has provisions to have surfaces that adapt to the swapchain size. If that's
            // the case it is very likely that the target extent works, but clamp it just in case.
            // Using the target extent for the blit is better when possible so that texels don't
            // get stretched. This case is exposed by having the special "-1" value in both
            // dimensions of the extent.
            constexpr uint32_t kSpecialValue = 0xFFFF'FFFF;
            if (surfaceInfo.capabilities.currentExtent.width == kSpecialValue &&
                surfaceInfo.capabilities.currentExtent.height == kSpecialValue) {
                // extent = clamp(targetExtent, minExtent, maxExtent)
                config.extent.width = GetWidth();
                config.extent.width =
                    std::min(config.extent.width, surfaceInfo.capabilities.maxImageExtent.width);
                config.extent.width =
                    std::max(config.extent.width, surfaceInfo.capabilities.minImageExtent.width);

                config.extent.height = GetHeight();
                config.extent.height =
                    std::min(config.extent.height, surfaceInfo.capabilities.maxImageExtent.height);
                config.extent.height =
                    std::max(config.extent.height, surfaceInfo.capabilities.minImageExtent.height);
            } else {
                // If it is not an adaptable swapchain, just use the current extent for the blit
                // texture.
                config.extent = surfaceInfo.capabilities.currentExtent;
            }

            // TODO(cwallez@chromium.org): If the swapchain image doesn't support TRANSFER_DST
            // then we'll need to have a second fallback that uses a blit shader :(
            if ((supportedUsages & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                return DAWN_INTERNAL_ERROR(
                    "Swapchain cannot fallback to a blit because of a missing "
                    "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
            }
            config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            config.wgpuUsage = wgpu::TextureUsage::CopyDst;
        }

        return config;
    }

    MaybeError SwapChain::PresentImpl() {
        Device* device = ToBackend(GetDevice());

        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();

        if (mConfig.needsBlit) {
            // TODO ditto same as present below: eagerly transition the blit texture to CopySrc.
            mBlitTexture->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopySrc,
                                             mBlitTexture->GetAllSubresources());
            mTexture->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst,
                                         mTexture->GetAllSubresources());

            VkImageBlit region;
            region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.srcSubresource.mipLevel = 0;
            region.srcSubresource.baseArrayLayer = 0;
            region.srcSubresource.layerCount = 1;
            region.srcOffsets[0] = {0, 0, 0};
            region.srcOffsets[1] = {static_cast<int32_t>(mBlitTexture->GetWidth()),
                                    static_cast<int32_t>(mBlitTexture->GetHeight()), 1};

            region.dstSubresource = region.srcSubresource;
            region.dstOffsets[0] = {0, 0, 0};
            region.dstOffsets[1] = {static_cast<int32_t>(mTexture->GetWidth()),
                                    static_cast<int32_t>(mTexture->GetHeight()), 1};

            device->fn.CmdBlitImage(recordingContext->commandBuffer, mBlitTexture->GetHandle(),
                                    mBlitTexture->GetCurrentLayoutForSwapChain(),
                                    mTexture->GetHandle(), mTexture->GetCurrentLayoutForSwapChain(),
                                    1, &region, VK_FILTER_LINEAR);

            // TODO(cwallez@chromium.org): Find a way to reuse the blit texture between frames
            // instead of creating a new one every time. This will involve "un-destroying" the
            // texture or making the blit texture "external".
            mBlitTexture->Destroy();
            mBlitTexture = nullptr;
        }

        // TODO(cwallez@chromium.org): Remove the need for this by eagerly transitioning the
        // presentable texture to present at the end of submits that use them and ideally even
        // folding that in the free layout transition at the end of render passes.
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

        switch (result) {
            case VK_SUCCESS:
            // VK_SUBOPTIMAL_KHR means "a swapchain no longer matches the surface properties
            // exactly, but can still be used to present to the surface successfully", so we
            // can also treat it as a "success" error code of vkQueuePresentKHR().
            case VK_SUBOPTIMAL_KHR:
                return {};

            // This present cannot be recovered. Re-initialize the VkSwapchain so that future
            // presents work..
            case VK_ERROR_OUT_OF_DATE_KHR:
                return Initialize(this);

            // TODO(cwallez@chromium.org): Allow losing the surface at Dawn's API level?
            case VK_ERROR_SURFACE_LOST_KHR:
            default:
                return CheckVkSuccess(::VkResult(result), "QueuePresent");
        }
    }

    ResultOrError<TextureViewBase*> SwapChain::GetCurrentTextureViewImpl() {
        return GetCurrentTextureViewInternal();
    }

    ResultOrError<TextureViewBase*> SwapChain::GetCurrentTextureViewInternal(bool isReentrant) {
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

        VkResult result = VkResult::WrapUnsafe(device->fn.AcquireNextImageKHR(
            device->GetVkDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), semaphore,
            VkFence{}, &mLastImageIndex));

        if (result == VK_SUCCESS) {
            // TODO(cwallez@chromium.org) put the semaphore on the texture so it is waited on when
            // used instead of directly on the recording context?
            device->GetPendingRecordingContext()->waitSemaphores.push_back(semaphore);
        } else {
            // The semaphore wasn't actually used (? this is unclear in the spec). Delete it when
            // we get a chance.
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(semaphore);
        }

        switch (result) {
            // TODO(cwallez@chromium.org): Introduce a mechanism to notify the application that
            // the swapchain is in a suboptimal state?
            case VK_SUBOPTIMAL_KHR:
            case VK_SUCCESS:
                break;

            case VK_ERROR_OUT_OF_DATE_KHR: {
                // Prevent infinite recursive calls to GetCurrentTextureViewInternal when the
                // swapchains always return that they are out of date.
                if (isReentrant) {
                    // TODO(cwallez@chromium.org): Allow losing the surface instead?
                    return DAWN_INTERNAL_ERROR(
                        "Wasn't able to recuperate the surface after a VK_ERROR_OUT_OF_DATE_KHR");
                }

                // Re-initialize the VkSwapchain and try getting the texture again.
                DAWN_TRY(Initialize(this));
                return GetCurrentTextureViewInternal(true);
            }

            // TODO(cwallez@chromium.org): Allow losing the surface at Dawn's API level?
            case VK_ERROR_SURFACE_LOST_KHR:
            default:
                DAWN_TRY(CheckVkSuccess(::VkResult(result), "AcquireNextImage"));
        }

        TextureDescriptor textureDesc;
        textureDesc.size.width = mConfig.extent.width;
        textureDesc.size.height = mConfig.extent.height;
        textureDesc.format = mConfig.wgpuFormat;
        textureDesc.usage = mConfig.wgpuUsage;

        VkImage currentImage = mSwapChainImages[mLastImageIndex];
        mTexture = Texture::CreateForSwapChain(device, &textureDesc, currentImage);

        // In the happy path we can use the swapchain image directly.
        if (!mConfig.needsBlit) {
            return mTexture->CreateView();
        }

        // The blit texture always perfectly matches what the user requested for the swapchain.
        // We need to add the Vulkan TRANSFER_SRC flag for the vkCmdBlitImage call.
        TextureDescriptor desc = GetSwapChainBaseTextureDescriptor(this);
        DAWN_TRY_ASSIGN(mBlitTexture,
                        Texture::Create(device, &desc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
        return mBlitTexture->CreateView();
    }

    void SwapChain::DetachFromSurfaceImpl() {
        if (mTexture != nullptr) {
            mTexture->Destroy();
            mTexture = nullptr;
        }

        if (mBlitTexture != nullptr) {
            mBlitTexture->Destroy();
            mBlitTexture = nullptr;
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
