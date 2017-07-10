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

#include "backend/d3d12/DescriptorHeapAllocator.h"

#include "backend/d3d12/D3D12Backend.h"
#include "common/Assert.h"

namespace backend {
namespace d3d12 {

    DescriptorHeapHandle::DescriptorHeapHandle() {
    }

    DescriptorHeapHandle::DescriptorHeapHandle(ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t sizeIncrement, uint32_t offset)
        : device(device), descriptorHeap(descriptorHeap), sizeIncrement(sizeIncrement), offset(offset) {
    }

    ID3D12DescriptorHeap* DescriptorHeapHandle::Get() const {
        return descriptorHeap.Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapHandle::GetCPUHandle(uint32_t index) const {
        ASSERT(descriptorHeap);
        auto handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += sizeIncrement * (index + offset);
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapHandle::GetGPUHandle(uint32_t index) const {
        ASSERT(descriptorHeap);
        auto handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += sizeIncrement * (index + offset);
        return handle;
    }


    DescriptorHeapAllocator::DescriptorHeapAllocator(Device* device)
        : device(device),
          sizeIncrements {
            device->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            device->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
            device->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
            device->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV),
          } {
    }

    DescriptorHeapHandle DescriptorHeapAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, uint32_t allocationSize, DescriptorHeapInfo* heapInfo, D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
        // TODO(enga@google.com): This is just a linear allocator so the heap will quickly run out of space causing a new one to be allocated
        // We should reuse heap subranges that have been released
        if (count == 0) {
            return DescriptorHeapHandle();
        }

        {
            // If the current pool for this type has space, linearly allocate count bytes in the pool
            auto& allocationInfo = heapInfo->second;
            if (allocationInfo.remaining >= count) {
                DescriptorHeapHandle handle(heapInfo->first, sizeIncrements[type], allocationInfo.size - allocationInfo.remaining);
                allocationInfo.remaining -= count;
                Release(handle);
                return handle;
            }
        }

        // If the pool has no more space, replace the pool with a new one of the specified size

        D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
        heapDescriptor.Type = type;
        heapDescriptor.NumDescriptors = allocationSize;
        heapDescriptor.Flags = flags;
        heapDescriptor.NodeMask = 0;
        ComPtr<ID3D12DescriptorHeap> heap;
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateDescriptorHeap(&heapDescriptor, IID_PPV_ARGS(&heap)));

        AllocationInfo allocationInfo = { allocationSize, allocationSize - count };
        *heapInfo = std::make_pair(heap, allocationInfo);

        DescriptorHeapHandle handle(heap, sizeIncrements[type], 0);
        Release(handle);
        return handle;
    }

    DescriptorHeapHandle DescriptorHeapAllocator::AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count) {
        return Allocate(type, count, count, &cpuDescriptorHeapInfos[type], D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }

    DescriptorHeapHandle DescriptorHeapAllocator::AllocateGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count) {
        ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        unsigned int heapSize = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? kMaxCbvUavSrvHeapSize : kMaxSamplerHeapSize);
        return Allocate(type, count, heapSize, &gpuDescriptorHeapInfos[type], D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }

    void DescriptorHeapAllocator::Tick(uint64_t lastCompletedSerial) {
        releasedHandles.ClearUpTo(lastCompletedSerial);
    }

    void DescriptorHeapAllocator::Release(DescriptorHeapHandle handle) {
        releasedHandles.Enqueue(handle, device->GetSerial());
    }
}
}
