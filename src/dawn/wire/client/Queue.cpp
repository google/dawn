// Copyright 2020 The Dawn Authors
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

#include "dawn/wire/client/Queue.h"

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"

namespace dawn::wire::client {
namespace {

struct WorkDoneEvent : public TrackedEvent {
    explicit WorkDoneEvent(const WGPUQueueWorkDoneCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode, callbackInfo.userdata),
          mCallback(callbackInfo.callback) {}

    void CompleteImpl(EventCompletionType completionType) override {
        WGPUQueueWorkDoneStatus status = completionType == EventCompletionType::Shutdown
                                             ? WGPUQueueWorkDoneStatus_DeviceLost
                                             : WGPUQueueWorkDoneStatus_Success;
        if (mStatus) {
            // TODO(crbug.com/dawn/2021): Pretend things success when the device is lost.
            status = *mStatus == WGPUQueueWorkDoneStatus_DeviceLost
                         ? WGPUQueueWorkDoneStatus_Success
                         : *mStatus;
        }
        if (mCallback) {
            mCallback(status, mUserdata);
        }
    }

    static void WorkDoneEventReady(TrackedEvent& event, WGPUQueueWorkDoneStatus status) {
        static_cast<WorkDoneEvent&>(event).mStatus = status;
    }

    WGPUQueueWorkDoneCallback mCallback;
    std::optional<WGPUQueueWorkDoneStatus> mStatus;
};

}  // anonymous namespace

Queue::~Queue() = default;

bool Queue::OnWorkDoneCallback(WGPUFuture future, WGPUQueueWorkDoneStatus status) {
    GetClient()->GetEventManager()->SetFutureReady(
        future.id, std::bind(WorkDoneEvent::WorkDoneEventReady, std::placeholders::_1, status));
    return true;
}

void Queue::OnSubmittedWorkDone(uint64_t signalValue,
                                WGPUQueueWorkDoneCallback callback,
                                void* userdata) {
    WGPUQueueWorkDoneCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.callback = callback;
    callbackInfo.userdata = userdata;
    OnSubmittedWorkDoneF(callbackInfo);
}

WGPUFuture Queue::OnSubmittedWorkDoneF(const WGPUQueueWorkDoneCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        client->GetEventManager()->TrackEvent(new WorkDoneEvent(callbackInfo));
    if (!tracked) {
        return {futureIDInternal};
    }

    QueueOnSubmittedWorkDoneCmd cmd;
    cmd.queueId = GetWireId();
    cmd.signalValue = 0;
    cmd.future = {futureIDInternal};

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

void Queue::WriteBuffer(WGPUBuffer cBuffer, uint64_t bufferOffset, const void* data, size_t size) {
    Buffer* buffer = FromAPI(cBuffer);

    QueueWriteBufferCmd cmd;
    cmd.queueId = GetWireId();
    cmd.bufferId = buffer->GetWireId();
    cmd.bufferOffset = bufferOffset;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.size = size;

    GetClient()->SerializeCommand(cmd);
}

void Queue::WriteTexture(const WGPUImageCopyTexture* destination,
                         const void* data,
                         size_t dataSize,
                         const WGPUTextureDataLayout* dataLayout,
                         const WGPUExtent3D* writeSize) {
    QueueWriteTextureCmd cmd;
    cmd.queueId = GetWireId();
    cmd.destination = destination;
    cmd.data = static_cast<const uint8_t*>(data);
    cmd.dataSize = dataSize;
    cmd.dataLayout = dataLayout;
    cmd.writeSize = writeSize;

    GetClient()->SerializeCommand(cmd);
}

}  // namespace dawn::wire::client
