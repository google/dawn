// Copyright 2017 The Dawn Authors
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

#include "dawn_native/vulkan/BufferVk.h"

#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/ResourceHeapVk.h"
#include "dawn_native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn_native/vulkan/VulkanError.h"

#include <cstring>

namespace dawn_native { namespace vulkan {

    namespace {

        VkBufferUsageFlags VulkanBufferUsage(wgpu::BufferUsage usage) {
            VkBufferUsageFlags flags = 0;

            if (usage & wgpu::BufferUsage::CopySrc) {
                flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            if (usage & wgpu::BufferUsage::CopyDst) {
                flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if (usage & wgpu::BufferUsage::Index) {
                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::Vertex) {
                flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::Uniform) {
                flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::Storage) {
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            }

            return flags;
        }

        VkPipelineStageFlags VulkanPipelineStage(wgpu::BufferUsage usage) {
            VkPipelineStageFlags flags = 0;

            if (usage & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) {
                flags |= VK_PIPELINE_STAGE_HOST_BIT;
            }
            if (usage & (wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst)) {
                flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            if (usage & (wgpu::BufferUsage::Index | wgpu::BufferUsage::Vertex)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            }
            if (usage &
                (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | kReadOnlyStorage)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
            }

            return flags;
        }

        VkAccessFlags VulkanAccessFlags(wgpu::BufferUsage usage) {
            VkAccessFlags flags = 0;

            if (usage & wgpu::BufferUsage::MapRead) {
                flags |= VK_ACCESS_HOST_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::MapWrite) {
                flags |= VK_ACCESS_HOST_WRITE_BIT;
            }
            if (usage & wgpu::BufferUsage::CopySrc) {
                flags |= VK_ACCESS_TRANSFER_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::CopyDst) {
                flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            if (usage & wgpu::BufferUsage::Index) {
                flags |= VK_ACCESS_INDEX_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::Vertex) {
                flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::Uniform) {
                flags |= VK_ACCESS_UNIFORM_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::Storage) {
                flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            }

            return flags;
        }

    }  // namespace

    // static
    ResultOrError<Buffer*> Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
        std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(device, descriptor);
        DAWN_TRY(buffer->Initialize());
        return buffer.release();
    }

    MaybeError Buffer::Initialize() {
        // Avoid passing ludicrously large sizes to drivers because it causes issues: drivers add
        // some constants to the size passed and align it, but for values close to the maximum
        // VkDeviceSize this can cause overflows and makes drivers crash or return bad sizes in the
        // VkmemoryRequirements. See https://gitlab.khronos.org/vulkan/vulkan/issues/1904
        // Any size with one of two top bits of VkDeviceSize set is a HUGE allocation and we can
        // safely return an OOM error.
        if (GetSize() & (uint64_t(3) << uint64_t(62))) {
            return DAWN_OUT_OF_MEMORY_ERROR("Buffer size is HUGE and could cause overflows");
        }

        VkBufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.size = GetSize();
        // Add CopyDst for non-mappable buffer initialization in CreateBufferMapped
        // and robust resource initialization.
        createInfo.usage = VulkanBufferUsage(GetUsage() | wgpu::BufferUsage::CopyDst);
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;

        Device* device = ToBackend(GetDevice());
        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
            "vkCreateBuffer"));

        VkMemoryRequirements requirements;
        device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

        bool requestMappable =
            (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) != 0;
        DAWN_TRY_ASSIGN(mMemoryAllocation, device->AllocateMemory(requirements, requestMappable));

        DAWN_TRY(CheckVkSuccess(
            device->fn.BindBufferMemory(device->GetVkDevice(), mHandle,
                                        ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory(),
                                        mMemoryAllocation.GetOffset()),
            "vkBindBufferMemory"));

        return {};
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    void Buffer::OnMapReadCommandSerialFinished(uint32_t mapSerial, const void* data) {
        CallMapReadCallback(mapSerial, WGPUBufferMapAsyncStatus_Success, data, GetSize());
    }

    void Buffer::OnMapWriteCommandSerialFinished(uint32_t mapSerial, void* data) {
        CallMapWriteCallback(mapSerial, WGPUBufferMapAsyncStatus_Success, data, GetSize());
    }

    VkBuffer Buffer::GetHandle() const {
        return mHandle;
    }

    void Buffer::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                    wgpu::BufferUsage usage) {
        bool lastIncludesTarget = (mLastUsage & usage) == usage;
        bool lastReadOnly = (mLastUsage & kReadOnlyBufferUsages) == mLastUsage;

        // We can skip transitions to already current read-only usages.
        if (lastIncludesTarget && lastReadOnly) {
            return;
        }

        // Special-case for the initial transition: Vulkan doesn't allow access flags to be 0.
        if (mLastUsage == wgpu::BufferUsage::None) {
            mLastUsage = usage;
            return;
        }

        VkPipelineStageFlags srcStages = VulkanPipelineStage(mLastUsage);
        VkPipelineStageFlags dstStages = VulkanPipelineStage(usage);

        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VulkanAccessFlags(mLastUsage);
        barrier.dstAccessMask = VulkanAccessFlags(usage);
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        barrier.buffer = mHandle;
        barrier.offset = 0;
        barrier.size = GetSize();

        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                    nullptr, 1, &barrier, 0, nullptr);

        mLastUsage = usage;
    }

    bool Buffer::IsMapWritable() const {
        // TODO(enga): Handle CPU-visible memory on UMA
        return mMemoryAllocation.GetMappedPointer() != nullptr;
    }

    MaybeError Buffer::MapAtCreationImpl(uint8_t** mappedPointer) {
        *mappedPointer = mMemoryAllocation.GetMappedPointer();
        return {};
    }

    MaybeError Buffer::MapReadAsyncImpl(uint32_t serial) {
        Device* device = ToBackend(GetDevice());

        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapRead);

        uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);

        MapRequestTracker* tracker = device->GetMapRequestTracker();
        tracker->Track(this, serial, memory, false);
        return {};
    }

    MaybeError Buffer::MapWriteAsyncImpl(uint32_t serial) {
        Device* device = ToBackend(GetDevice());

        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapWrite);

        uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);

        MapRequestTracker* tracker = device->GetMapRequestTracker();
        tracker->Track(this, serial, memory, true);
        return {};
    }

    void Buffer::UnmapImpl() {
        // No need to do anything, we keep CPU-visible memory mapped at all time.
    }

    void Buffer::DestroyImpl() {
        ToBackend(GetDevice())->DeallocateMemory(&mMemoryAllocation);

        if (mHandle != VK_NULL_HANDLE) {
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    // MapRequestTracker

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

        mInflightRequests.Enqueue(std::move(request), mDevice->GetPendingCommandSerial());
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

}}  // namespace dawn_native::vulkan
