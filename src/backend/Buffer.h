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

#ifndef BACKEND_BUFFER_H_
#define BACKEND_BUFFER_H_

#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class BufferBase : public RefCounted {
      public:
        BufferBase(BufferBuilder* builder);
        ~BufferBase();

        uint32_t GetSize() const;
        nxt::BufferUsageBit GetAllowedUsage() const;
        nxt::BufferUsageBit GetUsage() const;
        static bool IsUsagePossible(nxt::BufferUsageBit allowedUsage, nxt::BufferUsageBit usage);
        bool IsTransitionPossible(nxt::BufferUsageBit usage) const;
        bool IsFrozen() const;
        bool HasFrozenUsage(nxt::BufferUsageBit usage) const;
        void UpdateUsageInternal(nxt::BufferUsageBit usage);

        DeviceBase* GetDevice() const;

        // NXT API
        BufferViewBuilder* CreateBufferViewBuilder();
        void SetSubData(uint32_t start, uint32_t count, const uint32_t* data);
        void MapReadAsync(uint32_t start,
                          uint32_t size,
                          nxtBufferMapReadCallback callback,
                          nxtCallbackUserdata userdata);
        void MapWriteAsync(uint32_t start,
                           uint32_t size,
                           nxtBufferMapWriteCallback callback,
                           nxtCallbackUserdata userdata);
        void Unmap();
        void TransitionUsage(nxt::BufferUsageBit usage);
        void FreezeUsage(nxt::BufferUsageBit usage);

      protected:
        void CallMapReadCallback(uint32_t serial,
                                 nxtBufferMapAsyncStatus status,
                                 const void* pointer);
        void CallMapWriteCallback(uint32_t serial, nxtBufferMapAsyncStatus status, void* pointer);

      private:
        virtual void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) = 0;
        virtual void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t size) = 0;
        virtual void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t size) = 0;
        virtual void UnmapImpl() = 0;
        virtual void TransitionUsageImpl(nxt::BufferUsageBit currentUsage,
                                         nxt::BufferUsageBit targetUsage) = 0;

        bool ValidateMapBase(uint32_t start, uint32_t size, nxt::BufferUsageBit requiredUsage);

        DeviceBase* mDevice;
        uint32_t mSize;
        nxt::BufferUsageBit mAllowedUsage = nxt::BufferUsageBit::None;
        nxt::BufferUsageBit mCurrentUsage = nxt::BufferUsageBit::None;

        nxtBufferMapReadCallback mMapReadCallback = nullptr;
        nxtBufferMapWriteCallback mMapWriteCallback = nullptr;
        nxtCallbackUserdata mMapUserdata = 0;
        uint32_t mMapSerial = 0;

        bool mIsFrozen = false;
        bool mIsMapped = false;
    };

    class BufferBuilder : public Builder<BufferBase> {
      public:
        BufferBuilder(DeviceBase* device);

        // NXT API
        void SetAllowedUsage(nxt::BufferUsageBit usage);
        void SetInitialUsage(nxt::BufferUsageBit usage);
        void SetSize(uint32_t size);

      private:
        friend class BufferBase;

        BufferBase* GetResultImpl() override;

        uint32_t mSize;
        nxt::BufferUsageBit mAllowedUsage = nxt::BufferUsageBit::None;
        nxt::BufferUsageBit mCurrentUsage = nxt::BufferUsageBit::None;
        int mPropertiesSet = 0;
    };

    class BufferViewBase : public RefCounted {
      public:
        BufferViewBase(BufferViewBuilder* builder);

        BufferBase* GetBuffer();
        uint32_t GetSize() const;
        uint32_t GetOffset() const;

      private:
        Ref<BufferBase> mBuffer;
        uint32_t mSize;
        uint32_t mOffset;
    };

    class BufferViewBuilder : public Builder<BufferViewBase> {
      public:
        BufferViewBuilder(DeviceBase* device, BufferBase* buffer);

        // NXT API
        void SetExtent(uint32_t offset, uint32_t size);

      private:
        friend class BufferViewBase;

        BufferViewBase* GetResultImpl() override;

        Ref<BufferBase> mBuffer;
        uint32_t mOffset = 0;
        uint32_t mSize = 0;
        int mPropertiesSet = 0;
    };

}  // namespace backend

#endif  // BACKEND_BUFFER_H_
