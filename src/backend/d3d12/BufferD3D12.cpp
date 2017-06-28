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
#include "ResourceAllocator.h"
#include "ResourceUploader.h"

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
        resourceDescriptor.Width = GetD3D12Size();
        resourceDescriptor.Height = 1;
        resourceDescriptor.DepthOrArraySize = 1;
        resourceDescriptor.MipLevels = 1;
        resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

        resource = device->GetResourceAllocator()->Allocate(D3D12_HEAP_TYPE_DEFAULT, resourceDescriptor, D3D12BufferUsage(GetUsage()));
    }

    Buffer::~Buffer() {
        device->GetResourceAllocator()->Release(resource);
    }

    uint32_t Buffer::GetD3D12Size() const {
        // TODO(enga@google.com): TODO investigate if this needs to be a constraint at the API level
        return ((GetSize() + 256 - 1) / 256) * 256; // size is required to be 256-byte aligned.
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
        device->GetResourceUploader()->BufferSubData(resource, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
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


    BufferView::BufferView(Device* device, BufferViewBuilder* builder)
        : BufferViewBase(builder), device(device) {

        cbvDesc.BufferLocation = ToBackend(GetBuffer())->GetVA() + GetOffset();
        cbvDesc.SizeInBytes = GetD3D12Size();

        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = GetOffset();
        uavDesc.Buffer.NumElements = GetD3D12Size();
        uavDesc.Buffer.StructureByteStride = 1;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    }

    uint32_t BufferView::GetD3D12Size() const {
        // TODO(enga@google.com): TODO investigate if this needs to be a constraint at the API level
        return ((GetSize() + 256 - 1) / 256) * 256; // size is required to be 256-byte aligned.
    }

    const D3D12_CONSTANT_BUFFER_VIEW_DESC& BufferView::GetCBVDescriptor() const {
        return cbvDesc;
    }

    const D3D12_UNORDERED_ACCESS_VIEW_DESC& BufferView::GetUAVDescriptor() const {
        return uavDesc;
    }

}
}
