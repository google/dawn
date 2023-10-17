// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/BufferVk.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

constexpr wgpu::BufferUsage kMapUsages = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;

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
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer | kReadOnlyStorageBuffer)) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::Indirect) {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (usage & wgpu::BufferUsage::QueryResolve) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return flags;
}

VkPipelineStageFlags VulkanPipelineStage(wgpu::BufferUsage usage) {
    VkPipelineStageFlags flags = 0;

    if (usage & kMappableBufferUsages) {
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    }
    if (usage & (wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst)) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (usage & (wgpu::BufferUsage::Index | wgpu::BufferUsage::Vertex)) {
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (usage & (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | kInternalStorageBuffer |
                 kReadOnlyStorageBuffer)) {
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
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
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
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

    const BufferHostMappedPointer* hostMappedDesc = nullptr;
    FindInChain(descriptor->nextInChain, &hostMappedDesc);

    if (hostMappedDesc != nullptr) {
        DAWN_TRY(buffer->InitializeHostMapped(hostMappedDesc));
    } else {
        DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
    }
    return std::move(buffer);
}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
    // vkCmdFillBuffer requires the size to be a multiple of 4.
    constexpr size_t kAlignment = 4u;

    uint32_t extraBytes = 0u;
    if (GetUsage() & (wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index)) {
        // vkCmdSetIndexBuffer and vkCmdSetVertexBuffer are invalid if the offset
        // is equal to the whole buffer size. Allocate at least one more byte so it
        // is valid to setVertex/IndexBuffer with a zero-sized range at the end
        // of the buffer with (offset=buffer.size, size=0).
        extraBytes = 1u;
    }

    uint64_t size = GetSize();
    if (size > std::numeric_limits<uint64_t>::max() - extraBytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }

    size += extraBytes;

    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    // Also, Vulkan requires the size to be non-zero.
    size = std::max(size, uint64_t(4u));

    if (size > std::numeric_limits<uint64_t>::max() - kAlignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, kAlignment);

    // Avoid passing ludicrously large sizes to drivers because it causes issues: drivers add
    // some constants to the size passed and align it, but for values close to the maximum
    // VkDeviceSize this can cause overflows and makes drivers crash or return bad sizes in the
    // VkmemoryRequirements. See https://gitlab.khronos.org/vulkan/vulkan/issues/1904
    // Any size with one of two top bits of VkDeviceSize set is a HUGE allocation and we can
    // safely return an OOM error.
    if (mAllocatedSize & (uint64_t(3) << uint64_t(62))) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer size is HUGE and could cause overflows");
    }

    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = mAllocatedSize;
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

    // Gather requirements for the buffer's memory and allocate it.
    VkMemoryRequirements requirements;
    device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    MemoryKind requestKind = MemoryKind::Linear;
    if (GetUsage() & wgpu::BufferUsage::MapRead) {
        requestKind = MemoryKind::LinearReadMappable;
    } else if (GetUsage() & wgpu::BufferUsage::MapWrite) {
        requestKind = MemoryKind::LinearWriteMappable;
    }
    DAWN_TRY_ASSIGN(mMemoryAllocation,
                    device->GetResourceMemoryAllocator()->Allocate(requirements, requestKind));

    // Finally associate it with the buffer.
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

    // Initialize the padding bytes to zero.
    if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) && !mappedAtCreation) {
        uint32_t paddingBytes = GetAllocatedSize() - GetSize();
        if (paddingBytes > 0) {
            uint32_t clearSize = Align(paddingBytes, 4);
            uint64_t clearOffset = GetAllocatedSize() - clearSize;

            CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
            ClearBuffer(recordingContext, 0, clearOffset, clearSize);
        }
    }

    SetLabelImpl();

    return {};
}

