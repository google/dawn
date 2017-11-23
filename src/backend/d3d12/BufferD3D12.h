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

#ifndef BACKEND_D3D12_BUFFERD3D12_H_
#define BACKEND_D3D12_BUFFERD3D12_H_

#include "backend/Buffer.h"
#include "common/SerialQueue.h"

#include "backend/d3d12/d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class Device;

    class Buffer : public BufferBase {
        public:
            Buffer(Device* device, BufferBuilder* builder);
            ~Buffer();

            uint32_t GetD3D12Size() const;
            ComPtr<ID3D12Resource> GetD3D12Resource();
            D3D12_GPU_VIRTUAL_ADDRESS GetVA() const;
            bool GetResourceTransitionBarrier(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage, D3D12_RESOURCE_BARRIER* barrier);
            void OnMapReadCommandSerialFinished(uint32_t mapSerial, const void* data);

        private:
            Device* mDevice;
            ComPtr<ID3D12Resource> mResource;

            // NXT API
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
            void TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) override;

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

    class MapReadRequestTracker {
        public:
            MapReadRequestTracker(Device* device);
            ~MapReadRequestTracker();

            void Track(Buffer* buffer, uint32_t mapSerial, const void* data);
            void Tick(Serial finishedSerial);

        private:
            Device* mDevice;

            struct Request {
                Ref<Buffer> buffer;
                uint32_t mapSerial;
                const void* data;
            };
            SerialQueue<Request> mInflightRequests;
    };

}
}

#endif // BACKEND_D3D12_BUFFERD3D12_H_
