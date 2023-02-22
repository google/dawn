// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_BUFFER_H_
#define SRC_DAWN_NATIVE_BUFFER_H_

#include <memory>

#include "dawn/common/NonCopyable.h"

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct CopyTextureToBufferCmd;

enum class MapType : uint32_t;

MaybeError ValidateBufferDescriptor(DeviceBase* device, const BufferDescriptor* descriptor);

static constexpr wgpu::BufferUsage kReadOnlyBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Index |
    wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Uniform | kReadOnlyStorageBuffer |
    wgpu::BufferUsage::Indirect;

static constexpr wgpu::BufferUsage kMappableBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;

class BufferBase : public ApiObjectBase {
  public:
    enum class BufferState {
        Unmapped,
        PendingMap,
        Mapped,
        MappedAtCreation,
        Destroyed,
    };
    static BufferBase* MakeError(DeviceBase* device, const BufferDescriptor* descriptor);

    ObjectType GetType() const override;

    uint64_t GetSize() const;
    uint64_t GetAllocatedSize() const;

    // |GetUsageExternalOnly| returns the usage with which the buffer was created using the
    // base WebGPU API. Additional usages may be added for internal state tracking. |GetUsage|
    // returns the union of base usage and the usages added internally.
    wgpu::BufferUsage GetUsage() const;
    wgpu::BufferUsage GetUsageExternalOnly() const;

    MaybeError MapAtCreation();
    void OnMapRequestCompleted(MapRequestID mapID, WGPUBufferMapAsyncStatus status);

    MaybeError ValidateCanUseOnQueueNow() const;

    bool IsFullBufferRange(uint64_t offset, uint64_t size) const;
    bool NeedsInitialization() const;
    bool IsDataInitialized() const;
    void SetIsDataInitialized();
    void MarkUsedInPendingCommands();

    virtual void* GetMappedPointer() = 0;
    void* GetMappedRange(size_t offset, size_t size, bool writable = true);
    void Unmap();

    // Dawn API
    void APIMapAsync(wgpu::MapMode mode,
                     size_t offset,
                     size_t size,
                     WGPUBufferMapCallback callback,
                     void* userdata);
    void* APIGetMappedRange(size_t offset, size_t size);
    const void* APIGetConstMappedRange(size_t offset, size_t size);
    void APIUnmap();
    void APIDestroy();
    wgpu::BufferUsage APIGetUsage() const;
    wgpu::BufferMapState APIGetMapState() const;
    uint64_t APIGetSize() const;

  protected:
    BufferBase(DeviceBase* device, const BufferDescriptor* descriptor);
    BufferBase(DeviceBase* device, const BufferDescriptor* descriptor, ObjectBase::ErrorTag tag);

    void DestroyImpl() override;

    ~BufferBase() override;

    MaybeError MapAtCreationInternal();

    uint64_t mAllocatedSize = 0;

    ExecutionSerial mLastUsageSerial = ExecutionSerial(0);

  private:
    // A helper structure to enforce that the mapAsync callback is called only at the very end of
    // methods that might trigger callbacks. Non-copyable but movable for the assertion in the
    // destructor to ensure not to forget to call the callback
    struct [[nodiscard]] PendingMappingCallback : public NonCopyable {
        WGPUBufferMapCallback callback;
        void* userdata;
        WGPUBufferMapAsyncStatus status;

        PendingMappingCallback();
        ~PendingMappingCallback();

        PendingMappingCallback(PendingMappingCallback&& other);
        PendingMappingCallback& operator=(PendingMappingCallback&& other);

        void Call();
    };
    PendingMappingCallback WillCallMappingCallback(MapRequestID mapID,
                                                   WGPUBufferMapAsyncStatus status);

    virtual MaybeError MapAtCreationImpl() = 0;
    virtual MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) = 0;
    virtual void UnmapImpl() = 0;

    virtual bool IsCPUWritableAtCreation() const = 0;
    MaybeError CopyFromStagingBuffer();

    MaybeError ValidateMapAsync(wgpu::MapMode mode,
                                size_t offset,
                                size_t size,
                                WGPUBufferMapAsyncStatus* status) const;
    MaybeError ValidateUnmap() const;
    bool CanGetMappedRange(bool writable, size_t offset, size_t size) const;
    PendingMappingCallback UnmapInternal(WGPUBufferMapAsyncStatus callbackStatus);

    uint64_t mSize = 0;
    wgpu::BufferUsage mUsage = wgpu::BufferUsage::None;
    BufferState mState;
    bool mIsDataInitialized = false;

    // mStagingBuffer is used to implement mappedAtCreation for
    // buffers with non-mappable usage. It is transiently allocated
    // and released when the mappedAtCreation-buffer is unmapped.
    // Because `mStagingBuffer` itself is directly mappable, it will
    // not create another staging buffer.
    // i.e. buffer->mStagingBuffer->mStagingBuffer... is not possible.
    Ref<BufferBase> mStagingBuffer;

    WGPUBufferMapCallback mMapCallback = nullptr;
    void* mMapUserdata = nullptr;
    MapRequestID mLastMapID = MapRequestID(0);
    wgpu::MapMode mMapMode = wgpu::MapMode::None;
    size_t mMapOffset = 0;
    size_t mMapSize = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUFFER_H_
