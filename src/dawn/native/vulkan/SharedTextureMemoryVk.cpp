// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/SharedTextureMemoryVk.h"

#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/SystemHandle.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SharedFenceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

#if DAWN_PLATFORM_IS(LINUX)

// Encoding from <drm/drm_fourcc.h>
constexpr uint32_t DrmFourccCode(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return a | (b << 8) | (c << 16) | (d << 24);
}

constexpr uint32_t DrmFourccCode(char a, char b, char c, char d) {
    return DrmFourccCode(static_cast<uint32_t>(a), static_cast<uint32_t>(b),
                         static_cast<uint32_t>(c), static_cast<uint32_t>(d));
}

constexpr auto kDrmFormatR8 = DrmFourccCode('R', '8', ' ', ' '); /* [7:0] R */
constexpr auto kDrmFormatGR88 =
    DrmFourccCode('G', 'R', '8', '8'); /* [15:0] G:R 8:8 little endian */
constexpr auto kDrmFormatXRGB8888 =
    DrmFourccCode('X', 'R', '2', '4'); /* [15:0] x:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatXBGR8888 =
    DrmFourccCode('X', 'B', '2', '4'); /* [15:0] x:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatARGB8888 =
    DrmFourccCode('A', 'R', '2', '4'); /* [31:0] A:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR8888 =
    DrmFourccCode('A', 'B', '2', '4'); /* [31:0] A:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR2101010 =
    DrmFourccCode('A', 'B', '3', '0'); /* [31:0] A:B:G:R 2:10:10:10 little endian */
constexpr auto kDrmFormatABGR16161616F =
    DrmFourccCode('A', 'B', '4', 'H'); /* [63:0] A:B:G:R 16:16:16:16 little endian */
constexpr auto kDrmFormatNV12 = DrmFourccCode('N', 'V', '1', '2'); /* 2x2 subsampled Cr:Cb plane */

ResultOrError<wgpu::TextureFormat> FormatFromDrmFormat(uint32_t drmFormat) {
    switch (drmFormat) {
        case kDrmFormatR8:
            return wgpu::TextureFormat::R8Unorm;
        case kDrmFormatGR88:
            return wgpu::TextureFormat::RG8Unorm;
        case kDrmFormatXRGB8888:
        case kDrmFormatARGB8888:
            return wgpu::TextureFormat::BGRA8Unorm;
        case kDrmFormatXBGR8888:
        case kDrmFormatABGR8888:
            return wgpu::TextureFormat::RGBA8Unorm;
        case kDrmFormatABGR2101010:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case kDrmFormatABGR16161616F:
            return wgpu::TextureFormat::RGBA16Float;
        case kDrmFormatNV12:
            return wgpu::TextureFormat::R8BG8Biplanar420Unorm;
        default:
            return DAWN_VALIDATION_ERROR("Unsupported drm format %x.", drmFormat);
    }
}

// Get the properties for the (format, modifier) pair.
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDrmFormatModifierPropertiesEXT.html
ResultOrError<VkDrmFormatModifierPropertiesEXT> GetFormatModifierProps(
    const VulkanFunctions& fn,
    VkPhysicalDevice vkPhysicalDevice,
    VkFormat format,
    uint64_t modifier) {
    VkFormatProperties2 formatProps = {};
    formatProps.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    PNextChainBuilder formatPropsChain(&formatProps);

    // Obtain the list of Linux DRM format modifiers compatible with a VkFormat
    VkDrmFormatModifierPropertiesListEXT formatModifierPropsList = {};
    formatModifierPropsList.drmFormatModifierCount = 0;
    formatModifierPropsList.pDrmFormatModifierProperties = nullptr;
    formatPropsChain.Add(&formatModifierPropsList,
                         VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT);

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);

    const uint32_t modifierCount = formatModifierPropsList.drmFormatModifierCount;

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierPropsVector;
    formatModifierPropsVector.resize(modifierCount);
    formatModifierPropsList.pDrmFormatModifierProperties = formatModifierPropsVector.data();

    fn.GetPhysicalDeviceFormatProperties2(vkPhysicalDevice, format, &formatProps);

    // Find the modifier props that match the modifier, and return them.
    for (const auto& props : formatModifierPropsVector) {
        if (props.drmFormatModifier == modifier) {
            return VkDrmFormatModifierPropertiesEXT{props};
        }
    }
    return DAWN_VALIDATION_ERROR("DRM format modifier %u not supported.", modifier);
}

