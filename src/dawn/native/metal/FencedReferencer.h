// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_FENCEDREFERENCER_H_
#define SRC_DAWN_NATIVE_METAL_FENCEDREFERENCER_H_

#import <Metal/Metal.h>

#include <atomic>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/NSRef.h"
#include "src/dawn/common/SerialQueue.h"
#include "src/dawn/native/ExecutionQueue.h"
#include "src/dawn/native/IntegerTypes.h"

namespace dawn::native::metal {

class Device;

class FencedReferencer final : public ExecutionQueueBase::SerialProcessor {
  public:
    explicit FencedReferencer(Device* device);
    ~FencedReferencer() override;

    // Hold a ref to the object until the queue is done using it. (Note the
    // object won't be destroyed on release unless this is the last ref.)
    void RefUntilUnused(id handle);

    // SerialProcessor API.
    void UpdateCompletedSerialTo(ExecutionSerial completedSerial) override;

    bool HasPendingObjectsForTesting() const;

  private:
    raw_ptr<Device> mDevice = nullptr;

    struct PendingDeletions {
        SerialQueue<ExecutionSerial, NSPRef<id>> mObjects;
        ExecutionSerial mCompletedSerial = kBeginningOfGPUTime;
    };
    MutexProtected<PendingDeletions> mPendingDeletions;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_FENCEDREFERENCER_H_
