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

#include "dawn/wire/WireCmd_autogen.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/BufferConsumer_impl.h"
#include "src/dawn/wire/WireResult.h"
#include "src/dawn/wire/server/Server.h"
#include "src/utils/assert.h"

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
        if (buffer->mappedAtCreation &&
            !(buffer->usage & (WGPUBufferUsage_MapWrite | WGPUBufferUsage_MapRead))) {
            // This indicates the memoryHandle is for mappedAtCreation only. Destroy on unmap
            // memoryHandle could have possibly been deleted if buffer is already destroyed so we
            // don't assert it's non-null
            mapState->memoryHandle = nullptr;
        }
    });

    return WireResult::Success;
}

WireResult Server::PreHandleBufferDestroy(const BufferDestroyCmd& cmd) {
    // Destroying a buffer does an implicit unmapping.
    Known<WGPUBuffer> buffer;
    WIRE_TRY(Get(cmd.selfId, &buffer));

    // The buffer was destroyed. Clear the MemoryHandle.
    buffer->mapState.Use([](auto mapState) { mapState->memoryHandle = nullptr; });

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
                                        size_t memoryHandleCreateInfoLength,
                                        const std::byte* memoryHandleCreateInfo) {
    // Create and register the buffer object.
    Reserved<WGPUBuffer> buffer;
    WIRE_TRY(Allocate(&buffer, bufferHandle));
    buffer->handle = mProcs->deviceCreateBuffer(device->handle, descriptor);
    buffer->usage = descriptor->usage;
    buffer->mappedAtCreation = (descriptor->mappedAtCreation != 0u);

    // A null buffer indicates that mapping-at-creation failed inside createBuffer. Unmark the
    // buffer as allocated so we will skip freeing it.
    if (buffer->handle == nullptr) {
        DAWN_ASSERT(descriptor->mappedAtCreation);
        buffer->state = AllocationState::Reserved;
        return WireResult::Success;
    }

    bool isMappable =
        descriptor->mappedAtCreation != 0u ||
        (descriptor->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)) != 0u;

    return buffer->mapState.Use([&](auto mapState) {
        if (isMappable) {
            // Deserialize metadata produced from the client to create a companion server handle.
            // TODO(https://crbug.com/526533386): Spanify the input API of dawn::wire::server.
            Span<const std::byte> DAWN_UNSAFE_TODO(
                creationData{memoryHandleCreateInfo, memoryHandleCreateInfoLength});
            mapState->memoryHandle = mMemoryTransferService->DeserializeMemoryHandle(creationData);
            if (mapState->memoryHandle == nullptr) {
                return WireResult::FatalError;
            }
        }
        return WireResult::Success;
    });
}

WireResult Server::DoBufferUpdateMappedData(Known<WGPUBuffer> buffer,
                                            size_t writeDataUpdateInfoLength,
                                            const std::byte* writeDataUpdateInfo,
                                            size_t offset,
                                            size_t size) {
    return buffer->mapState.Use([&](auto mapState) {
        if (size == WGPU_WHOLE_MAP_SIZE) {
            return WireResult::FatalError;
        }
        std::byte* mappedData =
            static_cast<std::byte*>(mProcs->bufferGetMappedRange(buffer->handle, offset, size));

        // There are a few valid reasons why getting the mapped range would fail here:
        //  - The buffer was implicitly unmapped because of a device.Destroy() call.
        //  - The buffer was an error buffer created just to replace an OOM mappedAtCreation buffer.
        // Unfortunately validating exactly that the failure is due to a valid reason and not
        // another is difficult, so we return WireResult::Success even for misuses of the wire
        // protocol (like a size being larger than the buffer's size, etc).
        if (mappedData == nullptr) {
            return WireResult::Success;
        }

        // SAFETY: If GetMappedRange returns non-null, it points to at least `size` valid bytes.
        Span<std::byte> DAWN_UNSAFE_BUFFERS(mappedRange{mappedData, size});

        // However it is easy to check for misuses of the wire protocol to UpdateMappedData without
        // a MemoryHandle.
        if (!mapState->memoryHandle) {
            return WireResult::FatalError;
        }

        // Deserialize the flush info and flush updated data from the handle into mappedRange.
        // TODO(https://crbug.com/526533386): Spanify the input API of dawn::wire::server.
        Span<const std::byte> DAWN_UNSAFE_TODO(
            writeDataUpdateInfoSpan(writeDataUpdateInfo, writeDataUpdateInfoLength));
        if (!mapState->memoryHandle->DeserializeDataUpdate(writeDataUpdateInfoSpan, offset, size,
                                                           mappedRange)) {
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
                DAWN_ASSERT(data->size != WGPU_WHOLE_MAP_SIZE);  // Validated in DoBufferMapAsync.
                const std::byte* mappedData = static_cast<const std::byte*>(
                    mProcs->bufferGetConstMappedRange(data->bufferObj, data->offset, data->size));

                // SAFETY: If GetConstMappedRange with size != WGPU_WHOLE_MAP_SIZE returns non-null,
                // it points to at least `size` valid bytes.
                Span<const std::byte> DAWN_UNSAFE_BUFFERS(mappedRange{mappedData, data->size});

                size_t dataUpdateInfoLength =
                    mapState->memoryHandle->GetSerializeDataUpdateSize(data->offset, data->size);
                cmd.readDataUpdateInfoLength = dataUpdateInfoLength;
                SerializeCommand(
                    cmd,
                    // Extensions to replace fields skipped by skip_serialize.
                    CommandExtension{dataUpdateInfoLength, [&](Span<std::byte> serializeBuffer) {
                                         // The in-flight map request returned successfully.
                                         mapState->memoryHandle->SerializeDataUpdate(
                                             serializeBuffer, data->offset, data->size,
                                             mappedRange);
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