#endif  // DAWN_PLATFORM_IS(LINUX)

}  // namespace

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    const char* label,
    const SharedTextureMemoryDmaBufDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(LINUX)
    VkDevice vkDevice = device->GetVkDevice();
    VkPhysicalDevice vkPhysicalDevice =
        ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice();

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "depthOrArrayLayers was not 1.");

    SharedTextureMemoryProperties properties;
    properties.size = {descriptor->size.width, descriptor->size.height,
                       descriptor->size.depthOrArrayLayers};

    DAWN_TRY_ASSIGN(properties.format, FormatFromDrmFormat(descriptor->drmFormat));

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    VkFormat vkFormat = VulkanImageFormat(device, properties.format);

    if (internalFormat->IsMultiPlanar()) {
        properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;
    } else {
        properties.usage =
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
            wgpu::TextureUsage::TextureBinding |
            (internalFormat->supportsStorageUsage ? wgpu::TextureUsage::StorageBinding
                                                  : wgpu::TextureUsage::None) |
            (internalFormat->isRenderable ? wgpu::TextureUsage::RenderAttachment
                                          : wgpu::TextureUsage::None);
    }

    // Usage flags to create the image with.
    VkImageUsageFlags vkUsageFlags = VulkanImageUsage(properties.usage, *internalFormat);

    // Number of memory planes in the image which will be queried from the DRM modifier.
    uint32_t memoryPlaneCount;

    // Info describing the image import. We will use this to check the import is valid, and then
    // perform the actual VkImage creation.
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    // List of view formats the image can be created.
    std::array<VkFormat, 2> viewFormats;
    VkImageFormatListCreateInfo imageFormatListInfo = {};

    // Validate that the import is valid.
    {
        // Verify plane count for the modifier.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDrmFormatModifierPropertiesEXT.html#_description
        VkDrmFormatModifierPropertiesEXT drmModifierProps;
        DAWN_TRY_ASSIGN(drmModifierProps,
                        GetFormatModifierProps(device->fn, vkPhysicalDevice, vkFormat,
                                               descriptor->drmModifier));
        memoryPlaneCount = drmModifierProps.drmFormatModifierPlaneCount;
        if (drmModifierProps.drmFormatModifier == 0 /* DRM_FORMAT_MOD_LINEAR */) {
            uint32_t formatPlaneCount = GetAspectCount(internalFormat->aspects);
            DAWN_INVALID_IF(memoryPlaneCount != formatPlaneCount,
                            "DRM format plane count (%u) must match the format plane count (%u) if "
                            "drmModifier is DRM_FORMAT_MOD_LINEAR.",
                            memoryPlaneCount, formatPlaneCount);
        }
        DAWN_INVALID_IF(
            memoryPlaneCount != descriptor->planeCount,
            "Memory plane count (%x) for drm format (%u) and modifier (%u) specify a plane "
            "count of %u which "
            "does not match the provided plane count (%u)",
            vkFormat, descriptor->drmFormat, descriptor->drmModifier, memoryPlaneCount,
            descriptor->planeCount);
        DAWN_INVALID_IF(memoryPlaneCount == 0, "Memory plane count must not be 0");
        DAWN_INVALID_IF(
            memoryPlaneCount > 1 && !(drmModifierProps.drmFormatModifierTilingFeatures &
                                      VK_FORMAT_FEATURE_DISJOINT_BIT),
            "VK_FORMAT_FEATURE_DISJOINT_BIT tiling is not supported for multi-planar DRM "
            "format (%u) with drm modifier (%u).",
            descriptor->drmFormat, descriptor->drmModifier);
        DAWN_INVALID_IF(memoryPlaneCount > kMaxPlanesPerFormat,
                        "Memory plane count (%u) must not exceed %u.", memoryPlaneCount,
                        kMaxPlanesPerFormat);
        DAWN_INVALID_IF(memoryPlaneCount > 1,
                        "TODO(crbug.com/dawn/1548): Disjoint planar import not supported yet.");

        // Verify that the format modifier of the external memory and the requested Vulkan format
        // are actually supported together in a dma-buf import.
        imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        imageFormatInfo.format = vkFormat;
        imageFormatInfo.type = VK_IMAGE_TYPE_2D;
        imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        imageFormatInfo.usage = vkUsageFlags;
        imageFormatInfo.flags = 0;

        if (memoryPlaneCount > 1) {
            imageFormatInfo.flags |= VK_IMAGE_CREATE_DISJOINT_BIT;
        }

        PNextChainBuilder imageFormatInfoChain(&imageFormatInfo);

        VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
        externalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        imageFormatInfoChain.Add(&externalImageFormatInfo,
                                 VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO);

        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmModifierInfo = {};
        drmModifierInfo.drmFormatModifier = descriptor->drmModifier;
        drmModifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageFormatInfoChain.Add(
            &drmModifierInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT);

        constexpr wgpu::TextureUsage kUsageRequiringView = wgpu::TextureUsage::RenderAttachment |
                                                           wgpu::TextureUsage::TextureBinding |
                                                           wgpu::TextureUsage::StorageBinding;
        const bool mayNeedView = (properties.usage & kUsageRequiringView) != 0;
        const bool supportsImageFormatList =
            device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList);
        if (mayNeedView) {
            // Add the mutable format bit for view reinterpretation.
            imageFormatInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

            // Append the list of view formats the image must be compatible with.
            if (supportsImageFormatList) {
                if (internalFormat->IsMultiPlanar()) {
                    viewFormats = {
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane0).format),
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane1).format)};
                    imageFormatListInfo.viewFormatCount = 2;
                } else {
                    // Pass the format as the one and only allowed view format.
                    // Use of VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT requires passing a non-zero
                    // list.
                    // TODO(crbug.com/dawn/1745): Allow other types of WebGPU format
                    // reinterpretation (srgb).
                    viewFormats = {vkFormat};
                    imageFormatListInfo.viewFormatCount = 1;
                }
                imageFormatListInfo.pViewFormats = viewFormats.data();
                imageFormatInfoChain.Add(&imageFormatListInfo,
                                         VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
            }
        }

        VkImageFormatProperties2 imageFormatProps = {};
        imageFormatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        PNextChainBuilder imageFormatPropsChain(&imageFormatProps);

        VkExternalImageFormatProperties externalImageFormatProps = {};
        imageFormatPropsChain.Add(&externalImageFormatProps,
                                  VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES);

        DAWN_TRY_CONTEXT(CheckVkSuccess(device->fn.GetPhysicalDeviceImageFormatProperties2(
                                            vkPhysicalDevice, &imageFormatInfo, &imageFormatProps),
                                        "vkGetPhysicalDeviceImageFormatProperties"),
                         "checking import support for fd import of dma buf with %s %s\n",
                         properties.format, properties.usage);

        VkExternalMemoryFeatureFlags featureFlags =
            externalImageFormatProps.externalMemoryProperties.externalMemoryFeatures;
        DAWN_INVALID_IF(!(featureFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT),
                        "Vulkan memory is not importable.");
    }

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory =
        AcquireRef(new SharedTextureMemory(device, label, properties));
    sharedTextureMemory->Initialize();
    sharedTextureMemory->mQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;

    // Create the VkImage for the import.
    {
        VkImageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.flags = imageFormatInfo.flags;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.format = vkFormat;
        createInfo.extent = {properties.size.width, properties.size.height,
                             properties.size.depthOrArrayLayers};
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        createInfo.usage = vkUsageFlags;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        PNextChainBuilder createInfoChain(&createInfo);

        createInfoChain.Add(&imageFormatListInfo, VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);

        VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
        externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
        createInfoChain.Add(&externalMemoryImageCreateInfo,
                            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

        std::array<VkSubresourceLayout, kMaxPlanesPerFormat> planeLayouts;
        for (uint32_t plane = 0u; plane < memoryPlaneCount; ++plane) {
            planeLayouts[plane].offset = descriptor->planes[plane].offset;
            planeLayouts[plane].size = 0;  // VK_EXT_image_drm_format_modifier mandates size = 0.
            planeLayouts[plane].rowPitch = descriptor->planes[plane].stride;
            planeLayouts[plane].arrayPitch = 0;  // Not an array texture
            planeLayouts[plane].depthPitch = 0;  // Not a depth texture
        }

        VkImageDrmFormatModifierExplicitCreateInfoEXT explicitCreateInfo = {};
        explicitCreateInfo.drmFormatModifier = descriptor->drmModifier;
        explicitCreateInfo.drmFormatModifierPlaneCount = memoryPlaneCount;
        explicitCreateInfo.pPlaneLayouts = planeLayouts.data();

        createInfoChain.Add(&explicitCreateInfo,
                            VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);

        // Create the VkImage.
        VkImage vkImage;
        DAWN_TRY(CheckVkSuccess(device->fn.CreateImage(vkDevice, &createInfo, nullptr, &*vkImage),
                                "vkCreateImage"));
        sharedTextureMemory->mVkImage =
            AcquireRef(new RefCountedVkHandle<VkImage>(device, vkImage));
    }

    // Import the memory plane(s) as VkDeviceMemory and bind to the VkImage.
    if (memoryPlaneCount > 1u) {
        // TODO(crbug.com/dawn/1548): Disjoint planar import not supported yet.
        DAWN_UNREACHABLE();
    } else {
        VkMemoryFdPropertiesKHR fdProperties;
        fdProperties.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
        fdProperties.pNext = nullptr;

        // Get the valid memory types that the external memory can be imported as.
        DAWN_TRY(CheckVkSuccess(device->fn.GetMemoryFdPropertiesKHR(
                                    vkDevice, VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
                                    descriptor->planes[0].fd, &fdProperties),
                                "vkGetMemoryFdPropertiesKHR"));

        // Get the valid memory types for the VkImage.
        VkMemoryRequirements memoryRequirements;
        device->fn.GetImageMemoryRequirements(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                              &memoryRequirements);

        // Choose the best memory type that satisfies both the image's constraint and the
        // import's constraint.
        memoryRequirements.memoryTypeBits &= fdProperties.memoryTypeBits;
        int memoryTypeIndex = device->GetResourceMemoryAllocator()->FindBestTypeIndex(
            memoryRequirements, MemoryKind::Opaque);
        DAWN_INVALID_IF(memoryTypeIndex == -1,
                        "Unable to find an appropriate memory type for import.");

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        PNextChainBuilder memoryAllocateInfoChain(&memoryAllocateInfo);

        VkImportMemoryFdInfoKHR importMemoryFdInfo;
        importMemoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

        SystemHandle memoryHandle;
        DAWN_TRY_ASSIGN(memoryHandle, SystemHandle::Duplicate(descriptor->planes[0].fd));
        importMemoryFdInfo.fd = memoryHandle.Get();
        memoryAllocateInfoChain.Add(&importMemoryFdInfo,
                                    VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR);

        // Import the fd as VkDeviceMemory
        VkDeviceMemory vkDeviceMemory;
        DAWN_TRY(CheckVkSuccess(
            device->fn.AllocateMemory(vkDevice, &memoryAllocateInfo, nullptr, &*vkDeviceMemory),
            "vkAllocateMemory"));

        memoryHandle.Detach();  // Ownership transfered to the VkDeviceMemory.
        sharedTextureMemory->mVkDeviceMemory =
            AcquireRef(new RefCountedVkHandle<VkDeviceMemory>(device, vkDeviceMemory));

        // Bind the VkImage to the memory.
        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                       sharedTextureMemory->mVkDeviceMemory->Get(), 0),
            "vkBindImageMemory"));
    }
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(LINUX)
}

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         const char* label,
                                         const SharedTextureMemoryProperties& properties)
    : SharedTextureMemoryBase(device, label, properties) {}

