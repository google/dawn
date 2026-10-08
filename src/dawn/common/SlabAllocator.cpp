// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/common/SlabAllocator.h"

#include <algorithm>
#include <cstdlib>
#include <new>

#include "src/dawn/common/AlignedAlloc.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/common/MemoryBlockAllocator.h"
#include "src/dawn/common/Range.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn {

// IndexLinkNode

SlabAllocatorImpl::IndexLinkNode::IndexLinkNode(Index index, Index nextIndex)
    : index(index), nextIndex(nextIndex) {}

// Slab

SlabAllocatorImpl::Slab::Slab() = default;
SlabAllocatorImpl::Slab::Slab(HeapArray<std::byte> allocation)
    : allocation(std::move(allocation)) {}

SlabAllocatorImpl::Slab::Slab(Slab&& rhs) = default;

// SentinelSlab

SlabAllocatorImpl::SentinelSlab::SentinelSlab() = default;
SlabAllocatorImpl::SentinelSlab::SentinelSlab(SentinelSlab&& rhs) = default;

SlabAllocatorImpl::SentinelSlab::~SentinelSlab() {
    DAWN_CHECK(next == nullptr);
}

void SlabAllocatorImpl::SentinelSlab::Destroy(MemoryBlockAllocator* allocator) {
    // Delete the full linked list.
    while (next) {
        Slab* slab = next;
        slab->Splice();
        DAWN_ASSERT(slab->blocksInUse == 0);
        HeapArray<std::byte> allocation = std::move(slab->allocation);
        slab->~Slab();  // Placement delete.
        allocator->Return(std::move(allocation));
    }
}

// SlabAllocatorImpl

SlabAllocatorImpl::SlabAllocatorImpl(Index blocksPerSlab,
                                     uint32_t objectSize,
                                     uint32_t objectAlignment)
    : mAllocationAlignment(std::max(u32_alignof<Slab>, objectAlignment)),
      mSlabBlocksOffset(Align(u32_sizeof<Slab>, objectAlignment)),
      mIndexLinkNodeOffset(Align(objectSize, alignof(IndexLinkNode))),
      mBlockStride(Align(mIndexLinkNodeOffset + u32_sizeof<IndexLinkNode>, objectAlignment)),
      mBlocksPerSlab(blocksPerSlab),
      mTotalAllocationSize(static_cast<size_t>(mSlabBlocksOffset) +
                           static_cast<size_t>(mBlocksPerSlab) * mBlockStride),
      mMemoryBlockAllocator(std::make_unique<MemoryBlockAllocator>(mTotalAllocationSize)) {
    DAWN_ASSERT(blocksPerSlab > 0);
    // TODO(542009502): Currently only support standard alignment. So that standard malloc used by
    // MemoryBlockAllocator can return a pointer satisfies this alignment.
    DAWN_CHECK(mAllocationAlignment <= alignof(std::max_align_t));
    DAWN_ASSERT(IsPowerOfTwo(mAllocationAlignment));
}

SlabAllocatorImpl::SlabAllocatorImpl(SlabAllocatorImpl&& rhs)
    : mAllocationAlignment(rhs.mAllocationAlignment),
      mSlabBlocksOffset(rhs.mSlabBlocksOffset),
      mIndexLinkNodeOffset(rhs.mIndexLinkNodeOffset),
      mBlockStride(rhs.mBlockStride),
      mBlocksPerSlab(rhs.mBlocksPerSlab),
      mTotalAllocationSize(rhs.mTotalAllocationSize),
      mMemoryBlockAllocator(std::move(rhs.mMemoryBlockAllocator)),
      mAvailableSlabs(std::move(rhs.mAvailableSlabs)),
      mFullSlabs(std::move(rhs.mFullSlabs)),
      mRecycledSlabs(std::move(rhs.mRecycledSlabs)) {}

SlabAllocatorImpl::~SlabAllocatorImpl() {
    mAvailableSlabs.Destroy(mMemoryBlockAllocator.get());
    mFullSlabs.Destroy(mMemoryBlockAllocator.get());
    mRecycledSlabs.Destroy(mMemoryBlockAllocator.get());
}

