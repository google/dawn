// Copyright 2020 The Dawn Authors
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

#ifndef DAWNNATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_
#define DAWNNATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_

#include "common/SerialQueue.h"
#include "dawn_native/ResourceHeapAllocator.h"

#include <deque>

namespace dawn_native {

    class DeviceBase;

    // |PooledResourceMemoryAllocator| allocates a fixed-size resource memory from a resource memory
    // pool. Internally, it manages a list of heaps using LIFO (newest heaps are recycled first).
    // The heap is in one of two states: AVAILABLE or not. Upon de-allocate, the heap is returned
    // the pool and made AVAILABLE.
    class PooledResourceMemoryAllocator : public ResourceHeapAllocator {
      public:
        PooledResourceMemoryAllocator(ResourceHeapAllocator* heapAllocator);
        ~PooledResourceMemoryAllocator() override = default;

        ResultOrError<std::unique_ptr<ResourceHeapBase>> AllocateResourceHeap(
            uint64_t size) override;
        void DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> allocation) override;

        void DestroyPool();

        // For testing purposes.
        uint64_t GetPoolSizeForTesting() const;

      private:
        ResourceHeapAllocator* mHeapAllocator = nullptr;

        std::deque<std::unique_ptr<ResourceHeapBase>> mPool;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_
