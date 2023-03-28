// Copyright 2022 The Dawn Authors
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

#include "dawn/common/Assert.h"
#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"

namespace dawn::native::vulkan::external_memory {

Service::Service(Device* device)
    : mDevice(device), mSupported(CheckSupport(device->GetDeviceInfo())) {}

Service::~Service() = default;

// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo) {
    return deviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer);
}

bool Service::SupportsImportMemory(VkFormat format,
                                   VkImageType type,
                                   VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkImageCreateFlags flags) {
    // Early out before we try using extension functions
    if (!mSupported) {
        return false;
    }

    VkPhysicalDeviceImageFormatInfo2 formatInfo = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR,
        .pNext = nullptr,
        .format = format,
        .type = type,
        .tiling = tiling,
        .usage = usage,
        .flags = flags,
    };

    PNextChainBuilder formatInfoChain(&formatInfo);

    VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo = {
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };
    formatInfoChain.Add(&externalFormatInfo,
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR);

    VkImageFormatProperties2 formatProperties = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR,
        .pNext = nullptr,
    };

    PNextChainBuilder formatPropertiesChain(&formatProperties);

    VkExternalImageFormatProperties externalFormatProperties;
    formatPropertiesChain.Add(&externalFormatProperties,
                              VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR);

    VkResult result = VkResult::WrapUnsafe(mDevice->fn.GetPhysicalDeviceImageFormatProperties2(
        ToBackend(mDevice->GetAdapter())->GetPhysicalDevice(), &formatInfo, &formatProperties));

    // If handle not supported, result == VK_ERROR_FORMAT_NOT_SUPPORTED
    if (result != VK_SUCCESS) {
        return false;
    }

    // TODO(http://crbug.com/dawn/206): Investigate dedicated only images
    VkFlags memoryFlags = externalFormatProperties.externalMemoryProperties.externalMemoryFeatures;
    return (memoryFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR) != 0;
}

bool Service::SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  bool* supportsDisjoint) {
    *supportsDisjoint = false;
    return mSupported;
}

ResultOrError<MemoryImportParams> Service::GetMemoryImportParams(
    const ExternalImageDescriptor* descriptor,
    VkImage image) {
    DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::AHardwareBuffer,
                    "ExternalImageDescriptor is not an AHardwareBuffer descriptor.");

    const ExternalImageDescriptorAHardwareBuffer* aHardwareBufferDescriptor =
        static_cast<const ExternalImageDescriptorAHardwareBuffer*>(descriptor);

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
        .pNext = nullptr,
    };

    PNextChainBuilder bufferPropertiesChain(&bufferProperties);

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.GetAndroidHardwareBufferPropertiesANDROID(
            mDevice->GetVkDevice(), aHardwareBufferDescriptor->handle, &bufferProperties),
        "vkGetAndroidHardwareBufferPropertiesANDROID"));

    MemoryImportParams params;
    params.allocationSize = bufferProperties.allocationSize;
    params.memoryTypeIndex = bufferProperties.memoryTypeBits;
    params.dedicatedAllocation = RequiresDedicatedAllocation(aHardwareBufferDescriptor, image);
    return params;
}

uint32_t Service::GetQueueFamilyIndex() {
    return VK_QUEUE_FAMILY_FOREIGN_EXT;
}

ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalMemoryHandle handle,
                                                    const MemoryImportParams& importParams,
                                                    VkImage image) {
    DAWN_INVALID_IF(handle == nullptr, "Importing memory with an invalid handle.");

    VkMemoryRequirements requirements;
    mDevice->fn.GetImageMemoryRequirements(mDevice->GetVkDevice(), image, &requirements);
    DAWN_INVALID_IF(requirements.size > importParams.allocationSize,
                    "Requested allocation size (%u) is smaller than the image requires (%u).",
                    importParams.allocationSize, requirements.size);

    VkMemoryAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = importParams.allocationSize,
        .memoryTypeIndex = importParams.memoryTypeIndex,
    };

    PNextChainBuilder allocateInfoChain(&allocateInfo);

    VkImportAndroidHardwareBufferInfoANDROID importMemoryAHBInfo = {
        .buffer = handle,
    };
    allocateInfoChain.Add(&importMemoryAHBInfo,
                          VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID);

    VkMemoryDedicatedAllocateInfo dedicatedAllocateInfo;
    if (importParams.dedicatedAllocation) {
        dedicatedAllocateInfo.image = image;
        dedicatedAllocateInfo.buffer = VkBuffer{};
        allocateInfoChain.Add(&dedicatedAllocateInfo,
                              VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
    }

    VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                       nullptr, &*allocatedMemory),
                            "vkAllocateMemory"));
    return allocatedMemory;
}

ResultOrError<VkImage> Service::CreateImage(const ExternalImageDescriptor* descriptor,
                                            const VkImageCreateInfo& baseCreateInfo) {
    VkImageCreateInfo createInfo = baseCreateInfo;
    createInfo.flags |= VK_IMAGE_CREATE_ALIAS_BIT_KHR;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    PNextChainBuilder createInfoChain(&createInfo);

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };
    createInfoChain.Add(&externalMemoryImageCreateInfo,
                        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

    ASSERT(IsSampleCountSupported(mDevice, createInfo));

    VkImage image;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateImage(mDevice->GetVkDevice(), &createInfo, nullptr, &*image),
        "CreateImage"));
    return image;
}

}  // namespace dawn::native::vulkan::external_memory
