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

#ifndef SRC_DAWN_WIRE_CLIENT_BUFFER_H_
#define SRC_DAWN_WIRE_CLIENT_BUFFER_H_

#include <memory>
#include <optional>

#include "dawn/common/FutureUtils.h"
#include "dawn/webgpu.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

class Buffer final : public ObjectBase {
  public:
    static WGPUBuffer Create(Device* device, const WGPUBufferDescriptor* descriptor);

    Buffer(const ObjectBaseParams& params, const WGPUBufferDescriptor* descriptor);
    ~Buffer() override;

    bool OnMapAsyncCallback(WGPUFuture future,
                            uint32_t status,
                            uint64_t readDataUpdateInfoLength,
                            const uint8_t* readDataUpdateInfo);
    void MapAsync(WGPUMapModeFlags mode,
                  size_t offset,
                  size_t size,
                  WGPUBufferMapCallback callback,
                  void* userdata);
    WGPUFuture MapAsyncF(WGPUMapModeFlags mode,
                         size_t offset,
                         size_t size,
                         const WGPUBufferMapCallbackInfo& callbackInfo);
    void* GetMappedRange(size_t offset, size_t size);
    const void* GetConstMappedRange(size_t offset, size_t size);
    void Unmap();

    void Destroy();

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    WGPUBufferUsage GetUsage() const;
    uint64_t GetSize() const;

    WGPUBufferMapState GetMapState() const;

  private:
    bool InvokeAndClearCallback(WGPUBufferMapAsyncStatus status);

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

    // Up to only one request can exist at a single time. Other requests are rejected.
    struct MapRequestData {
        FutureID futureID = kNullFutureID;
        size_t offset = 0;
        size_t size = 0;
        MapRequestType type = MapRequestType::None;
    };
    std::optional<MapRequestData> mPendingMapRequest;

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
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_BUFFER_H_
