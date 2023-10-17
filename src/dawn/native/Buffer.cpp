// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/Buffer.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

namespace {
struct MapRequestTask : TrackTaskCallback {
    MapRequestTask(dawn::platform::Platform* platform, Ref<BufferBase> buffer, MapRequestID id)
        : TrackTaskCallback(platform), buffer(std::move(buffer)), id(id) {}
    ~MapRequestTask() override = default;

  private:
    void FinishImpl() override {
        {
            // This is called from a callback, and no lock will be held by default. Hence, we need
            // to lock the mutex now because mSerial might be changed by another thread.
            auto deviceLock(buffer->GetDevice()->GetScopedLock());
            DAWN_ASSERT(mSerial != kMaxExecutionSerial);
            TRACE_EVENT1(mPlatform, General, "Buffer::TaskInFlight::Finished", "serial",
                         uint64_t(mSerial));
        }
        buffer->CallbackOnMapRequestCompleted(id, WGPUBufferMapAsyncStatus_Success);
    }
    void HandleDeviceLossImpl() override {
        buffer->CallbackOnMapRequestCompleted(id, WGPUBufferMapAsyncStatus_DeviceLost);
    }
    void HandleShutDownImpl() override {
        buffer->CallbackOnMapRequestCompleted(id, WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
    }

    Ref<BufferBase> buffer;
    MapRequestID id;
};

class ErrorBuffer final : public BufferBase {
  public:
    ErrorBuffer(DeviceBase* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor, ObjectBase::kError) {
        if (descriptor->mappedAtCreation) {
            // Check that the size can be used to allocate an mFakeMappedData. A malloc(0)
            // is invalid, and on 32bit systems we should avoid a narrowing conversion that
            // would make size = 1 << 32 + 1 allocate one byte.
            bool isValidSize = descriptor->size != 0 &&
                               descriptor->size < uint64_t(std::numeric_limits<size_t>::max());

            if (isValidSize) {
                mFakeMappedData =
                    std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(descriptor->size));
            }
            // Since error buffers in this case may allocate memory, we need to track them
            // for destruction on the device.
            GetObjectTrackingList()->Track(this);
        }
    }

  private:
    bool IsCPUWritableAtCreation() const override { DAWN_UNREACHABLE(); }

    MaybeError MapAtCreationImpl() override { DAWN_UNREACHABLE(); }

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override {
        DAWN_UNREACHABLE();
    }

    void* GetMappedPointer() override { return mFakeMappedData.get(); }

    void UnmapImpl() override { mFakeMappedData.reset(); }

    std::unique_ptr<uint8_t[]> mFakeMappedData;
};

// GetMappedRange on a zero-sized buffer returns a pointer to this value.
static uint32_t sZeroSizedMappingData = 0xCAFED00D;

}  // anonymous namespace

