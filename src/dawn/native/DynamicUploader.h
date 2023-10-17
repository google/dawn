// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
#define SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_

#include <memory>
#include <vector>

#include "dawn/common/Ref.h"
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

    ResultOrError<UploadHandle> AllocateInternal(uint64_t allocationSize,
                                                 ExecutionSerial serial,
                                                 uint64_t offsetAlignment);

    std::vector<std::unique_ptr<RingBuffer>> mRingBuffers;
    SerialQueue<ExecutionSerial, Ref<BufferBase>> mReleasedStagingBuffers;
    DeviceBase* mDevice;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICUPLOADER_H_
