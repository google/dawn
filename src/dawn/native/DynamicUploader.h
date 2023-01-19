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

#ifndef SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
#define SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_

#include <memory>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/RingBufferAllocator.h"

// DynamicUploader is the front-end implementation used to manage multiple ring buffers for upload
// usage.
namespace dawn::native {

class BufferBase;

struct UploadHandle {
    uint8_t* mappedBuffer = nullptr;
    uint64_t startOffset = 0;
    BufferBase* stagingBuffer = nullptr;
};

class DynamicUploader {
  public:
    explicit DynamicUploader(DeviceBase* device);
    ~DynamicUploader() = default;

    // We add functions to Release StagingBuffers to the DynamicUploader as there's
    // currently no place to track the allocated staging buffers such that they're freed after
    // pending commands are finished. This should be changed when better resource allocation is
    // implemented.
    void ReleaseStagingBuffer(Ref<BufferBase> stagingBuffer);

    ResultOrError<UploadHandle> Allocate(uint64_t allocationSize,
                                         ExecutionSerial serial,
                                         uint64_t offsetAlignment);
    void Deallocate(ExecutionSerial lastCompletedSerial);

    bool ShouldFlush();

  private:
    static constexpr uint64_t kRingBufferSize = 4 * 1024 * 1024;
    uint64_t GetTotalAllocatedSize();

    struct RingBuffer {
        Ref<BufferBase> mStagingBuffer;
        RingBufferAllocator mAllocator;
    };

    ResultOrError<UploadHandle> AllocateInternal(uint64_t allocationSize, ExecutionSerial serial);

    std::vector<std::unique_ptr<RingBuffer>> mRingBuffers;
    SerialQueue<ExecutionSerial, Ref<BufferBase>> mReleasedStagingBuffers;
    DeviceBase* mDevice;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
