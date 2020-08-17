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

#include <gtest/gtest.h>

#include "dawn_native/BuddyMemoryAllocator.h"
#include "dawn_native/PooledResourceMemoryAllocator.h"
#include "dawn_native/ResourceHeapAllocator.h"

#include <set>
#include <vector>

using namespace dawn_native;

class DummyResourceHeapAllocator : public ResourceHeapAllocator {
  public:
    ResultOrError<std::unique_ptr<ResourceHeapBase>> AllocateResourceHeap(uint64_t size) override {
        return std::make_unique<ResourceHeapBase>();
    }
    void DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> allocation) override {
    }
};

class DummyBuddyResourceAllocator {
  public:
    DummyBuddyResourceAllocator(uint64_t maxBlockSize, uint64_t memorySize)
        : mAllocator(maxBlockSize, memorySize, &mHeapAllocator) {
    }

    DummyBuddyResourceAllocator(uint64_t maxBlockSize,
                                uint64_t memorySize,
                                ResourceHeapAllocator* heapAllocator)
        : mAllocator(maxBlockSize, memorySize, heapAllocator) {
    }

    ResourceMemoryAllocation Allocate(uint64_t allocationSize, uint64_t alignment = 1) {
        ResultOrError<ResourceMemoryAllocation> result =
            mAllocator.Allocate(allocationSize, alignment);
        return (result.IsSuccess()) ? result.AcquireSuccess() : ResourceMemoryAllocation{};
    }

    void Deallocate(ResourceMemoryAllocation& allocation) {
        mAllocator.Deallocate(allocation);
    }

    uint64_t ComputeTotalNumOfHeapsForTesting() const {
        return mAllocator.ComputeTotalNumOfHeapsForTesting();
    }

  private:
    DummyResourceHeapAllocator mHeapAllocator;
    BuddyMemoryAllocator mAllocator;
};

// Verify a single resource allocation in a single heap.
TEST(BuddyMemoryAllocatorTests, SingleHeap) {
    // After one 128 byte resource allocation:
    //
    // max block size -> ---------------------------
    //                   |          A1/H0          |       Hi - Heap at index i
    // max heap size  -> ---------------------------       An - Resource allocation n
    //
    constexpr uint64_t heapSize = 128;
    constexpr uint64_t maxBlockSize = heapSize;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    // Cannot allocate greater than heap size.
    ResourceMemoryAllocation invalidAllocation = allocator.Allocate(heapSize * 2);
    ASSERT_EQ(invalidAllocation.GetInfo().mMethod, AllocationMethod::kInvalid);

    // Allocate one 128 byte allocation (same size as heap).
    ResourceMemoryAllocation allocation1 = allocator.Allocate(128);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);

    // Cannot allocate when allocator is full.
    invalidAllocation = allocator.Allocate(128);
    ASSERT_EQ(invalidAllocation.GetInfo().mMethod, AllocationMethod::kInvalid);

    allocator.Deallocate(allocation1);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 0u);
}

// Verify that multiple allocation are created in separate heaps.
TEST(BuddyMemoryAllocatorTests, MultipleHeaps) {
    // After two 128 byte resource allocations:
    //
    // max block size -> ---------------------------
    //                   |                         |       Hi - Heap at index i
    // max heap size  -> ---------------------------       An - Resource allocation n
    //                   |   A1/H0    |    A2/H1   |
    //                   ---------------------------
    //
    constexpr uint64_t maxBlockSize = 256;
    constexpr uint64_t heapSize = 128;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    // Cannot allocate greater than heap size.
    ResourceMemoryAllocation invalidAllocation = allocator.Allocate(heapSize * 2);
    ASSERT_EQ(invalidAllocation.GetInfo().mMethod, AllocationMethod::kInvalid);

    // Cannot allocate greater than max block size.
    invalidAllocation = allocator.Allocate(maxBlockSize * 2);
    ASSERT_EQ(invalidAllocation.GetInfo().mMethod, AllocationMethod::kInvalid);

    // Allocate two 128 byte allocations.
    ResourceMemoryAllocation allocation1 = allocator.Allocate(heapSize);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // First allocation creates first heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);

    ResourceMemoryAllocation allocation2 = allocator.Allocate(heapSize);
    ASSERT_EQ(allocation2.GetInfo().mBlockOffset, heapSize);
    ASSERT_EQ(allocation2.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // Second allocation creates second heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);
    ASSERT_NE(allocation1.GetResourceHeap(), allocation2.GetResourceHeap());

    // Deallocate both allocations
    allocator.Deallocate(allocation1);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);  // Released H0

    allocator.Deallocate(allocation2);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 0u);  // Released H1
}