MaybeError ValidateBufferDescriptor(DeviceBase* device, const BufferDescriptor* descriptor) {
    UnpackedBufferDescriptorChain unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpackChain(descriptor));

    DAWN_TRY(ValidateBufferUsage(descriptor->usage));

    if (const auto* hostMappedDesc = std::get<const BufferHostMappedPointer*>(unpacked)) {
        // TODO(crbug.com/dawn/2018): Properly expose this limit.
        uint32_t requiredAlignment = 4096;
        if (device->GetAdapter()->GetPhysicalDevice()->GetBackendType() ==
            wgpu::BackendType::D3D12) {
            requiredAlignment = 65536;
        }

        DAWN_INVALID_IF(!device->HasFeature(Feature::HostMappedPointer), "%s requires %s.",
                        hostMappedDesc->sType, ToAPI(Feature::HostMappedPointer));
        DAWN_INVALID_IF(!IsAligned(descriptor->size, requiredAlignment),
                        "Buffer size (%u) wrapping host-mapped memory was not aligned to %u.",
                        descriptor->size, requiredAlignment);
        DAWN_INVALID_IF(!IsPtrAligned(hostMappedDesc->pointer, requiredAlignment),
                        "Host-mapped memory pointer (%p) was not aligned to %u.",
                        hostMappedDesc->pointer, requiredAlignment);

        // TODO(dawn:2018) consider allowing the host-mapped buffers to be mapped through WebGPU.
        DAWN_INVALID_IF(
            descriptor->mappedAtCreation,
            "Buffer created from host-mapped pointer requires mappedAtCreation to be false.");
    }

    wgpu::BufferUsage usage = descriptor->usage;

    DAWN_INVALID_IF(usage == wgpu::BufferUsage::None, "Buffer usages must not be 0.");

    const wgpu::BufferUsage kMapWriteAllowedUsages =
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    DAWN_INVALID_IF(
        usage & wgpu::BufferUsage::MapWrite && !IsSubset(usage, kMapWriteAllowedUsages),
        "Buffer usages (%s) is invalid. If a buffer usage contains %s the only other allowed "
        "usage is %s.",
        usage, wgpu::BufferUsage::MapWrite, wgpu::BufferUsage::CopySrc);

    const wgpu::BufferUsage kMapReadAllowedUsages =
        wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    DAWN_INVALID_IF(
        usage & wgpu::BufferUsage::MapRead && !IsSubset(usage, kMapReadAllowedUsages),
        "Buffer usages (%s) is invalid. If a buffer usage contains %s the only other allowed "
        "usage is %s.",
        usage, wgpu::BufferUsage::MapRead, wgpu::BufferUsage::CopyDst);

    DAWN_INVALID_IF(descriptor->mappedAtCreation && descriptor->size % 4 != 0,
                    "Buffer is mapped at creation but its size (%u) is not a multiple of 4.",
                    descriptor->size);

    DAWN_INVALID_IF(descriptor->size > device->GetLimits().v1.maxBufferSize,
                    "Buffer size (%u) exceeds the max buffer size limit (%u).", descriptor->size,
                    device->GetLimits().v1.maxBufferSize);

    return {};
}

// Buffer