size_t SlabAllocatorImpl::GetBlockOffset(Index index) const {
    DAWN_ASSERT(index < mBlocksPerSlab);
    return mSlabBlocksOffset + static_cast<size_t>(index) * mBlockStride;
}

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::GetNodeAtIndex(Slab* slab, Index index) const {
    DAWN_ASSERT(index < mBlocksPerSlab);
    size_t byteOffset = GetBlockOffset(index) + mIndexLinkNodeOffset;
    return reinterpret_cast<IndexLinkNode*>(&slab->allocation[byteOffset]);
}

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::GetNodeFromBlock(
    dawn::Span<std::byte> object) const {
    DAWN_ASSERT(object.size() >= mIndexLinkNodeOffset + sizeof(IndexLinkNode));
    return reinterpret_cast<IndexLinkNode*>(&object[mIndexLinkNodeOffset]);
}

void* SlabAllocatorImpl::GetObjectFromNode(Slab* slab, IndexLinkNode* node) const {
    DAWN_ASSERT(IsNodeInSlab(slab, node));
    return &slab->allocation[GetBlockOffset(node->index)];
}

bool SlabAllocatorImpl::IsNodeInSlab(Slab* slab, IndexLinkNode* node) const {
    if (node == nullptr || node->index >= mBlocksPerSlab) {
        return false;
    }
    size_t nodeOffset = GetBlockOffset(node->index) + mIndexLinkNodeOffset;
    if (nodeOffset + sizeof(IndexLinkNode) > slab->allocation.size()) {
        return false;
    }
    return node == reinterpret_cast<const IndexLinkNode*>(&slab->allocation[nodeOffset]);
}

void SlabAllocatorImpl::PushFront(Slab* slab, IndexLinkNode* node) const {
    DAWN_ASSERT(IsNodeInSlab(slab, node));

    IndexLinkNode* head = slab->freeList;
    if (head == nullptr) {
        node->nextIndex = kInvalidIndex;
    } else {
        DAWN_ASSERT(IsNodeInSlab(slab, head));
        node->nextIndex = head->index;
    }
    slab->freeList = node;

    DAWN_ASSERT(slab->blocksInUse != 0);
    slab->blocksInUse--;
}

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::PopFront(Slab* slab) const {
    DAWN_ASSERT(slab->freeList != nullptr);

    IndexLinkNode* head = slab->freeList;
    if (head->nextIndex == kInvalidIndex) {
        slab->freeList = nullptr;
    } else {
        DAWN_ASSERT(IsNodeInSlab(slab, head));
        slab->freeList = GetNodeAtIndex(slab, head->nextIndex);
        DAWN_ASSERT(IsNodeInSlab(slab, slab->freeList));
    }

    DAWN_ASSERT(slab->blocksInUse < mBlocksPerSlab);
    slab->blocksInUse++;
    return head;
}

void SlabAllocatorImpl::SentinelSlab::Prepend(SlabAllocatorImpl::Slab* slab) {
    if (next != nullptr) {
        next->prev = slab;
    }
    slab->prev = this;
    slab->next = next;
    next = slab;
}

void SlabAllocatorImpl::Slab::Splice() {
    DAWN_ASSERT(prev != nullptr);
    prev->next = next;
    if (next != nullptr) {
        next->prev = prev;
    }
    prev = nullptr;
    next = nullptr;
}

void SlabAllocatorImpl::DeleteEmptySlabs() {
    // TODO(542009502): this should be removed. SlabAllocator should eagerly return the memory block
    // to the memory block allocator then memory block allocator will implicitly trim the memory
    // when needed.
    auto DeleteEmptyFromList = [&](const SentinelSlab& sentinel) {
        for (Slab* current = sentinel.next; current != nullptr;) {
            if (current->blocksInUse == 0) {
                Slab* next = current->next;

                // Remove from list and then delete to avoid dangling pointers.
                current->Splice();
                HeapArray<std::byte> allocation = std::move(current->allocation);
                current->~Slab();
                mMemoryBlockAllocator->Return(std::move(allocation));

                current = next;
            } else {
                current = current->next;
            }
        }
    };
    DeleteEmptyFromList(mRecycledSlabs);
    DeleteEmptyFromList(mAvailableSlabs);
    mMemoryBlockAllocator->TrimMemory();
}