// Verify multiple sub-allocations can re-use heaps.
TEST(BuddyMemoryAllocatorTests, MultipleSplitHeaps) {
    // After two 64 byte allocations with 128 byte heaps.
    //
    // max block size -> ---------------------------
    //                   |                         |       Hi - Heap at index i
    // max heap size  -> ---------------------------       An - Resource allocation n
    //                   |     H0     |     H1     |
    //                   ---------------------------
    //                   |  A1 |  A2  |  A3 |      |
    //                   ---------------------------
    //
    constexpr uint64_t maxBlockSize = 256;
    constexpr uint64_t heapSize = 128;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    // Allocate two 64 byte sub-allocations.
    ResourceMemoryAllocation allocation1 = allocator.Allocate(heapSize / 2);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // First sub-allocation creates first heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);

    ResourceMemoryAllocation allocation2 = allocator.Allocate(heapSize / 2);
    ASSERT_EQ(allocation2.GetInfo().mBlockOffset, heapSize / 2);
    ASSERT_EQ(allocation2.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // Second allocation re-uses first heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);
    ASSERT_EQ(allocation1.GetResourceHeap(), allocation2.GetResourceHeap());

    ResourceMemoryAllocation allocation3 = allocator.Allocate(heapSize / 2);
    ASSERT_EQ(allocation3.GetInfo().mBlockOffset, heapSize);
    ASSERT_EQ(allocation3.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // Third allocation creates second heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);
    ASSERT_NE(allocation1.GetResourceHeap(), allocation3.GetResourceHeap());

    // Deallocate all allocations in reverse order.
    allocator.Deallocate(allocation1);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(),
              2u);  // A2 pins H0.

    allocator.Deallocate(allocation2);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);  // Released H0

    allocator.Deallocate(allocation3);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 0u);  // Released H1
}

// Verify resource sub-allocation of various sizes over multiple heaps.
TEST(BuddyMemoryAllocatorTests, MultiplSplitHeapsVariableSizes) {
    // After three 64 byte allocations and two 128 byte allocations.
    //
    // max block size -> -------------------------------------------------------
    //                   |                                                     |
    //                   -------------------------------------------------------
    //                   |                         |                           |
    // max heap size  -> -------------------------------------------------------
    //                   |     H0     |    A3/H1   |      H2     |    A5/H3    |
    //                   -------------------------------------------------------
    //                   |  A1 |  A2  |            |   A4  |     |             |
    //                   -------------------------------------------------------
    //
    constexpr uint64_t heapSize = 128;
    constexpr uint64_t maxBlockSize = 512;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    // Allocate two 64-byte allocations.
    ResourceMemoryAllocation allocation1 = allocator.Allocate(64);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetOffset(), 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ResourceMemoryAllocation allocation2 = allocator.Allocate(64);
    ASSERT_EQ(allocation2.GetInfo().mBlockOffset, 64u);
    ASSERT_EQ(allocation2.GetOffset(), 64u);
    ASSERT_EQ(allocation2.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // A1 and A2 share H0
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);
    ASSERT_EQ(allocation1.GetResourceHeap(), allocation2.GetResourceHeap());

    ResourceMemoryAllocation allocation3 = allocator.Allocate(128);
    ASSERT_EQ(allocation3.GetInfo().mBlockOffset, 128u);
    ASSERT_EQ(allocation3.GetOffset(), 0u);
    ASSERT_EQ(allocation3.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    // A3 creates and fully occupies a new heap.
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);
    ASSERT_NE(allocation2.GetResourceHeap(), allocation3.GetResourceHeap());

    ResourceMemoryAllocation allocation4 = allocator.Allocate(64);
    ASSERT_EQ(allocation4.GetInfo().mBlockOffset, 256u);
    ASSERT_EQ(allocation4.GetOffset(), 0u);
    ASSERT_EQ(allocation4.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 3u);
    ASSERT_NE(allocation3.GetResourceHeap(), allocation4.GetResourceHeap());

    // R5 size forms 64 byte hole after R4.
    ResourceMemoryAllocation allocation5 = allocator.Allocate(128);
    ASSERT_EQ(allocation5.GetInfo().mBlockOffset, 384u);
    ASSERT_EQ(allocation5.GetOffset(), 0u);
    ASSERT_EQ(allocation5.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 4u);
    ASSERT_NE(allocation4.GetResourceHeap(), allocation5.GetResourceHeap());

    // Deallocate allocations in staggered order.
    allocator.Deallocate(allocation1);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 4u);  // A2 pins H0

    allocator.Deallocate(allocation5);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 3u);  // Released H3

    allocator.Deallocate(allocation2);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);  // Released H0

    allocator.Deallocate(allocation4);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);  // Released H2

    allocator.Deallocate(allocation3);
    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 0u);  // Released H1
}

