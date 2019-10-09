// Copyright 2019 The Dawn Authors
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

#include "dawn_native/d3d12/CommittedResourceAllocatorD3D12.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

namespace dawn_native { namespace d3d12 {

    CommittedResourceAllocator::CommittedResourceAllocator(Device* device, D3D12_HEAP_TYPE heapType)
        : mDevice(device), mHeapType(heapType) {
    }

    ResultOrError<ResourceHeapAllocation> CommittedResourceAllocator::Allocate(
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage,
        D3D12_HEAP_FLAGS heapFlags) {
        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = mHeapType;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        ComPtr<ID3D12Resource> committedResource;
        if (FAILED(mDevice->GetD3D12Device()->CreateCommittedResource(
                &heapProperties, heapFlags, &resourceDescriptor, initialUsage, nullptr,
                IID_PPV_ARGS(&committedResource)))) {
            return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate resource");
        }

        AllocationInfo info;
        info.mMethod = AllocationMethod::kDirect;

        return ResourceHeapAllocation{info,
                                      /*offset*/ 0, std::move(committedResource)};
    }

    void CommittedResourceAllocator::Deallocate(ResourceHeapAllocation& allocation) {
        mDevice->ReferenceUntilUnused(allocation.GetD3D12Resource());
    }
}}  // namespace dawn_native::d3d12
