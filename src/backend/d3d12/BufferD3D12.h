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

#include "common/Buffer.h"

#include "d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class Device;

    class Buffer : public BufferBase {
        public:
            Buffer(Device* device, BufferBuilder* builder);
            ~Buffer();

            ComPtr<ID3D12Resource> GetD3D12Resource();
            D3D12_GPU_VIRTUAL_ADDRESS GetVA() const;
            bool GetResourceTransitionBarrier(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage, D3D12_RESOURCE_BARRIER* barrier);

        private:
            Device* device;
            ComPtr<ID3D12Resource> resource;

            // NXT API
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
            void TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) override;

    };

}
}

#endif // BACKEND_D3D12_BUFFERD3D12_H_
