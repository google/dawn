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

#include <limits>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

bool Server::PreHandleBufferUnmap(const BufferUnmapCmd& cmd) {
    auto* buffer = BufferObjects().Get(cmd.selfId);
    DAWN_ASSERT(buffer != nullptr);

    if (buffer->mappedAtCreation && !(buffer->usage & WGPUMapMode_Write)) {
        // This indicates the writeHandle is for mappedAtCreation only. Destroy on unmap
        // writeHandle could have possibly been deleted if buffer is already destroyed so we
        // don't assert it's non-null
        buffer->writeHandle = nullptr;
    }

    buffer->mapWriteState = BufferMapWriteState::Unmapped;

    return true;
}

bool Server::PreHandleBufferDestroy(const BufferDestroyCmd& cmd) {
    // Destroying a buffer does an implicit unmapping.
    auto* buffer = BufferObjects().Get(cmd.selfId);
    DAWN_ASSERT(buffer != nullptr);

    // The buffer was destroyed. Clear the Read/WriteHandle.
    buffer->readHandle = nullptr;
    buffer->writeHandle = nullptr;
    buffer->mapWriteState = BufferMapWriteState::Unmapped;

    return true;
}

bool Server::DoBufferMapAsync(ObjectId bufferId,
                              uint64_t requestSerial,
                              WGPUMapModeFlags mode,
                              uint64_t offset64,
                              uint64_t size64) {
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

    std::unique_ptr<MapUserdata> userdata = MakeUserdata<MapUserdata>();
    userdata->buffer = ObjectHandle{bufferId, buffer->generation};
    userdata->bufferObj = buffer->handle;
    userdata->requestSerial = requestSerial;
    userdata->mode = mode;

    // Make sure that the deserialized offset and size are no larger than
    // std::numeric_limits<size_t>::max() so that they are CPU-addressable, and size is not
    // WGPU_WHOLE_MAP_SIZE, which is by definition std::numeric_limits<size_t>::max(). Since
    // client does the default size computation, we should always have a valid actual size here
    // in server. All other invalid actual size can be caught by dawn native side validation.
    if (offset64 > std::numeric_limits<size_t>::max() || size64 >= WGPU_WHOLE_MAP_SIZE) {
        OnBufferMapAsyncCallback(userdata.get(), WGPUBufferMapAsyncStatus_Error);
        return true;
    }

    size_t offset = static_cast<size_t>(offset64);
    size_t size = static_cast<size_t>(size64);

    userdata->offset = offset;
    userdata->size = size;

    mProcs.bufferMapAsync(buffer->handle, mode, offset, size,
                          ForwardToServer<&Server::OnBufferMapAsyncCallback>, userdata.release());

    return true;
}

bool Server::DoDeviceCreateBuffer(ObjectId deviceId,
                                  const WGPUBufferDescriptor* descriptor,
                                  ObjectHandle bufferResult,
                                  uint64_t readHandleCreateInfoLength,
                                  const uint8_t* readHandleCreateInfo,
                                  uint64_t writeHandleCreateInfoLength,
                                  const uint8_t* writeHandleCreateInfo) {
    auto* device = DeviceObjects().Get(deviceId);
    if (device == nullptr) {
        return false;
    }

    // Create and register the buffer object.
    auto* resultData = BufferObjects().Allocate(bufferResult);
    if (resultData == nullptr) {
        return false;
    }
    resultData->generation = bufferResult.generation;
    resultData->handle = mProcs.deviceCreateBuffer(device->handle, descriptor);
    resultData->usage = descriptor->usage;
    resultData->mappedAtCreation = descriptor->mappedAtCreation;

    // isReadMode and isWriteMode could be true at the same time if usage contains
    // WGPUMapMode_Read and buffer is mappedAtCreation
    bool isReadMode = descriptor->usage & WGPUMapMode_Read;
    bool isWriteMode = descriptor->usage & WGPUMapMode_Write || descriptor->mappedAtCreation;

    // This is the size of data deserialized from the command stream to create the read/write
    // handle, which must be CPU-addressable.
    if (readHandleCreateInfoLength > std::numeric_limits<size_t>::max() ||
        writeHandleCreateInfoLength > std::numeric_limits<size_t>::max() ||
        readHandleCreateInfoLength >
            std::numeric_limits<size_t>::max() - writeHandleCreateInfoLength) {
        return false;
    }

    if (isWriteMode) {
        MemoryTransferService::WriteHandle* writeHandle = nullptr;
        // Deserialize metadata produced from the client to create a companion server handle.
        if (!mMemoryTransferService->DeserializeWriteHandle(
                writeHandleCreateInfo, static_cast<size_t>(writeHandleCreateInfoLength),
                &writeHandle)) {
            return false;
        }
        ASSERT(writeHandle != nullptr);
        resultData->writeHandle.reset(writeHandle);
        writeHandle->SetDataLength(descriptor->size);

        if (descriptor->mappedAtCreation) {
            void* mapping = mProcs.bufferGetMappedRange(resultData->handle, 0, descriptor->size);
            if (mapping == nullptr) {
                // A zero mapping is used to indicate an allocation error of an error buffer.
                // This is a valid case and isn't fatal. Remember the buffer is an error so as
                // to skip subsequent mapping operations.
                resultData->mapWriteState = BufferMapWriteState::MapError;
                return true;
            }
            ASSERT(mapping != nullptr);
            writeHandle->SetTarget(mapping);

            resultData->mapWriteState = BufferMapWriteState::Mapped;
        }
    }

    if (isReadMode) {
        MemoryTransferService::ReadHandle* readHandle = nullptr;
        // Deserialize metadata produced from the client to create a companion server handle.
        if (!mMemoryTransferService->DeserializeReadHandle(
                readHandleCreateInfo, static_cast<size_t>(readHandleCreateInfoLength),
                &readHandle)) {
            return false;
        }
        ASSERT(readHandle != nullptr);

        resultData->readHandle.reset(readHandle);
    }

    return true;
}

