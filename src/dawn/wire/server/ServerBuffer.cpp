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

WireResult Server::PreHandleBufferUnmap(const BufferUnmapCmd& cmd) {
    Known<WGPUBuffer> buffer;
    WIRE_TRY(BufferObjects().Get(cmd.selfId, &buffer));

    if (buffer->mappedAtCreation && !(buffer->usage & WGPUMapMode_Write)) {
        // This indicates the writeHandle is for mappedAtCreation only. Destroy on unmap
        // writeHandle could have possibly been deleted if buffer is already destroyed so we
        // don't assert it's non-null
        buffer->writeHandle = nullptr;
    }

    buffer->mapWriteState = BufferMapWriteState::Unmapped;

    return WireResult::Success;
}

WireResult Server::PreHandleBufferDestroy(const BufferDestroyCmd& cmd) {
    // Destroying a buffer does an implicit unmapping.
    Known<WGPUBuffer> buffer;
    WIRE_TRY(BufferObjects().Get(cmd.selfId, &buffer));

    // The buffer was destroyed. Clear the Read/WriteHandle.
    buffer->readHandle = nullptr;
    buffer->writeHandle = nullptr;
    buffer->mapWriteState = BufferMapWriteState::Unmapped;

    return WireResult::Success;
}

WireResult Server::DoBufferMapAsync(Known<WGPUBuffer> buffer,
                                    WGPUFuture future,
                                    WGPUMapModeFlags mode,
                                    uint64_t offset64,
                                    uint64_t size64) {
    // These requests are just forwarded to the buffer, with userdata containing what the
    // client will require in the return command.
    std::unique_ptr<MapUserdata> userdata = MakeUserdata<MapUserdata>();
    userdata->buffer = buffer.AsHandle();
    userdata->bufferObj = buffer->handle;
    userdata->future = future;
    userdata->mode = mode;

    // Make sure that the deserialized offset and size are no larger than
    // std::numeric_limits<size_t>::max() so that they are CPU-addressable, and size is not
    // WGPU_WHOLE_MAP_SIZE, which is by definition std::numeric_limits<size_t>::max(). Since
    // client does the default size computation, we should always have a valid actual size here
    // in server. All other invalid actual size can be caught by dawn native side validation.
    if (offset64 > std::numeric_limits<size_t>::max()) {
        OnBufferMapAsyncCallback(userdata.get(), WGPUBufferMapAsyncStatus_OffsetOutOfRange);
        return WireResult::Success;
    }
    if (size64 >= WGPU_WHOLE_MAP_SIZE) {
        OnBufferMapAsyncCallback(userdata.get(), WGPUBufferMapAsyncStatus_SizeOutOfRange);
        return WireResult::Success;
    }

    size_t offset = static_cast<size_t>(offset64);
    size_t size = static_cast<size_t>(size64);

    userdata->offset = offset;
    userdata->size = size;

    mProcs.bufferMapAsync(buffer->handle, mode, offset, size,
                          ForwardToServer<&Server::OnBufferMapAsyncCallback>, userdata.release());

    return WireResult::Success;
}

WireResult Server::DoDeviceCreateBuffer(Known<WGPUDevice> device,
                                        const WGPUBufferDescriptor* descriptor,
                                        ObjectHandle bufferHandle,
                                        uint64_t readHandleCreateInfoLength,
                                        const uint8_t* readHandleCreateInfo,
                                        uint64_t writeHandleCreateInfoLength,
                                        const uint8_t* writeHandleCreateInfo) {
    // Create and register the buffer object.
    Known<WGPUBuffer> buffer;
    WIRE_TRY(BufferObjects().Allocate(&buffer, bufferHandle));
    buffer->handle = mProcs.deviceCreateBuffer(device->handle, descriptor);
    buffer->usage = descriptor->usage;
    buffer->mappedAtCreation = descriptor->mappedAtCreation;

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
        return WireResult::FatalError;
    }

    if (isWriteMode) {
        MemoryTransferService::WriteHandle* writeHandle = nullptr;
        // Deserialize metadata produced from the client to create a companion server handle.
        if (!mMemoryTransferService->DeserializeWriteHandle(
                writeHandleCreateInfo, static_cast<size_t>(writeHandleCreateInfoLength),
                &writeHandle)) {
            return WireResult::FatalError;
        }
        DAWN_ASSERT(writeHandle != nullptr);
        buffer->writeHandle.reset(writeHandle);
        writeHandle->SetDataLength(descriptor->size);

        if (descriptor->mappedAtCreation) {
            void* mapping = mProcs.bufferGetMappedRange(buffer->handle, 0, descriptor->size);
            if (mapping == nullptr) {
                // A zero mapping is used to indicate an allocation error of an error buffer.
                // This is a valid case and isn't fatal. Remember the buffer is an error so as
                // to skip subsequent mapping operations.
                buffer->mapWriteState = BufferMapWriteState::MapError;
                return WireResult::Success;
            }
            DAWN_ASSERT(mapping != nullptr);
            writeHandle->SetTarget(mapping);

            buffer->mapWriteState = BufferMapWriteState::Mapped;
        }
    }

    if (isReadMode) {
        MemoryTransferService::ReadHandle* readHandle = nullptr;
        // Deserialize metadata produced from the client to create a companion server handle.
        if (!mMemoryTransferService->DeserializeReadHandle(
                readHandleCreateInfo, static_cast<size_t>(readHandleCreateInfoLength),
                &readHandle)) {
            return WireResult::FatalError;
        }
        DAWN_ASSERT(readHandle != nullptr);

        buffer->readHandle.reset(readHandle);
    }

    return WireResult::Success;
}

