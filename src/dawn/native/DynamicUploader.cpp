// Copyright 2018 The Dawn Authors
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

#include "dawn/native/DynamicUploader.h"

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"

namespace dawn::native {

DynamicUploader::DynamicUploader(DeviceBase* device) : mDevice(device) {
    mRingBuffers.emplace_back(
        std::unique_ptr<RingBuffer>(new RingBuffer{nullptr, RingBufferAllocator(kRingBufferSize)}));
}

void DynamicUploader::ReleaseStagingBuffer(Ref<BufferBase> stagingBuffer) {
    mReleasedStagingBuffers.Enqueue(std::move(stagingBuffer), mDevice->GetPendingCommandSerial());
}

ResultOrError<UploadHandle> DynamicUploader::AllocateInternal(uint64_t allocationSize,
                                                              ExecutionSerial serial) {
    // Disable further sub-allocation should the request be too large.
    if (allocationSize > kRingBufferSize) {
        BufferDescriptor bufferDesc = {};
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        bufferDesc.size = Align(allocationSize, 4);
        bufferDesc.mappedAtCreation = true;
        bufferDesc.label = "Dawn_DynamicUploaderStaging";

        IgnoreLazyClearCountScope scope(mDevice);
        Ref<BufferBase> stagingBuffer;
        DAWN_TRY_ASSIGN(stagingBuffer, mDevice->CreateBuffer(&bufferDesc));

        UploadHandle uploadHandle;
        uploadHandle.mappedBuffer = static_cast<uint8_t*>(stagingBuffer->GetMappedPointer());
        uploadHandle.stagingBuffer = stagingBuffer.Get();

        ReleaseStagingBuffer(std::move(stagingBuffer));
        return uploadHandle;
    }

    // Note: Validation ensures size is already aligned.
    // First-fit: find next smallest buffer large enough to satisfy the allocation request.
    RingBuffer* targetRingBuffer = mRingBuffers.back().get();
    for (auto& ringBuffer : mRingBuffers) {
        const RingBufferAllocator& ringBufferAllocator = ringBuffer->mAllocator;
        // Prevent overflow.
        ASSERT(ringBufferAllocator.GetSize() >= ringBufferAllocator.GetUsedSize());
        const uint64_t remainingSize =
            ringBufferAllocator.GetSize() - ringBufferAllocator.GetUsedSize();
        if (allocationSize <= remainingSize) {
            targetRingBuffer = ringBuffer.get();
            break;
        }
    }

    uint64_t startOffset = RingBufferAllocator::kInvalidOffset;
    if (targetRingBuffer != nullptr) {
        startOffset = targetRingBuffer->mAllocator.Allocate(allocationSize, serial);
    }

    // Upon failure, append a newly created ring buffer to fulfill the
    // request.
    if (startOffset == RingBufferAllocator::kInvalidOffset) {
        mRingBuffers.emplace_back(std::unique_ptr<RingBuffer>(
            new RingBuffer{nullptr, RingBufferAllocator(kRingBufferSize)}));

        targetRingBuffer = mRingBuffers.back().get();
        startOffset = targetRingBuffer->mAllocator.Allocate(allocationSize, serial);
    }

    ASSERT(startOffset != RingBufferAllocator::kInvalidOffset);

    // Allocate the staging buffer backing the ringbuffer.
    // Note: the first ringbuffer will be lazily created.
    if (targetRingBuffer->mStagingBuffer == nullptr) {
        BufferDescriptor bufferDesc = {};
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        bufferDesc.size = Align(targetRingBuffer->mAllocator.GetSize(), 4);
        bufferDesc.mappedAtCreation = true;
        bufferDesc.label = "Dawn_DynamicUploaderStaging";

        IgnoreLazyClearCountScope scope(mDevice);
        Ref<BufferBase> stagingBuffer;
        DAWN_TRY_ASSIGN(stagingBuffer, mDevice->CreateBuffer(&bufferDesc));
        targetRingBuffer->mStagingBuffer = std::move(stagingBuffer);
    }

    ASSERT(targetRingBuffer->mStagingBuffer != nullptr);

    UploadHandle uploadHandle;
    uploadHandle.stagingBuffer = targetRingBuffer->mStagingBuffer.Get();
    uploadHandle.mappedBuffer =
        static_cast<uint8_t*>(uploadHandle.stagingBuffer->GetMappedPointer()) + startOffset;
    uploadHandle.startOffset = startOffset;

    return uploadHandle;
}

void DynamicUploader::Deallocate(ExecutionSerial lastCompletedSerial) {
    // Reclaim memory within the ring buffers by ticking (or removing requests no longer
    // in-flight).
    for (size_t i = 0; i < mRingBuffers.size(); ++i) {
        mRingBuffers[i]->mAllocator.Deallocate(lastCompletedSerial);

        // Never erase the last buffer as to prevent re-creating smaller buffers
        // again. The last buffer is the largest.
        if (mRingBuffers[i]->mAllocator.Empty() && i < mRingBuffers.size() - 1) {
            mRingBuffers.erase(mRingBuffers.begin() + i);
        }
    }
    mReleasedStagingBuffers.ClearUpTo(lastCompletedSerial);
}

// TODO(dawn:512): Optimize this function so that it doesn't allocate additional memory
// when it's not necessary.
ResultOrError<UploadHandle> DynamicUploader::Allocate(uint64_t allocationSize,
                                                      ExecutionSerial serial,
                                                      uint64_t offsetAlignment) {
    ASSERT(offsetAlignment > 0);
    UploadHandle uploadHandle;
    DAWN_TRY_ASSIGN(uploadHandle, AllocateInternal(allocationSize + offsetAlignment - 1, serial));
    uint64_t additionalOffset =
        Align(uploadHandle.startOffset, offsetAlignment) - uploadHandle.startOffset;
    uploadHandle.mappedBuffer = static_cast<uint8_t*>(uploadHandle.mappedBuffer) + additionalOffset;
    uploadHandle.startOffset += additionalOffset;
    return uploadHandle;
}

bool DynamicUploader::ShouldFlush() {
    uint64_t kTotalAllocatedSizeThreshold = 64 * 1024 * 1024;
    // We use total allocated size instead of pending-upload size to prevent Dawn from allocating
    // too much GPU memory so that the risk of OOM can be minimized.
    return GetTotalAllocatedSize() > kTotalAllocatedSizeThreshold;
}

uint64_t DynamicUploader::GetTotalAllocatedSize() {
    uint64_t size = 0;
    for (const auto& buffer : mReleasedStagingBuffers.IterateAll()) {
        size += buffer->GetSize();
    }
    for (const auto& buffer : mRingBuffers) {
        if (buffer->mStagingBuffer != nullptr) {
            size += buffer->mStagingBuffer->GetSize();
        }
    }
    return size;
}

}  // namespace dawn::native
