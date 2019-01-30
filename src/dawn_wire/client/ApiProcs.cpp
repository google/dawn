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

#include "dawn_wire/client/ApiObjects.h"
#include "dawn_wire/client/ApiProcs_autogen.h"
#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    void ClientBufferMapReadAsync(dawnBuffer cBuffer,
                                  uint32_t start,
                                  uint32_t size,
                                  dawnBufferMapReadCallback callback,
                                  dawnCallbackUserdata userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        uint32_t serial = buffer->requestSerial++;
        ASSERT(buffer->requests.find(serial) == buffer->requests.end());

        Buffer::MapRequestData request;
        request.readCallback = callback;
        request.userdata = userdata;
        request.size = size;
        request.isWrite = false;
        buffer->requests[serial] = request;

        BufferMapAsyncCmd cmd;
        cmd.bufferId = buffer->id;
        cmd.requestSerial = serial;
        cmd.start = start;
        cmd.size = size;
        cmd.isWrite = false;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer =
            static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);
    }

    void ClientBufferMapWriteAsync(dawnBuffer cBuffer,
                                   uint32_t start,
                                   uint32_t size,
                                   dawnBufferMapWriteCallback callback,
                                   dawnCallbackUserdata userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        uint32_t serial = buffer->requestSerial++;
        ASSERT(buffer->requests.find(serial) == buffer->requests.end());

        Buffer::MapRequestData request;
        request.writeCallback = callback;
        request.userdata = userdata;
        request.size = size;
        request.isWrite = true;
        buffer->requests[serial] = request;

        BufferMapAsyncCmd cmd;
        cmd.bufferId = buffer->id;
        cmd.requestSerial = serial;
        cmd.start = start;
        cmd.size = size;
        cmd.isWrite = true;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer =
            static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);
    }

    uint64_t ClientFenceGetCompletedValue(dawnFence cSelf) {
        auto fence = reinterpret_cast<Fence*>(cSelf);
        return fence->completedValue;
    }

    void ClientFenceOnCompletion(dawnFence cFence,
                                 uint64_t value,
                                 dawnFenceOnCompletionCallback callback,
                                 dawnCallbackUserdata userdata) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        if (value > fence->signaledValue) {
            fence->device->HandleError("Value greater than fence signaled value");
            callback(DAWN_FENCE_COMPLETION_STATUS_ERROR, userdata);
            return;
        }

        if (value <= fence->completedValue) {
            callback(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata);
            return;
        }

        Fence::OnCompletionData request;
        request.completionCallback = callback;
        request.userdata = userdata;
        fence->requests.Enqueue(std::move(request), value);
    }

    void ProxyClientBufferUnmap(dawnBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        // Invalidate the local pointer, and cancel all other in-flight requests that would
        // turn into
        // errors anyway (you can't double map). This prevents race when the following happens,
        // where
        // the application code would have unmapped a buffer but still receive a callback:
        //  - Client -> Server: MapRequest1, Unmap, MapRequest2
        //  - Server -> Client: Result of MapRequest1
        //  - Unmap locally on the client
        //  - Server -> Client: Result of MapRequest2
        if (buffer->mappedData) {
            // If the buffer was mapped for writing, send the update to the data to the server
            if (buffer->isWriteMapped) {
                BufferUpdateMappedDataCmd cmd;
                cmd.bufferId = buffer->id;
                cmd.dataLength = static_cast<uint32_t>(buffer->mappedDataSize);
                cmd.data = reinterpret_cast<const uint8_t*>(buffer->mappedData);

                size_t requiredSize = cmd.GetRequiredSize();
                char* allocatedBuffer =
                    static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
                cmd.Serialize(allocatedBuffer);
            }

            free(buffer->mappedData);
            buffer->mappedData = nullptr;
        }
        buffer->ClearMapRequests(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN);

        ClientBufferUnmap(cBuffer);
    }

    dawnFence ProxyClientDeviceCreateFence(dawnDevice cSelf,
                                           dawnFenceDescriptor const* descriptor) {
        dawnFence cFence = ClientDeviceCreateFence(cSelf, descriptor);
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        fence->signaledValue = descriptor->initialValue;
        fence->completedValue = descriptor->initialValue;
        return cFence;
    }

    void ProxyClientQueueSignal(dawnQueue cQueue, dawnFence cFence, uint64_t signalValue) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        if (signalValue <= fence->signaledValue) {
            fence->device->HandleError("Fence value less than or equal to signaled value");
            return;
        }
        fence->signaledValue = signalValue;
        ClientQueueSignal(cQueue, cFence, signalValue);
    }

    void ClientDeviceReference(dawnDevice) {
    }

    void ClientDeviceRelease(dawnDevice) {
    }

    void ClientDeviceSetErrorCallback(dawnDevice cSelf,
                                      dawnDeviceErrorCallback callback,
                                      dawnCallbackUserdata userdata) {
        Device* device = reinterpret_cast<Device*>(cSelf);
        device->SetErrorCallback(callback, userdata);
    }

}}  // namespace dawn_wire::client
