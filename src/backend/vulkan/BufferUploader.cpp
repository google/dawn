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

#include "backend/vulkan/BufferUploader.h"

#include "backend/vulkan/MemoryAllocator.h"
#include "backend/vulkan/VulkanBackend.h"

#include <cstring>

namespace backend {
namespace vulkan {

    BufferUploader::BufferUploader(Device* device)
        : device(device) {
    }

    BufferUploader::~BufferUploader() {
        ASSERT(buffersToDelete.Empty());
    }

    void BufferUploader::BufferSubData(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, const void* data) {
        // TODO(cwallez@chromium.org): this is soooooo bad. We should use some sort of ring buffer for this.

        // Create a staging buffer
        VkBufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = size;
        createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        if (device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
            ASSERT(false);
        }

        VkMemoryRequirements requirements;
        device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), stagingBuffer, &requirements);

        DeviceMemoryAllocation allocation;
        if (!device->GetMemoryAllocator()->Allocate(requirements, true, &allocation)) {
            ASSERT(false);
        }

        if (device->fn.BindBufferMemory(device->GetVkDevice(), stagingBuffer, allocation.GetMemory(),
                                        allocation.GetMemoryOffset()) != VK_SUCCESS) {
            ASSERT(false);
        }

        // Write to the staging buffer
        ASSERT(allocation.GetMappedPointer() != nullptr);
        memcpy(allocation.GetMappedPointer(), data, size);

        // Enqueue host write -> transfer src barrier and copy command
        VkCommandBuffer commands = device->GetPendingCommandBuffer();

        VkMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        device->fn.CmdPipelineBarrier(commands,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                1, &barrier,
                0, nullptr,
                0, nullptr);

        VkBufferCopy copy;
        copy.srcOffset = 0;
        copy.dstOffset = offset;
        copy.size = size;
        device->fn.CmdCopyBuffer(commands, stagingBuffer, buffer, 1, &copy);

        // TODO(cwallez@chromium.org): Buffers must be deleted before the memory.
        // This happens to work for now, but is fragile.
        device->GetMemoryAllocator()->Free(&allocation);
        buffersToDelete.Enqueue(stagingBuffer, device->GetSerial());
    }

    void BufferUploader::Tick(Serial completedSerial) {
        for (VkBuffer buffer : buffersToDelete.IterateUpTo(completedSerial)) {
            device->fn.DestroyBuffer(device->GetVkDevice(), buffer, nullptr);
        }
        buffersToDelete.ClearUpTo(completedSerial);
    }

}
}
