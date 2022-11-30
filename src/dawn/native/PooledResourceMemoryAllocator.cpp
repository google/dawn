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

#include "dawn/native/PooledResourceMemoryAllocator.h"

#include <utility>

#include "dawn/native/Device.h"

namespace dawn::native {

PooledResourceMemoryAllocator::PooledResourceMemoryAllocator(ResourceHeapAllocator* heapAllocator)
    : mHeapAllocator(heapAllocator) {}

PooledResourceMemoryAllocator::~PooledResourceMemoryAllocator() {
    ASSERT(mPool.empty());
}

void PooledResourceMemoryAllocator::DestroyPool() {
    for (auto& resourceHeap : mPool) {
        ASSERT(resourceHeap != nullptr);
        mHeapAllocator->DeallocateResourceHeap(std::move(resourceHeap));
    }

    mPool.clear();
}

ResultOrError<std::unique_ptr<ResourceHeapBase>>
PooledResourceMemoryAllocator::AllocateResourceHeap(uint64_t size) {
    // Pooled memory is LIFO because memory can be evicted by LRU. However, this means
    // pooling is disabled in-frame when the memory is still pending. For high in-frame
    // memory users, FIFO might be preferable when memory consumption is a higher priority.
    std::unique_ptr<ResourceHeapBase> memory;
    if (!mPool.empty()) {
        memory = std::move(mPool.front());
        mPool.pop_front();
    }

    if (memory == nullptr) {
        DAWN_TRY_ASSIGN(memory, mHeapAllocator->AllocateResourceHeap(size));
    }

    return std::move(memory);
}

void PooledResourceMemoryAllocator::DeallocateResourceHeap(
    std::unique_ptr<ResourceHeapBase> allocation) {
    mPool.push_front(std::move(allocation));
}

uint64_t PooledResourceMemoryAllocator::GetPoolSizeForTesting() const {
    return mPool.size();
}
}  // namespace dawn::native
