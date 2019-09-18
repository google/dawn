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

#include "dawn_native/DynamicUploader.h"
#include "common/Math.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    DynamicUploader::DynamicUploader(DeviceBase* device, size_t size) : mDevice(device) {
        mRingBuffers.emplace_back(
            std::unique_ptr<RingBuffer>(new RingBuffer{nullptr, RingBufferAllocator(size)}));
    }

    void DynamicUploader::ReleaseStagingBuffer(std::unique_ptr<StagingBufferBase> stagingBuffer) {
        mReleasedStagingBuffers.Enqueue(std::move(stagingBuffer),
                                        mDevice->GetPendingCommandSerial());
    }

    ResultOrError<UploadHandle> DynamicUploader::Allocate(size_t allocationSize, Serial serial) {
        // Note: Validation ensures size is already aligned.
        // First-fit: find next smallest buffer large enough to satisfy the allocation request.
        RingBuffer* targetRingBuffer = mRingBuffers.back().get();
        for (auto& ringBuffer : mRingBuffers) {
            const RingBufferAllocator& ringBufferAllocator = ringBuffer->mAllocator;
            // Prevent overflow.
            ASSERT(ringBufferAllocator.GetSize() >= ringBufferAllocator.GetUsedSize());
            const size_t remainingSize =
                ringBufferAllocator.GetSize() - ringBufferAllocator.GetUsedSize();
            if (allocationSize <= remainingSize) {
                targetRingBuffer = ringBuffer.get();
                break;
            }
        }

        size_t startOffset = kInvalidOffset;
        if (targetRingBuffer != nullptr) {
            startOffset = targetRingBuffer->mAllocator.Allocate(allocationSize, serial);
        }

        // Upon failure, append a newly created (and much larger) ring buffer to fulfill the
        // request.
        if (startOffset == kInvalidOffset) {
            // Compute the new max size (in powers of two to preserve alignment).
            size_t newMaxSize = targetRingBuffer->mAllocator.GetSize() * 2;
            while (newMaxSize < allocationSize) {
                newMaxSize *= 2;
            }

            // TODO(bryan.bernhart@intel.com): Fall-back to no sub-allocations should this fail.
            mRingBuffers.emplace_back(std::unique_ptr<RingBuffer>(
                new RingBuffer{nullptr, RingBufferAllocator(newMaxSize)}));

            targetRingBuffer = mRingBuffers.back().get();
            startOffset = targetRingBuffer->mAllocator.Allocate(allocationSize, serial);
        }

        ASSERT(startOffset != kInvalidOffset);

        // Allocate the staging buffer backing the ringbuffer.
        // Note: the first ringbuffer will be lazily created.
        if (targetRingBuffer->mStagingBuffer == nullptr) {
            std::unique_ptr<StagingBufferBase> stagingBuffer;
            DAWN_TRY_ASSIGN(stagingBuffer,
                            mDevice->CreateStagingBuffer(targetRingBuffer->mAllocator.GetSize()));
            targetRingBuffer->mStagingBuffer = std::move(stagingBuffer);
        }

        ASSERT(targetRingBuffer->mStagingBuffer != nullptr);

        UploadHandle uploadHandle;
        uploadHandle.stagingBuffer = targetRingBuffer->mStagingBuffer.get();
        uploadHandle.mappedBuffer =
            static_cast<uint8_t*>(uploadHandle.stagingBuffer->GetMappedPointer()) + startOffset;
        uploadHandle.startOffset = startOffset;

        return uploadHandle;
    }

    void DynamicUploader::Deallocate(Serial lastCompletedSerial) {
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
}  // namespace dawn_native