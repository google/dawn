// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/MemoryBlockAllocator.h"

#include <utility>

namespace dawn::native {

MemoryBlockAllocator::MemoryBlockAllocator(size_t blockSize, uint64_t keepAliveWindow)
    : mBlockSize(blockSize), mKeepAliveWindow(keepAliveWindow) {}

MemoryBlockAllocator::~MemoryBlockAllocator() {
    TrimMemory();
}

size_t MemoryBlockAllocator::GetBlockSize() const {
    return mBlockSize;
}

HeapArray<std::byte> MemoryBlockAllocator::Allocate(size_t minimumSize) {
    if (minimumSize > mBlockSize) {
        // SAFETY: caller is responsible for initializing the memory
        return DAWN_UNSAFE_BUFFERS(HeapArray<std::byte>::Uninit(minimumSize));
    }

    HeapArray<std::byte> block = mFreeList.Use([&](auto freeList) -> HeapArray<std::byte> {
        if (freeList->empty()) {
            return {};
        }
        HeapArray<std::byte> result = std::move(freeList->back().block);
        freeList->pop_back();
        return result;
    });

    if (!block.empty()) {
        return block;
    }
    // SAFETY: caller is responsible for initializing the memory
    return DAWN_UNSAFE_BUFFERS(HeapArray<std::byte>::Uninit(mBlockSize));
}

void MemoryBlockAllocator::Return(std::vector<HeapArray<std::byte>>&& blocks) {
    mFreeList.Use([&](auto freeList) {
        for (HeapArray<std::byte>& block : blocks) {
            if (block.size() <= mBlockSize) {
                freeList->push_back({mTickSerial, std::move(block)});
            }
        }
    });
    blocks.clear();
}

void MemoryBlockAllocator::Tick() {
    mFreeList.Use([&](auto freeList) {
        mTickSerial++;
        while (!freeList->empty() &&
               freeList->front().lastKnownSerial + mKeepAliveWindow <= mTickSerial) {
            freeList->pop_front();
        }
    });
}

void MemoryBlockAllocator::TrimMemory() {
    mFreeList.Use([&](auto freeList) { std::deque<FreeBlock>().swap(*freeList); });
}

size_t MemoryBlockAllocator::GetRecycledBlockCountForTesting() {
    return mFreeList.Use([&](auto freeList) { return freeList->size(); });
}

}  // namespace dawn::native