// Verify resource sub-allocation of same sizes with various alignments.
TEST(BuddyMemoryAllocatorTests, SameSizeVariousAlignment) {
    // After three 64 byte and one 128 byte resource allocations.
    //
    // max block size -> -------------------------------------------------------
    //                   |                                                     |
    //                   -------------------------------------------------------
    //                   |                         |                           |
    // max heap size  -> -------------------------------------------------------
    //                   |     H0     |     H1     |     H2     |              |
    //                   -------------------------------------------------------
    //                   |  A1  |     |  A2  |     |  A3  |  A4 |              |
    //                   -------------------------------------------------------
    //
    constexpr uint64_t heapSize = 128;
    constexpr uint64_t maxBlockSize = 512;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    ResourceMemoryAllocation allocation1 = allocator.Allocate(64, 128);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetOffset(), 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);

    ResourceMemoryAllocation allocation2 = allocator.Allocate(64, 128);
    ASSERT_EQ(allocation2.GetInfo().mBlockOffset, 128u);
    ASSERT_EQ(allocation2.GetOffset(), 0u);
    ASSERT_EQ(allocation2.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);
    ASSERT_NE(allocation1.GetResourceHeap(), allocation2.GetResourceHeap());

    ResourceMemoryAllocation allocation3 = allocator.Allocate(64, 128);
    ASSERT_EQ(allocation3.GetInfo().mBlockOffset, 256u);
    ASSERT_EQ(allocation3.GetOffset(), 0u);
    ASSERT_EQ(allocation3.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 3u);
    ASSERT_NE(allocation2.GetResourceHeap(), allocation3.GetResourceHeap());

    ResourceMemoryAllocation allocation4 = allocator.Allocate(64, 64);
    ASSERT_EQ(allocation4.GetInfo().mBlockOffset, 320u);
    ASSERT_EQ(allocation4.GetOffset(), 64u);
    ASSERT_EQ(allocation4.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 3u);
    ASSERT_EQ(allocation3.GetResourceHeap(), allocation4.GetResourceHeap());
}

// Verify resource sub-allocation of various sizes with same alignments.
TEST(BuddyMemoryAllocatorTests, VariousSizeSameAlignment) {
    // After two 64 byte and two 128 byte resource allocations:
    //
    // max block size -> -------------------------------------------------------
    //                   |                                                     |
    //                   -------------------------------------------------------
    //                   |                         |                           |
    // max heap size  -> -------------------------------------------------------
    //                   |     H0     |    A3/H1   |    A4/H2   |              |
    //                   -------------------------------------------------------
    //                   |  A1 |  A2  |            |            |              |
    //                   -------------------------------------------------------
    //
    constexpr uint64_t heapSize = 128;
    constexpr uint64_t maxBlockSize = 512;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    constexpr uint64_t alignment = 64;

    ResourceMemoryAllocation allocation1 = allocator.Allocate(64, alignment);
    ASSERT_EQ(allocation1.GetInfo().mBlockOffset, 0u);
    ASSERT_EQ(allocation1.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);

    ResourceMemoryAllocation allocation2 = allocator.Allocate(64, alignment);
    ASSERT_EQ(allocation2.GetInfo().mBlockOffset, 64u);
    ASSERT_EQ(allocation2.GetOffset(), 64u);
    ASSERT_EQ(allocation2.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 1u);  // Reuses H0
    ASSERT_EQ(allocation1.GetResourceHeap(), allocation2.GetResourceHeap());

    ResourceMemoryAllocation allocation3 = allocator.Allocate(128, alignment);
    ASSERT_EQ(allocation3.GetInfo().mBlockOffset, 128u);
    ASSERT_EQ(allocation3.GetOffset(), 0u);
    ASSERT_EQ(allocation3.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 2u);
    ASSERT_NE(allocation2.GetResourceHeap(), allocation3.GetResourceHeap());

    ResourceMemoryAllocation allocation4 = allocator.Allocate(128, alignment);
    ASSERT_EQ(allocation4.GetInfo().mBlockOffset, 256u);
    ASSERT_EQ(allocation4.GetOffset(), 0u);
    ASSERT_EQ(allocation4.GetInfo().mMethod, AllocationMethod::kSubAllocated);

    ASSERT_EQ(allocator.ComputeTotalNumOfHeapsForTesting(), 3u);
    ASSERT_NE(allocation3.GetResourceHeap(), allocation4.GetResourceHeap());
}