bool Server::DoBufferUpdateMappedData(ObjectId bufferId,
                                      uint64_t writeDataUpdateInfoLength,
                                      const uint8_t* writeDataUpdateInfo,
                                      uint64_t offset,
                                      uint64_t size) {
    // The null object isn't valid as `self`
    if (bufferId == 0) {
        return false;
    }

    if (writeDataUpdateInfoLength > std::numeric_limits<size_t>::max() ||
        offset > std::numeric_limits<size_t>::max() || size > std::numeric_limits<size_t>::max()) {
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
    return buffer->writeHandle->DeserializeDataUpdate(
        writeDataUpdateInfo, static_cast<size_t>(writeDataUpdateInfoLength),
        static_cast<size_t>(offset), static_cast<size_t>(size));
}

void Server::OnBufferMapAsyncCallback(MapUserdata* data, WGPUBufferMapAsyncStatus status) {
    // Skip sending the callback if the buffer has already been destroyed.
    auto* bufferData = BufferObjects().Get(data->buffer.id);
    if (bufferData == nullptr || bufferData->generation != data->buffer.generation) {
        return;
    }

    bool isRead = data->mode & WGPUMapMode_Read;
    bool isSuccess = status == WGPUBufferMapAsyncStatus_Success;

    ReturnBufferMapAsyncCallbackCmd cmd;
    cmd.buffer = data->buffer;
    cmd.requestSerial = data->requestSerial;
    cmd.status = status;
    cmd.readDataUpdateInfoLength = 0;
    cmd.readDataUpdateInfo = nullptr;

    const void* readData = nullptr;
    size_t readDataUpdateInfoLength = 0;
    if (isSuccess) {
        if (isRead) {
            // Get the serialization size of the message to initialize ReadHandle data.
            readData = mProcs.bufferGetConstMappedRange(data->bufferObj, data->offset, data->size);
            readDataUpdateInfoLength =
                bufferData->readHandle->SizeOfSerializeDataUpdate(data->offset, data->size);
            cmd.readDataUpdateInfoLength = readDataUpdateInfoLength;
        } else {
            ASSERT(data->mode & WGPUMapMode_Write);
            // The in-flight map request returned successfully.
            bufferData->mapWriteState = BufferMapWriteState::Mapped;
            // Set the target of the WriteHandle to the mapped buffer data.
            // writeHandle Target always refers to the buffer base address.
            // but we call getMappedRange exactly with the range of data that is potentially
            // modified (i.e. we don't want getMappedRange(0, wholeBufferSize) if only a
            // subset of the buffer is actually mapped) in case the implementation does some
            // range tracking.
            bufferData->writeHandle->SetTarget(static_cast<uint8_t*>(mProcs.bufferGetMappedRange(
                                                   data->bufferObj, data->offset, data->size)) -
                                               data->offset);
        }
    }

    SerializeCommand(cmd, CommandExtension{readDataUpdateInfoLength, [&](char* readHandleBuffer) {
                                               if (isSuccess && isRead) {
                                                   // The in-flight map request returned
                                                   // successfully.
                                                   bufferData->readHandle->SerializeDataUpdate(
                                                       readData, data->offset, data->size,
                                                       readHandleBuffer);
                                               }
                                           }});
}

}  // namespace dawn::wire::server
