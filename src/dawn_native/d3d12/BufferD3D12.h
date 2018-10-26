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

#ifndef DAWNNATIVE_D3D12_BUFFERD3D12_H_
#define DAWNNATIVE_D3D12_BUFFERD3D12_H_

#include "common/SerialQueue.h"
#include "dawn_native/Buffer.h"

#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class Device;

    class Buffer : public BufferBase {
      public:
        Buffer(Device* device, const BufferDescriptor* descriptor);
        ~Buffer();

        uint32_t GetD3D12Size() const;
        ComPtr<ID3D12Resource> GetD3D12Resource();
        D3D12_GPU_VIRTUAL_ADDRESS GetVA() const;
        void OnMapCommandSerialFinished(uint32_t mapSerial, void* data, bool isWrite);

        void TransitionUsageNow(ComPtr<ID3D12GraphicsCommandList> commandList,
                                dawn::BufferUsageBit usage);

      private:
        // Dawn API
        void SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) override;
        void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void UnmapImpl() override;

        ComPtr<ID3D12Resource> mResource;
        bool mFixedResourceState = false;
        dawn::BufferUsageBit mLastUsage = dawn::BufferUsageBit::None;
        D3D12_RANGE mWrittenMappedRange;
    };

    class BufferView : public BufferViewBase {
      public:
        BufferView(BufferViewBuilder* builder);

        uint32_t GetD3D12Size() const;
        const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetCBVDescriptor() const;
        const D3D12_UNORDERED_ACCESS_VIEW_DESC& GetUAVDescriptor() const;

      private:
        D3D12_CONSTANT_BUFFER_VIEW_DESC mCbvDesc;
        D3D12_UNORDERED_ACCESS_VIEW_DESC mUavDesc;
    };

    class MapRequestTracker {
      public:
        MapRequestTracker(Device* device);
        ~MapRequestTracker();

        void Track(Buffer* buffer, uint32_t mapSerial, void* data, bool isWrite);
        void Tick(Serial finishedSerial);

      private:
        Device* mDevice;

        struct Request {
            Ref<Buffer> buffer;
            uint32_t mapSerial;
            void* data;
            bool isWrite;
        };
        SerialQueue<Request> mInflightRequests;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_BUFFERD3D12_H_
