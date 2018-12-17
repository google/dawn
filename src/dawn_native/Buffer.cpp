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
#include "dawn_native/ValidationUtils_autogen.h"

#include <cstdio>
#include <utility>

namespace dawn_native {

    MaybeError ValidateBufferDescriptor(DeviceBase*, const BufferDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        DAWN_TRY(ValidateBufferUsageBit(descriptor->usage));

        dawn::BufferUsageBit usage = descriptor->usage;

        const dawn::BufferUsageBit kMapWriteAllowedUsages =
            dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc;
        if (usage & dawn::BufferUsageBit::MapWrite && (usage & kMapWriteAllowedUsages) != usage) {
            return DAWN_VALIDATION_ERROR("Only TransferSrc is allowed with MapWrite");
        }

        const dawn::BufferUsageBit kMapReadAllowedUsages =
            dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;
        if (usage & dawn::BufferUsageBit::MapRead && (usage & kMapReadAllowedUsages) != usage) {
            return DAWN_VALIDATION_ERROR("Only TransferDst is allowed with MapRead");
        }

        return {};
    }

    // Buffer

    BufferBase::BufferBase(DeviceBase* device, const BufferDescriptor* descriptor)
        : ObjectBase(device), mSize(descriptor->size), mUsage(descriptor->usage) {
    }

    BufferBase::~BufferBase() {
        if (mIsMapped) {
            CallMapReadCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
            CallMapWriteCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
        }
    }

    uint32_t BufferBase::GetSize() const {
        return mSize;
    }

    dawn::BufferUsageBit BufferBase::GetUsage() const {
        return mUsage;
    }

    MaybeError BufferBase::ValidateCanUseInSubmitNow() const {
        if (mIsMapped) {
            return DAWN_VALIDATION_ERROR("Buffer used in a submit while mapped");
        }
        return {};
    }

    void BufferBase::CallMapReadCallback(uint32_t serial,
                                         dawnBufferMapAsyncStatus status,
                                         const void* pointer) {
        if (mMapReadCallback != nullptr && serial == mMapSerial) {
            ASSERT(mMapWriteCallback == nullptr);
            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            dawnBufferMapReadCallback callback = mMapReadCallback;
            mMapReadCallback = nullptr;
            callback(status, pointer, mMapUserdata);
        }
    }

    void BufferBase::CallMapWriteCallback(uint32_t serial,
                                          dawnBufferMapAsyncStatus status,
                                          void* pointer) {
        if (mMapWriteCallback != nullptr && serial == mMapSerial) {
            ASSERT(mMapReadCallback == nullptr);
            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            dawnBufferMapWriteCallback callback = mMapWriteCallback;
            mMapWriteCallback = nullptr;
            callback(status, pointer, mMapUserdata);
        }
    }

    void BufferBase::SetSubData(uint32_t start, uint32_t count, const uint8_t* data) {
        if (GetDevice()->ConsumedError(ValidateSetSubData(start, count))) {
            return;
        }

        SetSubDataImpl(start, count, data);
    }

    void BufferBase::MapReadAsync(uint32_t start,
                                  uint32_t size,
                                  dawnBufferMapReadCallback callback,
                                  dawnCallbackUserdata userdata) {
        if (GetDevice()->ConsumedError(ValidateMap(start, size, dawn::BufferUsageBit::MapRead))) {
            callback(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
            return;
        }

        ASSERT(mMapWriteCallback == nullptr);

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mMapSerial++;
        mMapReadCallback = callback;
        mMapUserdata = userdata;
        mIsMapped = true;

        MapReadAsyncImpl(mMapSerial, start, size);
    }

    void BufferBase::MapWriteAsync(uint32_t start,
                                   uint32_t size,
                                   dawnBufferMapWriteCallback callback,
                                   dawnCallbackUserdata userdata) {
        if (GetDevice()->ConsumedError(ValidateMap(start, size, dawn::BufferUsageBit::MapWrite))) {
            callback(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
            return;
        }

        ASSERT(mMapReadCallback == nullptr);

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mMapSerial++;
        mMapWriteCallback = callback;
        mMapUserdata = userdata;
        mIsMapped = true;

        MapWriteAsyncImpl(mMapSerial, start, size);
    }

    void BufferBase::Unmap() {
        if (GetDevice()->ConsumedError(ValidateUnmap())) {
            return;
        }

        // A map request can only be called once, so this will fire only if the request wasn't
        // completed before the Unmap
        CallMapReadCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
        CallMapWriteCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
        UnmapImpl();
        mIsMapped = false;
        mMapReadCallback = nullptr;
        mMapWriteCallback = nullptr;
        mMapUserdata = 0;
    }

    MaybeError BufferBase::ValidateSetSubData(uint32_t start, uint32_t count) const {
        if (count > GetSize()) {
            return DAWN_VALIDATION_ERROR("Buffer subdata with too much data");
        }

        // Note that no overflow can happen because we already checked for GetSize() >= count
        if (start > GetSize() - count) {
            return DAWN_VALIDATION_ERROR("Buffer subdata out of range");
        }

        if (!(mUsage & dawn::BufferUsageBit::TransferDst)) {
            return DAWN_VALIDATION_ERROR("Buffer needs the transfer dst usage bit");
        }

        return {};
    }

    MaybeError BufferBase::ValidateMap(uint32_t start,
                                       uint32_t size,
                                       dawn::BufferUsageBit requiredUsage) const {
        if (size > GetSize()) {
            return DAWN_VALIDATION_ERROR("Buffer mapping with too big a region");
        }

        // Note that no overflow can happen because we already checked for GetSize() >= size
        if (start > GetSize() - size) {
            return DAWN_VALIDATION_ERROR("Buffer mapping out of range");
        }

        if (mIsMapped) {
            return DAWN_VALIDATION_ERROR("Buffer already mapped");
        }

        if (!(mUsage & requiredUsage)) {
            return DAWN_VALIDATION_ERROR("Buffer needs the correct map usage bit");
        }

        return {};
    }

    MaybeError BufferBase::ValidateUnmap() const {
        if (!mIsMapped) {
            return DAWN_VALIDATION_ERROR("Buffer wasn't mapped");
        }

        return {};
    }

}  // namespace dawn_native
