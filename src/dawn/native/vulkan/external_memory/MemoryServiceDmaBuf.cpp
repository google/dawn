// Copyright 2019 The Dawn Authors
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

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"

namespace dawn::native::vulkan::external_memory {

namespace {

bool GetFormatModifierProps(const VulkanFunctions& fn,
                            VkPhysicalDevice physicalDevice,
                            VkFormat format,
                            uint64_t modifier,
                            VkDrmFormatModifierPropertiesEXT* formatModifierProps) {
    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierPropsVector;
    VkFormatProperties2 formatProps = {};
    formatProps.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    PNextChainBuilder formatPropsChain(&formatProps);

    VkDrmFormatModifierPropertiesListEXT formatModifierPropsList = {};
    formatModifierPropsList.drmFormatModifierCount = 0;
    formatModifierPropsList.pDrmFormatModifierProperties = nullptr;
    formatPropsChain.Add(&formatModifierPropsList,
                         VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT);

    fn.GetPhysicalDeviceFormatProperties2(physicalDevice, format, &formatProps);

    uint32_t modifierCount = formatModifierPropsList.drmFormatModifierCount;
    formatModifierPropsVector.resize(modifierCount);
    formatModifierPropsList.pDrmFormatModifierProperties = formatModifierPropsVector.data();

    fn.GetPhysicalDeviceFormatProperties2(physicalDevice, format, &formatProps);
    for (const auto& props : formatModifierPropsVector) {
        if (props.drmFormatModifier == modifier) {
            *formatModifierProps = props;
            return true;
        }
    }
    return false;
}

// Some modifiers use multiple planes (for example, see the comment for
// I915_FORMAT_MOD_Y_TILED_CCS in drm/drm_fourcc.h).
ResultOrError<uint32_t> GetModifierPlaneCount(const VulkanFunctions& fn,
                                              VkPhysicalDevice physicalDevice,
                                              VkFormat format,
                                              uint64_t modifier) {
    VkDrmFormatModifierPropertiesEXT props;
    if (GetFormatModifierProps(fn, physicalDevice, format, modifier, &props)) {
        return static_cast<uint32_t>(props.drmFormatModifierPlaneCount);
    }
    return DAWN_VALIDATION_ERROR("DRM format modifier not supported.");
}

bool IsMultiPlanarVkFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return true;

        default:
            return false;
    }
}

bool SupportsDisjoint(const VulkanFunctions& fn,
                      VkPhysicalDevice physicalDevice,
                      VkFormat format,
                      uint64_t modifier) {
    if (IsMultiPlanarVkFormat(format)) {
        VkDrmFormatModifierPropertiesEXT props;
        return (GetFormatModifierProps(fn, physicalDevice, format, modifier, &props) &&
                (props.drmFormatModifierTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT));
    }
    return false;
}

}  // namespace

Service::Service(Device* device)
    : mDevice(device), mSupported(CheckSupport(device->GetDeviceInfo())) {}

Service::~Service() = default;

// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo) {
    return deviceInfo.HasExt(DeviceExt::ExternalMemoryFD) &&
           deviceInfo.HasExt(DeviceExt::ImageDrmFormatModifier);
}

bool Service::SupportsImportMemory(VkFormat format,
                                   VkImageType type,
                                   VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkImageCreateFlags flags) {
    return mSupported && (!IsMultiPlanarVkFormat(format) ||
                          (format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM &&
                           mDevice->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)));
}

