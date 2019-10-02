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

#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn_native/d3d12/Forward.h"
#include "dawn_native/d3d12/ResourceHeapD3D12.h"

namespace dawn_native { namespace d3d12 {

    ResourceAllocatorManager::ResourceAllocatorManager(Device* device) : mDevice(device) {
    }

    ResultOrError<ResourceMemoryAllocation> ResourceAllocatorManager::AllocateMemory(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage,
        D3D12_HEAP_FLAGS heapFlags) {
        const size_t heapTypeIndex = GetD3D12HeapTypeToIndex(heapType);
        ASSERT(heapTypeIndex < kNumHeapTypes);

        // Get the direct allocator using a tightly sized heap (aka CreateCommittedResource).
        CommittedResourceAllocator* allocator = mDirectResourceAllocators[heapTypeIndex].get();
        if (allocator == nullptr) {
            mDirectResourceAllocators[heapTypeIndex] =
                std::make_unique<CommittedResourceAllocator>(mDevice, heapType);
            allocator = mDirectResourceAllocators[heapTypeIndex].get();
        }

        ResourceMemoryAllocation allocation;
        DAWN_TRY_ASSIGN(allocation,
                        allocator->Allocate(resourceDescriptor, initialUsage, heapFlags));

        return allocation;
    }

    size_t ResourceAllocatorManager::GetD3D12HeapTypeToIndex(D3D12_HEAP_TYPE heapType) const {
        ASSERT(heapType > 0);
        ASSERT(static_cast<uint32_t>(heapType) <= kNumHeapTypes);
        return heapType - 1;
    }

    void ResourceAllocatorManager::DeallocateMemory(ResourceMemoryAllocation& allocation) {
        if (allocation.GetAllocationMethod() == AllocationMethod::kInvalid) {
            return;
        }
        CommittedResourceAllocator* allocator = nullptr;
        D3D12_HEAP_PROPERTIES heapProp;
        ToBackend(allocation.GetResourceHeap())
            ->GetD3D12Resource()
            ->GetHeapProperties(&heapProp, nullptr);
        const size_t heapTypeIndex = GetD3D12HeapTypeToIndex(heapProp.Type);
        ASSERT(heapTypeIndex < kNumHeapTypes);
        allocator = mDirectResourceAllocators[heapTypeIndex].get();
        allocator->Deallocate(allocation);

        // Invalidate the underlying resource heap in case the client accidentally
        // calls DeallocateMemory again using the same allocation.
        allocation.Invalidate();
    }
}}  // namespace dawn_native::d3d12
