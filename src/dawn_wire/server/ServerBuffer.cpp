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
#include "dawn_wire/server/Server.h"

#include <memory>

namespace dawn_wire { namespace server {

    bool Server::PreHandleBufferUnmap(const BufferUnmapCmd& cmd) {
        auto* selfData = BufferObjects().Get(cmd.selfId);
        ASSERT(selfData != nullptr);

        selfData->mappedData = nullptr;

        return true;
    }

    bool Server::HandleBufferMapAsync(const char** commands, size_t* size) {
        // These requests are just forwarded to the buffer, with userdata containing what the
        // client will require in the return command.
        BufferMapAsyncCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        ObjectId bufferId = cmd.bufferId;
        uint32_t requestSerial = cmd.requestSerial;
        uint32_t requestSize = cmd.size;
        uint32_t requestStart = cmd.start;
        bool isWrite = cmd.isWrite;

        // The null object isn't valid as `self`
        if (bufferId == 0) {
            return false;
        }

        auto* buffer = BufferObjects().Get(bufferId);
        if (buffer == nullptr) {
            return false;
        }

        auto* data = new MapUserdata;
        data->server = this;
        data->buffer = ObjectHandle{bufferId, buffer->serial};
        data->requestSerial = requestSerial;
        data->size = requestSize;
        data->isWrite = isWrite;

        auto userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data));

        if (!buffer->valid) {
            // Fake the buffer returning a failure, data will be freed in this call.
            if (isWrite) {
                ForwardBufferMapWriteAsync(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
            } else {
                ForwardBufferMapReadAsync(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
            }
            return true;
        }

        if (isWrite) {
            mProcs.bufferMapWriteAsync(buffer->handle, requestStart, requestSize,
                                       ForwardBufferMapWriteAsync, userdata);
        } else {
            mProcs.bufferMapReadAsync(buffer->handle, requestStart, requestSize,
                                      ForwardBufferMapReadAsync, userdata);
        }

        return true;
    }

    bool Server::HandleBufferUpdateMappedData(const char** commands, size_t* size) {
        BufferUpdateMappedDataCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        ObjectId bufferId = cmd.bufferId;
        size_t dataLength = cmd.dataLength;

        // The null object isn't valid as `self`
        if (bufferId == 0) {
            return false;
        }

        auto* buffer = BufferObjects().Get(bufferId);
        if (buffer == nullptr || !buffer->valid || buffer->mappedData == nullptr ||
            buffer->mappedDataSize != dataLength) {
            return false;
        }

        DAWN_ASSERT(cmd.data != nullptr);

        memcpy(buffer->mappedData, cmd.data, dataLength);

        return true;
    }

    void Server::ForwardBufferMapReadAsync(dawnBufferMapAsyncStatus status,
                                           const void* ptr,
                                           dawnCallbackUserdata userdata) {
        auto data = reinterpret_cast<MapUserdata*>(static_cast<uintptr_t>(userdata));
        data->server->OnBufferMapReadAsyncCallback(status, ptr, data);
    }

    void Server::ForwardBufferMapWriteAsync(dawnBufferMapAsyncStatus status,
                                            void* ptr,
                                            dawnCallbackUserdata userdata) {
        auto data = reinterpret_cast<MapUserdata*>(static_cast<uintptr_t>(userdata));
        data->server->OnBufferMapWriteAsyncCallback(status, ptr, data);
    }

    void Server::OnBufferMapReadAsyncCallback(dawnBufferMapAsyncStatus status,
                                              const void* ptr,
                                              MapUserdata* userdata) {
        std::unique_ptr<MapUserdata> data(userdata);

        // Skip sending the callback if the buffer has already been destroyed.
        auto* bufferData = BufferObjects().Get(data->buffer.id);
        if (bufferData == nullptr || bufferData->serial != data->buffer.serial) {
            return;
        }

        ReturnBufferMapReadAsyncCallbackCmd cmd;
        cmd.buffer = data->buffer;
        cmd.requestSerial = data->requestSerial;
        cmd.status = status;
        cmd.dataLength = 0;
        cmd.data = reinterpret_cast<const uint8_t*>(ptr);

        if (status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
            cmd.dataLength = data->size;
        }

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);
    }

    void Server::OnBufferMapWriteAsyncCallback(dawnBufferMapAsyncStatus status,
                                               void* ptr,
                                               MapUserdata* userdata) {
        std::unique_ptr<MapUserdata> data(userdata);

        // Skip sending the callback if the buffer has already been destroyed.
        auto* bufferData = BufferObjects().Get(data->buffer.id);
        if (bufferData == nullptr || bufferData->serial != data->buffer.serial) {
            return;
        }

        ReturnBufferMapWriteAsyncCallbackCmd cmd;
        cmd.buffer = data->buffer;
        cmd.requestSerial = data->requestSerial;
        cmd.status = status;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);

        if (status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
            bufferData->mappedData = ptr;
            bufferData->mappedDataSize = data->size;
        }
    }

}}  // namespace dawn_wire::server