BufferBase::BufferBase(DeviceBase* device, const BufferDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mState(BufferState::Unmapped) {
    // Add readonly storage usage if the buffer has a storage usage. The validation rules in
    // ValidateSyncScopeResourceUsage will make sure we don't use both at the same time.
    if (mUsage & wgpu::BufferUsage::Storage) {
        mUsage |= kReadOnlyStorageBuffer;
    }

    // The query resolve buffer need to be used as a storage buffer in the internal compute
    // pipeline which does timestamp uint conversion for timestamp query, it requires the buffer
    // has Storage usage in the binding group. Implicitly add an InternalStorage usage which is
    // only compatible with InternalStorageBuffer binding type in BGL. It shouldn't be
    // compatible with StorageBuffer binding type and the query resolve buffer cannot be bound
    // as storage buffer if it's created without Storage usage.
    if (mUsage & wgpu::BufferUsage::QueryResolve) {
        mUsage |= kInternalStorageBuffer;
    }

    // We also add internal storage usage for Indirect buffers for some transformations before
    // DispatchIndirect calls on the backend (e.g. validations, support of [[num_workgroups]] on
    // D3D12), since these transformations involve binding them as storage buffers for use in a
    // compute pass.
    if (mUsage & wgpu::BufferUsage::Indirect) {
        mUsage |= kInternalStorageBuffer;
    }

    if (mUsage & wgpu::BufferUsage::CopyDst) {
        if (device->IsToggleEnabled(Toggle::UseBlitForDepth16UnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForDepth32FloatTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForStencilTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForSnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForBGRA8UnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy)) {
            mUsage |= kInternalStorageBuffer;
        }
    }

    const BufferHostMappedPointer* hostMappedDesc = nullptr;
    FindInChain(descriptor->nextInChain, &hostMappedDesc);
    if (hostMappedDesc != nullptr) {
        mState = BufferState::HostMappedPersistent;
    }

    GetObjectTrackingList()->Track(this);
}

BufferBase::BufferBase(DeviceBase* device,
                       const BufferDescriptor* descriptor,
                       ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mState(BufferState::Unmapped) {
    if (descriptor->mappedAtCreation) {
        mState = BufferState::MappedAtCreation;
        mMapOffset = 0;
        mMapSize = mSize;
    }
}

BufferBase::~BufferBase() {
    DAWN_ASSERT(mState == BufferState::Unmapped || mState == BufferState::Destroyed);
}

void BufferBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    if (mState == BufferState::Mapped || mState == BufferState::PendingMap) {
        UnmapInternal(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
    } else if (mState == BufferState::MappedAtCreation) {
        if (mStagingBuffer != nullptr) {
            mStagingBuffer = nullptr;
        } else if (mSize != 0) {
            UnmapInternal(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
        }
    }

    mState = BufferState::Destroyed;
}

// static
BufferBase* BufferBase::MakeError(DeviceBase* device, const BufferDescriptor* descriptor) {
    return new ErrorBuffer(device, descriptor);
}

ObjectType BufferBase::GetType() const {
    return ObjectType::Buffer;
}

uint64_t BufferBase::GetSize() const {
    DAWN_ASSERT(!IsError());
    return mSize;
}

uint64_t BufferBase::GetAllocatedSize() const {
    DAWN_ASSERT(!IsError());
    // The backend must initialize this value.
    DAWN_ASSERT(mAllocatedSize != 0);
    return mAllocatedSize;
}

wgpu::BufferUsage BufferBase::GetUsage() const {
    DAWN_ASSERT(!IsError());
    return mUsage;
}

wgpu::BufferUsage BufferBase::GetUsageExternalOnly() const {
    DAWN_ASSERT(!IsError());
    return GetUsage() & ~kAllInternalBufferUsages;
}

wgpu::BufferUsage BufferBase::APIGetUsage() const {
    return mUsage & ~kAllInternalBufferUsages;
}

wgpu::BufferMapState BufferBase::APIGetMapState() const {
    switch (mState) {
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return wgpu::BufferMapState::Mapped;
        case BufferState::PendingMap:
            return wgpu::BufferMapState::Pending;
        case BufferState::Unmapped:
        case BufferState::Destroyed:
            return wgpu::BufferMapState::Unmapped;
        default:
            DAWN_UNREACHABLE();
            return wgpu::BufferMapState::Unmapped;
    }
}

MaybeError BufferBase::MapAtCreation() {
    DAWN_TRY(MapAtCreationInternal());

    void* ptr;
    size_t size;
    if (mSize == 0) {
        return {};
    } else if (mStagingBuffer != nullptr) {
        // If there is a staging buffer for initialization, clear its contents directly.
        // It should be exactly as large as the buffer allocation.
        ptr = mStagingBuffer->GetMappedPointer();
        size = mStagingBuffer->GetSize();
        DAWN_ASSERT(size == GetAllocatedSize());
    } else {
        // Otherwise, the buffer is directly mappable on the CPU.
        ptr = GetMappedPointer();
        size = GetAllocatedSize();
    }

    DeviceBase* device = GetDevice();
    if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        memset(ptr, uint8_t(0u), size);
        SetIsDataInitialized();
        device->IncrementLazyClearCountForTesting();
    } else if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        memset(ptr, uint8_t(1u), size);
    }

    return {};
}

MaybeError BufferBase::MapAtCreationInternal() {
    DAWN_ASSERT(!IsError());
    mMapOffset = 0;
    mMapSize = mSize;

    // 0-sized buffers are not supposed to be written to. Return back any non-null pointer.
    // Skip handling 0-sized buffers so we don't try to map them in the backend.
    if (mSize != 0) {
        // Mappable buffers don't use a staging buffer and are just as if mapped through
        // MapAsync.
        if (IsCPUWritableAtCreation()) {
            DAWN_TRY(MapAtCreationImpl());
        } else {
            // If any of these fail, the buffer will be deleted and replaced with an error
            // buffer. The staging buffer is used to return mappable data to inititalize the
            // buffer contents. Allocate one as large as the real buffer size so that every byte
            // is initialized.
            // TODO(crbug.com/dawn/828): Suballocate and reuse memory from a larger staging
            // buffer so we don't create many small buffers.
            BufferDescriptor stagingBufferDesc = {};
            stagingBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
            stagingBufferDesc.size = Align(GetAllocatedSize(), 4);
            stagingBufferDesc.mappedAtCreation = true;
            stagingBufferDesc.label = "Dawn_MappedAtCreationStaging";

            IgnoreLazyClearCountScope scope(GetDevice());
            DAWN_TRY_ASSIGN(mStagingBuffer, GetDevice()->CreateBuffer(&stagingBufferDesc));
        }
    }

    // Only set the state to mapped at creation if we did no fail any point in this helper.
    // Otherwise, if we override the default unmapped state before succeeding to create a
    // staging buffer, we will have issues when we try to destroy the buffer.
    mState = BufferState::MappedAtCreation;
    return {};
}

MaybeError BufferBase::ValidateCanUseOnQueueNow() const {
    DAWN_ASSERT(!IsError());

    switch (mState) {
        case BufferState::Destroyed:
            return DAWN_VALIDATION_ERROR("%s used in submit while destroyed.", this);
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return DAWN_VALIDATION_ERROR("%s used in submit while mapped.", this);
        case BufferState::PendingMap:
            return DAWN_VALIDATION_ERROR("%s used in submit while pending map.", this);
        case BufferState::HostMappedPersistent:
        case BufferState::Unmapped:
            return {};
    }
    DAWN_UNREACHABLE();
}

std::function<void()> BufferBase::PrepareMappingCallback(MapRequestID mapID,
                                                         WGPUBufferMapAsyncStatus status) {
    DAWN_ASSERT(!IsError());

    if (mMapCallback != nullptr && mapID == mLastMapID) {
        auto callback = std::move(mMapCallback);
        auto userdata = std::move(mMapUserdata);
        WGPUBufferMapAsyncStatus actualStatus;
        if (GetDevice()->IsLost()) {
            actualStatus = WGPUBufferMapAsyncStatus_DeviceLost;
        } else {
            actualStatus = status;
        }

        // Tag the callback as fired before firing it, otherwise it could fire a second time if
        // for example buffer.Unmap() is called before the MapRequestTask completes.
        mMapCallback = nullptr;
        mMapUserdata = nullptr;

        return std::bind(callback, actualStatus, userdata);
    }

    return [] {};
}

void BufferBase::APIMapAsync(wgpu::MapMode mode,
                             size_t offset,
                             size_t size,
                             WGPUBufferMapCallback callback,
                             void* userdata) {
    // Check for an existing pending map first because it just
    // rejects the callback and doesn't produce a validation error.
    if (mState == BufferState::PendingMap) {
        if (callback) {
            GetDevice()->GetCallbackTaskManager()->AddCallbackTask(
                callback, WGPUBufferMapAsyncStatus_MappingAlreadyPending, userdata);
        }
        return;
    }

    // Handle the defaulting of size required by WebGPU, even if in webgpu_cpp.h it is not
    // possible to default the function argument (because there is the callback later in the
    // argument list)
    if ((size == wgpu::kWholeMapSize) && (offset <= mSize)) {
        size = mSize - offset;
    }

    WGPUBufferMapAsyncStatus status;
    if (GetDevice()->ConsumedError(ValidateMapAsync(mode, offset, size, &status),
                                   "calling %s.MapAsync(%s, %u, %u, ...).", this, mode, offset,
                                   size)) {
        if (callback) {
            GetDevice()->GetCallbackTaskManager()->AddCallbackTask(callback, status, userdata);
        }
        return;
    }
    DAWN_ASSERT(!IsError());

    mLastMapID++;
    mMapMode = mode;
    mMapOffset = offset;
    mMapSize = size;
    mMapCallback = callback;
    mMapUserdata = userdata;
    mState = BufferState::PendingMap;

    if (GetDevice()->ConsumedError(MapAsyncImpl(mode, offset, size))) {
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(
            PrepareMappingCallback(mLastMapID, WGPUBufferMapAsyncStatus_DeviceLost));
        return;
    }
    std::unique_ptr<MapRequestTask> request =
        std::make_unique<MapRequestTask>(GetDevice()->GetPlatform(), this, mLastMapID);
    TRACE_EVENT1(GetDevice()->GetPlatform(), General, "Buffer::APIMapAsync", "serial",
                 uint64_t(mLastUsageSerial));
    GetDevice()->GetQueue()->TrackTask(std::move(request), mLastUsageSerial);
}

void* BufferBase::APIGetMappedRange(size_t offset, size_t size) {
    return GetMappedRange(offset, size, true);
}

const void* BufferBase::APIGetConstMappedRange(size_t offset, size_t size) {
    return GetMappedRange(offset, size, false);
}

void* BufferBase::GetMappedRange(size_t offset, size_t size, bool writable) {
    if (!CanGetMappedRange(writable, offset, size)) {
        return nullptr;
    }

    if (mStagingBuffer != nullptr) {
        return static_cast<uint8_t*>(mStagingBuffer->GetMappedPointer()) + offset;
    }
    if (mSize == 0) {
        return &sZeroSizedMappingData;
    }
    uint8_t* start = static_cast<uint8_t*>(GetMappedPointer());
    return start == nullptr ? nullptr : start + offset;
}

void BufferBase::APIDestroy() {
    Destroy();
}

uint64_t BufferBase::APIGetSize() const {
    return mSize;
}

MaybeError BufferBase::CopyFromStagingBuffer() {
    DAWN_ASSERT(mStagingBuffer != nullptr && mSize != 0);

    DAWN_TRY(
        GetDevice()->CopyFromStagingToBuffer(mStagingBuffer.Get(), 0, this, 0, GetAllocatedSize()));

    DynamicUploader* uploader = GetDevice()->GetDynamicUploader();
    uploader->ReleaseStagingBuffer(std::move(mStagingBuffer));

    return {};
}

void BufferBase::APIUnmap() {
    if (GetDevice()->ConsumedError(ValidateUnmap(), "calling %s.Unmap().", this)) {
        return;
    }
    DAWN_UNUSED(GetDevice()->ConsumedError(Unmap(), "calling %s.Unmap().", this));
}

MaybeError BufferBase::Unmap() {
    if (mState == BufferState::Destroyed) {
        return {};
    }

    // Make sure writes are now visibile to the GPU if we used a staging buffer.
    if (mState == BufferState::MappedAtCreation && mStagingBuffer != nullptr) {
        DAWN_TRY(CopyFromStagingBuffer());
    }
    UnmapInternal(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback);
    return {};
}

void BufferBase::UnmapInternal(WGPUBufferMapAsyncStatus callbackStatus) {
    // Unmaps resources on the backend.
    if (mState == BufferState::PendingMap) {
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(
            PrepareMappingCallback(mLastMapID, callbackStatus));
        UnmapImpl();
    } else if (mState == BufferState::Mapped) {
        UnmapImpl();
    } else if (mState == BufferState::MappedAtCreation) {
        if (!IsError() && mSize != 0 && IsCPUWritableAtCreation()) {
            UnmapImpl();
        }
    }

    mState = BufferState::Unmapped;
}

MaybeError BufferBase::ValidateMapAsync(wgpu::MapMode mode,
                                        size_t offset,
                                        size_t size,
                                        WGPUBufferMapAsyncStatus* status) const {
    *status = WGPUBufferMapAsyncStatus_DeviceLost;
    DAWN_TRY(GetDevice()->ValidateIsAlive());

    *status = WGPUBufferMapAsyncStatus_ValidationError;
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(uint64_t(offset) > mSize,
                    "Mapping offset (%u) is larger than the size (%u) of %s.", offset, mSize, this);

    DAWN_INVALID_IF(offset % 8 != 0, "Offset (%u) must be a multiple of 8.", offset);
    DAWN_INVALID_IF(size % 4 != 0, "Size (%u) must be a multiple of 4.", size);

    DAWN_INVALID_IF(uint64_t(size) > mSize - uint64_t(offset),
                    "Mapping range (offset:%u, size: %u) doesn't fit in the size (%u) of %s.",
                    offset, size, mSize, this);

    switch (mState) {
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return DAWN_VALIDATION_ERROR("%s is already mapped.", this);
        case BufferState::PendingMap:
            DAWN_UNREACHABLE();
        case BufferState::Destroyed:
            return DAWN_VALIDATION_ERROR("%s is destroyed.", this);
        case BufferState::HostMappedPersistent:
            return DAWN_VALIDATION_ERROR("Host-mapped %s cannot be mapped again.", this);
        case BufferState::Unmapped:
            break;
    }

    bool isReadMode = mode & wgpu::MapMode::Read;
    bool isWriteMode = mode & wgpu::MapMode::Write;
    DAWN_INVALID_IF(!(isReadMode ^ isWriteMode), "Map mode (%s) is not one of %s or %s.", mode,
                    wgpu::MapMode::Write, wgpu::MapMode::Read);

    if (mode & wgpu::MapMode::Read) {
        DAWN_INVALID_IF(!(mUsage & wgpu::BufferUsage::MapRead),
                        "The buffer usages (%s) do not contain %s.", mUsage,
                        wgpu::BufferUsage::MapRead);
    } else {
        DAWN_ASSERT(mode & wgpu::MapMode::Write);
        DAWN_INVALID_IF(!(mUsage & wgpu::BufferUsage::MapWrite),
                        "The buffer usages (%s) do not contain %s.", mUsage,
                        wgpu::BufferUsage::MapWrite);
    }

    *status = WGPUBufferMapAsyncStatus_Success;
    return {};
}

bool BufferBase::CanGetMappedRange(bool writable, size_t offset, size_t size) const {
    if (offset % 8 != 0 || offset < mMapOffset || offset > mSize) {
        return false;
    }

    size_t rangeSize = size == WGPU_WHOLE_MAP_SIZE ? mSize - offset : size;

    if (rangeSize % 4 != 0 || rangeSize > mMapSize) {
        return false;
    }

    size_t offsetInMappedRange = offset - mMapOffset;
    if (offsetInMappedRange > mMapSize - rangeSize) {
        return false;
    }

    // Note that:
    //
    //   - We don't check that the device is alive because the application can ask for the
    //     mapped pointer before it knows, and even Dawn knows, that the device was lost, and
    //     still needs to work properly.
    //   - We don't check that the object is alive because we need to return mapped pointers
    //     for error buffers too.

    switch (mState) {
        // It is never valid to call GetMappedRange on a host-mapped buffer.
        // TODO(crbug.com/dawn/2018): consider returning the same pointer here.
        case BufferState::HostMappedPersistent:
            return false;

        // Writeable Buffer::GetMappedRange is always allowed when mapped at creation.
        case BufferState::MappedAtCreation:
            return true;

        case BufferState::Mapped:
            DAWN_ASSERT(bool{mMapMode & wgpu::MapMode::Read} ^
                        bool{mMapMode & wgpu::MapMode::Write});
            return !writable || (mMapMode & wgpu::MapMode::Write);

        case BufferState::PendingMap:
        case BufferState::Unmapped:
        case BufferState::Destroyed:
            return false;
    }
    DAWN_UNREACHABLE();
}

MaybeError BufferBase::ValidateUnmap() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_INVALID_IF(mState == BufferState::HostMappedPersistent,
                    "Persistently mapped buffer cannot be unmapped.");
    return {};
}

