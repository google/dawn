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

#include "common/Assert.h"
#include "dawn_native/Device.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <cstdio>
#include <cstring>
#include <utility>

namespace dawn_native {

    namespace {

        class ErrorBuffer : public BufferBase {
          public:
            ErrorBuffer(DeviceBase* device) : BufferBase(device, ObjectBase::kError) {
            }

            static ErrorBuffer* MakeMapped(DeviceBase* device,
                                           uint64_t size,
                                           uint8_t** mappedPointer) {
                ASSERT(mappedPointer != nullptr);

                ErrorBuffer* buffer = new ErrorBuffer(device);
                buffer->mFakeMappedData =
                    std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[size]);
                *mappedPointer = buffer->mFakeMappedData.get();

                return buffer;
            }

            void ClearMappedData() {
                mFakeMappedData.reset();
            }

          private:
            bool IsMapWritable() const override {
                UNREACHABLE();
                return false;
            }

            MaybeError MapAtCreationImpl(uint8_t** mappedPointer) override {
                UNREACHABLE();
                return {};
            }

            MaybeError SetSubDataImpl(uint32_t start, uint32_t count, const void* data) override {
                UNREACHABLE();
                return {};
            }
            MaybeError MapReadAsyncImpl(uint32_t serial) override {
                UNREACHABLE();
                return {};
            }
            MaybeError MapWriteAsyncImpl(uint32_t serial) override {
                UNREACHABLE();
                return {};
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
            mUsage |= kReadOnlyStorage;
        }
    }

