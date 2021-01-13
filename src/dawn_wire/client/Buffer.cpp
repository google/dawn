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

#include "dawn_wire/client/Buffer.h"

#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device.h"

namespace dawn_wire { namespace client {

    // static
    WGPUBuffer Buffer::Create(Device* device, const WGPUBufferDescriptor* descriptor) {
        Client* wireClient = device->client;

        bool mappable =
            (descriptor->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)) != 0 ||
            descriptor->mappedAtCreation;
        if (mappable && descriptor->size >= std::numeric_limits<size_t>::max()) {
            device->InjectError(WGPUErrorType_OutOfMemory, "Buffer is too large for map usage");
            return device->CreateErrorBuffer();
        }

        std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;
        void* writeData = nullptr;
        size_t writeHandleCreateInfoLength = 0;

        // If the buffer is mapped at creation, create a write handle that will represent the
        // mapping of the whole buffer.
        if (descriptor->mappedAtCreation) {
            // Create the handle.
            writeHandle.reset(
                wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));
            if (writeHandle == nullptr) {
                device->InjectError(WGPUErrorType_OutOfMemory, "Buffer mapping allocation failed");
                return device->CreateErrorBuffer();
            }

            // Open the handle, it may fail by returning a nullptr in writeData.
            size_t writeDataLength = 0;
            std::tie(writeData, writeDataLength) = writeHandle->Open();
            if (writeData == nullptr) {
                device->InjectError(WGPUErrorType_OutOfMemory, "Buffer mapping allocation failed");
                return device->CreateErrorBuffer();
            }
            ASSERT(writeDataLength == descriptor->size);

            // Get the serialization size of the write handle.
            writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();
        }

        // Create the buffer and send the creation command.
        auto* bufferObjectAndSerial = wireClient->BufferAllocator().New(wireClient);
        Buffer* buffer = bufferObjectAndSerial->object.get();
        buffer->mDevice = device;
        buffer->mSize = descriptor->size;

        DeviceCreateBufferCmd cmd;
        cmd.deviceId = device->id;
        cmd.descriptor = descriptor;
        cmd.result = ObjectHandle{buffer->id, bufferObjectAndSerial->generation};
        cmd.handleCreateInfoLength = writeHandleCreateInfoLength;
        cmd.handleCreateInfo = nullptr;

