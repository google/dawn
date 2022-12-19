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

#ifndef SRC_DAWN_WIRE_CLIENT_BUFFER_H_
#define SRC_DAWN_WIRE_CLIENT_BUFFER_H_

#include <memory>

#include "dawn/webgpu.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

class Buffer final : public ObjectBase {
  public:
    static WGPUBuffer Create(Device* device, const WGPUBufferDescriptor* descriptor);
    static WGPUBuffer CreateError(Device* device, const WGPUBufferDescriptor* descriptor);

    Buffer(const ObjectBaseParams& params, Device* device, const WGPUBufferDescriptor* descriptor);
    ~Buffer() override;

    bool OnMapAsyncCallback(uint64_t requestSerial,
                            uint32_t status,
                            uint64_t readDataUpdateInfoLength,
                            const uint8_t* readDataUpdateInfo);
    void MapAsync(WGPUMapModeFlags mode,
                  size_t offset,
                  size_t size,
                  WGPUBufferMapCallback callback,
                  void* userdata);
    void* GetMappedRange(size_t offset, size_t size);
    const void* GetConstMappedRange(size_t offset, size_t size);
    void Unmap();

    void Destroy();

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    WGPUBufferUsage GetUsage() const;
    uint64_t GetSize() const;

    WGPUBufferMapState GetMapState() const;

  private:
    void CancelCallbacksForDisconnect() override;
    void InvokeAndClearCallback(WGPUBufferMapAsyncStatus status);

    bool IsMappedForReading() const;
    bool IsMappedForWriting() const;
    bool CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const;

    void FreeMappedData();

    enum class MapRequestType { None, Read, Write };

    enum class MapState {
        Unmapped,
        MappedForRead,
        MappedForWrite,
        MappedAtCreation,
    };

    // Up to only one request can exist at a single time.
    // Other requests are rejected.
    struct MapRequestData {
        WGPUBufferMapCallback callback = nullptr;
        void* userdata = nullptr;
        size_t offset = 0;
        size_t size = 0;
        MapRequestType type = MapRequestType::None;
    };
    MapRequestData mRequest;
    bool mPendingMap = false;
    uint64_t mSerial = 0;
    uint64_t mSize = 0;
    WGPUBufferUsage mUsage;

    // Only one mapped pointer can be active at a time
    // TODO(enga): Use a tagged pointer to save space.
    std::unique_ptr<MemoryTransferService::ReadHandle> mReadHandle = nullptr;
    std::unique_ptr<MemoryTransferService::WriteHandle> mWriteHandle = nullptr;
    MapState mMapState = MapState::Unmapped;
    bool mDestructWriteHandleOnUnmap = false;

    void* mMappedData = nullptr;
    size_t mMapOffset = 0;
    size_t mMapSize = 0;

    std::weak_ptr<bool> mDeviceIsAlive;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_BUFFER_H_