// Verify allocating a very large resource does not overflow.
TEST(BuddyMemoryAllocatorTests, AllocationOverflow) {
    constexpr uint64_t heapSize = 128;
    constexpr uint64_t maxBlockSize = 512;
    DummyBuddyResourceAllocator allocator(maxBlockSize, heapSize);

    constexpr uint64_t largeBlock = (1ull << 63) + 1;
    ResourceMemoryAllocation invalidAllocation = allocator.Allocate(largeBlock);
    ASSERT_EQ(invalidAllocation.GetInfo().mMethod, AllocationMethod::kInvalid);
}

// Verify resource heaps will be reused from a pool.
TEST(BuddyMemoryAllocatorTests, ReuseFreedHeaps) {
    constexpr uint64_t kHeapSize = 128;
    constexpr uint64_t kMaxBlockSize = 4096;

    DummyResourceHeapAllocator heapAllocator;
    PooledResourceMemoryAllocator poolAllocator(&heapAllocator);
    DummyBuddyResourceAllocator allocator(kMaxBlockSize, kHeapSize, &poolAllocator);

    std::set<ResourceHeapBase*> heaps = {};
    std::vector<ResourceMemoryAllocation> allocations = {};

    constexpr uint32_t kNumOfAllocations = 100;

    // Allocate |kNumOfAllocations|.
    for (uint32_t i = 0; i < kNumOfAllocations; i++) {
        ResourceMemoryAllocation allocation = allocator.Allocate(4);
        ASSERT_EQ(allocation.GetInfo().mMethod, AllocationMethod::kSubAllocated);
        heaps.insert(allocation.GetResourceHeap());
        allocations.push_back(std::move(allocation));
    }

    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), 0u);

    // Return the allocations to the pool.
    for (ResourceMemoryAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
    }

    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), heaps.size());

    // Allocate again reusing the same heaps.
    for (uint32_t i = 0; i < kNumOfAllocations; i++) {
        ResourceMemoryAllocation allocation = allocator.Allocate(4);
        ASSERT_EQ(allocation.GetInfo().mMethod, AllocationMethod::kSubAllocated);
        ASSERT_FALSE(heaps.insert(allocation.GetResourceHeap()).second);
    }

    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), 0u);
}

// Verify resource heaps that were reused from a pool can be destroyed.
TEST(BuddyMemoryAllocatorTests, DestroyHeaps) {
    constexpr uint64_t kHeapSize = 128;
    constexpr uint64_t kMaxBlockSize = 4096;

    DummyResourceHeapAllocator heapAllocator;
    PooledResourceMemoryAllocator poolAllocator(&heapAllocator);
    DummyBuddyResourceAllocator allocator(kMaxBlockSize, kHeapSize, &poolAllocator);

    std::set<ResourceHeapBase*> heaps = {};
    std::vector<ResourceMemoryAllocation> allocations = {};

    // Count by heap (vs number of allocations) to ensure there are exactly |kNumOfHeaps| worth of
    // buffers. Otherwise, the heap may be reused if not full.
    constexpr uint32_t kNumOfHeaps = 10;

    // Allocate |kNumOfHeaps| worth.
    while (heaps.size() < kNumOfHeaps) {
        ResourceMemoryAllocation allocation = allocator.Allocate(4);
        ASSERT_EQ(allocation.GetInfo().mMethod, AllocationMethod::kSubAllocated);
        heaps.insert(allocation.GetResourceHeap());
        allocations.push_back(std::move(allocation));
    }

    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), 0u);

    // Return the allocations to the pool.
    for (ResourceMemoryAllocation& allocation : allocations) {
        allocator.Deallocate(allocation);
    }

    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), kNumOfHeaps);

    // Make sure we can destroy the remaining heaps.
    poolAllocator.DestroyPool();
    ASSERT_EQ(poolAllocator.GetPoolSizeForTesting(), 0u);
}