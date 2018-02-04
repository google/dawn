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

#include "backend/vulkan/BufferVk.h"

#include "backend/vulkan/BufferUploader.h"
#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/VulkanBackend.h"

#include <cstring>

namespace backend { namespace vulkan {

    namespace {

        VkBufferUsageFlags VulkanBufferUsage(nxt::BufferUsageBit usage) {
            VkBufferUsageFlags flags = 0;

            if (usage & nxt::BufferUsageBit::TransferSrc) {
                flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            if (usage & nxt::BufferUsageBit::TransferDst) {
                flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if (usage & nxt::BufferUsageBit::Index) {
                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if (usage & nxt::BufferUsageBit::Vertex) {
                flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if (usage & nxt::BufferUsageBit::Uniform) {
                flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            if (usage & nxt::BufferUsageBit::Storage) {
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }

            return flags;
        }

        VkPipelineStageFlags VulkanPipelineStage(nxt::BufferUsageBit usage) {
            VkPipelineStageFlags flags = 0;

            if (usage & (nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::MapWrite)) {
                flags |= VK_PIPELINE_STAGE_HOST_BIT;
            }
            if (usage & (nxt::BufferUsageBit::TransferSrc | nxt::BufferUsageBit::TransferDst)) {
                flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            if (usage & (nxt::BufferUsageBit::Index | nxt::BufferUsageBit::Vertex)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            }
            if (usage & (nxt::BufferUsageBit::Uniform | nxt::BufferUsageBit::Storage)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            return flags;
        }

        VkAccessFlags VulkanAccessFlags(nxt::BufferUsageBit usage) {
            VkAccessFlags flags = 0;

            if (usage & nxt::BufferUsageBit::MapRead) {
                flags |= VK_ACCESS_HOST_READ_BIT;
            }
            if (usage & nxt::BufferUsageBit::MapWrite) {
                flags |= VK_ACCESS_HOST_WRITE_BIT;
            }
            if (usage & nxt::BufferUsageBit::TransferSrc) {
                flags |= VK_ACCESS_TRANSFER_READ_BIT;
            }
            if (usage & nxt::BufferUsageBit::TransferDst) {
                flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            if (usage & nxt::BufferUsageBit::Index) {
                flags |= VK_ACCESS_INDEX_READ_BIT;
            }
            if (usage & nxt::BufferUsageBit::Vertex) {
                flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            }
            if (usage & nxt::BufferUsageBit::Uniform) {
                flags |= VK_ACCESS_UNIFORM_READ_BIT;
            }
            if (usage & nxt::BufferUsageBit::Storage) {
                flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            }

            return flags;
        }

    }  // namespace

    Buffer::Buffer(BufferBuilder* builder) : BufferBase(builder) {
        Device* device = ToBackend(GetDevice());

        VkBufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = GetSize();
        createInfo.usage = VulkanBufferUsage(GetAllowedUsage());
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;

        if (device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &mHandle) !=
            VK_SUCCESS) {
            ASSERT(false);
        }

        VkMemoryRequirements requirements;
        device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

        bool requestMappable =
            (GetAllowedUsage() & (nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::MapWrite)) !=
            0;
        if (!device->GetMemoryAllocator()->Allocate(requirements, requestMappable,
                                                    &mMemoryAllocation)) {
            ASSERT(false);
        }

        if (device->fn.BindBufferMemory(device->GetVkDevice(), mHandle,
                                        mMemoryAllocation.GetMemory(),
                                        mMemoryAllocation.GetMemoryOffset()) != VK_SUCCESS) {
            ASSERT(false);
        }
    }

    Buffer::~Buffer() {
        Device* device = ToBackend(GetDevice());

        device->GetMemoryAllocator()->Free(&mMemoryAllocation);

        if (mHandle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    void Buffer::OnMapReadCommandSerialFinished(uint32_t mapSerial, const void* data) {
        CallMapReadCallback(mapSerial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data);
    }

    void Buffer::OnMapWriteCommandSerialFinished(uint32_t mapSerial, void* data) {
        CallMapWriteCallback(mapSerial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data);
    }

    VkBuffer Buffer::GetHandle() const {
        return mHandle;
    }

    void Buffer::RecordBarrier(VkCommandBuffer commands,
                               nxt::BufferUsageBit currentUsage,
                               nxt::BufferUsageBit targetUsage) const {
        VkPipelineStageFlags srcStages = VulkanPipelineStage(currentUsage);
        VkPipelineStageFlags dstStages = VulkanPipelineStage(targetUsage);

        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VulkanAccessFlags(currentUsage);
        barrier.dstAccessMask = VulkanAccessFlags(targetUsage);
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        barrier.buffer = mHandle;
        barrier.offset = 0;
        barrier.size = GetSize();

        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(commands, srcStages, dstStages, 0, 0, nullptr, 1, &barrier, 0,
                                    nullptr);
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) {
        BufferUploader* uploader = ToBackend(GetDevice())->GetBufferUploader();
        uploader->BufferSubData(mHandle, start, count, data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t /*count*/) {
        uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);

        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapRequestTracker();
        tracker->Track(this, serial, memory + start, false);
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t /*count*/) {
        uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);

        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapRequestTracker();
        tracker->Track(this, serial, memory + start, true);
    }

    void Buffer::UnmapImpl() {
        // No need to do anything, we keep CPU-visible memory mapped at all time.
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit currentUsage,
                                     nxt::BufferUsageBit targetUsage) {
        VkCommandBuffer commands = ToBackend(GetDevice())->GetPendingCommandBuffer();
        RecordBarrier(commands, currentUsage, targetUsage);
    }

    MapRequestTracker::MapRequestTracker(Device* device) : mDevice(device) {
    }

    MapRequestTracker::~MapRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapRequestTracker::Track(Buffer* buffer, uint32_t mapSerial, void* data, bool isWrite) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.data = data;
        request.isWrite = isWrite;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetSerial());
    }

    void MapRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            if (request.isWrite) {
                request.buffer->OnMapWriteCommandSerialFinished(request.mapSerial, request.data);
            } else {
                request.buffer->OnMapReadCommandSerialFinished(request.mapSerial, request.data);
            }
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }

}}  // namespace backend::vulkan