RefCountedVkHandle<VkDeviceMemory>* SharedTextureMemory::GetVkDeviceMemory() const {
    return mVkDeviceMemory.Get();
}

RefCountedVkHandle<VkImage>* SharedTextureMemory::GetVkImage() const {
    return mVkImage.Get();
}

uint32_t SharedTextureMemory::GetQueueFamilyIndex() const {
    return mQueueFamilyIndex;
}

void SharedTextureMemory::DestroyImpl() {
    mVkImage = nullptr;
    mVkDeviceMemory = nullptr;
}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const TextureDescriptor* descriptor) {
    return Texture::CreateFromSharedTextureMemory(this, descriptor);
}

MaybeError SharedTextureMemory::BeginAccessImpl(TextureBase* texture,
                                                const BeginAccessDescriptor* descriptor) {
    DAWN_TRY(ValidateSTypes(descriptor->nextInChain,
                            {{wgpu::SType::SharedTextureMemoryVkImageLayoutBeginState}}));

    const SharedTextureMemoryVkImageLayoutBeginState* vkLayoutBeginState = nullptr;
    FindInChain(descriptor->nextInChain, &vkLayoutBeginState);
    DAWN_INVALID_IF(vkLayoutBeginState == nullptr,
                    "SharedTextureMemoryVkImageLayoutBeginState was not provided.");

    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        // All fences are backed by binary semaphores.
        DAWN_INVALID_IF(descriptor->signaledValues[i] != 1, "%s signaled value (%u) was not 1.",
                        descriptor->fences[i], descriptor->signaledValues[i]);
    }
    ToBackend(texture)->SetPendingAcquire(
        static_cast<VkImageLayout>(vkLayoutBeginState->oldLayout),
        static_cast<VkImageLayout>(vkLayoutBeginState->newLayout));
    return {};
}

