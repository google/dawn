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

#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

#include <memory>

namespace dawn_native {

    MaybeError ValidateBufferDescriptor(DeviceBase* device, const BufferDescriptor* descriptor);

    // Add an extra buffer usage (readonly storage buffer usage) for render pass resource tracking
    static constexpr wgpu::BufferUsage kReadOnlyStorage =
        static_cast<wgpu::BufferUsage>(0x80000000);

    static constexpr wgpu::BufferUsage kReadOnlyBufferUsages =
        wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Index |
        wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Uniform | kReadOnlyStorage;

    static constexpr wgpu::BufferUsage kWritableBufferUsages =
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;

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
        static BufferBase* MakeErrorMapped(DeviceBase* device,
                                           uint64_t size,
                                           uint8_t** mappedPointer);

        uint64_t GetSize() const;
        wgpu::BufferUsage GetUsage() const;

        MaybeError MapAtCreation(uint8_t** mappedPointer);

        MaybeError ValidateCanUseInSubmitNow() const;

        // Dawn API
        void SetSubData(uint32_t start, uint32_t count, const void* data);
        void MapReadAsync(WGPUBufferMapReadCallback callback, void* userdata);
        void MapWriteAsync(WGPUBufferMapWriteCallback callback, void* userdata);
        void Unmap();
        void Destroy();

      protected:
        BufferBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        void CallMapReadCallback(uint32_t serial,
                                 WGPUBufferMapAsyncStatus status,
                                 const void* pointer,
                                 uint32_t dataLength);
        void CallMapWriteCallback(uint32_t serial,
                                  WGPUBufferMapAsyncStatus status,
                                  void* pointer,
                                  uint32_t dataLength);

        void DestroyInternal();

        bool IsMapped() const;

      private:
        virtual MaybeError MapAtCreationImpl(uint8_t** mappedPointer) = 0;
        virtual MaybeError SetSubDataImpl(uint32_t start, uint32_t count, const void* data);
        virtual MaybeError MapReadAsyncImpl(uint32_t serial) = 0;
        virtual MaybeError MapWriteAsyncImpl(uint32_t serial) = 0;
        virtual void UnmapImpl() = 0;
        virtual void DestroyImpl() = 0;

        virtual bool IsMapWritable() const = 0;
        MaybeError CopyFromStagingBuffer();

        MaybeError ValidateSetSubData(uint32_t start, uint32_t count) const;
        MaybeError ValidateMap(wgpu::BufferUsage requiredUsage,
                               WGPUBufferMapAsyncStatus* status) const;
        MaybeError ValidateUnmap() const;
        MaybeError ValidateDestroy() const;

        uint64_t mSize = 0;
        wgpu::BufferUsage mUsage = wgpu::BufferUsage::None;

        WGPUBufferMapReadCallback mMapReadCallback = nullptr;
        WGPUBufferMapWriteCallback mMapWriteCallback = nullptr;
        void* mMapUserdata = 0;
        uint32_t mMapSerial = 0;

        std::unique_ptr<StagingBufferBase> mStagingBuffer;

        BufferState mState;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BUFFER_H_
