// Copyright 2017 The NXT Authors
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
        : mDevice(builder->mDevice),
          mSize(builder->mSize),
          mAllowedUsage(builder->mAllowedUsage),
          mCurrentUsage(builder->mCurrentUsage) {
    }

    BufferBase::~BufferBase() {
        if (mIsMapped) {
            CallMapReadCallback(mMapReadSerial, NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr);
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

    nxt::BufferUsageBit BufferBase::GetAllowedUsage() const {
        return mAllowedUsage;
    }

    nxt::BufferUsageBit BufferBase::GetUsage() const {
        return mCurrentUsage;
    }

    void BufferBase::CallMapReadCallback(uint32_t serial,
                                         nxtBufferMapReadStatus status,
                                         const void* pointer) {
        if (mMapReadCallback && serial == mMapReadSerial) {
            // Tag the callback as fired before firing it, otherwise it could fire a second time if
            // for example buffer.Unmap() is called inside the application-provided callback.
            nxtBufferMapReadCallback callback = mMapReadCallback;
            mMapReadCallback = nullptr;
            callback(status, pointer, mMapReadUserdata);
        }
    }

    void BufferBase::SetSubData(uint32_t start, uint32_t count, const uint32_t* data) {
        if ((start + count) * sizeof(uint32_t) > GetSize()) {
            mDevice->HandleError("Buffer subdata out of range");
            return;
        }

        if (!(mCurrentUsage & nxt::BufferUsageBit::TransferDst)) {
            mDevice->HandleError("Buffer needs the transfer dst usage bit");
            return;
        }

        SetSubDataImpl(start, count, data);
    }

    void BufferBase::MapReadAsync(uint32_t start,
                                  uint32_t size,
                                  nxtBufferMapReadCallback callback,
                                  nxtCallbackUserdata userdata) {
        if (start + size > GetSize()) {
            mDevice->HandleError("Buffer map read out of range");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        if (!(mCurrentUsage & nxt::BufferUsageBit::MapRead)) {
            mDevice->HandleError("Buffer needs the map read usage bit");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        if (mIsMapped) {
            mDevice->HandleError("Buffer already mapped");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mMapReadSerial++;
        mMapReadCallback = callback;
        mMapReadUserdata = userdata;
        MapReadAsyncImpl(mMapReadSerial, start, size);
        mIsMapped = true;
    }

    void BufferBase::Unmap() {
        if (!mIsMapped) {
            mDevice->HandleError("Buffer wasn't mapped");
            return;
        }

        // A map request can only be called once, so this will fire only if the request wasn't
        // completed before the Unmap
        CallMapReadCallback(mMapReadSerial, NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr);
        UnmapImpl();
        mIsMapped = false;
    }

    bool BufferBase::IsFrozen() const {
        return mIsFrozen;
    }

    bool BufferBase::HasFrozenUsage(nxt::BufferUsageBit usage) const {
        return mIsFrozen && (usage & mAllowedUsage);
    }

    bool BufferBase::IsUsagePossible(nxt::BufferUsageBit allowedUsage, nxt::BufferUsageBit usage) {
        const nxt::BufferUsageBit allReadBits =
            nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferSrc |
            nxt::BufferUsageBit::Index | nxt::BufferUsageBit::Vertex | nxt::BufferUsageBit::Uniform;
        bool allowed = (usage & allowedUsage) == usage;
        bool readOnly = (usage & allReadBits) == usage;
        bool singleUse = nxt::HasZeroOrOneBits(usage);
        return allowed && (readOnly || singleUse);
    }

    bool BufferBase::IsTransitionPossible(nxt::BufferUsageBit usage) const {
        if (mIsFrozen || mIsMapped) {
            return false;
        }
        return IsUsagePossible(mAllowedUsage, usage);
    }

    void BufferBase::UpdateUsageInternal(nxt::BufferUsageBit usage) {
        ASSERT(IsTransitionPossible(usage));
        mCurrentUsage = usage;
    }

    void BufferBase::TransitionUsage(nxt::BufferUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            mDevice->HandleError("Buffer frozen or usage not allowed");
            return;
        }
        TransitionUsageImpl(mCurrentUsage, usage);
        mCurrentUsage = usage;
    }

    void BufferBase::FreezeUsage(nxt::BufferUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            mDevice->HandleError("Buffer frozen or usage not allowed");
            return;
        }
        mAllowedUsage = usage;
        TransitionUsageImpl(mCurrentUsage, usage);
        mCurrentUsage = usage;
        mIsFrozen = true;
    }

    // BufferBuilder

    enum BufferSetProperties {
        BUFFER_PROPERTY_ALLOWED_USAGE = 0x1,
        BUFFER_PROPERTY_INITIAL_USAGE = 0x2,
        BUFFER_PROPERTY_SIZE = 0x4,
    };

    BufferBuilder::BufferBuilder(DeviceBase* device) : Builder(device) {
    }

    BufferBase* BufferBuilder::GetResultImpl() {
        constexpr int allProperties = BUFFER_PROPERTY_ALLOWED_USAGE | BUFFER_PROPERTY_SIZE;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Buffer missing properties");
            return nullptr;
        }

        const nxt::BufferUsageBit kMapWriteAllowedUsages =
            nxt::BufferUsageBit::MapWrite | nxt::BufferUsageBit::TransferSrc;
        if (mAllowedUsage & nxt::BufferUsageBit::MapWrite &&
            (mAllowedUsage & kMapWriteAllowedUsages) != mAllowedUsage) {
            HandleError("Only TransferSrc is allowed with MapWrite");
            return nullptr;
        }

        const nxt::BufferUsageBit kMapReadAllowedUsages =
            nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst;
        if (mAllowedUsage & nxt::BufferUsageBit::MapRead &&
            (mAllowedUsage & kMapReadAllowedUsages) != mAllowedUsage) {
            HandleError("Only TransferDst is allowed with MapRead");
            return nullptr;
        }

        if (!BufferBase::IsUsagePossible(mAllowedUsage, mCurrentUsage)) {
            HandleError("Initial buffer usage is not allowed");
            return nullptr;
        }

        return mDevice->CreateBuffer(this);
    }

    void BufferBuilder::SetAllowedUsage(nxt::BufferUsageBit usage) {
        if ((mPropertiesSet & BUFFER_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Buffer allowedUsage property set multiple times");
            return;
        }

        mAllowedUsage = usage;
        mPropertiesSet |= BUFFER_PROPERTY_ALLOWED_USAGE;
    }

    void BufferBuilder::SetInitialUsage(nxt::BufferUsageBit usage) {
        if ((mPropertiesSet & BUFFER_PROPERTY_INITIAL_USAGE) != 0) {
            HandleError("Buffer initialUsage property set multiple times");
            return;
        }

        mCurrentUsage = usage;
        mPropertiesSet |= BUFFER_PROPERTY_INITIAL_USAGE;
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