WireResult Server::DoBufferUpdateMappedData(Known<WGPUBuffer> buffer,
                                            uint64_t writeDataUpdateInfoLength,
                                            const uint8_t* writeDataUpdateInfo,
                                            uint64_t offset,
                                            uint64_t size) {
    if (writeDataUpdateInfoLength > std::numeric_limits<size_t>::max() ||
        offset > std::numeric_limits<size_t>::max() || size > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    switch (buffer->mapWriteState) {
        case BufferMapWriteState::Unmapped:
            return WireResult::FatalError;
        case BufferMapWriteState::MapError:
            // The buffer is mapped but there was an error allocating mapped data.
            // Do not perform the memcpy.
            return WireResult::Success;
        case BufferMapWriteState::Mapped:
            break;
    }
    if (!buffer->writeHandle) {
        // This check is performed after the check for the MapError state. It is permissible
        // to Unmap and attempt to update mapped data of an error buffer.
        return WireResult::FatalError;
    }

    // Deserialize the flush info and flush updated data from the handle into the target
    // of the handle. The target is set via WriteHandle::SetTarget.
    if (!buffer->writeHandle->DeserializeDataUpdate(
            writeDataUpdateInfo, static_cast<size_t>(writeDataUpdateInfoLength),
            static_cast<size_t>(offset), static_cast<size_t>(size))) {
        return WireResult::FatalError;
    }
    return WireResult::Success;
}

void Server::OnBufferMapAsyncCallback(MapUserdata* data, WGPUBufferMapAsyncStatus status) {
    // Skip sending the callback if the buffer has already been destroyed.
    Known<WGPUBuffer> buffer;
    if (BufferObjects().Get(data->buffer.id, &buffer) != WireResult::Success ||
        buffer->generation != data->buffer.generation) {
        return;
    }

    bool isRead = data->mode & WGPUMapMode_Read;
    bool isSuccess = status == WGPUBufferMapAsyncStatus_Success;

    ReturnBufferMapAsyncCallbackCmd cmd;
    cmd.buffer = data->buffer;
    cmd.future = data->future;
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
                buffer->readHandle->SizeOfSerializeDataUpdate(data->offset, data->size);
            cmd.readDataUpdateInfoLength = readDataUpdateInfoLength;
        } else {
            DAWN_ASSERT(data->mode & WGPUMapMode_Write);
            // The in-flight map request returned successfully.
            buffer->mapWriteState = BufferMapWriteState::Mapped;
            // Set the target of the WriteHandle to the mapped buffer data.
            // writeHandle Target always refers to the buffer base address.
            // but we call getMappedRange exactly with the range of data that is potentially
            // modified (i.e. we don't want getMappedRange(0, wholeBufferSize) if only a
            // subset of the buffer is actually mapped) in case the implementation does some
            // range tracking.
            buffer->writeHandle->SetTarget(static_cast<uint8_t*>(mProcs.bufferGetMappedRange(
                                               data->bufferObj, data->offset, data->size)) -
                                           data->offset);
        }
    }

    SerializeCommand(cmd, CommandExtension{readDataUpdateInfoLength, [&](char* readHandleBuffer) {
                                               if (isSuccess && isRead) {
                                                   // The in-flight map request returned
                                                   // successfully.
                                                   buffer->readHandle->SerializeDataUpdate(
                                                       readData, data->offset, data->size,
                                                       readHandleBuffer);
                                               }
                                           }});
}

}  // namespace dawn::wire::server
