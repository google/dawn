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

#include "dawn_wire/BufferConsumer_impl.h"
#include "dawn_wire/WireCmd_autogen.h"
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

        std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
        std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;

        DeviceCreateBufferCmd cmd;
        cmd.deviceId = device->id;
        cmd.descriptor = descriptor;
        cmd.readHandleCreateInfoLength = 0;
        cmd.readHandleCreateInfo = nullptr;
        cmd.writeHandleCreateInfoLength = 0;
        cmd.writeHandleCreateInfo = nullptr;

        if (mappable) {
            if ((descriptor->usage & WGPUBufferUsage_MapRead) != 0) {
                // Create the read handle on buffer creation.
                readHandle.reset(
                    wireClient->GetMemoryTransferService()->CreateReadHandle(descriptor->size));
                if (readHandle == nullptr) {
                    device->InjectError(WGPUErrorType_OutOfMemory,
                                        "Failed to create buffer mapping");
                    return device->CreateErrorBuffer();
                }
                cmd.readHandleCreateInfoLength = readHandle->SerializeCreateSize();
            }

            if ((descriptor->usage & WGPUBufferUsage_MapWrite) != 0 ||
                descriptor->mappedAtCreation) {
                // Create the write handle on buffer creation.
                writeHandle.reset(
                    wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));
                if (writeHandle == nullptr) {
                    device->InjectError(WGPUErrorType_OutOfMemory,
                                        "Failed to create buffer mapping");
                    return device->CreateErrorBuffer();
                }
                cmd.writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();
            }
        }

        // Create the buffer and send the creation command.
        // This must happen after any potential device->CreateErrorBuffer()
        // as server expects allocating ids to be monotonically increasing
        auto* bufferObjectAndSerial = wireClient->BufferAllocator().New(wireClient);
        Buffer* buffer = bufferObjectAndSerial->object.get();
        buffer->mDevice = device;
        buffer->mDeviceIsAlive = device->GetAliveWeakPtr();
        buffer->mSize = descriptor->size;
        buffer->mDestructWriteHandleOnUnmap = false;

        if (descriptor->mappedAtCreation) {
            // If the buffer is mapped at creation, a write handle is created and will be
            // destructed on unmap if the buffer doesn't have MapWrite usage
            // The buffer is mapped right now.
            buffer->mMapState = MapState::MappedAtCreation;

            // This flag is for write handle created by mappedAtCreation
            // instead of MapWrite usage. We don't have such a case for read handle
            buffer->mDestructWriteHandleOnUnmap =
                (descriptor->usage & WGPUBufferUsage_MapWrite) == 0;

            buffer->mMapOffset = 0;
            buffer->mMapSize = buffer->mSize;
            ASSERT(writeHandle != nullptr);
            buffer->mMappedData = writeHandle->GetData();
        }

        cmd.result = ObjectHandle{buffer->id, bufferObjectAndSerial->generation};

        wireClient->SerializeCommand(
            cmd, cmd.readHandleCreateInfoLength + cmd.writeHandleCreateInfoLength,
            [&](SerializeBuffer* serializeBuffer) {
                if (readHandle != nullptr) {
                    char* readHandleBuffer;
                    WIRE_TRY(
                        serializeBuffer->NextN(cmd.readHandleCreateInfoLength, &readHandleBuffer));
                    // Serialize the ReadHandle into the space after the command.
                    readHandle->SerializeCreate(readHandleBuffer);
                    buffer->mReadHandle = std::move(readHandle);
                }
                if (writeHandle != nullptr) {
                    char* writeHandleBuffer;
                    WIRE_TRY(serializeBuffer->NextN(cmd.writeHandleCreateInfoLength,
                                                    &writeHandleBuffer));
                    // Serialize the WriteHandle into the space after the command.
                    writeHandle->SerializeCreate(writeHandleBuffer);
                    buffer->mWriteHandle = std::move(writeHandle);
                }

                return WireResult::Success;
            });
        return ToAPI(buffer);
    }

    // static
    WGPUBuffer Buffer::CreateError(Device* device) {
        auto* allocation = device->client->BufferAllocator().New(device->client);
        allocation->object->mDevice = device;
        allocation->object->mDeviceIsAlive = device->GetAliveWeakPtr();

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

        FreeMappedData();
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

        // Create the request structure that will hold information while this mapping is
        // in flight.
        uint64_t serial = mRequestSerial++;
        ASSERT(mRequests.find(serial) == mRequests.end());

        Buffer::MapRequestData request = {};
        request.callback = callback;
        request.userdata = userdata;
        request.offset = offset;
        request.size = size;
        if (mode & WGPUMapMode_Read) {
            request.type = MapRequestType::Read;
        } else if (mode & WGPUMapMode_Write) {
            request.type = MapRequestType::Write;
        }

        // Serialize the command to send to the server.
        BufferMapAsyncCmd cmd;
        cmd.bufferId = this->id;
        cmd.requestSerial = serial;
        cmd.mode = mode;
        cmd.offset = offset;
        cmd.size = size;

        client->SerializeCommand(cmd);

        // Register this request so that we can retrieve it from its serial when the server
        // sends the callback.
        mRequests[serial] = std::move(request);
    }

    bool Buffer::OnMapAsyncCallback(uint64_t requestSerial,
                                    uint32_t status,
                                    uint64_t readDataUpdateInfoLength,
                                    const uint8_t* readDataUpdateInfo) {
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

        // Take into account the client-side status of the request if the server says it is a success.
        if (status == WGPUBufferMapAsyncStatus_Success) {
            status = request.clientStatus;
        }

        if (status == WGPUBufferMapAsyncStatus_Success) {
            switch (request.type) {
                case MapRequestType::Read: {
                    if (readDataUpdateInfoLength > std::numeric_limits<size_t>::max()) {
                        // This is the size of data deserialized from the command stream, which must
                        // be CPU-addressable.
                        return FailRequest();
                    }

                    // Validate to prevent bad map request; buffer destroyed during map request
                    if (mReadHandle == nullptr) {
                        return FailRequest();
                    }
                    // Update user map data with server returned data
                    if (!mReadHandle->DeserializeDataUpdate(
                            readDataUpdateInfo, static_cast<size_t>(readDataUpdateInfoLength),
                            request.offset, request.size)) {
                        return FailRequest();
                    }
                    mMapState = MapState::MappedForRead;
                    mMappedData = const_cast<void*>(mReadHandle->GetData());
                    break;
                }
                case MapRequestType::Write: {
                    if (mWriteHandle == nullptr) {
                        return FailRequest();
                    }
                    mMapState = MapState::MappedForWrite;
                    mMappedData = mWriteHandle->GetData();
                    break;
                }
                default:
                    UNREACHABLE();
            }

            mMapOffset = request.offset;
            mMapSize = request.size;
        }

        if (request.callback) {
            request.callback(static_cast<WGPUBufferMapAsyncStatus>(status), request.userdata);
        }

        return true;
    }

    void* Buffer::GetMappedRange(size_t offset, size_t size) {
        if (!IsMappedForWriting() || !CheckGetMappedRangeOffsetSize(offset, size)) {
            return nullptr;
        }
        return static_cast<uint8_t*>(mMappedData) + offset;
    }

    const void* Buffer::GetConstMappedRange(size_t offset, size_t size) {
        if (!(IsMappedForWriting() || IsMappedForReading()) ||
            !CheckGetMappedRangeOffsetSize(offset, size)) {
            return nullptr;
        }
        return static_cast<uint8_t*>(mMappedData) + offset;
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

        // TODO(dawn:608): mDevice->InjectError(WGPUErrorType_Validation) for map oom failure
        // and separate from buffer destroyed before unmap case

        // mWriteHandle can still be nullptr if buffer has been destroyed before unmap
        if ((mMapState == MapState::MappedForWrite || mMapState == MapState::MappedAtCreation) &&
            mWriteHandle != nullptr) {
            // Writes need to be flushed before Unmap is sent. Unmap calls all associated
            // in-flight callbacks which may read the updated data.

            // Get the serialization size of data update writes.
            size_t writeDataUpdateInfoLength =
                mWriteHandle->SizeOfSerializeDataUpdate(mMapOffset, mMapSize);

            BufferUpdateMappedDataCmd cmd;
            cmd.bufferId = id;
            cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
            cmd.writeDataUpdateInfo = nullptr;
            cmd.offset = mMapOffset;
            cmd.size = mMapSize;

            client->SerializeCommand(
                cmd, writeDataUpdateInfoLength, [&](SerializeBuffer* serializeBuffer) {
                    char* writeHandleBuffer;
                    WIRE_TRY(serializeBuffer->NextN(writeDataUpdateInfoLength, &writeHandleBuffer));

                    // Serialize flush metadata into the space after the command.
                    // This closes the handle for writing.
                    mWriteHandle->SerializeDataUpdate(writeHandleBuffer, cmd.offset, cmd.size);

                    return WireResult::Success;
                });

            // If mDestructWriteHandleOnUnmap is true, that means the write handle is merely
            // for mappedAtCreation usage. It is destroyed on unmap after flush to server
            // instead of at buffer destruction.
            if (mMapState == MapState::MappedAtCreation && mDestructWriteHandleOnUnmap) {
                mWriteHandle = nullptr;
                if (mReadHandle) {
                    // If it's both mappedAtCreation and MapRead we need to reset
                    // mMappedData to readHandle's GetData(). This could be changed to
                    // merging read/write handle in future
                    mMappedData = const_cast<void*>(mReadHandle->GetData());
                }
            }
        }

        // Free map access tokens
        mMapState = MapState::Unmapped;
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
        // TODO(dawn:608): add tracking mapped status and change to return bool for returning unmap
        // success/failure
    }

    void Buffer::Destroy() {
        // Remove the current mapping and destroy Read/WriteHandles.
        FreeMappedData();

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
        return mMapState == MapState::MappedForRead;
    }

    bool Buffer::IsMappedForWriting() const {
        return mMapState == MapState::MappedForWrite || mMapState == MapState::MappedAtCreation;
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

    void Buffer::FreeMappedData() {
#if defined(DAWN_ENABLE_ASSERTS)
        // When in "debug" mode, 0xCA-out the mapped data when we free it so that in we can detect
        // use-after-free of the mapped data. This is particularly useful for WebGPU test about the
        // interaction of mapping and GC.
        if (mMappedData) {
            memset(static_cast<uint8_t*>(mMappedData) + mMapOffset, 0xCA, mMapSize);
        }
#endif  // defined(DAWN_ENABLE_ASSERTS)

        mMapOffset = 0;
        mMapSize = 0;
        mReadHandle = nullptr;
        mWriteHandle = nullptr;
        mMappedData = nullptr;
    }

}}  // namespace dawn_wire::client
