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

#include "common/Assert.h"
#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device.h"

namespace dawn_wire { namespace client {

    bool Client::HandleDeviceErrorCallback(const char** commands, size_t* size) {
        ReturnDeviceErrorCallbackCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        DAWN_ASSERT(cmd.message != nullptr);
        mDevice->HandleError(cmd.message);

        return true;
    }

    bool Client::HandleBufferMapReadAsyncCallback(const char** commands, size_t* size) {
        ReturnBufferMapReadAsyncCallbackCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        auto* buffer = mDevice->GetClient()->BufferAllocator().GetObject(cmd.buffer.id);
        uint32_t bufferSerial = mDevice->GetClient()->BufferAllocator().GetSerial(cmd.buffer.id);

        // The buffer might have been deleted or recreated so this isn't an error.
        if (buffer == nullptr || bufferSerial != cmd.buffer.serial) {
            return true;
        }

        // The requests can have been deleted via an Unmap so this isn't an error.
        auto requestIt = buffer->requests.find(cmd.requestSerial);
        if (requestIt == buffer->requests.end()) {
            return true;
        }

        // It is an error for the server to call the read callback when we asked for a map write
        if (requestIt->second.isWrite) {
            return false;
        }

        auto request = requestIt->second;
        // Delete the request before calling the callback otherwise the callback could be fired a
        // second time. If, for example, buffer.Unmap() is called inside the callback.
        buffer->requests.erase(requestIt);

        // On success, we copy the data locally because the IPC buffer isn't valid outside of this
        // function
        if (cmd.status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
            // The server didn't send the right amount of data, this is an error and could cause
            // the application to crash if we did call the callback.
            if (request.size != cmd.dataLength) {
                return false;
            }

            ASSERT(cmd.data != nullptr);

            if (buffer->mappedData != nullptr) {
                return false;
            }

            buffer->isWriteMapped = false;
            buffer->mappedDataSize = request.size;
            buffer->mappedData = malloc(request.size);
            memcpy(buffer->mappedData, cmd.data, request.size);

            request.readCallback(static_cast<dawnBufferMapAsyncStatus>(cmd.status),
                                 buffer->mappedData, request.userdata);
        } else {
            request.readCallback(static_cast<dawnBufferMapAsyncStatus>(cmd.status), nullptr,
                                 request.userdata);
        }

        return true;
    }

    bool Client::HandleBufferMapWriteAsyncCallback(const char** commands, size_t* size) {
        ReturnBufferMapWriteAsyncCallbackCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        auto* buffer = mDevice->GetClient()->BufferAllocator().GetObject(cmd.buffer.id);
        uint32_t bufferSerial = mDevice->GetClient()->BufferAllocator().GetSerial(cmd.buffer.id);

        // The buffer might have been deleted or recreated so this isn't an error.
        if (buffer == nullptr || bufferSerial != cmd.buffer.serial) {
            return true;
        }

        // The requests can have been deleted via an Unmap so this isn't an error.
        auto requestIt = buffer->requests.find(cmd.requestSerial);
        if (requestIt == buffer->requests.end()) {
            return true;
        }

        // It is an error for the server to call the write callback when we asked for a map read
        if (!requestIt->second.isWrite) {
            return false;
        }

        auto request = requestIt->second;
        // Delete the request before calling the callback otherwise the callback could be fired a
        // second time. If, for example, buffer.Unmap() is called inside the callback.
        buffer->requests.erase(requestIt);

        // On success, we copy the data locally because the IPC buffer isn't valid outside of this
        // function
        if (cmd.status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
            if (buffer->mappedData != nullptr) {
                return false;
            }

            buffer->isWriteMapped = true;
            buffer->mappedDataSize = request.size;
            buffer->mappedData = malloc(request.size);
            memset(buffer->mappedData, 0, request.size);

            request.writeCallback(static_cast<dawnBufferMapAsyncStatus>(cmd.status),
                                  buffer->mappedData, request.userdata);
        } else {
            request.writeCallback(static_cast<dawnBufferMapAsyncStatus>(cmd.status), nullptr,
                                  request.userdata);
        }

        return true;
    }

    bool Client::HandleFenceUpdateCompletedValue(const char** commands, size_t* size) {
        ReturnFenceUpdateCompletedValueCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        auto* fence = mDevice->GetClient()->FenceAllocator().GetObject(cmd.fence.id);
        uint32_t fenceSerial = mDevice->GetClient()->FenceAllocator().GetSerial(cmd.fence.id);

        // The fence might have been deleted or recreated so this isn't an error.
        if (fence == nullptr || fenceSerial != cmd.fence.serial) {
            return true;
        }

        fence->completedValue = cmd.value;
        fence->CheckPassedFences();
        return true;
    }

}}  // namespace dawn_wire::client
