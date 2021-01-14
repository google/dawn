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

#include "dawn_native/Buffer.h"

#include "common/Alloc.h"
#include "common/Assert.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/Queue.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <cstdio>
#include <cstring>
#include <utility>

namespace dawn_native {

    namespace {
        struct MapRequestTask : QueueBase::TaskInFlight {
            MapRequestTask(Ref<BufferBase> buffer, MapRequestID id)
                : buffer(std::move(buffer)), id(id) {
            }
            void Finish() override {
                buffer->OnMapRequestCompleted(id);
            }
            ~MapRequestTask() override = default;

          private:
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
                    bool isValidSize =
                        descriptor->size != 0 &&
                        descriptor->size < uint64_t(std::numeric_limits<size_t>::max());

                    if (isValidSize) {
                        mFakeMappedData =
                            std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(descriptor->size));
                    }
                }
            }

            void ClearMappedData() {
                mFakeMappedData.reset();
            }

          private:
            bool IsCPUWritableAtCreation() const override {
                UNREACHABLE();
            }

            MaybeError MapAtCreationImpl() override {
                UNREACHABLE();
            }

            MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override {
                UNREACHABLE();
            }
            void* GetMappedPointerImpl() override {
                return mFakeMappedData.get();
            }
            void UnmapImpl() override {
                UNREACHABLE();
            }
            void DestroyImpl() override {
                UNREACHABLE();
            }

            std::unique_ptr<uint8_t[]> mFakeMappedData;
        };

    }  // anonymous namespace

    MaybeError ValidateBufferDescriptor(DeviceBase*, const BufferDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        DAWN_TRY(ValidateBufferUsage(descriptor->usage));

        wgpu::BufferUsage usage = descriptor->usage;

        const wgpu::BufferUsage kMapWriteAllowedUsages =
            wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        if (usage & wgpu::BufferUsage::MapWrite && (usage & kMapWriteAllowedUsages) != usage) {
            return DAWN_VALIDATION_ERROR("Only CopySrc is allowed with MapWrite");
        }

        const wgpu::BufferUsage kMapReadAllowedUsages =
            wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        if (usage & wgpu::BufferUsage::MapRead && (usage & kMapReadAllowedUsages) != usage) {
            return DAWN_VALIDATION_ERROR("Only CopyDst is allowed with MapRead");
        }

        if (descriptor->mappedAtCreation && descriptor->size % 4 != 0) {
            return DAWN_VALIDATION_ERROR("size must be aligned to 4 when mappedAtCreation is true");
        }

        return {};
    }

    // Buffer

    BufferBase::BufferBase(DeviceBase* device, const BufferDescriptor* descriptor)
        : ObjectBase(device),
          mSize(descriptor->size),
          mUsage(descriptor->usage),
          mState(BufferState::Unmapped) {
        // Add readonly storage usage if the buffer has a storage usage. The validation rules in
        // ValidatePassResourceUsage will make sure we don't use both at the same
        // time.
        if (mUsage & wgpu::BufferUsage::Storage) {
            mUsage |= kReadOnlyStorageBuffer;
        }

        // TODO(hao.x.li@intel.com): This is just a workaround to make QueryResolve buffer pass the
        // binding group validation when used as an internal resource. Instead the buffer made with
        // QueryResolve usage would implicitly get StorageInternal usage which is only compatible
        // with StorageBufferInternal binding type in BGL, not StorageBuffer binding type.
        if (mUsage & wgpu::BufferUsage::QueryResolve) {
            mUsage |= wgpu::BufferUsage::Storage;
        }
    }

    BufferBase::BufferBase(DeviceBase* device,
                           const BufferDescriptor* descriptor,
                           ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag), mSize(descriptor->size), mState(BufferState::Unmapped) {
        if (descriptor->mappedAtCreation) {
            mState = BufferState::MappedAtCreation;
            mMapOffset = 0;
            mMapSize = mSize;
        }
    }

    BufferBase::~BufferBase() {
        if (mState == BufferState::Mapped) {
            ASSERT(!IsError());
            CallMapCallback(mLastMapID, WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
        }
    }

    // static
    BufferBase* BufferBase::MakeError(DeviceBase* device, const BufferDescriptor* descriptor) {
        return new ErrorBuffer(device, descriptor);
    }

    uint64_t BufferBase::GetSize() const {
        ASSERT(!IsError());
        return mSize;
    }

    wgpu::BufferUsage BufferBase::GetUsage() const {
        ASSERT(!IsError());
        return mUsage;
    }

    MaybeError BufferBase::MapAtCreation() {
        DAWN_TRY(MapAtCreationInternal());

        DeviceBase* device = GetDevice();
        if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            memset(GetMappedRange(0, mSize), uint8_t(0u), mSize);
            SetIsDataInitialized();
            device->IncrementLazyClearCountForTesting();
        } else if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            memset(GetMappedRange(0, mSize), uint8_t(1u), mSize);
        }

        return {};
    }

    MaybeError BufferBase::MapAtCreationInternal() {
        ASSERT(!IsError());
        mState = BufferState::MappedAtCreation;
        mMapOffset = 0;
        mMapSize = mSize;

        // 0-sized buffers are not supposed to be written to, Return back any non-null pointer.
        // Handle 0-sized buffers first so we don't try to map them in the backend.
        if (mSize == 0) {
            return {};
        }

        // Mappable buffers don't use a staging buffer and are just as if mapped through MapAsync.
        if (IsCPUWritableAtCreation()) {
            DAWN_TRY(MapAtCreationImpl());
        } else {
            // If any of these fail, the buffer will be deleted and replaced with an
            // error buffer.
            // TODO(enga): Suballocate and reuse memory from a larger staging buffer so we don't
            // create many small buffers.
            DAWN_TRY_ASSIGN(mStagingBuffer, GetDevice()->CreateStagingBuffer(GetSize()));
        }

        return {};
    }

    MaybeError BufferBase::ValidateCanUseOnQueueNow() const {
        ASSERT(!IsError());

        switch (mState) {
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Destroyed buffer used in a submit");
            case BufferState::Mapped:
            case BufferState::MappedAtCreation:
                return DAWN_VALIDATION_ERROR("Buffer used in a submit while mapped");
            case BufferState::Unmapped:
                return {};
        }
    }

    void BufferBase::CallMapCallback(MapRequestID mapID, WGPUBufferMapAsyncStatus status) {
        ASSERT(!IsError());
        if (mMapCallback != nullptr && mapID == mLastMapID) {
            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            WGPUBufferMapCallback callback = mMapCallback;
            mMapCallback = nullptr;

            if (GetDevice()->IsLost()) {
                callback(WGPUBufferMapAsyncStatus_DeviceLost, mMapUserdata);
            } else {
                callback(status, mMapUserdata);
            }
        }
    }

    void BufferBase::MapAsync(wgpu::MapMode mode,
                              size_t offset,
                              size_t size,
                              WGPUBufferMapCallback callback,
                              void* userdata) {
        // Handle the defaulting of size required by WebGPU, even if in webgpu_cpp.h it is not
        // possible to default the function argument (because there is the callback later in the
        // argument list)
        if (size == 0 && offset < mSize) {
            size = mSize - offset;
        }

        WGPUBufferMapAsyncStatus status;
        if (GetDevice()->ConsumedError(ValidateMapAsync(mode, offset, size, &status))) {
            if (callback) {
                callback(status, userdata);
            }
            return;
        }
        ASSERT(!IsError());

        mLastMapID++;
        mMapMode = mode;
        mMapOffset = offset;
        mMapSize = size;
        mMapCallback = callback;
        mMapUserdata = userdata;
        mState = BufferState::Mapped;

        if (GetDevice()->ConsumedError(MapAsyncImpl(mode, offset, size))) {
            CallMapCallback(mLastMapID, WGPUBufferMapAsyncStatus_DeviceLost);
            return;
        }
        std::unique_ptr<MapRequestTask> request =
            std::make_unique<MapRequestTask>(this, mLastMapID);
        GetDevice()->GetDefaultQueue()->TrackTask(std::move(request),
                                                  GetDevice()->GetPendingCommandSerial());
    }

    void* BufferBase::GetMappedRange(size_t offset, size_t size) {
        return GetMappedRangeInternal(true, offset, size);
    }

    const void* BufferBase::GetConstMappedRange(size_t offset, size_t size) {
        return GetMappedRangeInternal(false, offset, size);
    }

    void* BufferBase::GetMappedRangeInternal(bool writable, size_t offset, size_t size) {
        if (!CanGetMappedRange(writable, offset, size)) {
            return nullptr;
        }

        if (mStagingBuffer != nullptr) {
            return static_cast<uint8_t*>(mStagingBuffer->GetMappedPointer()) + offset;
        }
        if (mSize == 0) {
            return reinterpret_cast<uint8_t*>(intptr_t(0xCAFED00D));
        }
        uint8_t* start = static_cast<uint8_t*>(GetMappedPointerImpl());
        return start == nullptr ? nullptr : start + offset;
    }

    void BufferBase::Destroy() {
        if (IsError()) {
            // It is an error to call Destroy() on an ErrorBuffer, but we still need to reclaim the
            // fake mapped staging data.
            static_cast<ErrorBuffer*>(this)->ClearMappedData();
            mState = BufferState::Destroyed;
        }
        if (GetDevice()->ConsumedError(ValidateDestroy())) {
            return;
        }
        ASSERT(!IsError());

        if (mState == BufferState::Mapped) {
            UnmapInternal(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
        } else if (mState == BufferState::MappedAtCreation) {
            if (mStagingBuffer != nullptr) {
                mStagingBuffer.reset();
            } else if (mSize != 0) {
                ASSERT(IsCPUWritableAtCreation());
                UnmapInternal(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
            }
        }

        DestroyInternal();
    }

    MaybeError BufferBase::CopyFromStagingBuffer() {
        ASSERT(mStagingBuffer);
        if (GetSize() == 0) {
            return {};
        }

        DAWN_TRY(GetDevice()->CopyFromStagingToBuffer(mStagingBuffer.get(), 0, this, 0, GetSize()));

        DynamicUploader* uploader = GetDevice()->GetDynamicUploader();
        uploader->ReleaseStagingBuffer(std::move(mStagingBuffer));

        return {};
    }

    void BufferBase::Unmap() {
        UnmapInternal(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback);
    }

    void BufferBase::UnmapInternal(WGPUBufferMapAsyncStatus callbackStatus) {
        if (IsError()) {
            // It is an error to call Unmap() on an ErrorBuffer, but we still need to reclaim the
            // fake mapped staging data.
            static_cast<ErrorBuffer*>(this)->ClearMappedData();
            mState = BufferState::Unmapped;
        }
        if (GetDevice()->ConsumedError(ValidateUnmap())) {
            return;
        }
        ASSERT(!IsError());

        if (mState == BufferState::Mapped) {
            // A map request can only be called once, so this will fire only if the request wasn't
            // completed before the Unmap.
            // Callbacks are not fired if there is no callback registered, so this is correct for
            // mappedAtCreation = true.
            CallMapCallback(mLastMapID, callbackStatus);
            UnmapImpl();

            mMapCallback = nullptr;
            mMapUserdata = 0;

        } else if (mState == BufferState::MappedAtCreation) {
            if (mStagingBuffer != nullptr) {
                GetDevice()->ConsumedError(CopyFromStagingBuffer());
            } else if (mSize != 0) {
                ASSERT(IsCPUWritableAtCreation());
                UnmapImpl();
            }
        }

        mState = BufferState::Unmapped;
    }

    MaybeError BufferBase::ValidateMap(wgpu::BufferUsage requiredUsage,
                                       WGPUBufferMapAsyncStatus* status) const {
        *status = WGPUBufferMapAsyncStatus_DeviceLost;
        DAWN_TRY(GetDevice()->ValidateIsAlive());

        *status = WGPUBufferMapAsyncStatus_Error;
        DAWN_TRY(GetDevice()->ValidateObject(this));

        switch (mState) {
            case BufferState::Mapped:
            case BufferState::MappedAtCreation:
                return DAWN_VALIDATION_ERROR("Buffer is already mapped");
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Buffer is destroyed");
            case BufferState::Unmapped:
                break;
        }

        if (!(mUsage & requiredUsage)) {
            return DAWN_VALIDATION_ERROR("Buffer needs the correct map usage bit");
        }

        *status = WGPUBufferMapAsyncStatus_Success;
        return {};
    }

    MaybeError BufferBase::ValidateMapAsync(wgpu::MapMode mode,
                                            size_t offset,
                                            size_t size,
                                            WGPUBufferMapAsyncStatus* status) const {
        *status = WGPUBufferMapAsyncStatus_DeviceLost;
        DAWN_TRY(GetDevice()->ValidateIsAlive());

        *status = WGPUBufferMapAsyncStatus_Error;
        DAWN_TRY(GetDevice()->ValidateObject(this));

        if (offset % 8 != 0) {
            return DAWN_VALIDATION_ERROR("offset must be a multiple of 8");
        }

        if (size % 4 != 0) {
            return DAWN_VALIDATION_ERROR("size must be a multiple of 4");
        }

        if (uint64_t(offset) > mSize || uint64_t(size) > mSize - uint64_t(offset)) {
            return DAWN_VALIDATION_ERROR("size + offset must fit in the buffer");
        }

        switch (mState) {
            case BufferState::Mapped:
            case BufferState::MappedAtCreation:
                return DAWN_VALIDATION_ERROR("Buffer is already mapped");
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Buffer is destroyed");
            case BufferState::Unmapped:
                break;
        }

        bool isReadMode = mode & wgpu::MapMode::Read;
        bool isWriteMode = mode & wgpu::MapMode::Write;
        if (!(isReadMode ^ isWriteMode)) {
            return DAWN_VALIDATION_ERROR("Exactly one of Read or Write mode must be set");
        }

        if (mode & wgpu::MapMode::Read) {
            if (!(mUsage & wgpu::BufferUsage::MapRead)) {
                return DAWN_VALIDATION_ERROR("The buffer must have the MapRead usage");
            }
        } else {
            ASSERT(mode & wgpu::MapMode::Write);

            if (!(mUsage & wgpu::BufferUsage::MapWrite)) {
                return DAWN_VALIDATION_ERROR("The buffer must have the MapWrite usage");
            }
        }

        *status = WGPUBufferMapAsyncStatus_Success;
        return {};
    }

    bool BufferBase::CanGetMappedRange(bool writable, size_t offset, size_t size) const {
        if (offset % 8 != 0 || size % 4 != 0) {
            return false;
        }

        if (size > mMapSize || offset < mMapOffset) {
            return false;
        }

        size_t offsetInMappedRange = offset - mMapOffset;
        if (offsetInMappedRange > mMapSize - size) {
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
            // Writeable Buffer::GetMappedRange is always allowed when mapped at creation.
            case BufferState::MappedAtCreation:
                return true;

            case BufferState::Mapped:
                ASSERT(bool(mMapMode & wgpu::MapMode::Read) ^
                       bool(mMapMode & wgpu::MapMode::Write));
                return !writable || (mMapMode & wgpu::MapMode::Write);

            case BufferState::Unmapped:
            case BufferState::Destroyed:
                return false;
        }
    }

    MaybeError BufferBase::ValidateUnmap() const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));

        switch (mState) {
            case BufferState::Mapped:
            case BufferState::MappedAtCreation:
                // A buffer may be in the Mapped state if it was created with mappedAtCreation
                // even if it did not have a mappable usage.
                return {};
            case BufferState::Unmapped:
                return DAWN_VALIDATION_ERROR("Buffer is unmapped");
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Buffer is destroyed");
        }
    }

    MaybeError BufferBase::ValidateDestroy() const {
        DAWN_TRY(GetDevice()->ValidateObject(this));
        return {};
    }

    void BufferBase::DestroyInternal() {
        if (mState != BufferState::Destroyed) {
            DestroyImpl();
        }
        mState = BufferState::Destroyed;
    }

    void BufferBase::OnMapRequestCompleted(MapRequestID mapID) {
        CallMapCallback(mapID, WGPUBufferMapAsyncStatus_Success);
    }

    bool BufferBase::IsDataInitialized() const {
        return mIsDataInitialized;
    }

    void BufferBase::SetIsDataInitialized() {
        mIsDataInitialized = true;
    }

    bool BufferBase::IsFullBufferRange(uint64_t offset, uint64_t size) const {
        return offset == 0 && size == GetSize();
    }
}  // namespace dawn_native
