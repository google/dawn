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
            device->fn.DestroyBuffer(device->GetVkDevice(), mHandle, nullptr);
            mHandle = VK_NULL_HANDLE;
        }
    }

    void Buffer::OnMapReadCommandSerialFinished(uint32_t mapSerial, const void* data) {
        CallMapReadCallback(mapSerial, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, data);
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        BufferUploader* uploader = ToBackend(GetDevice())->GetBufferUploader();
        uploader->BufferSubData(mHandle, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t /*count*/) {
        const uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);

        MapReadRequestTracker* tracker = ToBackend(GetDevice())->GetMapReadRequestTracker();
        tracker->Track(this, serial, memory + start);
    }

    void Buffer::UnmapImpl() {
        // No need to do anything, we keep CPU-visible memory mapped at all time.
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit, nxt::BufferUsageBit) {
    }

    MapReadRequestTracker::MapReadRequestTracker(Device* device) : mDevice(device) {
    }

    MapReadRequestTracker::~MapReadRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapReadRequestTracker::Track(Buffer* buffer, uint32_t mapSerial, const void* data) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.data = data;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetSerial());
    }

    void MapReadRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapReadCommandSerialFinished(request.mapSerial, request.data);
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }

}}  // namespace backend::vulkan