MaybeError Buffer::InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc) {
    static constexpr auto kHandleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    mAllocatedSize = GetSize();

    VkExternalMemoryBufferCreateInfo externalMemoryCreateInfo;
    externalMemoryCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externalMemoryCreateInfo.pNext = nullptr;
    externalMemoryCreateInfo.handleTypes = kHandleType;

    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = &externalMemoryCreateInfo;
    createInfo.flags = 0;
    createInfo.size = mAllocatedSize;
    createInfo.usage = VulkanBufferUsage(GetUsage());
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.CreateBuffer(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "vkCreateBuffer"));

    // Gather requirements for the buffer's memory and allocate it.
    VkMemoryRequirements requirements;
    device->fn.GetBufferMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    // Gather memory requirements from the pointer.
    VkMemoryHostPointerPropertiesEXT hostPointerProperties;
    hostPointerProperties.sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;
    hostPointerProperties.pNext = nullptr;
    DAWN_TRY(CheckVkSuccess(
        device->fn.GetMemoryHostPointerPropertiesEXT(
            device->GetVkDevice(), VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
            hostMappedDesc->pointer, &hostPointerProperties),
        "vkGetHostPointerPropertiesEXT"));

    // Merge the memory type requirements from buffer and the host pointer.
    // Don't do this on SwiftShader which reports incompatible memory types even though there
    // is no real Device/Host distinction.
    if (!gpu_info::IsGoogleSwiftshader(GetDevice()->GetPhysicalDevice()->GetVendorId(),
                                       GetDevice()->GetPhysicalDevice()->GetDeviceId())) {
        requirements.memoryTypeBits &= hostPointerProperties.memoryTypeBits;
    }

    MemoryKind requestKind;
    if (GetUsage() & wgpu::BufferUsage::MapRead) {
        requestKind = MemoryKind::LinearReadMappable;
    } else if (GetUsage() & wgpu::BufferUsage::MapWrite) {
        requestKind = MemoryKind::LinearWriteMappable;
    } else {
        requestKind = MemoryKind::Linear;
    }

    int memoryTypeIndex =
        device->GetResourceMemoryAllocator()->FindBestTypeIndex(requirements, requestKind);
    DAWN_INVALID_IF(memoryTypeIndex < 0, "Failed to find suitable memory type.");

    // Make a device memory wrapping the host pointer.
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = mAllocatedSize;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo;
    importMemoryHostPointerInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importMemoryHostPointerInfo.pNext = nullptr;
    importMemoryHostPointerInfo.handleType = kHandleType;
    importMemoryHostPointerInfo.pHostPointer = hostMappedDesc->pointer;
    allocateInfo.pNext = &importMemoryHostPointerInfo;

    DAWN_TRY(CheckVkSuccess(device->fn.AllocateMemory(device->GetVkDevice(), &allocateInfo, nullptr,
                                                      &*mDedicatedDeviceMemory),
                            "vkAllocateMemory"));

    // Finally associate it with the buffer.
    DAWN_TRY(CheckVkSuccess(
        device->fn.BindBufferMemory(device->GetVkDevice(), mHandle, mDedicatedDeviceMemory, 0),
        "vkBindBufferMemory"));

    mHostMappedDisposeCallback = hostMappedDesc->disposeCallback;
    mHostMappedDisposeUserdata = hostMappedDesc->userdata;

    SetLabelImpl();

    // Assume the data is initialized since an external pointer was provided.
    SetIsDataInitialized();
    return {};
}

Buffer::~Buffer() = default;

VkBuffer Buffer::GetHandle() const {
    return mHandle;
}

void Buffer::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                wgpu::BufferUsage usage) {
    VkBufferMemoryBarrier barrier;
    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    if (TrackUsageAndGetResourceBarrier(recordingContext, usage, &barrier, &srcStages,
                                        &dstStages)) {
        DAWN_ASSERT(srcStages != 0 && dstStages != 0);
        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                    nullptr, 1u, &barrier, 0, nullptr);
    }
}

bool Buffer::TrackUsageAndGetResourceBarrier(CommandRecordingContext* recordingContext,
                                             wgpu::BufferUsage usage,
                                             VkBufferMemoryBarrier* barrier,
                                             VkPipelineStageFlags* srcStages,
                                             VkPipelineStageFlags* dstStages) {
    if (usage & kMapUsages) {
        // The pipeline barrier isn't needed, the buffer can be mapped immediately.
        if (mLastUsage == usage) {
            return false;
        }

        // Special-case for the initial transition: the pipeline barrier isn't needed.
        if (mLastUsage == wgpu::BufferUsage::None) {
            mLastUsage = usage;
            return false;
        }

        // For other cases, a pipeline barrier is needed, so mark the buffer is used within the
        // pending commands.
        MarkUsedInPendingCommands();
    } else {
        // Request non CPU usage, so assume the buffer will be used in pending commands.
        MarkUsedInPendingCommands();

        // If the buffer is mappable and the requested usage is not map usage, we need add it into
        // mappableBuffersForEagerTransition, so the buffer can be transitioned backed to map
        // usages at end of the submit.
        if (GetUsage() & kMapUsages) {
            recordingContext->mappableBuffersForEagerTransition.insert(this);
        }

        // Special-case for the initial transition: Vulkan doesn't allow access flags to be 0.
        if (mLastUsage == wgpu::BufferUsage::None) {
            mLastUsage = usage;
            return false;
        }

        bool lastIncludesTarget = IsSubset(usage, mLastUsage);
        bool lastReadOnly = IsSubset(mLastUsage, kReadOnlyBufferUsages);

        // We can skip transitions to already current read-only usages.
        if (lastIncludesTarget && lastReadOnly) {
            return false;
        }
    }

    *srcStages |= VulkanPipelineStage(mLastUsage);
    *dstStages |= VulkanPipelineStage(usage);

    barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier->pNext = nullptr;
    barrier->srcAccessMask = VulkanAccessFlags(mLastUsage);
    barrier->dstAccessMask = VulkanAccessFlags(usage);
    barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier->buffer = mHandle;
    barrier->offset = 0;
    // VK_WHOLE_SIZE doesn't work on old Windows Intel Vulkan drivers, so we don't use it.
    barrier->size = GetAllocatedSize();

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

    // TODO(crbug.com/dawn/852): initialize mapped buffer in CPU side.
    EnsureDataInitialized(recordingContext);

    if (mode & wgpu::MapMode::Read) {
        TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapRead);
    } else {
        DAWN_ASSERT(mode & wgpu::MapMode::Write);
        TransitionUsageNow(recordingContext, wgpu::BufferUsage::MapWrite);
    }
    return {};
}