    BufferBase::BufferBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag), mState(BufferState::Unmapped) {
    }

    BufferBase::~BufferBase() {
        if (mState == BufferState::Mapped) {
            ASSERT(!IsError());
            CallMapReadCallback(mMapSerial, WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u);
            CallMapWriteCallback(mMapSerial, WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u);
        }
    }

    // static
    BufferBase* BufferBase::MakeError(DeviceBase* device) {
        return new ErrorBuffer(device);
    }

    // static
    BufferBase* BufferBase::MakeErrorMapped(DeviceBase* device,
                                            uint64_t size,
                                            uint8_t** mappedPointer) {
        return ErrorBuffer::MakeMapped(device, size, mappedPointer);
    }

    uint64_t BufferBase::GetSize() const {
        ASSERT(!IsError());
        return mSize;
    }

    wgpu::BufferUsage BufferBase::GetUsage() const {
        ASSERT(!IsError());
        return mUsage;
    }

    MaybeError BufferBase::MapAtCreation(uint8_t** mappedPointer) {
        ASSERT(!IsError());
        ASSERT(mappedPointer != nullptr);

        mState = BufferState::Mapped;

        if (IsMapWritable()) {
            DAWN_TRY(MapAtCreationImpl(mappedPointer));
            ASSERT(*mappedPointer != nullptr);
            return {};
        }

        // If any of these fail, the buffer will be deleted and replaced with an
        // error buffer.
        // TODO(enga): Suballocate and reuse memory from a larger staging buffer so we don't create
        // many small buffers.
        DAWN_TRY_ASSIGN(mStagingBuffer, GetDevice()->CreateStagingBuffer(GetSize()));

        ASSERT(mStagingBuffer->GetMappedPointer() != nullptr);
        *mappedPointer = reinterpret_cast<uint8_t*>(mStagingBuffer->GetMappedPointer());

        return {};
    }

    MaybeError BufferBase::ValidateCanUseInSubmitNow() const {
        ASSERT(!IsError());

        switch (mState) {
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Destroyed buffer used in a submit");
            case BufferState::Mapped:
                return DAWN_VALIDATION_ERROR("Buffer used in a submit while mapped");
            case BufferState::Unmapped:
                return {};
            default:
                UNREACHABLE();
        }
    }

    void BufferBase::CallMapReadCallback(uint32_t serial,
                                         WGPUBufferMapAsyncStatus status,
                                         const void* pointer,
                                         uint32_t dataLength) {
        ASSERT(!IsError());
        if (mMapReadCallback != nullptr && serial == mMapSerial) {
            ASSERT(mMapWriteCallback == nullptr);

            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            WGPUBufferMapReadCallback callback = mMapReadCallback;
            mMapReadCallback = nullptr;

            if (GetDevice()->IsLost()) {
                callback(WGPUBufferMapAsyncStatus_DeviceLost, nullptr, 0, mMapUserdata);
            } else {
                callback(status, pointer, dataLength, mMapUserdata);
            }
        }
    }

    void BufferBase::CallMapWriteCallback(uint32_t serial,
                                          WGPUBufferMapAsyncStatus status,
                                          void* pointer,
                                          uint32_t dataLength) {
        ASSERT(!IsError());
        if (mMapWriteCallback != nullptr && serial == mMapSerial) {
            ASSERT(mMapReadCallback == nullptr);

            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            WGPUBufferMapWriteCallback callback = mMapWriteCallback;
            mMapWriteCallback = nullptr;

            if (GetDevice()->IsLost()) {
                callback(WGPUBufferMapAsyncStatus_DeviceLost, nullptr, 0, mMapUserdata);
            } else {
                callback(status, pointer, dataLength, mMapUserdata);
            }
        }
    }

    void BufferBase::SetSubData(uint32_t start, uint32_t count, const void* data) {
        if (GetDevice()->ConsumedError(ValidateSetSubData(start, count))) {
            return;
        }
        ASSERT(!IsError());

        if (GetDevice()->ConsumedError(SetSubDataImpl(start, count, data))) {
            return;
        }
    }

    void BufferBase::MapReadAsync(WGPUBufferMapReadCallback callback, void* userdata) {
        WGPUBufferMapAsyncStatus status;
        if (GetDevice()->ConsumedError(ValidateMap(wgpu::BufferUsage::MapRead, &status))) {
            callback(status, nullptr, 0, userdata);
            return;
        }
        ASSERT(!IsError());

        ASSERT(mMapWriteCallback == nullptr);

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mMapSerial++;
        mMapReadCallback = callback;
        mMapUserdata = userdata;
        mState = BufferState::Mapped;

        if (GetDevice()->ConsumedError(MapReadAsyncImpl(mMapSerial))) {
            return;
        }
    }

    MaybeError BufferBase::SetSubDataImpl(uint32_t start, uint32_t count, const void* data) {
        DynamicUploader* uploader = GetDevice()->GetDynamicUploader();

        UploadHandle uploadHandle;
        DAWN_TRY_ASSIGN(uploadHandle,
                        uploader->Allocate(count, GetDevice()->GetPendingCommandSerial()));
        ASSERT(uploadHandle.mappedBuffer != nullptr);

        memcpy(uploadHandle.mappedBuffer, data, count);

        DAWN_TRY(GetDevice()->CopyFromStagingToBuffer(
            uploadHandle.stagingBuffer, uploadHandle.startOffset, this, start, count));

        return {};
    }

    void BufferBase::MapWriteAsync(WGPUBufferMapWriteCallback callback, void* userdata) {
        WGPUBufferMapAsyncStatus status;
        if (GetDevice()->ConsumedError(ValidateMap(wgpu::BufferUsage::MapWrite, &status))) {
            callback(status, nullptr, 0, userdata);
            return;
        }
        ASSERT(!IsError());

        ASSERT(mMapReadCallback == nullptr);

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mMapSerial++;
        mMapWriteCallback = callback;
        mMapUserdata = userdata;
        mState = BufferState::Mapped;

        if (GetDevice()->ConsumedError(MapWriteAsyncImpl(mMapSerial))) {
            return;
        }
    }

    void BufferBase::Destroy() {
        if (IsError()) {
            // It is an error to call Destroy() on an ErrorBuffer, but we still need to reclaim the
            // fake mapped staging data.
            reinterpret_cast<ErrorBuffer*>(this)->ClearMappedData();
        }
        if (GetDevice()->ConsumedError(ValidateDestroy())) {
            return;
        }
        ASSERT(!IsError());

        if (mState == BufferState::Mapped) {
            if (mStagingBuffer == nullptr) {
                Unmap();
            }
            mStagingBuffer.reset();
        }
        DestroyInternal();
    }

    MaybeError BufferBase::CopyFromStagingBuffer() {
        ASSERT(mStagingBuffer);
        DAWN_TRY(GetDevice()->CopyFromStagingToBuffer(mStagingBuffer.get(), 0, this, 0, GetSize()));

        DynamicUploader* uploader = GetDevice()->GetDynamicUploader();
        uploader->ReleaseStagingBuffer(std::move(mStagingBuffer));

        return {};
    }

    void BufferBase::Unmap() {
        if (IsError()) {
            // It is an error to call Unmap() on an ErrorBuffer, but we still need to reclaim the
            // fake mapped staging data.
            reinterpret_cast<ErrorBuffer*>(this)->ClearMappedData();
        }
        if (GetDevice()->ConsumedError(ValidateUnmap())) {
            return;
        }
        ASSERT(!IsError());

        if (mStagingBuffer != nullptr) {
            GetDevice()->ConsumedError(CopyFromStagingBuffer());
        } else {
            // A map request can only be called once, so this will fire only if the request wasn't
            // completed before the Unmap.
            // Callbacks are not fired if there is no callback registered, so this is correct for
            // CreateBufferMapped.
            CallMapReadCallback(mMapSerial, WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u);
            CallMapWriteCallback(mMapSerial, WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u);
            UnmapImpl();
        }
        mState = BufferState::Unmapped;
        mMapReadCallback = nullptr;
        mMapWriteCallback = nullptr;
        mMapUserdata = 0;
    }

    MaybeError BufferBase::ValidateSetSubData(uint32_t start, uint32_t count) const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));

        switch (mState) {
            case BufferState::Mapped:
                return DAWN_VALIDATION_ERROR("Buffer is mapped");
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Buffer is destroyed");
            case BufferState::Unmapped:
                break;
        }

        if (count > GetSize()) {
            return DAWN_VALIDATION_ERROR("Buffer subdata with too much data");
        }

        // Metal requests buffer to buffer copy size must be a multiple of 4 bytes on macOS
        if (count % 4 != 0) {
            return DAWN_VALIDATION_ERROR("Buffer subdata size must be a multiple of 4 bytes");
        }

        // Metal requests offset of buffer to buffer copy must be a multiple of 4 bytes on macOS
        if (start % 4 != 0) {
            return DAWN_VALIDATION_ERROR("Start position must be a multiple of 4 bytes");
        }

        // Note that no overflow can happen because we already checked for GetSize() >= count
        if (start > GetSize() - count) {
            return DAWN_VALIDATION_ERROR("Buffer subdata out of range");
        }

        if (!(mUsage & wgpu::BufferUsage::CopyDst)) {
            return DAWN_VALIDATION_ERROR("Buffer needs the CopyDst usage bit");
        }

        return {};
    }

    MaybeError BufferBase::ValidateMap(wgpu::BufferUsage requiredUsage,
                                       WGPUBufferMapAsyncStatus* status) const {
        *status = WGPUBufferMapAsyncStatus_DeviceLost;
        DAWN_TRY(GetDevice()->ValidateIsAlive());

        *status = WGPUBufferMapAsyncStatus_Error;
        DAWN_TRY(GetDevice()->ValidateObject(this));

        switch (mState) {
            case BufferState::Mapped:
                return DAWN_VALIDATION_ERROR("Buffer already mapped");
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

    MaybeError BufferBase::ValidateUnmap() const {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));

        switch (mState) {
            case BufferState::Mapped:
                // A buffer may be in the Mapped state if it was created with CreateBufferMapped
                // even if it did not have a mappable usage.
                return {};
            case BufferState::Unmapped:
                if ((mUsage & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) == 0) {
                    return DAWN_VALIDATION_ERROR("Buffer does not have map usage");
                }
                return {};
            case BufferState::Destroyed:
                return DAWN_VALIDATION_ERROR("Buffer is destroyed");
            default:
                UNREACHABLE();
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

    bool BufferBase::IsMapped() const {
        return mState == BufferState::Mapped;
    }

}  // namespace dawn_native
