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

#include "dawn_native/vulkan/ResourceMemoryAllocatorVk.h"

#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/ResourceHeapVk.h"
#include "dawn_native/vulkan/VulkanError.h"

namespace dawn_native { namespace vulkan {

    ResourceMemoryAllocator::ResourceMemoryAllocator(Device* device) : mDevice(device) {
    }

    int ResourceMemoryAllocator::FindBestTypeIndex(VkMemoryRequirements requirements,
                                                   bool mappable) {
        const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();

        // Find a suitable memory type for this allocation
        int bestType = -1;
        for (size_t i = 0; i < info.memoryTypes.size(); ++i) {
            // Resource must support this memory type
            if ((requirements.memoryTypeBits & (1 << i)) == 0) {
                continue;
            }

            // Mappable resource must be host visible
            if (mappable &&
                (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
                continue;
            }

            // Mappable must also be host coherent.
            if (mappable &&
                (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
                continue;
            }

            // Found the first candidate memory type
            if (bestType == -1) {
                bestType = static_cast<int>(i);
                continue;
            }

            // For non-mappable resources, favor device local memory.
            if (!mappable) {
                if ((info.memoryTypes[bestType].propertyFlags &
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0 &&
                    (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) !=
                        0) {
                    bestType = static_cast<int>(i);
                    continue;
                }
            }

            // All things equal favor the memory in the biggest heap
            VkDeviceSize bestTypeHeapSize =
                info.memoryHeaps[info.memoryTypes[bestType].heapIndex].size;
            VkDeviceSize candidateHeapSize = info.memoryHeaps[info.memoryTypes[i].heapIndex].size;
            if (candidateHeapSize > bestTypeHeapSize) {
                bestType = static_cast<int>(i);
                continue;
            }
        }

        return bestType;
    }

    ResultOrError<ResourceMemoryAllocation> ResourceMemoryAllocator::Allocate(
        VkMemoryRequirements requirements,
        bool mappable) {
        int bestType = FindBestTypeIndex(requirements, mappable);

        // TODO(cwallez@chromium.org): I think the Vulkan spec guarantees this should never
        // happen
        if (bestType == -1) {
            return DAWN_DEVICE_LOST_ERROR("Unable to find memory for requirements.");
        }

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = static_cast<uint32_t>(bestType);

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                           nullptr, &allocatedMemory),
                                "vkAllocateMemory"));

        void* mappedPointer = nullptr;
        if (mappable) {
            DAWN_TRY(CheckVkSuccess(mDevice->fn.MapMemory(mDevice->GetVkDevice(), allocatedMemory,
                                                          0, requirements.size, 0, &mappedPointer),
                                    "vkMapMemory"));
        }

        AllocationInfo info;
        info.mMethod = AllocationMethod::kDirect;

        return ResourceMemoryAllocation(info, /*offset*/ 0, new ResourceHeap(allocatedMemory),
                                        static_cast<uint8_t*>(mappedPointer));
    }

    void ResourceMemoryAllocator::Deallocate(ResourceMemoryAllocation& allocation) {
        mDevice->GetFencedDeleter()->DeleteWhenUnused(
            ToBackend(allocation.GetResourceHeap())->GetMemory());
    }
}}  // namespace dawn_native::vulkan
