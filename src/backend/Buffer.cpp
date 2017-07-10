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

#include <utility>
#include <cstdio>

namespace backend {

    // Buffer

    BufferBase::BufferBase(BufferBuilder* builder)
        : device(builder->device),
          size(builder->size),
          allowedUsage(builder->allowedUsage),
          currentUsage(builder->currentUsage) {
    }

    BufferBase::~BufferBase() {
        if (mapped) {
            CallMapReadCallback(mapReadSerial, NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr);
        }
    }

    BufferViewBuilder* BufferBase::CreateBufferViewBuilder() {
        return new BufferViewBuilder(device, this);
    }

    DeviceBase* BufferBase::GetDevice() {
        return device;
    }

    uint32_t BufferBase::GetSize() const {
        return size;
    }

    nxt::BufferUsageBit BufferBase::GetAllowedUsage() const {
        return allowedUsage;
    }

    nxt::BufferUsageBit BufferBase::GetUsage() const {
        return currentUsage;
    }

    void BufferBase::CallMapReadCallback(uint32_t serial, nxtBufferMapReadStatus status, const void* pointer) {
        if (mapReadCallback && serial == mapReadSerial) {
            mapReadCallback(status, pointer, mapReadUserdata);
            mapReadCallback = nullptr;
        }
    }

    void BufferBase::SetSubData(uint32_t start, uint32_t count, const uint32_t* data) {
        if ((start + count) * sizeof(uint32_t) > GetSize()) {
            device->HandleError("Buffer subdata out of range");
            return;
        }

        if (!(currentUsage & nxt::BufferUsageBit::TransferDst)) {
            device->HandleError("Buffer needs the transfer dst usage bit");
            return;
        }

        SetSubDataImpl(start, count, data);
    }

    void BufferBase::MapReadAsync(uint32_t start, uint32_t size, nxtBufferMapReadCallback callback, nxtCallbackUserdata userdata) {
        if (start + size > GetSize()) {
            device->HandleError("Buffer map read out of range");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        if (!(currentUsage & nxt::BufferUsageBit::MapRead)) {
            device->HandleError("Buffer needs the map read usage bit");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        if (mapped) {
            device->HandleError("Buffer already mapped");
            callback(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata);
            return;
        }

        // TODO(cwallez@chromium.org): what to do on wraparound? Could cause crashes.
        mapReadSerial ++;
        mapReadCallback = callback;
        mapReadUserdata = userdata;
        MapReadAsyncImpl(mapReadSerial, start, size);
        mapped = true;
    }

    void BufferBase::Unmap() {
        if (!mapped) {
            device->HandleError("Buffer wasn't mapped");
            return;
        }

        // A map request can only be called once, so this will fire only if the request wasn't
        // completed before the Unmap
        CallMapReadCallback(mapReadSerial, NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr);
        UnmapImpl();
        mapped = false;
    }

    bool BufferBase::IsFrozen() const {
        return frozen;
    }

    bool BufferBase::HasFrozenUsage(nxt::BufferUsageBit usage) const {
        return frozen && (usage & allowedUsage);
    }

    bool BufferBase::IsUsagePossible(nxt::BufferUsageBit allowedUsage, nxt::BufferUsageBit usage) {
        const nxt::BufferUsageBit allReadBits =
            nxt::BufferUsageBit::MapRead |
            nxt::BufferUsageBit::TransferSrc |
            nxt::BufferUsageBit::Index |
            nxt::BufferUsageBit::Vertex |
            nxt::BufferUsageBit::Uniform;
        bool allowed = (usage & allowedUsage) == usage;
        bool readOnly = (usage & allReadBits) == usage;
        bool singleUse = nxt::HasZeroOrOneBits(usage);
        return allowed && (readOnly || singleUse);
    }

    bool BufferBase::IsTransitionPossible(nxt::BufferUsageBit usage) const {
        if (frozen || mapped) {
            return false;
        }
        return IsUsagePossible(allowedUsage, usage);
    }

    void BufferBase::UpdateUsageInternal(nxt::BufferUsageBit usage) {
        ASSERT(IsTransitionPossible(usage));
        currentUsage = usage;
    }

    void BufferBase::TransitionUsage(nxt::BufferUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            device->HandleError("Buffer frozen or usage not allowed");
            return;
        }
        TransitionUsageImpl(currentUsage, usage);
        currentUsage = usage;
    }

    void BufferBase::FreezeUsage(nxt::BufferUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            device->HandleError("Buffer frozen or usage not allowed");
            return;
        }
        allowedUsage = usage;
        TransitionUsageImpl(currentUsage, usage);
        currentUsage = usage;
        frozen = true;
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
        if ((propertiesSet & allProperties) != allProperties) {
            HandleError("Buffer missing properties");
            return nullptr;
        }

        const nxt::BufferUsageBit kMapWriteAllowedUsages = nxt::BufferUsageBit::MapWrite | nxt::BufferUsageBit::TransferSrc;
        if (allowedUsage & nxt::BufferUsageBit::MapWrite &&
            (allowedUsage & kMapWriteAllowedUsages) != allowedUsage) {
            HandleError("Only TransferSrc is allowed with MapWrite");
            return nullptr;
        }

        const nxt::BufferUsageBit kMapReadAllowedUsages = nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst;
        if (allowedUsage & nxt::BufferUsageBit::MapRead &&
            (allowedUsage & kMapReadAllowedUsages) != allowedUsage) {
            HandleError("Only TransferDst is allowed with MapRead");
            return nullptr;
        }

        if (!BufferBase::IsUsagePossible(allowedUsage, currentUsage)) {
            HandleError("Initial buffer usage is not allowed");
            return nullptr;
        }

        return device->CreateBuffer(this);
    }

