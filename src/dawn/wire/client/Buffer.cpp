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

#include "dawn/wire/client/Buffer.h"

#include <limits>
#include <utility>

#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

namespace {
WGPUBuffer CreateErrorBufferOOMAtClient(Device* device, const WGPUBufferDescriptor* descriptor) {
    if (descriptor->mappedAtCreation) {
        return nullptr;
    }
    WGPUBufferDescriptor errorBufferDescriptor = *descriptor;
    WGPUDawnBufferDescriptorErrorInfoFromWireClient errorInfo = {};
    errorInfo.chain.sType = WGPUSType_DawnBufferDescriptorErrorInfoFromWireClient;
    errorInfo.outOfMemory = true;
    errorBufferDescriptor.nextInChain = &errorInfo.chain;
    return device->CreateErrorBuffer(&errorBufferDescriptor);
}
}  // anonymous namespace

// static
WGPUBuffer Buffer::Create(Device* device, const WGPUBufferDescriptor* descriptor) {
    Client* wireClient = device->GetClient();

    bool mappable =
        (descriptor->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)) != 0 ||
        descriptor->mappedAtCreation;
    if (mappable && descriptor->size >= std::numeric_limits<size_t>::max()) {
        return CreateErrorBufferOOMAtClient(device, descriptor);
    }

    std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;

    DeviceCreateBufferCmd cmd;
    cmd.deviceId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.readHandleCreateInfoLength = 0;
    cmd.readHandleCreateInfo = nullptr;
    cmd.writeHandleCreateInfoLength = 0;
    cmd.writeHandleCreateInfo = nullptr;

    size_t readHandleCreateInfoLength = 0;
    size_t writeHandleCreateInfoLength = 0;
    if (mappable) {
        if ((descriptor->usage & WGPUBufferUsage_MapRead) != 0) {
            // Create the read handle on buffer creation.
            readHandle.reset(
                wireClient->GetMemoryTransferService()->CreateReadHandle(descriptor->size));
            if (readHandle == nullptr) {
                return CreateErrorBufferOOMAtClient(device, descriptor);
            }
            readHandleCreateInfoLength = readHandle->SerializeCreateSize();
            cmd.readHandleCreateInfoLength = readHandleCreateInfoLength;
        }

        if ((descriptor->usage & WGPUBufferUsage_MapWrite) != 0 || descriptor->mappedAtCreation) {
            // Create the write handle on buffer creation.
            writeHandle.reset(
                wireClient->GetMemoryTransferService()->CreateWriteHandle(descriptor->size));
            if (writeHandle == nullptr) {
                return CreateErrorBufferOOMAtClient(device, descriptor);
            }
            writeHandleCreateInfoLength = writeHandle->SerializeCreateSize();
            cmd.writeHandleCreateInfoLength = writeHandleCreateInfoLength;
        }
    }

    // Create the buffer and send the creation command.
    // This must happen after any potential device->CreateErrorBuffer()
    // as server expects allocating ids to be monotonically increasing
    Buffer* buffer = wireClient->Make<Buffer>(device, descriptor);
    buffer->mDestructWriteHandleOnUnmap = false;

    if (descriptor->mappedAtCreation) {
        // If the buffer is mapped at creation, a write handle is created and will be
        // destructed on unmap if the buffer doesn't have MapWrite usage
        // The buffer is mapped right now.
        buffer->mMapState = MapState::MappedAtCreation;

        // This flag is for write handle created by mappedAtCreation
        // instead of MapWrite usage. We don't have such a case for read handle
        buffer->mDestructWriteHandleOnUnmap = (descriptor->usage & WGPUBufferUsage_MapWrite) == 0;

        buffer->mMapOffset = 0;
        buffer->mMapSize = buffer->mSize;
        ASSERT(writeHandle != nullptr);
        buffer->mMappedData = writeHandle->GetData();
    }

    cmd.result = buffer->GetWireHandle();

    // clang-format off
    // Turning off clang format here because for some reason it does not format the
    // CommandExtensions consistently, making it harder to read.
    wireClient->SerializeCommand(
        cmd,
        CommandExtension{readHandleCreateInfoLength,
                         [&](char* readHandleBuffer) {
                             if (readHandle != nullptr) {
                                 // Serialize the ReadHandle into the space after the command.
                                 readHandle->SerializeCreate(readHandleBuffer);
                                 buffer->mReadHandle = std::move(readHandle);
                             }
                         }},
        CommandExtension{writeHandleCreateInfoLength,
                         [&](char* writeHandleBuffer) {
                             if (writeHandle != nullptr) {
                                 // Serialize the WriteHandle into the space after the command.
                                 writeHandle->SerializeCreate(writeHandleBuffer);
                                 buffer->mWriteHandle = std::move(writeHandle);
                             }
                         }});
    // clang-format on
    return ToAPI(buffer);
}

