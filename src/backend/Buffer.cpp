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

#include "backend/Buffer.h"

#include "backend/Device.h"
#include "common/Assert.h"

#include <cstdio>
#include <utility>

namespace backend {

    // Buffer

    BufferBase::BufferBase(BufferBuilder* builder)
        : mDevice(builder->mDevice), mSize(builder->mSize), mAllowedUsage(builder->mAllowedUsage) {
    }

    BufferBase::~BufferBase() {
        if (mIsMapped) {
            CallMapReadCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
            CallMapWriteCallback(mMapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr);
        }
    }

    BufferViewBuilder* BufferBase::CreateBufferViewBuilder() {
        return new BufferViewBuilder(mDevice, this);
    }

    DeviceBase* BufferBase::GetDevice() const {
        return mDevice;
    }

    uint32_t BufferBase::GetSize() const {
        return mSize;
    }

    dawn::BufferUsageBit BufferBase::GetAllowedUsage() const {
        return mAllowedUsage;
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
        if (start + count > GetSize()) {
            mDevice->HandleError("Buffer subdata out of range");
            return;
        }

        if (!(mAllowedUsage & dawn::BufferUsageBit::TransferDst)) {
            mDevice->HandleError("Buffer needs the transfer dst usage bit");
            return;
        }

        SetSubDataImpl(start, count, data);
    }

    void BufferBase::MapReadAsync(uint32_t start,
                                  uint32_t size,
                                  dawnBufferMapReadCallback callback,
                                  dawnCallbackUserdata userdata) {
        if (!ValidateMapBase(start, size, dawn::BufferUsageBit::MapRead)) {
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
        if (!ValidateMapBase(start, size, dawn::BufferUsageBit::MapWrite)) {
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
        if (!mIsMapped) {
            mDevice->HandleError("Buffer wasn't mapped");
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

    bool BufferBase::ValidateMapBase(uint32_t start,
                                     uint32_t size,
                                     dawn::BufferUsageBit requiredUsage) {
        // TODO(cwallez@chromium.org): check for overflows.
        if (start + size > GetSize()) {
            mDevice->HandleError("Buffer map read out of range");
            return false;
        }

        if (mIsMapped) {
            mDevice->HandleError("Buffer already mapped");
            return false;
        }

        if (!(mAllowedUsage & requiredUsage)) {
            mDevice->HandleError("Buffer needs the correct map usage bit");
            return false;
        }

        return true;
    }

    // BufferBuilder

    enum BufferSetProperties {
        BUFFER_PROPERTY_ALLOWED_USAGE = 0x1,
        BUFFER_PROPERTY_SIZE = 0x2,
    };

    BufferBuilder::BufferBuilder(DeviceBase* device) : Builder(device) {
    }

    BufferBase* BufferBuilder::GetResultImpl() {
        constexpr int allProperties = BUFFER_PROPERTY_ALLOWED_USAGE | BUFFER_PROPERTY_SIZE;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Buffer missing properties");
            return nullptr;
        }

        const dawn::BufferUsageBit kMapWriteAllowedUsages =
            dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc;
        if (mAllowedUsage & dawn::BufferUsageBit::MapWrite &&
            (mAllowedUsage & kMapWriteAllowedUsages) != mAllowedUsage) {
            HandleError("Only TransferSrc is allowed with MapWrite");
            return nullptr;
        }

        const dawn::BufferUsageBit kMapReadAllowedUsages =
            dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;
        if (mAllowedUsage & dawn::BufferUsageBit::MapRead &&
            (mAllowedUsage & kMapReadAllowedUsages) != mAllowedUsage) {
            HandleError("Only TransferDst is allowed with MapRead");
            return nullptr;
        }

        return mDevice->CreateBuffer(this);
    }

    void BufferBuilder::SetAllowedUsage(dawn::BufferUsageBit usage) {
        if ((mPropertiesSet & BUFFER_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Buffer allowedUsage property set multiple times");
            return;
        }

        mAllowedUsage = usage;
        mPropertiesSet |= BUFFER_PROPERTY_ALLOWED_USAGE;
    }

    void BufferBuilder::SetSize(uint32_t size) {
        if ((mPropertiesSet & BUFFER_PROPERTY_SIZE) != 0) {
            HandleError("Buffer size property set multiple times");
            return;
        }

        mSize = size;
        mPropertiesSet |= BUFFER_PROPERTY_SIZE;
    }

    // BufferViewBase

    BufferViewBase::BufferViewBase(BufferViewBuilder* builder)
        : mBuffer(std::move(builder->mBuffer)), mSize(builder->mSize), mOffset(builder->mOffset) {
    }

    BufferBase* BufferViewBase::GetBuffer() {
        return mBuffer.Get();
    }

    uint32_t BufferViewBase::GetSize() const {
        return mSize;
    }

    uint32_t BufferViewBase::GetOffset() const {
        return mOffset;
    }

    // BufferViewBuilder

    enum BufferViewSetProperties {
        BUFFER_VIEW_PROPERTY_EXTENT = 0x1,
    };

    BufferViewBuilder::BufferViewBuilder(DeviceBase* device, BufferBase* buffer)
        : Builder(device), mBuffer(buffer) {
    }

    BufferViewBase* BufferViewBuilder::GetResultImpl() {
        constexpr int allProperties = BUFFER_VIEW_PROPERTY_EXTENT;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Buffer view missing properties");
            return nullptr;
        }

        return mDevice->CreateBufferView(this);
    }

    void BufferViewBuilder::SetExtent(uint32_t offset, uint32_t size) {
        if ((mPropertiesSet & BUFFER_VIEW_PROPERTY_EXTENT) != 0) {
            HandleError("Buffer view extent property set multiple times");
            return;
        }

        uint64_t viewEnd = static_cast<uint64_t>(offset) + static_cast<uint64_t>(size);
        if (viewEnd > static_cast<uint64_t>(mBuffer->GetSize())) {
            HandleError("Buffer view end is OOB");
            return;
        }

        mOffset = offset;
        mSize = size;
        mPropertiesSet |= BUFFER_VIEW_PROPERTY_EXTENT;
    }

}  // namespace backend