void Buffer::UnmapImpl() {
    // No need to do anything, we keep CPU-visible memory mapped at all time.
}

void* Buffer::GetMappedPointer() {
    uint8_t* memory = mMemoryAllocation.GetMappedPointer();
    DAWN_ASSERT(memory != nullptr);
    return memory;
}

void Buffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    BufferBase::DestroyImpl();

    ToBackend(GetDevice())->GetResourceMemoryAllocator()->Deallocate(&mMemoryAllocation);

    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }

    if (mDedicatedDeviceMemory != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mDedicatedDeviceMemory);
        mDedicatedDeviceMemory = VK_NULL_HANDLE;
    }

    if (mHostMappedDisposeCallback) {
        struct DisposeTask : TrackTaskCallback {
            explicit DisposeTask(wgpu::Callback callback, void* userdata)
                : TrackTaskCallback(nullptr), callback(callback), userdata(userdata) {}
            ~DisposeTask() override = default;

            void FinishImpl() override { callback(userdata); }
            void HandleDeviceLossImpl() override { callback(userdata); }
            void HandleShutDownImpl() override { callback(userdata); }

            wgpu::Callback callback;
            void* userdata;
        };
        std::unique_ptr<DisposeTask> request =
            std::make_unique<DisposeTask>(mHostMappedDisposeCallback, mHostMappedDisposeUserdata);
        mHostMappedDisposeCallback = nullptr;

        GetDevice()->GetQueue()->TrackPendingTask(std::move(request));
    }
}

bool Buffer::EnsureDataInitialized(CommandRecordingContext* recordingContext) {
    if (!NeedsInitialization()) {
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                uint64_t offset,
                                                uint64_t size) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferRange(offset, size)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                                const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero(recordingContext);
    return true;
}

// static
void Buffer::TransitionMappableBuffersEagerly(const VulkanFunctions& fn,
                                              CommandRecordingContext* recordingContext,
                                              const std::set<Ref<Buffer>>& buffers) {
    DAWN_ASSERT(!buffers.empty());

    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    std::vector<VkBufferMemoryBarrier> barriers;
    barriers.reserve(buffers.size());

    size_t originalBufferCount = buffers.size();
    for (const Ref<Buffer>& buffer : buffers) {
        wgpu::BufferUsage mapUsage = buffer->GetUsage() & kMapUsages;
        DAWN_ASSERT(mapUsage == wgpu::BufferUsage::MapRead ||
                    mapUsage == wgpu::BufferUsage::MapWrite);
        VkBufferMemoryBarrier barrier;

        if (buffer->TrackUsageAndGetResourceBarrier(recordingContext, mapUsage, &barrier,
                                                    &srcStages, &dstStages)) {
            barriers.push_back(barrier);
        }
    }
    // TrackUsageAndGetResourceBarrier() should not modify recordingContext for map usages.
    DAWN_ASSERT(buffers.size() == originalBufferCount);

    if (barriers.empty()) {
        return;
    }

    DAWN_ASSERT(srcStages != 0 && dstStages != 0);
    fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0, nullptr,
                          barriers.size(), barriers.data(), 0, nullptr);
}

void Buffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_Buffer", GetLabel());
}

void Buffer::InitializeToZero(CommandRecordingContext* recordingContext) {
    DAWN_ASSERT(NeedsInitialization());

    ClearBuffer(recordingContext, 0u);
    GetDevice()->IncrementLazyClearCountForTesting();
    SetIsDataInitialized();
}

void Buffer::ClearBuffer(CommandRecordingContext* recordingContext,
                         uint32_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    DAWN_ASSERT(recordingContext != nullptr);
    size = size > 0 ? size : GetAllocatedSize();
    DAWN_ASSERT(size > 0);

    TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

    Device* device = ToBackend(GetDevice());
    // VK_WHOLE_SIZE doesn't work on old Windows Intel Vulkan drivers, so we don't use it.
    // Note: Allocated size must be a multiple of 4.
    DAWN_ASSERT(size % 4 == 0);
    device->fn.CmdFillBuffer(recordingContext->commandBuffer, mHandle, offset, size, clearValue);
}
}  // namespace dawn::native::vulkan
