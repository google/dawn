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

#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/BackendVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/VulkanError.h"
#include "dawn_native/vulkan/external_memory/MemoryService.h"

namespace dawn_native { namespace vulkan { namespace external_memory {

    Service::Service(Device* device) : mDevice(device) {
        const VulkanDeviceInfo& deviceInfo = mDevice->GetDeviceInfo();
        const VulkanGlobalInfo& globalInfo =
            ToBackend(mDevice->GetAdapter())->GetBackend()->GetGlobalInfo();

        mSupported = globalInfo.getPhysicalDeviceProperties2 &&
                     globalInfo.externalMemoryCapabilities && deviceInfo.externalMemory &&
                     deviceInfo.externalMemoryFD;
    }

    Service::~Service() = default;

    bool Service::Supported(VkFormat format,
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

        VkResult result = mDevice->fn.GetPhysicalDeviceImageFormatProperties2KHR(
            ToBackend(mDevice->GetAdapter())->GetPhysicalDevice(), &formatInfo, &formatProperties);

        // If handle not supported, result == VK_ERROR_FORMAT_NOT_SUPPORTED
        if (result != VK_SUCCESS) {
            return false;
        }

        // TODO(http://crbug.com/dawn/206): Investigate dedicated only images
        VkFlags memoryFlags =
            externalFormatProperties.externalMemoryProperties.externalMemoryFeatures;
        return (memoryFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR) &&
               !(memoryFlags & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR);
    }

    ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalMemoryHandle handle,
                                                        VkDeviceSize allocationSize,
                                                        uint32_t memoryTypeIndex) {
        if (handle < 0) {
            return DAWN_VALIDATION_ERROR("Trying to import memory with invalid handle");
        }

        VkImportMemoryFdInfoKHR importMemoryFdInfo;
        importMemoryFdInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
        importMemoryFdInfo.pNext = nullptr;
        importMemoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
        importMemoryFdInfo.fd = handle;

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = &importMemoryFdInfo;
        allocateInfo.allocationSize = allocationSize;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                           nullptr, &allocatedMemory),
                                "vkAllocateMemory"));
        return allocatedMemory;
    }

}}}  // namespace dawn_native::vulkan::external_memory