bool Service::SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  bool* supportsDisjoint) {
    *supportsDisjoint = false;
    // Early out before we try using extension functions
    if (!mSupported) {
        return false;
    }
    if (descriptor->GetType() != ExternalImageType::DmaBuf) {
        return false;
    }
    const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
        static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);

    // Verify plane count for the modifier.
    VkPhysicalDevice physicalDevice = ToBackend(mDevice->GetAdapter())->GetPhysicalDevice();
    uint32_t planeCount = 0;
    if (mDevice->ConsumedError(GetModifierPlaneCount(mDevice->fn, physicalDevice, format,
                                                     dmaBufDescriptor->drmModifier),
                               &planeCount)) {
        return false;
    }
    if (planeCount == 0) {
        return false;
    }
    // Only support the NV12 multi-planar format for now.
    if (planeCount > 1 && format != VK_FORMAT_G8_B8R8_2PLANE_420_UNORM) {
        return false;
    }
    *supportsDisjoint =
        SupportsDisjoint(mDevice->fn, physicalDevice, format, dmaBufDescriptor->drmModifier);

    // Verify that the format modifier of the external memory and the requested Vulkan format
    // are actually supported together in a dma-buf import.
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.format = format;
    imageFormatInfo.type = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    imageFormatInfo.usage = usage;
    imageFormatInfo.flags = 0;
    PNextChainBuilder imageFormatInfoChain(&imageFormatInfo);

    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
    externalImageFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    imageFormatInfoChain.Add(&externalImageFormatInfo,
                             VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO);

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmModifierInfo = {};
    drmModifierInfo.drmFormatModifier = dmaBufDescriptor->drmModifier;
    drmModifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageFormatInfoChain.Add(&drmModifierInfo,
                             VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT);

    // For mutable vkimage of multi-planar format, we also need to make sure the each
    // plane's view format can be supported.
    std::array<VkFormat, 2> viewFormats;
    VkImageFormatListCreateInfo imageFormatListInfo = {};

    if (planeCount > 1) {
        ASSERT(format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
        viewFormats = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM};
        imageFormatListInfo.viewFormatCount = 2;
        imageFormatListInfo.pViewFormats = viewFormats.data();
        imageFormatInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        imageFormatInfoChain.Add(&imageFormatListInfo,
                                 VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
    }

    VkImageFormatProperties2 imageFormatProps = {};
    imageFormatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    PNextChainBuilder imageFormatPropsChain(&imageFormatProps);

    VkExternalImageFormatProperties externalImageFormatProps = {};
    imageFormatPropsChain.Add(&externalImageFormatProps,
                              VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES);

    VkResult result = VkResult::WrapUnsafe(mDevice->fn.GetPhysicalDeviceImageFormatProperties2(
        physicalDevice, &imageFormatInfo, &imageFormatProps));
    if (result != VK_SUCCESS) {
        return false;
    }
    VkExternalMemoryFeatureFlags featureFlags =
        externalImageFormatProps.externalMemoryProperties.externalMemoryFeatures;
    return featureFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
}

ResultOrError<MemoryImportParams> Service::GetMemoryImportParams(
    const ExternalImageDescriptor* descriptor,
    VkImage image) {
    DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::DmaBuf,
                    "ExternalImageDescriptor is not a ExternalImageDescriptorDmaBuf.");

    const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
        static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);
    VkDevice device = mDevice->GetVkDevice();

    // Get the valid memory types for the VkImage.
    VkMemoryRequirements memoryRequirements;
    mDevice->fn.GetImageMemoryRequirements(device, image, &memoryRequirements);

    VkMemoryFdPropertiesKHR fdProperties;
    fdProperties.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
    fdProperties.pNext = nullptr;

    // Get the valid memory types that the external memory can be imported as.
    mDevice->fn.GetMemoryFdPropertiesKHR(device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
                                         dmaBufDescriptor->memoryFD, &fdProperties);
    // Choose the best memory type that satisfies both the image's constraint and the
    // import's constraint.
    memoryRequirements.memoryTypeBits &= fdProperties.memoryTypeBits;
    int memoryTypeIndex = mDevice->GetResourceMemoryAllocator()->FindBestTypeIndex(
        memoryRequirements, MemoryKind::Opaque);
    DAWN_INVALID_IF(memoryTypeIndex == -1, "Unable to find an appropriate memory type for import.");

    MemoryImportParams params;
    params.allocationSize = memoryRequirements.size;
    params.memoryTypeIndex = static_cast<uint32_t>(memoryTypeIndex);
    params.dedicatedAllocation = RequiresDedicatedAllocation(dmaBufDescriptor, image);
    return params;
}