// static
WGPUBuffer Buffer::CreateError(Device* device, const WGPUBufferDescriptor* descriptor) {
    Client* client = device->GetClient();
    Buffer* buffer = client->Make<Buffer>(device, descriptor);

    DeviceCreateErrorBufferCmd cmd;
    cmd.self = ToAPI(device);
    cmd.selfId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.result = buffer->GetWireHandle();
    client->SerializeCommand(cmd);

    return ToAPI(buffer);
}

Buffer::Buffer(const ObjectBaseParams& params,
               Device* device,
               const WGPUBufferDescriptor* descriptor)
    : ObjectBase(params),
      mSize(descriptor->size),
      mUsage(static_cast<WGPUBufferUsage>(descriptor->usage)),
      mDeviceIsAlive(device->GetAliveWeakPtr()) {}

Buffer::~Buffer() {
    FreeMappedData();
    InvokeAndClearCallback(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
}

void Buffer::CancelCallbacksForDisconnect() {
    InvokeAndClearCallback(WGPUBufferMapAsyncStatus_DeviceLost);
}

void Buffer::InvokeAndClearCallback(WGPUBufferMapAsyncStatus status) {
    WGPUBufferMapCallback callback = mRequest.callback;
    void* userdata = mRequest.userdata;
    mRequest.callback = nullptr;
    mRequest.userdata = nullptr;
    mPendingMap = false;
    if (callback != nullptr) {
        callback(status, userdata);
    }
}

void Buffer::MapAsync(WGPUMapModeFlags mode,
                      size_t offset,
                      size_t size,
                      WGPUBufferMapCallback callback,
                      void* userdata) {
    if (mPendingMap) {
        return callback(WGPUBufferMapAsyncStatus_Error, userdata);
    }

    Client* client = GetClient();
    if (client->IsDisconnected()) {
        return callback(WGPUBufferMapAsyncStatus_DeviceLost, userdata);
    }

    // Handle the defaulting of size required by WebGPU.
    if ((size == WGPU_WHOLE_MAP_SIZE) && (offset <= mSize)) {
        size = mSize - offset;
    }

    // Set up the request structure that will hold information while this mapping is
    // in flight.
    mRequest.callback = callback;
    mRequest.userdata = userdata;
    mRequest.offset = offset;
    mRequest.size = size;
    if (mode & WGPUMapMode_Read) {
        mRequest.type = MapRequestType::Read;
    } else if (mode & WGPUMapMode_Write) {
        mRequest.type = MapRequestType::Write;
    }

    // Serialize the command to send to the server.
    mPendingMap = true;
    mSerial++;
    BufferMapAsyncCmd cmd;
    cmd.bufferId = GetWireId();
    cmd.requestSerial = mSerial;
    cmd.mode = mode;
    cmd.offset = offset;
    cmd.size = size;

    client->SerializeCommand(cmd);
}

bool Buffer::OnMapAsyncCallback(uint64_t requestSerial,
                                uint32_t status,
                                uint64_t readDataUpdateInfoLength,
                                const uint8_t* readDataUpdateInfo) {
    // If requestSerial doesn't match mSerial the corresponding request must have
    // already been rejected by unmap or destroy and another MapAsync request must
    // have been issued.
    if (mSerial != requestSerial) {
        return true;
    }

    // If mPendingMap is false the request must have been already rejected
    // by unmap or destroy.
    if (!mPendingMap) {
        return true;
    }

    auto FailRequest = [this]() -> bool {
        InvokeAndClearCallback(WGPUBufferMapAsyncStatus_DeviceLost);
        return false;
    };

    if (status == WGPUBufferMapAsyncStatus_Success) {
        switch (mRequest.type) {
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
                        mRequest.offset, mRequest.size)) {
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

        mMapOffset = mRequest.offset;
        mMapSize = mRequest.size;
    }

    InvokeAndClearCallback(static_cast<WGPUBufferMapAsyncStatus>(status));

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
    Client* client = GetClient();

    // mWriteHandle can still be nullptr if buffer has been destroyed before unmap
    if ((mMapState == MapState::MappedForWrite || mMapState == MapState::MappedAtCreation) &&
        mWriteHandle != nullptr) {
        // Writes need to be flushed before Unmap is sent. Unmap calls all associated
        // in-flight callbacks which may read the updated data.

        // Get the serialization size of data update writes.
        size_t writeDataUpdateInfoLength =
            mWriteHandle->SizeOfSerializeDataUpdate(mMapOffset, mMapSize);

        BufferUpdateMappedDataCmd cmd;
        cmd.bufferId = GetWireId();
        cmd.writeDataUpdateInfoLength = writeDataUpdateInfoLength;
        cmd.writeDataUpdateInfo = nullptr;
        cmd.offset = mMapOffset;
        cmd.size = mMapSize;

        client->SerializeCommand(
            cmd, CommandExtension{writeDataUpdateInfoLength, [&](char* writeHandleBuffer) {
                                      // Serialize flush metadata into the space after the command.
                                      // This closes the handle for writing.
                                      mWriteHandle->SerializeDataUpdate(writeHandleBuffer,
                                                                        cmd.offset, cmd.size);
                                  }});

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

    BufferUnmapCmd cmd;
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);

    InvokeAndClearCallback(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback);
}

void Buffer::Destroy() {
    Client* client = GetClient();

    // Remove the current mapping and destroy Read/WriteHandles.
    FreeMappedData();
    mMapState = MapState::Unmapped;

    BufferDestroyCmd cmd;
    cmd.self = ToAPI(this);
    client->SerializeCommand(cmd);

    InvokeAndClearCallback(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
}

WGPUBufferUsage Buffer::GetUsage() const {
    return mUsage;
}

uint64_t Buffer::GetSize() const {
    return mSize;
}

WGPUBufferMapState Buffer::GetMapState() const {
    switch (mMapState) {
        case MapState::MappedForRead:
        case MapState::MappedForWrite:
        case MapState::MappedAtCreation:
            return WGPUBufferMapState_Mapped;
        case MapState::Unmapped:
            if (mPendingMap) {
                return WGPUBufferMapState_Pending;
            } else {
                return WGPUBufferMapState_Unmapped;
            }
    }
}

bool Buffer::IsMappedForReading() const {
    return mMapState == MapState::MappedForRead;
}

bool Buffer::IsMappedForWriting() const {
    return mMapState == MapState::MappedForWrite || mMapState == MapState::MappedAtCreation;
}

bool Buffer::CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const {
    if (offset % 8 != 0 || offset < mMapOffset || offset > mSize) {
        return false;
    }

    size_t rangeSize = size == WGPU_WHOLE_MAP_SIZE ? mSize - offset : size;

    if (rangeSize % 4 != 0 || rangeSize > mMapSize) {
        return false;
    }

    size_t offsetInMappedRange = offset - mMapOffset;
    return offsetInMappedRange <= mMapSize - rangeSize;
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

}  // namespace dawn::wire::client
