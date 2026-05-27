// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <limits>
#include <memory>
#include <span>

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireResult.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::PreHandleDeviceCreateErrorBuffer(const DeviceCreateErrorBufferCmd& cmd) {
    // mappedAtCreation isn't implemented in CreateErrorBuffer.
    // The client blocks this, so we can treat it as a fatal wire error (for fuzzers).
    if (cmd.descriptor->mappedAtCreation) {
        return WireResult::FatalError;
    }

    return WireResult::Success;
}

WireResult Server::PreHandleBufferUnmap(const BufferUnmapCmd& cmd) {
    Known<WGPUBuffer> buffer;
    WIRE_TRY(Get(cmd.selfId, &buffer));

    buffer->mapState.Use([&](auto mapState) {
        if (buffer->mappedAtCreation && !(buffer->usage & WGPUBufferUsage_MapWrite)) {
            // This indicates the writeHandle is for mappedAtCreation only. Destroy on unmap
            // writeHandle could have possibly been deleted if buffer is already destroyed so we
            // don't assert it's non-null
            mapState->writeHandle = nullptr;
        }
    });

    return WireResult::Success;
}

WireResult Server::PreHandleBufferDestroy(const BufferDestroyCmd& cmd) {
    // Destroying a buffer does an implicit unmapping.
    Known<WGPUBuffer> buffer;
    WIRE_TRY(Get(cmd.selfId, &buffer));

    // The buffer was destroyed. Clear the Read/WriteHandle.
    buffer->mapState.Use([](auto mapState) {
        mapState->readHandle = nullptr;
        mapState->writeHandle = nullptr;
    });

    return WireResult::Success;
}

WireResult Server::DoBufferMapAsync(Known<WGPUBuffer> buffer,
                                    ObjectHandle eventManager,
                                    WGPUFuture future,
                                    WGPUMapMode mode,
                                    size_t offset,
                                    size_t size) {
    // These requests are just forwarded to the buffer, with userdata containing what the
    // client will require in the return command.
    std::unique_ptr<MapUserdata> userdata = MakeUserdata<MapUserdata>();
    userdata->buffer = buffer.AsHandle();
    userdata->eventManager = eventManager;
    userdata->bufferObj = buffer->handle;
    userdata->future = future;
    userdata->mode = mode;

    // Make sure that the size is not WGPU_WHOLE_MAP_SIZE because we want the client to give us an
    // explicit size (so we don't have to handle the defaulting here). The client needs to track
    // the size of the buffer and can do the default size computation.
    if (size >= WGPU_WHOLE_MAP_SIZE) {
        return WireResult::FatalError;
    }

    userdata->offset = offset;
    userdata->size = size;

    mProcs->bufferMapAsync(
        buffer->handle, mode, offset, size,
        MakeCallbackInfo<WGPUBufferMapCallbackInfo, &Server::OnBufferMapAsyncCallback>(
            userdata.release()));

    return WireResult::Success;
}

WireResult Server::DoDeviceCreateBuffer(Known<WGPUDevice> device,
                                        const WGPUBufferDescriptor* descriptor,
                                        ObjectHandle bufferHandle,
                                        size_t readHandleCreateInfoLength,
                                        const uint8_t* readHandleCreateInfo,
                                        size_t writeHandleCreateInfoLength,
                                        const uint8_t* writeHandleCreateInfo) {
    // Create and register the buffer object.
    Reserved<WGPUBuffer> buffer;
    WIRE_TRY(Allocate(&buffer, bufferHandle));
    buffer->handle = mProcs->deviceCreateBuffer(device->handle, descriptor);
    buffer->usage = descriptor->usage;
    buffer->mappedAtCreation = (descriptor->mappedAtCreation != 0u);

    // isReadMode and isWriteMode could be true at the same time if usage contains
    // WGPUBufferUsage_MapRead and buffer is mappedAtCreation
    bool isReadMode = (descriptor->usage & WGPUBufferUsage_MapRead) != 0u;
    bool isWriteMode = ((descriptor->usage & WGPUBufferUsage_MapWrite) != 0u) ||
                       (descriptor->mappedAtCreation != 0u);

    return buffer->mapState.Use([&](auto mapState) {
        if (isWriteMode) {
            if (buffer->handle == nullptr) {
                DAWN_ASSERT(descriptor->mappedAtCreation);
                // A null buffer indicates that mapping-at-creation failed inside createBuffer.
                // Unmark the buffer as allocated so we will skip freeing it.
                buffer->state = AllocationState::Reserved;
                return WireResult::Success;
            }

            MemoryTransferService::WriteHandle* writeHandle = nullptr;
            // Deserialize metadata produced from the client to create a companion server handle.
            if (!mMemoryTransferService->DeserializeWriteHandle(
                    writeHandleCreateInfo, writeHandleCreateInfoLength, &writeHandle)) {
                return WireResult::FatalError;
            }
            DAWN_ASSERT(writeHandle != nullptr);
            mapState->writeHandle.reset(writeHandle);
        }

        if (isReadMode) {
            MemoryTransferService::ReadHandle* readHandle = nullptr;
            // Deserialize metadata produced from the client to create a companion server handle.
            if (!mMemoryTransferService->DeserializeReadHandle(
                    readHandleCreateInfo, readHandleCreateInfoLength, &readHandle)) {
                return WireResult::FatalError;
            }
            DAWN_ASSERT(readHandle != nullptr);
            mapState->readHandle.reset(readHandle);
        }

        return WireResult::Success;
    });
}