uint32_t Service::GetQueueFamilyIndex() {
    return VK_QUEUE_FAMILY_EXTERNAL_KHR;
}

ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalMemoryHandle handle,
                                                    const MemoryImportParams& importParams,
                                                    VkImage image) {
    DAWN_INVALID_IF(handle < 0, "Importing memory with an invalid handle.");

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = importParams.allocationSize;
    memoryAllocateInfo.memoryTypeIndex = importParams.memoryTypeIndex;
    PNextChainBuilder memoryAllocateInfoChain(&memoryAllocateInfo);

    VkImportMemoryFdInfoKHR importMemoryFdInfo;
    importMemoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
    importMemoryFdInfo.fd = handle;
    memoryAllocateInfoChain.Add(&importMemoryFdInfo, VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR);

    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo;
    if (importParams.dedicatedAllocation) {
        memoryDedicatedAllocateInfo.image = image;
        memoryDedicatedAllocateInfo.buffer = VkBuffer{};
        memoryAllocateInfoChain.Add(&memoryDedicatedAllocateInfo,
                                    VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
    }

    VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &memoryAllocateInfo,
                                                       nullptr, &*allocatedMemory),
                            "vkAllocateMemory"));
    return allocatedMemory;
}

ResultOrError<VkImage> Service::CreateImage(const ExternalImageDescriptor* descriptor,
                                            const VkImageCreateInfo& baseCreateInfo) {
    DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::DmaBuf,
                    "ExternalImageDescriptor is not a dma-buf descriptor.");

    const ExternalImageDescriptorDmaBuf* dmaBufDescriptor =
        static_cast<const ExternalImageDescriptorDmaBuf*>(descriptor);
    VkPhysicalDevice physicalDevice = ToBackend(mDevice->GetAdapter())->GetPhysicalDevice();
    VkDevice device = mDevice->GetVkDevice();

    uint32_t planeCount;
    DAWN_TRY_ASSIGN(planeCount,
                    GetModifierPlaneCount(mDevice->fn, physicalDevice, baseCreateInfo.format,
                                          dmaBufDescriptor->drmModifier));

    VkImageCreateInfo createInfo = baseCreateInfo;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;

    PNextChainBuilder createInfoChain(&createInfo);

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    createInfoChain.Add(&externalMemoryImageCreateInfo,
                        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

    VkSubresourceLayout planeLayouts[ExternalImageDescriptorDmaBuf::kMaxPlanes];
    for (uint32_t plane = 0u; plane < planeCount; ++plane) {
        planeLayouts[plane].offset = dmaBufDescriptor->planeLayouts[plane].offset;
        planeLayouts[plane].size = 0;  // VK_EXT_image_drm_format_modifier mandates size = 0.
        planeLayouts[plane].rowPitch = dmaBufDescriptor->planeLayouts[plane].stride;
        planeLayouts[plane].arrayPitch = 0;  // Not an array texture
        planeLayouts[plane].depthPitch = 0;  // Not a depth texture
    }

    VkImageDrmFormatModifierExplicitCreateInfoEXT explicitCreateInfo = {};
    explicitCreateInfo.drmFormatModifier = dmaBufDescriptor->drmModifier;
    explicitCreateInfo.drmFormatModifierPlaneCount = planeCount;
    explicitCreateInfo.pPlaneLayouts = &planeLayouts[0];

    if (planeCount > 1) {
        // For multi-planar formats, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT specifies that a
        // VkImageView can be plane's format which might differ from the image's format.
        createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }
    createInfoChain.Add(&explicitCreateInfo,
                        VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);

    // Create a new VkImage with tiling equal to the DRM format modifier.
    VkImage image;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.CreateImage(device, &createInfo, nullptr, &*image),
                            "CreateImage"));
    return image;
}

}  // namespace dawn::native::vulkan::external_memory
