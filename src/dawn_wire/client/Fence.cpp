// Copyright 2019 The Dawn Authors
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

#include "dawn_wire/client/Fence.h"

#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    Fence::~Fence() {
        // Callbacks need to be fired in all cases, as they can handle freeing resources
        // so we call them with "Unknown" status.
        for (auto& it : mOnCompletionRequests) {
            if (it.second.callback) {
                it.second.callback(WGPUFenceCompletionStatus_Unknown, it.second.userdata);
            }
        }
        mOnCompletionRequests.clear();
    }

    void Fence::CancelCallbacksForDisconnect() {
        for (auto& it : mOnCompletionRequests) {
            if (it.second.callback) {
                it.second.callback(WGPUFenceCompletionStatus_DeviceLost, it.second.userdata);
            }
        }
        mOnCompletionRequests.clear();
    }

    void Fence::Initialize(Queue* queue, const WGPUFenceDescriptor* descriptor) {
        mQueue = queue;

        mCompletedValue = descriptor != nullptr ? descriptor->initialValue : 0u;
    }

    void Fence::OnCompletion(uint64_t value,
                             WGPUFenceOnCompletionCallback callback,
                             void* userdata) {
        if (client->IsDisconnected()) {
            return callback(WGPUFenceCompletionStatus_DeviceLost, userdata);
        }

        uint32_t serial = mOnCompletionRequestSerial++;
        ASSERT(mOnCompletionRequests.find(serial) == mOnCompletionRequests.end());

        FenceOnCompletionCmd cmd;
        cmd.fenceId = this->id;
        cmd.value = value;
        cmd.requestSerial = serial;

        mOnCompletionRequests[serial] = {callback, userdata};

        client->SerializeCommand(cmd);
    }

    void Fence::OnUpdateCompletedValueCallback(uint64_t value) {
        mCompletedValue = value;
    }

    bool Fence::OnCompletionCallback(uint64_t requestSerial, WGPUFenceCompletionStatus status) {
        auto requestIt = mOnCompletionRequests.find(requestSerial);
        if (requestIt == mOnCompletionRequests.end()) {
            return false;
        }

        // Remove the request data so that the callback cannot be called again.
        // ex.) inside the callback: if the fence is deleted, all callbacks reject.
        OnCompletionData request = std::move(requestIt->second);
        mOnCompletionRequests.erase(requestIt);

        request.callback(status, request.userdata);
        return true;
    }

    uint64_t Fence::GetCompletedValue() const {
        return mCompletedValue;
    }

    Queue* Fence::GetQueue() const {
        return mQueue;
    }

}}  // namespace dawn_wire::client
