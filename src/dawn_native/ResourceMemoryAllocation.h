// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_RESOURCEMEMORYALLOCATION_H_
#define DAWNNATIVE_RESOURCEMEMORYALLOCATION_H_

#include <cstdint>

namespace dawn_native {

    class ResourceHeapBase;

    // Allocation method determines how memory was sub-divided.
    // Used by the device to get the allocator that was responsible for the allocation.
    enum class AllocationMethod {

        // Memory not sub-divided.
        kDirect,

        // Memory sub-divided using one or more blocks of various sizes.
        kSubAllocated,

        // Memory not allocated or freed.
        kInvalid
    };

    // Handle into a resource heap pool.
    class ResourceMemoryAllocation {
      public:
        ResourceMemoryAllocation();
        ResourceMemoryAllocation(uint64_t offset,
                                 ResourceHeapBase* resourceHeap,
                                 AllocationMethod method);
        ~ResourceMemoryAllocation() = default;

        ResourceHeapBase* GetResourceHeap() const;
        uint64_t GetOffset() const;
        AllocationMethod GetAllocationMethod() const;

        void Invalidate();

      private:
        AllocationMethod mMethod;
        uint64_t mOffset;
        ResourceHeapBase* mResourceHeap;
    };
}  // namespace dawn_native

#endif  // DAWNNATIVE_RESOURCEMEMORYALLOCATION_H_