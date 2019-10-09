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

#ifndef DAWNNATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_
#define DAWNNATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_

#include "dawn_native/d3d12/CommittedResourceAllocatorD3D12.h"

#include <array>

namespace dawn_native { namespace d3d12 {

    class Device;

    // Manages a list of resource allocators used by the device to create resources using multiple
    // allocation methods.
    class ResourceAllocatorManager {
      public:
        ResourceAllocatorManager(Device* device);

        ResultOrError<ResourceHeapAllocation> AllocateMemory(
            D3D12_HEAP_TYPE heapType,
            const D3D12_RESOURCE_DESC& resourceDescriptor,
            D3D12_RESOURCE_STATES initialUsage,
            D3D12_HEAP_FLAGS heapFlags);

        void DeallocateMemory(ResourceHeapAllocation& allocation);

      private:
        size_t GetD3D12HeapTypeToIndex(D3D12_HEAP_TYPE heapType) const;

        Device* mDevice;

        static constexpr uint32_t kNumHeapTypes = 4u;  // Number of D3D12_HEAP_TYPE

        static_assert(D3D12_HEAP_TYPE_READBACK <= kNumHeapTypes,
                      "Readback heap type enum exceeds max heap types");
        static_assert(D3D12_HEAP_TYPE_UPLOAD <= kNumHeapTypes,
                      "Upload heap type enum exceeds max heap types");
        static_assert(D3D12_HEAP_TYPE_DEFAULT <= kNumHeapTypes,
                      "Default heap type enum exceeds max heap types");
        static_assert(D3D12_HEAP_TYPE_CUSTOM <= kNumHeapTypes,
                      "Custom heap type enum exceeds max heap types");

        std::array<std::unique_ptr<CommittedResourceAllocator>, kNumHeapTypes>
            mDirectResourceAllocators;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_