void BufferBase::CallbackOnMapRequestCompleted(MapRequestID mapID,
                                               WGPUBufferMapAsyncStatus status) {
    {
        // This is called from a callback, and no lock will be held by default. Hence, we need to
        // lock the mutex now because this will modify the buffer's states.
        auto deviceLock(GetDevice()->GetScopedLock());
        if (mapID == mLastMapID && status == WGPUBufferMapAsyncStatus_Success &&
            mState == BufferState::PendingMap) {
            mState = BufferState::Mapped;
        }
    }

    auto cb = PrepareMappingCallback(mapID, status);
    cb();
}

bool BufferBase::NeedsInitialization() const {
    return !mIsDataInitialized && GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse);
}

bool BufferBase::IsDataInitialized() const {
    return mIsDataInitialized;
}

void BufferBase::SetIsDataInitialized() {
    mIsDataInitialized = true;
}

void BufferBase::MarkUsedInPendingCommands() {
    ExecutionSerial serial = GetDevice()->GetPendingCommandSerial();
    DAWN_ASSERT(serial >= mLastUsageSerial);
    mLastUsageSerial = serial;
}

bool BufferBase::IsFullBufferRange(uint64_t offset, uint64_t size) const {
    return offset == 0 && size == GetSize();
}

}  // namespace dawn::native
