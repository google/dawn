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

#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/VulkanError.h"
#include "dawn_native/vulkan/external_memory/MemoryService.h"

namespace dawn_native { namespace vulkan { namespace external_memory {

    Service::Service(Device* device) : mDevice(device) {
        const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();
        mSupportedFirstPass = info.externalMemory && info.externalMemoryFD;
    }

    Service::~Service() = default;

    bool Service::Supported() {
        // TODO(idanr): Query device here for additional support information, decide if supported
        // This will likely be done through vkGetPhysicalDeviceImageFormatProperties2KHR, where
        // we give it the intended image properties and handle type and see if it's supported
        return mSupportedFirstPass;
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
