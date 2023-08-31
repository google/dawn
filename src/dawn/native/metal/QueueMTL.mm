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

#include "dawn/native/metal/QueueMTL.h"

#include "dawn/common/Math.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/MetalBackend.h"
#include "dawn/native/metal/CommandBufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::metal {

ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    Ref<Queue> queue = AcquireRef(new Queue(device, descriptor));
    DAWN_TRY(queue->Initialize());
    return queue;
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor)
    : QueueBase(device, descriptor), mCompletedSerial(0) {}

Queue::~Queue() {}

void Queue::Destroy() {
    // Forget all pending commands.
    mCommandContext.AcquireCommands();
    mCommandQueue = nullptr;
    mLastSubmittedCommands = nullptr;
    mMtlSharedEvent = nullptr;
}

MaybeError Queue::Initialize() {
    id<MTLDevice> mtlDevice = ToBackend(GetDevice())->GetMTLDevice();

    mCommandQueue.Acquire([mtlDevice newCommandQueue]);
    if (mCommandQueue == nil) {
        return DAWN_INTERNAL_ERROR("Failed to allocate MTLCommandQueue.");
    }

    if (@available(macOS 10.14, iOS 12.0, *)) {
        mMtlSharedEvent.Acquire([mtlDevice newSharedEvent]);
    }

    return mCommandContext.PrepareNextCommandBuffer(*mCommandQueue);
}

MaybeError Queue::WaitForIdleForDestruction() {
    // Forget all pending commands.
    mCommandContext.AcquireCommands();
    DAWN_TRY(CheckPassedSerials());

    // Wait for all commands to be finished so we can free resources
    while (GetCompletedCommandSerial() != GetLastSubmittedCommandSerial()) {
        usleep(100);
        DAWN_TRY(CheckPassedSerials());
    }

    return {};
}

void Queue::WaitForCommandsToBeScheduled() {
    if (GetDevice()->ConsumedError(SubmitPendingCommandBuffer())) {
        return;
    }

    // Only lock the object while we take a reference to it, otherwise we could block further
    // progress if the driver calls the scheduled handler (which also acquires the lock) before
    // finishing the waitUntilScheduled.
    NSPRef<id<MTLCommandBuffer>> lastSubmittedCommands;
    {
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        lastSubmittedCommands = mLastSubmittedCommands;
    }
    [*lastSubmittedCommands waitUntilScheduled];
}

CommandRecordingContext* Queue::GetPendingCommandContext(SubmitMode submitMode) {
    if (submitMode == SubmitMode::Normal) {
        mCommandContext.SetNeedsSubmit();
    }
    mCommandContext.MarkUsed();
    return &mCommandContext;
}

MaybeError Queue::SubmitPendingCommandBuffer() {
    if (!mCommandContext.NeedsSubmit()) {
        return {};
    }

    auto platform = GetDevice()->GetPlatform();

    IncrementLastSubmittedCommandSerial();

    // Acquire the pending command buffer, which is retained. It must be released later.
    NSPRef<id<MTLCommandBuffer>> pendingCommands = mCommandContext.AcquireCommands();

    // Replace mLastSubmittedCommands with the mutex held so we avoid races between the
    // schedule handler and this code.
    {
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        mLastSubmittedCommands = pendingCommands;
    }

    // Make a local copy of the pointer to the commands because it's not clear how ObjC blocks
    // handle types with copy / move constructors being referenced in the block.
    id<MTLCommandBuffer> pendingCommandsPointer = pendingCommands.Get();
    [*pendingCommands addScheduledHandler:^(id<MTLCommandBuffer>) {
        // This is DRF because we hold the mutex for mLastSubmittedCommands and pendingCommands
        // is a local value (and not the member itself).
        std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
        if (this->mLastSubmittedCommands.Get() == pendingCommandsPointer) {
            this->mLastSubmittedCommands = nullptr;
        }
    }];

    // Update the completed serial once the completed handler is fired. Make a local copy of
    // mLastSubmittedSerial so it is captured by value.
    ExecutionSerial pendingSerial = GetLastSubmittedCommandSerial();
    // this ObjC block runs on a different thread
    [*pendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
        TRACE_EVENT_ASYNC_END0(platform, GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                               uint64_t(pendingSerial));
        ASSERT(uint64_t(pendingSerial) > mCompletedSerial.load());
        this->mCompletedSerial = uint64_t(pendingSerial);
    }];

    TRACE_EVENT_ASYNC_BEGIN0(platform, GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                             uint64_t(pendingSerial));
    if (@available(macOS 10.14, *)) {
        id rawEvent = *mMtlSharedEvent;
        id<MTLSharedEvent> sharedEvent = static_cast<id<MTLSharedEvent>>(rawEvent);
        [*pendingCommands encodeSignalEvent:sharedEvent value:static_cast<uint64_t>(pendingSerial)];
    }
    [*pendingCommands commit];

    return mCommandContext.PrepareNextCommandBuffer(*mCommandQueue);
}

void Queue::ExportLastSignaledEvent(ExternalImageMTLSharedEventDescriptor* desc) {
    // Ensure commands are submitted before getting the last submited serial.
    // Ignore the error since we still want to export the serial of the last successful
    // submission - that was the last serial that was actually signaled.
    ForceEventualFlushOfCommands();

    if (GetDevice()->ConsumedError(SubmitPendingCommandBuffer())) {
        desc->sharedEvent = nullptr;
        return;
    }

    desc->sharedEvent = *mMtlSharedEvent;
    desc->signaledValue = static_cast<uint64_t>(GetLastSubmittedCommandSerial());
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    @autoreleasepool {
        CommandRecordingContext* commandContext = GetPendingCommandContext();

        TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferMTL::FillCommands");
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->FillCommands(commandContext));
        }
        TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferMTL::FillCommands");

        DAWN_TRY(SubmitPendingCommandBuffer());

        return {};
    }
}

bool Queue::HasPendingCommands() const {
    return mCommandContext.NeedsSubmit();
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    uint64_t frontendCompletedSerial{GetCompletedCommandSerial()};
    // sometimes we increase the serials, in which case the completed serial in
    // the device base will surpass the completed serial we have in the metal backend, so we
    // must update ours when we see that the completed serial from device base has
    // increased.
    //
    // This update has to be atomic otherwise there is a race with the `addCompletedHandler`
    // call below and this call could set the mCompletedSerial backwards.
    uint64_t current = mCompletedSerial.load();
    while (frontendCompletedSerial > current &&
           !mCompletedSerial.compare_exchange_weak(current, frontendCompletedSerial)) {
    }

    return ExecutionSerial(mCompletedSerial.load());
}

void Queue::ForceEventualFlushOfCommands() {
    if (mCommandContext.WasUsed()) {
        mCommandContext.SetNeedsSubmit();
    }
}

}  // namespace dawn::native::metal