uint32_t SlabAllocatorImpl::CountAllocatedSlabsForTesting() const {
    auto CountSlabs = [](const SentinelSlab& sentinel) -> uint32_t {
        uint32_t count = 0;
        for (Slab* current = sentinel.next; current != nullptr;) {
            ++count;
            current = current->next;
        }
        return count;
    };

    return CountSlabs(mAvailableSlabs) + CountSlabs(mRecycledSlabs) + CountSlabs(mFullSlabs);
}

void* SlabAllocatorImpl::Allocate() {
    if (mAvailableSlabs.next == nullptr) {
        GetNewSlab();
    }

    Slab* slab = mAvailableSlabs.next;
    IndexLinkNode* node = PopFront(slab);
    DAWN_ASSERT(node != nullptr);

    // Move full slabs to a separate list, so allocate can always return quickly.
    if (slab->blocksInUse == mBlocksPerSlab) {
        slab->Splice();
        mFullSlabs.Prepend(slab);
    }

    return GetObjectFromNode(slab, node);
}

void SlabAllocatorImpl::Deallocate(void* object) {
    DAWN_ASSERT(object != nullptr);

    // This assumes `object` is pointing to at least `mBlockStride` bytes.
    Span<std::byte> DAWN_UNSAFE_TODO(blockSpan(static_cast<std::byte*>(object), mBlockStride));
    IndexLinkNode* node = GetNodeFromBlock(blockSpan);
    DAWN_ASSERT(IsPtrAligned(node, alignof(IndexLinkNode)));
    DAWN_ASSERT(node->index < mBlocksPerSlab);

    // SAFETY: `GetBlockOffset` is guaranteed to return a valid offset
    // from object to the start of the slab.
    Slab* slab = DAWN_UNSAFE_BUFFERS(
        reinterpret_cast<Slab*>(static_cast<std::byte*>(object) - GetBlockOffset(node->index)));
    DAWN_ASSERT(slab != nullptr);

    bool slabWasFull = slab->blocksInUse == mBlocksPerSlab;
    DAWN_ASSERT(slab->blocksInUse != 0);
    PushFront(slab, node);

    if (slabWasFull) {
        // Slab is in the full list. Move it to the recycled list.
        DAWN_ASSERT(slab->freeList != nullptr);
        slab->Splice();
        mRecycledSlabs.Prepend(slab);
    }
}

void SlabAllocatorImpl::GetNewSlab() {
    // Should only be called when there are no available slabs.
    DAWN_ASSERT(mAvailableSlabs.next == nullptr);

    if (mRecycledSlabs.next != nullptr) {
        // If the recycled list is non-empty, swap their contents.
        std::swap(mAvailableSlabs.next, mRecycledSlabs.next);

        // We swapped the next pointers, so the prev pointer is wrong.
        // Update it here.
        mAvailableSlabs.next->prev = &mAvailableSlabs;
        DAWN_ASSERT(mRecycledSlabs.next == nullptr);
        return;
    }

    // Allocate the slab
    HeapArray<std::byte> allocation = mMemoryBlockAllocator->Allocate(mTotalAllocationSize);
    std::byte* allocationPtr = allocation.data();
    DAWN_CHECK(IsPtrAligned(allocationPtr, mAllocationAlignment));

    Slab* slab = new (allocationPtr) Slab(std::move(allocation));
    slab->freeList = GetNodeAtIndex(slab, 0);
    mAvailableSlabs.Prepend(slab);

    // Initialize all of its nodes
    for (Index i : Range(mBlocksPerSlab)) {
        IndexLinkNode* node = GetNodeAtIndex(slab, i);
        new (node) IndexLinkNode(i, i + 1);
    }
    GetNodeAtIndex(slab, mBlocksPerSlab - 1)->nextIndex = kInvalidIndex;
}

}  // namespace dawn
