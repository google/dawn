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
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class Device;

class Buffer final : public ObjectBase {
  public:
    using ObjectBase::ObjectBase;

    static WGPUBuffer Create(Device* device, const WGPUBufferDescriptor* descriptor);
    static WGPUBuffer CreateError(Device* device);

    ~Buffer();

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

  private:
    void CancelCallbacksForDisconnect() override;
    void ClearAllCallbacks(WGPUBufferMapAsyncStatus status);

    bool IsMappedForReading() const;
    bool IsMappedForWriting() const;
    bool CheckGetMappedRangeOffsetSize(size_t offset, size_t size) const;

    void FreeMappedData();

    Device* mDevice;

    enum class MapRequestType { None, Read, Write };

    enum class MapState {
        Unmapped,
        MappedForRead,
        MappedForWrite,
        MappedAtCreation,
    };

    // We want to defer all the validation to the server, which means we could have multiple
    // map request in flight at a single time and need to track them separately.
    // On well-behaved applications, only one request should exist at a single time.
    struct MapRequestData {
        WGPUBufferMapCallback callback = nullptr;
        void* userdata = nullptr;
        size_t offset = 0;
        size_t size = 0;

        // When the buffer is destroyed or unmapped too early, the unmappedBeforeX status takes
        // precedence over the success value returned from the server. However Error statuses
        // from the server take precedence over the client-side status.
        WGPUBufferMapAsyncStatus clientStatus = WGPUBufferMapAsyncStatus_Success;

        MapRequestType type = MapRequestType::None;
    };
    RequestTracker<MapRequestData> mRequests;
    uint64_t mSize = 0;

    // Only one mapped pointer can be active at a time because Unmap clears all the in-flight
    // requests.
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
