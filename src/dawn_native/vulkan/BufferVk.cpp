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

#include "dawn_native/CommandBuffer.h"
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
            if (usage & (wgpu::BufferUsage::Storage | kReadOnlyStorageBuffer)) {
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            }
            if (usage & wgpu::BufferUsage::QueryResolve) {
                // VK_BUFFER_USAGE_TRANSFER_DST_BIT is required by vkCmdCopyQueryPoolResults
                // but we also add VK_BUFFER_USAGE_STORAGE_BUFFER_BIT because the queries will
                // be post-processed by a compute shader and written to this buffer.
                flags |= (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
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
            if (usage & (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage |
                         kReadOnlyStorageBuffer)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
            }
            if (usage & wgpu::BufferUsage::QueryResolve) {
                flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
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
            if (usage & kReadOnlyStorageBuffer) {
                flags |= VK_ACCESS_SHADER_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::Indirect) {
                flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
            }
            if (usage & wgpu::BufferUsage::QueryResolve) {
                flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            }

            return flags;
        }

    }  // namespace

    // static
    ResultOrError<Ref<Buffer>> Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
        Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));
        DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
        return std::move(buffer);
    }

    MaybeError Buffer::Initialize(bool mappedAtCreation) {
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
        // TODO(cwallez@chromium.org): Have a global "zero" buffer that can do everything instead
        // of creating a new 4-byte buffer?
        createInfo.size = std::max(GetSize(), uint64_t(4u));
        // Add CopyDst for non-mappable buffer initialization with mappedAtCreation
        // and robust resource initialization.
        createInfo.usage = VulkanBufferUsage(GetUsage() | wgpu::BufferUsage::CopyDst);
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;

        Device* device = ToBackend(GetDevice());
        DAWN_TRY(CheckVkOOMThenSuccess(
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

        // The buffers with mappedAtCreation == true will be initialized in
        // BufferBase::MapAtCreation().
        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
            !mappedAtCreation) {
            ClearBuffer(device->GetPendingRecordingContext(), 0x01010101);
        }

        return {};
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    VkBuffer Buffer::GetHandle() const {
        return mHandle;
    }

    void Buffer::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                    wgpu::BufferUsage usage) {
        VkBufferMemoryBarrier barrier;
        VkPipelineStageFlags srcStages = 0;
        VkPipelineStageFlags dstStages = 0;

        if (TransitionUsageAndGetResourceBarrier(usage, &barrier, &srcStages, &dstStages)) {
            ASSERT(srcStages != 0 && dstStages != 0);
            ToBackend(GetDevice())
                ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                        nullptr, 1u, &barrier, 0, nullptr);
        }
    }

    bool Buffer::TransitionUsageAndGetResourceBarrier(wgpu::BufferUsage usage,
                                                      VkBufferMemoryBarrier* barrier,
                                                      VkPipelineStageFlags* srcStages,
                                                      VkPipelineStageFlags* dstStages) {
        bool lastIncludesTarget = IsSubset(usage, mLastUsage);
        bool lastReadOnly = IsSubset(mLastUsage, kReadOnlyBufferUsages);

        // We can skip transitions to already current read-only usages.
        if (lastIncludesTarget && lastReadOnly) {
            return false;
        }

        // Special-case for the initial transition: Vulkan doesn't allow access flags to be 0.
        if (mLastUsage == wgpu::BufferUsage::None) {
            mLastUsage = usage;
            return false;
        }

        *srcStages |= VulkanPipelineStage(mLastUsage);
        *dstStages |= VulkanPipelineStage(usage);

        barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier->pNext = nullptr;
        barrier->srcAccessMask = VulkanAccessFlags(mLastUsage);
        barrier->dstAccessMask = VulkanAccessFlags(usage);
        barrier->srcQueueFamilyIndex = 0;
        barrier->dstQueueFamilyIndex = 0;
        barrier->buffer = mHandle;
        barrier->offset = 0;
        barrier->size = GetSize();

        mLastUsage = usage;

        return true;
    }

    bool Buffer::IsCPUWritableAtCreation() const {
        // TODO(enga): Handle CPU-visible memory on UMA
        return mMemoryAllocation.GetMappedPointer() != nullptr;
    }

    MaybeError Buffer::MapAtCreationImpl() {
        return {};
    }

    MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
        Device* device = ToBackend(GetDevice());

        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();

        // TODO(jiawei.shao@intel.com): initialize mapped buffer in CPU side.
        EnsureDataInitialized(recordingContext);

        if (mode & wgpu::MapMode::Read) {
            TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapRead);
        } else {
            ASSERT(mode & wgpu::MapMode::Write);
            TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapWrite);
        }
        return {};
    }

    void Buffer::UnmapImpl() {
        // No need to do anything, we keep CPU-visible memory mapped at all time.
    }

    void* Buffer::GetMappedPointerImpl() {
        uint8_t* memory = mMemoryAllocation.GetMappedPointer();
        ASSERT(memory != nullptr);
        return memory;
    }

    void Buffer::DestroyImpl() {
        ToBackend(GetDevice())->DeallocateMemory(&mMemoryAllocation);

        if (mHandle != VK_NULL_HANDLE) {
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    void Buffer::EnsureDataInitialized(CommandRecordingContext* recordingContext) {
        if (IsDataInitialized() ||
            !GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }

        InitializeToZero(recordingContext);
    }

    void Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                    uint64_t offset,
                                                    uint64_t size) {
        if (IsDataInitialized() ||
            !GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }

        if (IsFullBufferRange(offset, size)) {
            SetIsDataInitialized();
        } else {
            InitializeToZero(recordingContext);
        }
    }

    void Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                    const CopyTextureToBufferCmd* copy) {
        if (IsDataInitialized() ||
            !GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }

        if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
            SetIsDataInitialized();
        } else {
            InitializeToZero(recordingContext);
        }
    }

    void Buffer::InitializeToZero(CommandRecordingContext* recordingContext) {
        ASSERT(GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse));
        ASSERT(!IsDataInitialized());

        ClearBuffer(recordingContext, 0u);
        GetDevice()->IncrementLazyClearCountForTesting();
        SetIsDataInitialized();
    }

    void Buffer::ClearBuffer(CommandRecordingContext* recordingContext, uint32_t clearValue) {
        ASSERT(recordingContext != nullptr);

        // Vulkan validation layer doesn't allow the `size` in vkCmdFillBuffer() to be 0.
        if (GetSize() == 0u) {
            return;
        }

        TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

        Device* device = ToBackend(GetDevice());
        // TODO(jiawei.shao@intel.com): find out why VK_WHOLE_SIZE doesn't work on old Windows Intel
        // Vulkan drivers.
        device->fn.CmdFillBuffer(recordingContext->commandBuffer, mHandle, 0, GetSize(),
                                 clearValue);
    }
}}  // namespace dawn_native::vulkan
