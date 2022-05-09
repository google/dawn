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

#ifndef SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_
#define SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_

#include <memory>
#include <vector>

#include "dawn/native/BuddyAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/ResourceMemoryAllocation.h"

namespace dawn::native {

class ResourceHeapAllocator;

// BuddyMemoryAllocator uses the buddy allocator to sub-allocate blocks of device
// memory created by MemoryAllocator clients. It creates a very large buddy system
// where backing device memory blocks equal a specified level in the system.
//
// Upon sub-allocating, the offset gets mapped to device memory by computing the corresponding
// memory index and should the memory not exist, it is created. If two sub-allocations share the
// same memory index, the memory refcount is incremented to ensure de-allocating one doesn't
// release the other prematurely.
//
// The MemoryAllocator should return ResourceHeaps that are all compatible with each other.
// It should also outlive all the resources that are in the buddy allocator.
class BuddyMemoryAllocator {
  public:
    BuddyMemoryAllocator(uint64_t maxSystemSize,
                         uint64_t memoryBlockSize,
                         ResourceHeapAllocator* heapAllocator);
    ~BuddyMemoryAllocator();

    ResultOrError<ResourceMemoryAllocation> Allocate(uint64_t allocationSize, uint64_t alignment);
    void Deallocate(const ResourceMemoryAllocation& allocation);

    uint64_t GetMemoryBlockSize() const;

    // For testing purposes.
    uint64_t ComputeTotalNumOfHeapsForTesting() const;

  private:
    uint64_t GetMemoryIndex(uint64_t offset) const;

    uint64_t mMemoryBlockSize = 0;

    BuddyAllocator mBuddyBlockAllocator;
    ResourceHeapAllocator* mHeapAllocator;

    struct TrackedSubAllocations {
        size_t refcount = 0;
        std::unique_ptr<ResourceHeapBase> mMemoryAllocation;
    };

    std::vector<TrackedSubAllocations> mTrackedSubAllocations;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_
