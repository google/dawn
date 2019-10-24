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
        auto* buffer = BufferObjects().Get(cmd.selfId);
        DAWN_ASSERT(buffer != nullptr);

        // The buffer was unmapped. Clear the Read/WriteHandle.
        buffer->readHandle = nullptr;
        buffer->writeHandle = nullptr;
        buffer->mapWriteState = BufferMapWriteState::Unmapped;

        return true;
    }

    bool Server::DoBufferMapAsync(ObjectId bufferId,
                                  uint32_t requestSerial,
                                  bool isWrite,
                                  uint64_t handleCreateInfoLength,
                                  const uint8_t* handleCreateInfo) {
        // These requests are just forwarded to the buffer, with userdata containing what the
        // client will require in the return command.

        // The null object isn't valid as `self`
        if (bufferId == 0) {
            return false;
        }

        auto* buffer = BufferObjects().Get(bufferId);
        if (buffer == nullptr) {
            return false;
        }

        if (handleCreateInfoLength > std::numeric_limits<size_t>::max()) {
            // This is the size of data deserialized from the command stream, which must be
            // CPU-addressable.
            return false;
        }

        std::unique_ptr<MapUserdata> userdata = std::make_unique<MapUserdata>();
        userdata->server = this;
        userdata->buffer = ObjectHandle{bufferId, buffer->serial};
        userdata->requestSerial = requestSerial;

        // The handle will point to the mapped memory or staging memory for the mapping.
        // Store it on the map request.
        if (isWrite) {
            // Deserialize metadata produced from the client to create a companion server handle.
            MemoryTransferService::WriteHandle* writeHandle = nullptr;
            if (!mMemoryTransferService->DeserializeWriteHandle(
                    handleCreateInfo, static_cast<size_t>(handleCreateInfoLength), &writeHandle)) {
                return false;
            }
            ASSERT(writeHandle != nullptr);

            userdata->writeHandle =
                std::unique_ptr<MemoryTransferService::WriteHandle>(writeHandle);
            mProcs.bufferMapWriteAsync(buffer->handle, ForwardBufferMapWriteAsync,
                                       userdata.release());
        } else {
            // Deserialize metadata produced from the client to create a companion server handle.
            MemoryTransferService::ReadHandle* readHandle = nullptr;
            if (!mMemoryTransferService->DeserializeReadHandle(
                    handleCreateInfo, static_cast<size_t>(handleCreateInfoLength), &readHandle)) {
                return false;
            }
            ASSERT(readHandle != nullptr);

            userdata->readHandle = std::unique_ptr<MemoryTransferService::ReadHandle>(readHandle);
            mProcs.bufferMapReadAsync(buffer->handle, ForwardBufferMapReadAsync,
                                      userdata.release());
        }

        return true;
    }

    bool Server::DoDeviceCreateBufferMapped(WGPUDevice device,
                                            const WGPUBufferDescriptor* descriptor,
                                            ObjectHandle bufferResult,
                                            uint64_t handleCreateInfoLength,
                                            const uint8_t* handleCreateInfo) {
        if (handleCreateInfoLength > std::numeric_limits<size_t>::max()) {
            // This is the size of data deserialized from the command stream, which must be
            // CPU-addressable.
            return false;
        }

        auto* resultData = BufferObjects().Allocate(bufferResult.id);
        if (resultData == nullptr) {
            return false;
        }
        resultData->serial = bufferResult.serial;

        WGPUCreateBufferMappedResult result = mProcs.deviceCreateBufferMapped(device, descriptor);
        ASSERT(result.buffer != nullptr);
        if (result.data == nullptr && result.dataLength != 0) {
            // Non-zero dataLength but null data is used to indicate an allocation error.
            // Don't return false because this is not fatal. result.buffer is an ErrorBuffer
            // and subsequent operations will be errors.
            // This should only happen when fuzzing with the Null backend.
            resultData->mapWriteState = BufferMapWriteState::MapError;
        } else {
            // Deserialize metadata produced from the client to create a companion server handle.
            MemoryTransferService::WriteHandle* writeHandle = nullptr;
            if (!mMemoryTransferService->DeserializeWriteHandle(
                    handleCreateInfo, static_cast<size_t>(handleCreateInfoLength), &writeHandle)) {
                return false;
            }
            ASSERT(writeHandle != nullptr);

            // Set the target of the WriteHandle to the mapped GPU memory.
            writeHandle->SetTarget(result.data, result.dataLength);

            // The buffer is mapped and has a valid mappedData pointer.
            // The buffer may still be an error with fake staging data.
            resultData->mapWriteState = BufferMapWriteState::Mapped;
            resultData->writeHandle =
                std::unique_ptr<MemoryTransferService::WriteHandle>(writeHandle);
        }
        resultData->handle = result.buffer;

        return true;
    }

    bool Server::DoDeviceCreateBufferMappedAsync(WGPUDevice device,
                                                 const WGPUBufferDescriptor* descriptor,
                                                 uint32_t requestSerial,
                                                 ObjectHandle bufferResult,
                                                 uint64_t handleCreateInfoLength,
                                                 const uint8_t* handleCreateInfo) {
        if (!DoDeviceCreateBufferMapped(device, descriptor, bufferResult, handleCreateInfoLength,
                                        handleCreateInfo)) {
            return false;
        }

        auto* bufferData = BufferObjects().Get(bufferResult.id);
        ASSERT(bufferData != nullptr);

        ReturnBufferMapWriteAsyncCallbackCmd cmd;
        cmd.buffer = ObjectHandle{bufferResult.id, bufferResult.serial};
        cmd.requestSerial = requestSerial;
        cmd.status = bufferData->mapWriteState == BufferMapWriteState::Mapped
                         ? WGPUBufferMapAsyncStatus_Success
                         : WGPUBufferMapAsyncStatus_Error;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);

        return true;
    }

    bool Server::DoBufferSetSubDataInternal(ObjectId bufferId,
                                            uint64_t start,
                                            uint64_t offset,
                                            const uint8_t* data) {
        // The null object isn't valid as `self`
        if (bufferId == 0) {
            return false;
        }

        auto* buffer = BufferObjects().Get(bufferId);
        if (buffer == nullptr) {
            return false;
        }

        mProcs.bufferSetSubData(buffer->handle, start, offset, data);
        return true;
    }

    bool Server::DoBufferUpdateMappedData(ObjectId bufferId,
                                          uint64_t writeFlushInfoLength,
                                          const uint8_t* writeFlushInfo) {
        // The null object isn't valid as `self`
        if (bufferId == 0) {
            return false;
        }

        if (writeFlushInfoLength > std::numeric_limits<size_t>::max()) {
            return false;
        }

        auto* buffer = BufferObjects().Get(bufferId);
        if (buffer == nullptr) {
            return false;
        }
        switch (buffer->mapWriteState) {
            case BufferMapWriteState::Unmapped:
                return false;
            case BufferMapWriteState::MapError:
                // The buffer is mapped but there was an error allocating mapped data.
                // Do not perform the memcpy.
                return true;
            case BufferMapWriteState::Mapped:
                break;
        }
        if (!buffer->writeHandle) {
            // This check is performed after the check for the MapError state. It is permissible
            // to Unmap and attempt to update mapped data of an error buffer.
            return false;
        }
        // Deserialize the flush info and flush updated data from the handle into the target
        // of the handle. The target is set via WriteHandle::SetTarget.
        return buffer->writeHandle->DeserializeFlush(writeFlushInfo,
                                                     static_cast<size_t>(writeFlushInfoLength));
    }

    void Server::ForwardBufferMapReadAsync(WGPUBufferMapAsyncStatus status,
                                           const void* ptr,
                                           uint64_t dataLength,
                                           void* userdata) {
        auto data = static_cast<MapUserdata*>(userdata);
        data->server->OnBufferMapReadAsyncCallback(status, ptr, dataLength, data);
    }

    void Server::ForwardBufferMapWriteAsync(WGPUBufferMapAsyncStatus status,
                                            void* ptr,
                                            uint64_t dataLength,
                                            void* userdata) {
        auto data = static_cast<MapUserdata*>(userdata);
        data->server->OnBufferMapWriteAsyncCallback(status, ptr, dataLength, data);
    }

    void Server::OnBufferMapReadAsyncCallback(WGPUBufferMapAsyncStatus status,
                                              const void* ptr,
                                              uint64_t dataLength,
                                              MapUserdata* userdata) {
        std::unique_ptr<MapUserdata> data(userdata);

        // Skip sending the callback if the buffer has already been destroyed.
        auto* bufferData = BufferObjects().Get(data->buffer.id);
        if (bufferData == nullptr || bufferData->serial != data->buffer.serial) {
            return;
        }

        size_t initialDataInfoLength = 0;
        if (status == WGPUBufferMapAsyncStatus_Success) {
            // Get the serialization size of the message to initialize ReadHandle data.
            initialDataInfoLength = data->readHandle->SerializeInitialDataSize(ptr, dataLength);
        } else {
            dataLength = 0;
        }

        ReturnBufferMapReadAsyncCallbackCmd cmd;
        cmd.buffer = data->buffer;
        cmd.requestSerial = data->requestSerial;
        cmd.status = status;
        cmd.initialDataInfoLength = initialDataInfoLength;
        cmd.initialDataInfo = nullptr;

        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = commandSize + initialDataInfoLength;
        char* allocatedBuffer = static_cast<char*>(GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);

        if (status == WGPUBufferMapAsyncStatus_Success) {
            // Serialize the initialization message into the space after the command.
            data->readHandle->SerializeInitialData(ptr, dataLength, allocatedBuffer + commandSize);

            // The in-flight map request returned successfully.
            // Move the ReadHandle so it is owned by the buffer.
            bufferData->readHandle = std::move(data->readHandle);
        }
    }

    void Server::OnBufferMapWriteAsyncCallback(WGPUBufferMapAsyncStatus status,
                                               void* ptr,
                                               uint64_t dataLength,
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

        if (status == WGPUBufferMapAsyncStatus_Success) {
            // The in-flight map request returned successfully.
            // Move the WriteHandle so it is owned by the buffer.
            bufferData->writeHandle = std::move(data->writeHandle);
            bufferData->mapWriteState = BufferMapWriteState::Mapped;
            // Set the target of the WriteHandle to the mapped buffer data.
            bufferData->writeHandle->SetTarget(ptr, dataLength);
        }
    }

}}  // namespace dawn_wire::server
