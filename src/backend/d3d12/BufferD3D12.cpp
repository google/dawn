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

#include "BufferD3D12.h"

#include "D3D12Backend.h"

namespace backend {
namespace d3d12 {

    namespace {
        D3D12_RESOURCE_STATES D3D12BufferUsage(nxt::BufferUsageBit usage) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            if (usage & nxt::BufferUsageBit::TransferSrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & nxt::BufferUsageBit::TransferDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & (nxt::BufferUsageBit::Vertex | nxt::BufferUsageBit::Uniform)) {
                resourceState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            }
            if (usage & nxt::BufferUsageBit::Index) {
                resourceState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
            }
            if (usage & nxt::BufferUsageBit::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }

            return resourceState;
        }
    }

    Buffer::Buffer(Device* device, BufferBuilder* builder)
        : BufferBase(builder), device(device) {

        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = GetSize();
        resourceDescriptor.Height = 1;
        resourceDescriptor.DepthOrArraySize = 1;
        resourceDescriptor.MipLevels = 1;
        resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        // TODO(enga@google.com): Use a ResourceAllocationManager
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescriptor,
            D3D12BufferUsage(GetUsage()),
            nullptr,
            IID_PPV_ARGS(&resource)
        ));
    }

    ComPtr<ID3D12Resource> Buffer::GetD3D12Resource() {
        return resource;
    }

    bool Buffer::GetResourceTransitionBarrier(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage, D3D12_RESOURCE_BARRIER* barrier) {
        D3D12_RESOURCE_STATES stateBefore = D3D12BufferUsage(currentUsage);
        D3D12_RESOURCE_STATES stateAfter = D3D12BufferUsage(targetUsage);

        if (stateBefore == stateAfter) {
            return false;
        }

        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->Transition.pResource = resource.Get();
        barrier->Transition.StateBefore = stateBefore;
        barrier->Transition.StateAfter = stateAfter;
        barrier->Transition.Subresource = 0;

        return true;
    }

    D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetVA() const {
        return resource->GetGPUVirtualAddress();
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        device->GetResourceUploader()->UploadToBuffer(resource, start * sizeof(uint32_t), count * sizeof(uint32_t), reinterpret_cast<const uint8_t*>(data));
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        // TODO(cwallez@chromium.org): Implement Map Read for the null backend
    }

    void Buffer::UnmapImpl() {
        // TODO(cwallez@chromium.org): Implement Map Read for the null backend
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) {
        D3D12_RESOURCE_BARRIER barrier;
        if (GetResourceTransitionBarrier(currentUsage, targetUsage, &barrier)) {
            device->GetPendingCommandList()->ResourceBarrier(1, &barrier);
        }
    }

}
}
