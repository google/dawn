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

#ifndef SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_
#define SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_

#import <Metal/Metal.h>
#include "dawn/native/Queue.h"
#include "dawn/native/metal/CommandRecordingContext.h"

namespace dawn::native::metal {

class Device;
struct ExternalImageMTLSharedEventDescriptor;

class Queue final : public QueueBase {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    CommandRecordingContext* GetPendingCommandContext(SubmitMode submitMode = SubmitMode::Normal);
    MaybeError SubmitPendingCommandBuffer();
    void WaitForCommandsToBeScheduled();
    void ExportLastSignaledEvent(ExternalImageMTLSharedEventDescriptor* desc);
    void Destroy();

  private:
    Queue(Device* device, const QueueDescriptor* descriptor);
    ~Queue() override;

    MaybeError Initialize();

    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    bool HasPendingCommands() const override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;

    NSPRef<id<MTLCommandQueue>> mCommandQueue;
    CommandRecordingContext mCommandContext;

    // mLastSubmittedCommands will be accessed in a Metal schedule handler that can be fired on
    // a different thread so we guard access to it with a mutex.
    std::mutex mLastSubmittedCommandsMutex;
    NSPRef<id<MTLCommandBuffer>> mLastSubmittedCommands;

    // The completed serial is updated in a Metal completion handler that can be fired on a
    // different thread, so it needs to be atomic.
    std::atomic<uint64_t> mCompletedSerial;

    // A shared event that can be exported for synchronization with other users of Metal.
    // MTLSharedEvent is not available until macOS 10.14+ so use just `id`.
    NSPRef<id> mMtlSharedEvent = nullptr;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_QUEUEMTL_H_