#if DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)
ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(TextureBase* texture,
                                                                      EndAccessState* state) {
    DAWN_TRY(ValidateSTypes(state->nextInChain,
                            {{wgpu::SType::SharedTextureMemoryVkImageLayoutEndState}}));

    SharedTextureMemoryVkImageLayoutEndState* vkLayoutEndState = nullptr;
    FindInChain(state->nextInChain, &vkLayoutEndState);
    DAWN_INVALID_IF(vkLayoutEndState == nullptr,
                    "SharedTextureMemoryVkImageLayoutEndState was not provided.");

#if DAWN_PLATFORM_IS(FUCHSIA)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreZirconHandle),
                    "Required feature (%s) for %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreZirconHandle,
                    wgpu::SharedFenceType::VkSemaphoreZirconHandle);
#elif DAWN_PLATFORM_IS(LINUX)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreSyncFD) &&
                        !GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreOpaqueFD),
                    "Required feature (%s or %s) for %s or %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD,
                    wgpu::FeatureName::SharedFenceVkSemaphoreSyncFD,
                    wgpu::SharedFenceType::VkSemaphoreOpaqueFD,
                    wgpu::SharedFenceType::VkSemaphoreSyncFD);
#endif

    SystemHandle handle;
    {
        ExternalSemaphoreHandle semaphoreHandle;
        VkImageLayout releasedOldLayout;
        VkImageLayout releasedNewLayout;
        DAWN_TRY(ToBackend(texture)->EndAccess(&semaphoreHandle, &releasedOldLayout,
                                               &releasedNewLayout));
        // Handle is acquired from the texture so we need to make sure to close it.
        // TODO(dawn:1745): Consider using one event per submit that is tracked by the
        // CommandRecordingContext so that we don't need to create one handle per texture,
        // and so we don't need to acquire it here to close it.
        handle = SystemHandle::Acquire(semaphoreHandle);
        vkLayoutEndState->oldLayout = releasedOldLayout;
        vkLayoutEndState->newLayout = releasedNewLayout;
    }

    Ref<SharedFence> fence;

#if DAWN_PLATFORM_IS(FUCHSIA)
    SharedFenceVkSemaphoreZirconHandleDescriptor desc;
    desc.handle = handle.Get();

    DAWN_TRY_ASSIGN(fence,
                    SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
#elif DAWN_PLATFORM_IS(LINUX)
    if (GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreSyncFD)) {
        SharedFenceVkSemaphoreSyncFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    } else {
        SharedFenceVkSemaphoreOpaqueFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    }
#endif
    // All semaphores are binary semaphores.
    return FenceAndSignalValue{std::move(fence), 1};
}

#else  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(TextureBase* texture,
                                                                      EndAccessState* state) {
    return DAWN_VALIDATION_ERROR("No shared fence features supported.");
}

#endif  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

}  // namespace dawn::native::vulkan
