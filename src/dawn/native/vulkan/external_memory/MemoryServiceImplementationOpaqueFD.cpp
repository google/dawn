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
    return deviceInfo.HasExt(DeviceExt::ExternalMemoryFD);
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

    VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo;
    externalFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR;
    externalFormatInfo.pNext = nullptr;
    externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkPhysicalDeviceImageFormatInfo2 formatInfo;
    formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
    formatInfo.pNext = &externalFormatInfo;
    formatInfo.format = format;
    formatInfo.type = type;
    formatInfo.tiling = tiling;
    formatInfo.usage = usage;
    formatInfo.flags = flags;

    VkExternalImageFormatProperties externalFormatProperties;
    externalFormatProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR;
    externalFormatProperties.pNext = nullptr;

    VkImageFormatProperties2 formatProperties;
    formatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
    formatProperties.pNext = &externalFormatProperties;

    VkResult result = VkResult::WrapUnsafe(mDevice->fn.GetPhysicalDeviceImageFormatProperties2(
        ToBackend(mDevice->GetAdapter())->GetPhysicalDevice(), &formatInfo, &formatProperties));

    // If handle not supported, result == VK_ERROR_FORMAT_NOT_SUPPORTED
    if (result != VK_SUCCESS) {
        return false;
    }

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
    DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::OpaqueFD,
                    "ExternalImageDescriptor is not an OpaqueFD descriptor.");

    const ExternalImageDescriptorOpaqueFD* opaqueFDDescriptor =
        static_cast<const ExternalImageDescriptorOpaqueFD*>(descriptor);

    MemoryImportParams params;
    params.allocationSize = opaqueFDDescriptor->allocationSize;
    params.memoryTypeIndex = opaqueFDDescriptor->memoryTypeIndex;
    params.dedicatedAllocation = RequiresDedicatedAllocation(opaqueFDDescriptor, image);
    return params;
}

uint32_t Service::GetQueueFamilyIndex() {
    return VK_QUEUE_FAMILY_EXTERNAL_KHR;
}

ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalMemoryHandle handle,
                                                    const MemoryImportParams& importParams,
                                                    VkImage image) {
    DAWN_INVALID_IF(handle < 0, "Importing memory with an invalid handle.");

    VkMemoryRequirements requirements;
    mDevice->fn.GetImageMemoryRequirements(mDevice->GetVkDevice(), image, &requirements);
    DAWN_INVALID_IF(requirements.size > importParams.allocationSize,
                    "Requested allocation size (%u) is smaller than the image requires (%u).",
                    importParams.allocationSize, requirements.size);

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = importParams.allocationSize;
    allocateInfo.memoryTypeIndex = importParams.memoryTypeIndex;
    PNextChainBuilder allocateInfoChain(&allocateInfo);

    VkImportMemoryFdInfoKHR importMemoryFdInfo;
    importMemoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    importMemoryFdInfo.fd = handle;
    allocateInfoChain.Add(&importMemoryFdInfo, VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR);

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

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo;
    externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext = nullptr;
    externalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    PNextChainBuilder createInfoChain(&createInfo);
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
