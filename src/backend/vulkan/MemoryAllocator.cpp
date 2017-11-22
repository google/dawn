// Copyright 2017 The NXT Authors
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

#include "backend/vulkan/MemoryAllocator.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend {
namespace vulkan {

    DeviceMemoryAllocation::~DeviceMemoryAllocation() {
        ASSERT(memory == VK_NULL_HANDLE);
    }

    VkDeviceMemory DeviceMemoryAllocation::GetMemory() const {
        return memory;
    }

    size_t DeviceMemoryAllocation::GetMemoryOffset() const {
        return offset;
    }

    uint8_t* DeviceMemoryAllocation::GetMappedPointer() const {
        return mappedPointer;
    }

    MemoryAllocator::MemoryAllocator(Device* device)
        :device(device) {
    }

    MemoryAllocator::~MemoryAllocator() {
        ASSERT(releasedMemory.Empty());
    }

    bool MemoryAllocator::Allocate(VkMemoryRequirements requirements, bool mappable, DeviceMemoryAllocation* allocation) {
        const VulkanDeviceInfo& info = device->GetDeviceInfo();

        // Find a suitable memory type for this allocation
        int bestType = -1;
        for (size_t i = 0; i < info.memoryTypes.size(); ++i) {
            // Resource must support this memory type
            if ((requirements.memoryTypeBits & (1 << i)) == 0) {
                continue;
            }

            // Mappable resource must be host visible
            if (mappable && (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
                continue;
            }

            // Found the first candidate memory type
            if (bestType == -1) {
                bestType = static_cast<int>(i);
                continue;
            }

            // For non-mappable resources, favor device local memory.
            if (!mappable) {
                if ((info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0 &&
                    (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
                    bestType = static_cast<int>(i);
                    continue;
                }
            }

            // All things equal favor the memory in the biggest heap
            VkDeviceSize bestTypeHeapSize = info.memoryHeaps[info.memoryTypes[bestType].heapIndex].size;
            VkDeviceSize candidateHeapSize = info.memoryHeaps[info.memoryTypes[bestType].heapIndex].size;
            if (candidateHeapSize > bestTypeHeapSize) {
                bestType = static_cast<int>(i);
                continue;
            }
        }

        // TODO(cwallez@chromium.org): I think the Vulkan spec guarantees this should never happen
        if (bestType == -1) {
            ASSERT(false);
            return false;
        }

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = static_cast<uint32_t>(bestType);

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
        if (device->fn.AllocateMemory(device->GetVkDevice(), &allocateInfo, nullptr, &allocatedMemory) != VK_SUCCESS) {
            return false;
        }

        void* mappedPointer = nullptr;
        if (mappable) {
            if (device->fn.MapMemory(device->GetVkDevice(), allocatedMemory, 0, requirements.size, 0,
                                     &mappedPointer) != VK_SUCCESS) {
                return false;
            }
        }

        allocation->memory = allocatedMemory;
        allocation->offset = 0;
        allocation->mappedPointer = reinterpret_cast<uint8_t*>(mappedPointer);

        return true;
    }

    void MemoryAllocator::Free(DeviceMemoryAllocation* allocation) {
        releasedMemory.Enqueue(allocation->memory, device->GetSerial());
        allocation->memory = VK_NULL_HANDLE;
        allocation->offset = 0;
        allocation->mappedPointer = nullptr;
    }

    void MemoryAllocator::Tick(Serial finishedSerial) {
        for (auto memory : releasedMemory.IterateUpTo(finishedSerial)) {
            device->fn.FreeMemory(device->GetVkDevice(), memory, nullptr);
        }
        releasedMemory.ClearUpTo(finishedSerial);
    }
}
}
