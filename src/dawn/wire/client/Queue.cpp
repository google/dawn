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
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

Queue::~Queue() {
    ClearAllCallbacks(WGPUQueueWorkDoneStatus_Unknown);
}

bool Queue::OnWorkDoneCallback(uint64_t requestSerial, WGPUQueueWorkDoneStatus status) {
    OnWorkDoneData request;
    if (!mOnWorkDoneRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    request.callback(status, request.userdata);
    return true;
}

void Queue::OnSubmittedWorkDone(uint64_t signalValue,
                                WGPUQueueWorkDoneCallback callback,
                                void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPUQueueWorkDoneStatus_DeviceLost, userdata);
        return;
    }

    uint64_t serial = mOnWorkDoneRequests.Add({callback, userdata});

    QueueOnSubmittedWorkDoneCmd cmd;
    cmd.queueId = GetWireId();
    cmd.signalValue = signalValue;
    cmd.requestSerial = serial;

    client->SerializeCommand(cmd);
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

void Queue::CancelCallbacksForDisconnect() {
    ClearAllCallbacks(WGPUQueueWorkDoneStatus_DeviceLost);
}

void Queue::ClearAllCallbacks(WGPUQueueWorkDoneStatus status) {
    mOnWorkDoneRequests.CloseAll([status](OnWorkDoneData* request) {
        if (request->callback != nullptr) {
            request->callback(status, request->userdata);
        }
    });
}

}  // namespace dawn::wire::client
