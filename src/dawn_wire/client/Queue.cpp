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

#include "dawn_wire/client/Queue.h"

#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device.h"

namespace dawn_wire { namespace client {

    Queue::~Queue() {
        ClearAllCallbacks(WGPUQueueWorkDoneStatus_Unknown);
    }

    bool Queue::OnWorkDoneCallback(uint64_t requestSerial, WGPUQueueWorkDoneStatus status) {
        auto requestIt = mOnWorkDoneRequests.find(requestSerial);
        if (requestIt == mOnWorkDoneRequests.end()) {
            return false;
        }

        // Remove the request data so that the callback cannot be called again.
        // ex.) inside the callback: if the queue is deleted (when there are multiple queues),
        // all callbacks reject.
        OnWorkDoneData request = std::move(requestIt->second);
        mOnWorkDoneRequests.erase(requestIt);

        request.callback(status, request.userdata);
        return true;
    }

    void Queue::OnSubmittedWorkDone(uint64_t signalValue,
                                    WGPUQueueWorkDoneCallback callback,
                                    void* userdata) {
        if (client->IsDisconnected()) {
            callback(WGPUQueueWorkDoneStatus_DeviceLost, userdata);
            return;
        }

        uint32_t serial = mOnWorkDoneSerial++;
        ASSERT(mOnWorkDoneRequests.find(serial) == mOnWorkDoneRequests.end());

        QueueOnSubmittedWorkDoneCmd cmd;
        cmd.queueId = this->id;
        cmd.signalValue = signalValue;
        cmd.requestSerial = serial;

        mOnWorkDoneRequests[serial] = {callback, userdata};

        client->SerializeCommand(cmd);
    }

    WGPUFence Queue::CreateFence(WGPUFenceDescriptor const* descriptor) {
        auto* allocation = client->FenceAllocator().New(client);

        QueueCreateFenceCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};
        cmd.descriptor = descriptor;
        client->SerializeCommand(cmd);

        Fence* fence = allocation->object.get();
        fence->Initialize(descriptor);
        return ToAPI(fence);
    }

    void Queue::WriteBuffer(WGPUBuffer cBuffer,
                            uint64_t bufferOffset,
                            const void* data,
                            size_t size) {
        Buffer* buffer = FromAPI(cBuffer);

        QueueWriteBufferInternalCmd cmd;
        cmd.queueId = id;
        cmd.bufferId = buffer->id;
        cmd.bufferOffset = bufferOffset;
        cmd.data = static_cast<const uint8_t*>(data);
        cmd.size = size;

        client->SerializeCommand(cmd);
    }

    void Queue::WriteTexture(const WGPUTextureCopyView* destination,
                             const void* data,
                             size_t dataSize,
                             const WGPUTextureDataLayout* dataLayout,
                             const WGPUExtent3D* writeSize) {
        QueueWriteTextureInternalCmd cmd;
        cmd.queueId = id;
        cmd.destination = destination;
        cmd.data = static_cast<const uint8_t*>(data);
        cmd.dataSize = dataSize;
        cmd.dataLayout = dataLayout;
        cmd.writeSize = writeSize;

        client->SerializeCommand(cmd);
    }

    void Queue::CancelCallbacksForDisconnect() {
        ClearAllCallbacks(WGPUQueueWorkDoneStatus_DeviceLost);
    }

    void Queue::ClearAllCallbacks(WGPUQueueWorkDoneStatus status) {
        for (auto& it : mOnWorkDoneRequests) {
            if (it.second.callback) {
                it.second.callback(status, it.second.userdata);
            }
        }
        mOnWorkDoneRequests.clear();
    }

}}  // namespace dawn_wire::client