WireResult Server::DoBufferUpdateMappedData(Known<WGPUBuffer> buffer,
                                            size_t writeDataUpdateInfoLength,
                                            const uint8_t* writeDataUpdateInfo,
                                            size_t offset,
                                            size_t size) {
    return buffer->mapState.Use([&](auto mapState) {
        uint8_t* mappedData =
            static_cast<uint8_t*>(mProcs->bufferGetMappedRange(buffer->handle, offset, size));

        // There are a few valid reasons why getting the mapped range would fail here:
        //  - The buffer was implicitly unmapped because of a device.Destroy() call.
        //  - The buffer was an error buffer created just to replace an OOM mappedAtCreation buffer.
        // Unfortunately validating exactly that the failure is due to a valid reason and not
        // another is difficult, so we return WireResult::Success even for misuses of the wire
        // protocol (like a size being larger than the buffer's size, etc).
        if (mappedData == nullptr) {
            return WireResult::Success;
        }

        std::span<uint8_t> mappedRange = {mappedData, size};

        // However it is easy to check for misuses of the wire protocol to UpdateMappedData without
        // a WriteHandle.
        if (!mapState->writeHandle) {
            return WireResult::FatalError;
        }

        // Deserialize the flush info and flush updated data from the handle into mappedRange.
        std::span<const uint8_t> writeDataUpdateInfoSpan(writeDataUpdateInfo,
                                                         writeDataUpdateInfoLength);
        if (!mapState->writeHandle->DeserializeDataUpdate(writeDataUpdateInfoSpan, mappedRange,
                                                          offset)) {
            return WireResult::FatalError;
        }
        return WireResult::Success;
    });
}

void Server::OnBufferMapAsyncCallback(MapUserdata* data,
                                      WGPUMapAsyncStatus status,
                                      WGPUStringView message) {
    // Skip sending the callback if the buffer has already been destroyed.
    Known<WGPUBuffer> buffer;
    if (Get(data->buffer.id, &buffer) != WireResult::Success ||
        buffer->generation != data->buffer.generation) {
        return;
    }

    bool isSuccess = status == WGPUMapAsyncStatus_Success;

    ReturnBufferMapAsyncCallbackCmd cmd = {};
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;
    cmd.message = message;
    // Set the pointer length, but the pointed-to data itself won't be serialized as usual (due
    // to skip_serialize). Instead, the custom CommandExtension below fills that memory.
    cmd.readDataUpdateInfoLength = 0;
    cmd.readDataUpdateInfo = nullptr;  // Skipped by skip_serialize.

    if (!isSuccess) {
        SerializeCommand(cmd);
        return;
    }

    switch (data->mode) {
        case WGPUMapMode_Read: {
            buffer->mapState.Use([&](auto mapState) {
                const void* readData =
                    mProcs->bufferGetConstMappedRange(data->bufferObj, data->offset, data->size);
                size_t readDataUpdateInfoLength =
                    mapState->readHandle->SizeOfSerializeDataUpdate(data->offset, data->size);
                cmd.readDataUpdateInfoLength = readDataUpdateInfoLength;
                SerializeCommand(
                    cmd,
                    // Extensions to replace fields skipped by skip_serialize.
                    CommandExtension{readDataUpdateInfoLength, [&](char* readHandleBuffer) {
                                         // The in-flight map request returned successfully.
                                         std::span<const uint8_t> readDataSpan(
                                             static_cast<const uint8_t*>(readData), data->size);
                                         std::span<char> readHandleBufferSpan(
                                             static_cast<char*>(readHandleBuffer), data->size);
                                         mapState->readHandle->SerializeDataUpdate(
                                             readDataSpan, data->offset, readHandleBufferSpan);
                                     }});
            });
            break;
        }
        case WGPUMapMode_Write: {
            SerializeCommand(cmd);
            break;
        }
        default:
            // If we are not one of the two possible modes, we should never succeed.
            DAWN_UNREACHABLE();
            break;
    }
}

}  // namespace dawn::wire::server
