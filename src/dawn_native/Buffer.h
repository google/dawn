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

#ifndef BACKEND_BUFFER_H_
#define BACKEND_BUFFER_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RefCounted.h"

#include "dawn/dawncpp.h"

namespace backend {

    static constexpr dawn::BufferUsageBit kReadOnlyBufferUsages =
        dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferSrc |
        dawn::BufferUsageBit::Index | dawn::BufferUsageBit::Vertex | dawn::BufferUsageBit::Uniform;

    static constexpr dawn::BufferUsageBit kWritableBufferUsages =
        dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferDst |
        dawn::BufferUsageBit::Storage;

    class BufferBase : public RefCounted {
      public:
        BufferBase(BufferBuilder* builder);
        ~BufferBase();

        uint32_t GetSize() const;
        dawn::BufferUsageBit GetAllowedUsage() const;

        DeviceBase* GetDevice() const;

        // Dawn API
        BufferViewBuilder* CreateBufferViewBuilder();
        void SetSubData(uint32_t start, uint32_t count, const uint8_t* data);
        void MapReadAsync(uint32_t start,
                          uint32_t size,
                          dawnBufferMapReadCallback callback,
                          dawnCallbackUserdata userdata);
        void MapWriteAsync(uint32_t start,
                           uint32_t size,
                           dawnBufferMapWriteCallback callback,
                           dawnCallbackUserdata userdata);
        void Unmap();

      protected:
        void CallMapReadCallback(uint32_t serial,
                                 dawnBufferMapAsyncStatus status,
                                 const void* pointer);
        void CallMapWriteCallback(uint32_t serial, dawnBufferMapAsyncStatus status, void* pointer);

      private:
        virtual void SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) = 0;
        virtual void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t size) = 0;
        virtual void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t size) = 0;
        virtual void UnmapImpl() = 0;

        MaybeError ValidateSetSubData(uint32_t start, uint32_t count) const;
        MaybeError ValidateMap(uint32_t start,
                               uint32_t size,
                               dawn::BufferUsageBit requiredUsage) const;
        MaybeError ValidateUnmap() const;

        DeviceBase* mDevice;
        uint32_t mSize;
        dawn::BufferUsageBit mAllowedUsage = dawn::BufferUsageBit::None;

        dawnBufferMapReadCallback mMapReadCallback = nullptr;
        dawnBufferMapWriteCallback mMapWriteCallback = nullptr;
        dawnCallbackUserdata mMapUserdata = 0;
        uint32_t mMapSerial = 0;

        bool mIsMapped = false;
    };

    class BufferBuilder : public Builder<BufferBase> {
      public:
        BufferBuilder(DeviceBase* device);

        // Dawn API
        void SetAllowedUsage(dawn::BufferUsageBit usage);
        void SetInitialUsage(dawn::BufferUsageBit usage);
        void SetSize(uint32_t size);

      private:
        friend class BufferBase;

        BufferBase* GetResultImpl() override;

        uint32_t mSize;
        dawn::BufferUsageBit mAllowedUsage = dawn::BufferUsageBit::None;
        dawn::BufferUsageBit mCurrentUsage = dawn::BufferUsageBit::None;
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

        // Dawn API
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
