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

#ifndef DAWNNATIVE_BUFFER_H_
#define DAWNNATIVE_BUFFER_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    MaybeError ValidateBufferDescriptor(DeviceBase* device, const BufferDescriptor* descriptor);

    static constexpr dawn::BufferUsageBit kReadOnlyBufferUsages =
        dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferSrc |
        dawn::BufferUsageBit::Index | dawn::BufferUsageBit::Vertex | dawn::BufferUsageBit::Uniform;

    static constexpr dawn::BufferUsageBit kWritableBufferUsages =
        dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferDst |
        dawn::BufferUsageBit::Storage;

    class BufferBase : public ObjectBase {
        enum class BufferState {
            Unmapped,
            Mapped,
            Destroyed,
        };

      public:
        BufferBase(DeviceBase* device, const BufferDescriptor* descriptor);
        ~BufferBase();

        static BufferBase* MakeError(DeviceBase* device);

        uint32_t GetSize() const;
        dawn::BufferUsageBit GetUsage() const;

        MaybeError ValidateCanUseInSubmitNow() const;

        // Dawn API
        void SetSubData(uint32_t start, uint32_t count, const uint8_t* data);
        void MapReadAsync(dawnBufferMapReadCallback callback, dawnCallbackUserdata userdata);
        void MapWriteAsync(dawnBufferMapWriteCallback callback, dawnCallbackUserdata userdata);
        void Unmap();
        void Destroy();

      protected:
        BufferBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        void CallMapReadCallback(uint32_t serial,
                                 dawnBufferMapAsyncStatus status,
                                 const void* pointer,
                                 uint32_t dataLength);
        void CallMapWriteCallback(uint32_t serial,
                                  dawnBufferMapAsyncStatus status,
                                  void* pointer,
                                  uint32_t dataLength);

      private:
        virtual MaybeError SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data);
        virtual void MapReadAsyncImpl(uint32_t serial) = 0;
        virtual void MapWriteAsyncImpl(uint32_t serial) = 0;
        virtual void UnmapImpl() = 0;

        MaybeError ValidateSetSubData(uint32_t start, uint32_t count) const;
        MaybeError ValidateMap(dawn::BufferUsageBit requiredUsage) const;
        MaybeError ValidateUnmap() const;
        MaybeError ValidateDestroy() const;

        uint32_t mSize = 0;
        dawn::BufferUsageBit mUsage = dawn::BufferUsageBit::None;

        dawnBufferMapReadCallback mMapReadCallback = nullptr;
        dawnBufferMapWriteCallback mMapWriteCallback = nullptr;
        dawnCallbackUserdata mMapUserdata = 0;
        uint32_t mMapSerial = 0;

        BufferState mState;
    };

    // This builder class is kept around purely for testing but should not be used.
    class BufferBuilder : public Builder<BufferBase> {
      public:
        BufferBuilder(DeviceBase* device) : Builder(device) {
            UNREACHABLE();
        }

        void SetSize(uint32_t) {
            UNREACHABLE();
        }
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BUFFER_H_