        wireClient->SerializeCommand(cmd, writeHandleCreateInfoLength, [&](char* cmdSpace) {
            if (descriptor->mappedAtCreation) {
                // Serialize the WriteHandle into the space after the command.
                writeHandle->SerializeCreate(cmdSpace);

                // Set the buffer state for the mapping at creation. The buffer now owns the write
                // handle..
                buffer->mWriteHandle = std::move(writeHandle);
                buffer->mMappedData = writeData;
                buffer->mMapOffset = 0;
                buffer->mMapSize = buffer->mSize;
            }
        });
        return ToAPI(buffer);
    }

    // static
    WGPUBuffer Buffer::CreateError(Device* device) {
        auto* allocation = device->client->BufferAllocator().New(device->client);
        allocation->object->mDevice = device;

        DeviceCreateErrorBufferCmd cmd;
        cmd.self = ToAPI(device);
        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};
        device->client->SerializeCommand(cmd);

        return ToAPI(allocation->object.get());
    }

    Buffer::~Buffer() {
        // Callbacks need to be fired in all cases, as they can handle freeing resources
        // so we call them with "DestroyedBeforeCallback" status.
        for (auto& it : mRequests) {
            if (it.second.callback) {
                it.second.callback(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, it.second.userdata);
            }
        }
        mRequests.clear();
    }

    void Buffer::CancelCallbacksForDisconnect() {
        for (auto& it : mRequests) {
            if (it.second.callback) {
                it.second.callback(WGPUBufferMapAsyncStatus_DeviceLost, it.second.userdata);
            }
        }
        mRequests.clear();
    }

    void Buffer::MapAsync(WGPUMapModeFlags mode,
                          size_t offset,
                          size_t size,
                          WGPUBufferMapCallback callback,
                          void* userdata) {
        if (client->IsDisconnected()) {
            return callback(WGPUBufferMapAsyncStatus_DeviceLost, userdata);
        }

        // Handle the defaulting of size required by WebGPU.
        if (size == 0 && offset < mSize) {
            size = mSize - offset;
        }

        bool isReadMode = mode & WGPUMapMode_Read;
        bool isWriteMode = mode & WGPUMapMode_Write;

        // Step 1. Do early validation of READ ^ WRITE because the server rejects mode = 0.
        if (!(isReadMode ^ isWriteMode)) {
            mDevice->InjectError(WGPUErrorType_Validation,
                                 "MapAsync mode must be exactly one of Read or Write");
            if (callback != nullptr) {
                callback(WGPUBufferMapAsyncStatus_Error, userdata);
            }
            return;
        }

        // Step 2. Create the request structure that will hold information while this mapping is
        // in flight.
        uint32_t serial = mRequestSerial++;
        ASSERT(mRequests.find(serial) == mRequests.end());

        Buffer::MapRequestData request = {};
        request.callback = callback;
        request.userdata = userdata;
        request.size = size;
        request.offset = offset;

        // Step 2a: Create the read / write handles for this request.
        if (isReadMode) {
            request.readHandle.reset(client->GetMemoryTransferService()->CreateReadHandle(size));
            if (request.readHandle == nullptr) {
                mDevice->InjectError(WGPUErrorType_OutOfMemory, "Failed to create buffer mapping");
                callback(WGPUBufferMapAsyncStatus_Error, userdata);
                return;
            }
        } else {
            ASSERT(isWriteMode);
            request.writeHandle.reset(client->GetMemoryTransferService()->CreateWriteHandle(size));
            if (request.writeHandle == nullptr) {
                mDevice->InjectError(WGPUErrorType_OutOfMemory, "Failed to create buffer mapping");
                callback(WGPUBufferMapAsyncStatus_Error, userdata);
                return;
            }
        }

        // Step 3. Serialize the command to send to the server.
        BufferMapAsyncCmd cmd;
        cmd.bufferId = this->id;
        cmd.requestSerial = serial;
        cmd.mode = mode;
        cmd.offset = offset;
        cmd.size = size;
        cmd.handleCreateInfo = nullptr;

        // Step 3a. Fill the handle create info in the command.
        if (isReadMode) {
            cmd.handleCreateInfoLength = request.readHandle->SerializeCreateSize();
            client->SerializeCommand(cmd, cmd.handleCreateInfoLength, [&](char* cmdSpace) {
                request.readHandle->SerializeCreate(cmdSpace);
            });
        } else {
            ASSERT(isWriteMode);
            cmd.handleCreateInfoLength = request.writeHandle->SerializeCreateSize();
            client->SerializeCommand(cmd, cmd.handleCreateInfoLength, [&](char* cmdSpace) {
                request.writeHandle->SerializeCreate(cmdSpace);
            });
        }

        // Step 4. Register this request so that we can retrieve it from its serial when the server
        // sends the callback.
        mRequests[serial] = std::move(request);
    }

    bool Buffer::OnMapAsyncCallback(uint32_t requestSerial,
                                    uint32_t status,
                                    uint64_t readInitialDataInfoLength,
                                    const uint8_t* readInitialDataInfo) {
        auto requestIt = mRequests.find(requestSerial);
        if (requestIt == mRequests.end()) {
            return false;
        }

        auto request = std::move(requestIt->second);
        // Delete the request before calling the callback otherwise the callback could be fired a
        // second time. If, for example, buffer.Unmap() is called inside the callback.
        mRequests.erase(requestIt);

        auto FailRequest = [&request]() -> bool {
            if (request.callback != nullptr) {
                request.callback(WGPUBufferMapAsyncStatus_DeviceLost, request.userdata);
            }
            return false;
        };

        bool isRead = request.readHandle != nullptr;
        bool isWrite = request.writeHandle != nullptr;
        ASSERT(isRead != isWrite);

        // Take into account the client-side status of the request if the server says it is a success.
        if (status == WGPUBufferMapAsyncStatus_Success) {
            status = request.clientStatus;
        }

        size_t mappedDataLength = 0;
        const void* mappedData = nullptr;
        if (status == WGPUBufferMapAsyncStatus_Success) {
            if (mReadHandle || mWriteHandle) {
                // Buffer is already mapped.
                return FailRequest();
            }

            if (isRead) {
                if (readInitialDataInfoLength > std::numeric_limits<size_t>::max()) {
                    // This is the size of data deserialized from the command stream, which must be
                    // CPU-addressable.
                    return FailRequest();
                }

                // The server serializes metadata to initialize the contents of the ReadHandle.
                // Deserialize the message and return a pointer and size of the mapped data for
                // reading.
                if (!request.readHandle->DeserializeInitialData(
                        readInitialDataInfo, static_cast<size_t>(readInitialDataInfoLength),
                        &mappedData, &mappedDataLength)) {
                    // Deserialization shouldn't fail. This is a fatal error.
                    return FailRequest();
                }
                ASSERT(mappedData != nullptr);

            } else {
                // Open the WriteHandle. This returns a pointer and size of mapped memory.
                // On failure, |mappedData| may be null.
                std::tie(mappedData, mappedDataLength) = request.writeHandle->Open();

                if (mappedData == nullptr) {
                    return FailRequest();
                }
            }

            // The MapAsync request was successful. The buffer now owns the Read/Write handles
            // until Unmap().
            mReadHandle = std::move(request.readHandle);
            mWriteHandle = std::move(request.writeHandle);
        }

        mMapOffset = request.offset;
        mMapSize = request.size;
        mMappedData = const_cast<void*>(mappedData);
        if (request.callback) {
            request.callback(static_cast<WGPUBufferMapAsyncStatus>(status), request.userdata);
        }

        return true;
    }

    void* Buffer::GetMappedRange(size_t offset, size_t size) {
        if (!IsMappedForWriting() || !CheckGetMappedRangeOffsetSize(offset, size)) {
            return nullptr;
        }
        return static_cast<uint8_t*>(mMappedData) + (offset - mMapOffset);
    }

    const void* Buffer::GetConstMappedRange(size_t offset, size_t size) {
        if (!(IsMappedForWriting() || IsMappedForReading()) ||
            !CheckGetMappedRangeOffsetSize(offset, size)) {
            return nullptr;
        }
        return static_cast<uint8_t*>(mMappedData) + (offset - mMapOffset);
    }

    void Buffer::Unmap() {
        // Invalidate the local pointer, and cancel all other in-flight requests that would
        // turn into errors anyway (you can't double map). This prevents race when the following
        // happens, where the application code would have unmapped a buffer but still receive a
        // callback:
        //   - Client -> Server: MapRequest1, Unmap, MapRequest2
        //   - Server -> Client: Result of MapRequest1
        //   - Unmap locally on the client
        //   - Server -> Client: Result of MapRequest2
        if (mWriteHandle) {
            // Writes need to be flushed before Unmap is sent. Unmap calls all associated
            // in-flight callbacks which may read the updated data.
            ASSERT(mReadHandle == nullptr);

            // Get the serialization size of metadata to flush writes.
            size_t writeFlushInfoLength = mWriteHandle->SerializeFlushSize();

            BufferUpdateMappedDataCmd cmd;
            cmd.bufferId = id;
            cmd.writeFlushInfoLength = writeFlushInfoLength;
            cmd.writeFlushInfo = nullptr;

            client->SerializeCommand(cmd, writeFlushInfoLength, [&](char* cmdSpace) {
                // Serialize flush metadata into the space after the command.
                // This closes the handle for writing.
                mWriteHandle->SerializeFlush(cmdSpace);
            });
            mWriteHandle = nullptr;

        } else if (mReadHandle) {
            mReadHandle = nullptr;
        }

        mMappedData = nullptr;
        mMapOffset = 0;
        mMapSize = 0;

        // Tag all mapping requests still in flight as unmapped before callback.
        for (auto& it : mRequests) {
            if (it.second.clientStatus == WGPUBufferMapAsyncStatus_Success) {
                it.second.clientStatus = WGPUBufferMapAsyncStatus_UnmappedBeforeCallback;
            }
        }

        BufferUnmapCmd cmd;
        cmd.self = ToAPI(this);
        client->SerializeCommand(cmd);
    }

    void Buffer::Destroy() {
        // Remove the current mapping.
        mWriteHandle = nullptr;
        mReadHandle = nullptr;
        mMappedData = nullptr;

        // Tag all mapping requests still in flight as destroyed before callback.
        for (auto& it : mRequests) {
            if (it.second.clientStatus == WGPUBufferMapAsyncStatus_Success) {
                it.second.clientStatus = WGPUBufferMapAsyncStatus_DestroyedBeforeCallback;
            }
        }

        BufferDestroyCmd cmd;
        cmd.self = ToAPI(this);
        client->SerializeCommand(cmd);
    }

    bool Buffer::IsMappedForReading() const {
        return mReadHandle != nullptr;
    }

    bool Buffer::IsMappedForWriting() const {
        return mWriteHandle != nullptr;
    }

    bool Buffer::CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const {
        if (offset % 8 != 0 || size % 4 != 0) {
            return false;
        }

        if (size > mMapSize || offset < mMapOffset) {
            return false;
        }

        size_t offsetInMappedRange = offset - mMapOffset;
        return offsetInMappedRange <= mMapSize - size;
    }
}}  // namespace dawn_wire::client
