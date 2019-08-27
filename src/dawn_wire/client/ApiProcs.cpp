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

    namespace {
        template <typename Handle>
        void SerializeBufferMapAsync(const Buffer* buffer, uint32_t serial, Handle* handle) {
            // TODO(enga): Remove the template when Read/Write handles are combined in a tagged
            // pointer.
            constexpr bool isWrite =
                std::is_same<Handle, MemoryTransferService::WriteHandle>::value;

            // Get the serialization size of the handle.
            size_t handleCreateInfoLength = handle->SerializeCreateSize();

            BufferMapAsyncCmd cmd;
            cmd.bufferId = buffer->id;
            cmd.requestSerial = serial;
            cmd.isWrite = isWrite;
            cmd.handleCreateInfoLength = handleCreateInfoLength;
            cmd.handleCreateInfo = nullptr;

            size_t commandSize = cmd.GetRequiredSize();
            size_t requiredSize = commandSize + handleCreateInfoLength;
            char* allocatedBuffer =
                static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
            cmd.Serialize(allocatedBuffer);
            // Serialize the handle into the space after the command.
            handle->SerializeCreate(allocatedBuffer + commandSize);
        }
    }  // namespace

    void ClientBufferMapReadAsync(DawnBuffer cBuffer,
                                  DawnBufferMapReadCallback callback,
                                  void* userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        uint32_t serial = buffer->requestSerial++;
        ASSERT(buffer->requests.find(serial) == buffer->requests.end());

        // Create a ReadHandle for the map request. This is the client's intent to read GPU
        // memory.
        MemoryTransferService::ReadHandle* readHandle =
            buffer->device->GetClient()->GetMemoryTransferService()->CreateReadHandle(buffer->size);
        if (readHandle == nullptr) {
            callback(DAWN_BUFFER_MAP_ASYNC_STATUS_DEVICE_LOST, nullptr, 0, userdata);
            return;
        }

        Buffer::MapRequestData request = {};
        request.readCallback = callback;
        request.userdata = userdata;
        // The handle is owned by the MapRequest until the callback returns.
        request.readHandle = std::unique_ptr<MemoryTransferService::ReadHandle>(readHandle);

        // Store a mapping from serial -> MapRequest. The client can map/unmap before the map
        // operations are returned by the server so multiple requests may be in flight.
        buffer->requests[serial] = std::move(request);

        SerializeBufferMapAsync(buffer, serial, readHandle);
    }

    void ClientBufferMapWriteAsync(DawnBuffer cBuffer,
                                   DawnBufferMapWriteCallback callback,
                                   void* userdata) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        uint32_t serial = buffer->requestSerial++;
        ASSERT(buffer->requests.find(serial) == buffer->requests.end());

        // Create a WriteHandle for the map request. This is the client's intent to write GPU
        // memory.
        MemoryTransferService::WriteHandle* writeHandle =
            buffer->device->GetClient()->GetMemoryTransferService()->CreateWriteHandle(
                buffer->size);
        if (writeHandle == nullptr) {
            callback(DAWN_BUFFER_MAP_ASYNC_STATUS_DEVICE_LOST, nullptr, 0, userdata);
            return;
        }

        Buffer::MapRequestData request = {};
        request.writeCallback = callback;
        request.userdata = userdata;
        // The handle is owned by the MapRequest until the callback returns.
        request.writeHandle = std::unique_ptr<MemoryTransferService::WriteHandle>(writeHandle);

        // Store a mapping from serial -> MapRequest. The client can map/unmap before the map
        // operations are returned by the server so multiple requests may be in flight.
        buffer->requests[serial] = std::move(request);

        SerializeBufferMapAsync(buffer, serial, writeHandle);
    }

    DawnBuffer ClientDeviceCreateBuffer(DawnDevice cDevice,
                                        const DawnBufferDescriptor* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Client* wireClient = device->GetClient();

        auto* bufferObjectAndSerial = wireClient->BufferAllocator().New(device);
        Buffer* buffer = bufferObjectAndSerial->object.get();
        // Store the size of the buffer so that mapping operations can allocate a
        // MemoryTransfer handle of the proper size.
        buffer->size = descriptor->size;

        DeviceCreateBufferCmd cmd;
        cmd.self = cDevice;
        cmd.descriptor = descriptor;
        cmd.result = ObjectHandle{buffer->id, bufferObjectAndSerial->serial};

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *wireClient);

        return reinterpret_cast<DawnBuffer>(buffer);
    }

    DawnCreateBufferMappedResult ClientDeviceCreateBufferMapped(
        DawnDevice cDevice,
        const DawnBufferDescriptor* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Client* wireClient = device->GetClient();

        auto* bufferObjectAndSerial = wireClient->BufferAllocator().New(device);
        Buffer* buffer = bufferObjectAndSerial->object.get();
        buffer->size = descriptor->size;

        DawnCreateBufferMappedResult result;
        result.buffer = reinterpret_cast<DawnBuffer>(buffer);
        result.data = nullptr;
        result.dataLength = 0;

        // Create a WriteHandle for the map request. This is the client's intent to write GPU
        // memory.
        std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle =
            std::unique_ptr<MemoryTransferService::WriteHandle>(
                wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));

        if (writeHandle == nullptr) {
            // TODO(enga): Support context lost generated by the client.
            return result;
        }

        // CreateBufferMapped is synchronous and the staging buffer for upload should be immediately
        // available.
        // Open the WriteHandle. This returns a pointer and size of mapped memory.
        // |result.data| may be null on error.
        std::tie(result.data, result.dataLength) = writeHandle->Open();

        if (result.data == nullptr) {
            // TODO(enga): Support context lost generated by the client.
            return result;
        }

        // Successfully created staging memory. The buffer now owns the WriteHandle.
        buffer->writeHandle = std::move(writeHandle);

        // Get the serialization size of the WriteHandle.
        size_t handleCreateInfoLength = buffer->writeHandle->SerializeCreateSize();

        DeviceCreateBufferMappedCmd cmd;
        cmd.device = cDevice;
        cmd.descriptor = descriptor;
        cmd.result = ObjectHandle{buffer->id, bufferObjectAndSerial->serial};
        cmd.handleCreateInfoLength = handleCreateInfoLength;
        cmd.handleCreateInfo = nullptr;

        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = commandSize + handleCreateInfoLength;
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *wireClient);
        // Serialize the WriteHandle into the space after the command.
        buffer->writeHandle->SerializeCreate(allocatedBuffer + commandSize);

        return result;
    }

    void ClientDeviceCreateBufferMappedAsync(DawnDevice cDevice,
                                             const DawnBufferDescriptor* descriptor,
                                             DawnBufferCreateMappedCallback callback,
                                             void* userdata) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Client* wireClient = device->GetClient();

        auto* bufferObjectAndSerial = wireClient->BufferAllocator().New(device);
        Buffer* buffer = bufferObjectAndSerial->object.get();
        buffer->size = descriptor->size;

        uint32_t serial = buffer->requestSerial++;

        struct CreateBufferMappedInfo {
            DawnBuffer buffer;
            DawnBufferCreateMappedCallback callback;
            void* userdata;
        };

        CreateBufferMappedInfo* info = new CreateBufferMappedInfo;
        info->buffer = reinterpret_cast<DawnBuffer>(buffer);
        info->callback = callback;
        info->userdata = userdata;

        // Create a WriteHandle for the map request. This is the client's intent to write GPU
        // memory.
        MemoryTransferService::WriteHandle* writeHandle =
            wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size);
        if (writeHandle == nullptr) {
            DawnCreateBufferMappedResult result;
            result.buffer = reinterpret_cast<DawnBuffer>(buffer);
            result.data = nullptr;
            result.dataLength = 0;
            callback(DAWN_BUFFER_MAP_ASYNC_STATUS_DEVICE_LOST, result, userdata);
            return;
        }

        Buffer::MapRequestData request;
        request.writeCallback = [](DawnBufferMapAsyncStatus status, void* data, uint64_t dataLength,
                                   void* userdata) {
            auto info = std::unique_ptr<CreateBufferMappedInfo>(
                static_cast<CreateBufferMappedInfo*>(userdata));

            DawnCreateBufferMappedResult result;
            result.buffer = info->buffer;
            result.data = data;
            result.dataLength = dataLength;

            info->callback(status, result, info->userdata);
        };
        request.userdata = info;
        // The handle is owned by the MapRequest until the callback returns.
        request.writeHandle = std::unique_ptr<MemoryTransferService::WriteHandle>(writeHandle);
        buffer->requests[serial] = std::move(request);

        // Get the serialization size of the WriteHandle.
        size_t handleCreateInfoLength = writeHandle->SerializeCreateSize();

        DeviceCreateBufferMappedAsyncCmd cmd;
        cmd.device = cDevice;
        cmd.descriptor = descriptor;
        cmd.requestSerial = serial;
        cmd.result = ObjectHandle{buffer->id, bufferObjectAndSerial->serial};
        cmd.handleCreateInfoLength = handleCreateInfoLength;
        cmd.handleCreateInfo = nullptr;

        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = commandSize + handleCreateInfoLength;
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *wireClient);
        // Serialize the WriteHandle into the space after the command.
        writeHandle->SerializeCreate(allocatedBuffer + commandSize);
    }

    uint64_t ClientFenceGetCompletedValue(DawnFence cSelf) {
        auto fence = reinterpret_cast<Fence*>(cSelf);
        return fence->completedValue;
    }

    void ClientFenceOnCompletion(DawnFence cFence,
                                 uint64_t value,
                                 DawnFenceOnCompletionCallback callback,
                                 void* userdata) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        if (value > fence->signaledValue) {
            fence->device->HandleError(DAWN_ERROR_TYPE_VALIDATION,
                                       "Value greater than fence signaled value");
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

    void ClientBufferSetSubData(DawnBuffer cBuffer,
                                uint64_t start,
                                uint64_t count,
                                const void* data) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        BufferSetSubDataInternalCmd cmd;
        cmd.bufferId = buffer->id;
        cmd.start = start;
        cmd.count = count;
        cmd.data = static_cast<const uint8_t*>(data);

        Client* wireClient = buffer->device->GetClient();
        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(wireClient->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer);
    }

    void ClientBufferUnmap(DawnBuffer cBuffer) {
        Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

        // Invalidate the local pointer, and cancel all other in-flight requests that would
        // turn into errors anyway (you can't double map). This prevents race when the following
        // happens, where the application code would have unmapped a buffer but still receive a
        // callback:
        //   - Client -> Server: MapRequest1, Unmap, MapRequest2
        //   - Server -> Client: Result of MapRequest1
        //   - Unmap locally on the client
        //   - Server -> Client: Result of MapRequest2
        if (buffer->writeHandle) {
            // Writes need to be flushed before Unmap is sent. Unmap calls all associated
            // in-flight callbacks which may read the updated data.
            ASSERT(buffer->readHandle == nullptr);

            // Get the serialization size of metadata to flush writes.
            size_t writeFlushInfoLength = buffer->writeHandle->SerializeFlushSize();

            BufferUpdateMappedDataCmd cmd;
            cmd.bufferId = buffer->id;
            cmd.writeFlushInfoLength = writeFlushInfoLength;
            cmd.writeFlushInfo = nullptr;

            size_t commandSize = cmd.GetRequiredSize();
            size_t requiredSize = commandSize + writeFlushInfoLength;
            char* allocatedBuffer =
                static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
            cmd.Serialize(allocatedBuffer);
            // Serialize flush metadata into the space after the command.
            // This closes the handle for writing.
            buffer->writeHandle->SerializeFlush(allocatedBuffer + commandSize);
            buffer->writeHandle = nullptr;

        } else if (buffer->readHandle) {
            buffer->readHandle = nullptr;
        }
        buffer->ClearMapRequests(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN);

        BufferUnmapCmd cmd;
        cmd.self = cBuffer;
        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer =
            static_cast<char*>(buffer->device->GetClient()->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *buffer->device->GetClient());
    }

    DawnFence ClientQueueCreateFence(DawnQueue cSelf, DawnFenceDescriptor const* descriptor) {
        Queue* queue = reinterpret_cast<Queue*>(cSelf);
        Device* device = queue->device;

        QueueCreateFenceCmd cmd;
        cmd.self = cSelf;
        auto* allocation = device->GetClient()->FenceAllocator().New(device);
        cmd.result = ObjectHandle{allocation->object->id, allocation->serial};
        cmd.descriptor = descriptor;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer = static_cast<char*>(device->GetClient()->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *device->GetClient());

        DawnFence cFence = reinterpret_cast<DawnFence>(allocation->object.get());

        Fence* fence = reinterpret_cast<Fence*>(cFence);
        fence->queue = queue;
        fence->signaledValue = descriptor->initialValue;
        fence->completedValue = descriptor->initialValue;
        return cFence;
    }

    void ClientQueueSignal(DawnQueue cQueue, DawnFence cFence, uint64_t signalValue) {
        Fence* fence = reinterpret_cast<Fence*>(cFence);
        Queue* queue = reinterpret_cast<Queue*>(cQueue);
        if (fence->queue != queue) {
            fence->device->HandleError(
                DAWN_ERROR_TYPE_VALIDATION,
                "Fence must be signaled on the queue on which it was created.");
            return;
        }
        if (signalValue <= fence->signaledValue) {
            fence->device->HandleError(DAWN_ERROR_TYPE_VALIDATION,
                                       "Fence value less than or equal to signaled value");
            return;
        }
        fence->signaledValue = signalValue;

        QueueSignalCmd cmd;
        cmd.self = cQueue;
        cmd.fence = cFence;
        cmd.signalValue = signalValue;

        size_t requiredSize = cmd.GetRequiredSize();
        char* allocatedBuffer =
            static_cast<char*>(fence->device->GetClient()->GetCmdSpace(requiredSize));
        cmd.Serialize(allocatedBuffer, *fence->device->GetClient());
    }

    void ClientDeviceReference(DawnDevice) {
    }

    void ClientDeviceRelease(DawnDevice) {
    }

    void ClientDeviceSetUncapturedErrorCallback(DawnDevice cSelf,
                                                DawnErrorCallback callback,
                                                void* userdata) {
        Device* device = reinterpret_cast<Device*>(cSelf);
        device->SetUncapturedErrorCallback(callback, userdata);
    }

}}  // namespace dawn_wire::client