    void BufferBuilder::SetAllowedUsage(nxt::BufferUsageBit usage) {
        if ((propertiesSet & BUFFER_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Buffer allowedUsage property set multiple times");
            return;
        }

        this->allowedUsage = usage;
        propertiesSet |= BUFFER_PROPERTY_ALLOWED_USAGE;
    }

    void BufferBuilder::SetInitialUsage(nxt::BufferUsageBit usage) {
        if ((propertiesSet & BUFFER_PROPERTY_INITIAL_USAGE) != 0) {
            HandleError("Buffer initialUsage property set multiple times");
            return;
        }

        this->currentUsage = usage;
        propertiesSet |= BUFFER_PROPERTY_INITIAL_USAGE;
    }

    void BufferBuilder::SetSize(uint32_t size) {
        if ((propertiesSet & BUFFER_PROPERTY_SIZE) != 0) {
            HandleError("Buffer size property set multiple times");
            return;
        }

        this->size = size;
        propertiesSet |= BUFFER_PROPERTY_SIZE;
    }

    // BufferViewBase

    BufferViewBase::BufferViewBase(BufferViewBuilder* builder)
        : buffer(std::move(builder->buffer)), size(builder->size), offset(builder->offset) {
    }

    BufferBase* BufferViewBase::GetBuffer() {
        return buffer.Get();
    }

    uint32_t BufferViewBase::GetSize() const {
        return size;
    }

    uint32_t BufferViewBase::GetOffset() const {
        return offset;
    }

    // BufferViewBuilder

    enum BufferViewSetProperties {
        BUFFER_VIEW_PROPERTY_EXTENT = 0x1,
    };

    BufferViewBuilder::BufferViewBuilder(DeviceBase* device, BufferBase* buffer)
        : Builder(device), buffer(buffer) {
    }

    BufferViewBase* BufferViewBuilder::GetResultImpl() {
        constexpr int allProperties = BUFFER_VIEW_PROPERTY_EXTENT;
        if ((propertiesSet & allProperties) != allProperties) {
            HandleError("Buffer view missing properties");
            return nullptr;
        }

        return device->CreateBufferView(this);
    }

    void BufferViewBuilder::SetExtent(uint32_t offset, uint32_t size) {
        if ((propertiesSet & BUFFER_VIEW_PROPERTY_EXTENT) != 0) {
            HandleError("Buffer view extent property set multiple times");
            return;
        }

        uint64_t viewEnd = static_cast<uint64_t>(offset) + static_cast<uint64_t>(size);
        if (viewEnd > static_cast<uint64_t>(buffer->GetSize())) {
            HandleError("Buffer view end is OOB");
            return;
        }

        this->offset = offset;
        this->size = size;
        propertiesSet |= BUFFER_VIEW_PROPERTY_EXTENT;
    }

}
