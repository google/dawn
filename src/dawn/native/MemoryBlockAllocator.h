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

#ifndef SRC_DAWN_NATIVE_MEMORYBLOCKALLOCATOR_H_
#define SRC_DAWN_NATIVE_MEMORYBLOCKALLOCATOR_H_

#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

#include "src/dawn/common/MutexProtected.h"
#include "src/utils/heap_array.h"
#include "src/utils/non_copyable.h"

namespace dawn::native {

// MemoryBlockAllocator is a pool allocator that recycles fixed-size memory blocks
// (`HeapArray<std::byte>`).
//
// All pooled blocks are uniformly block size (defaults to 16 KB). Requests larger than the block
// size bypass the pool entirely: allocated and freed directly, unpooled.
//
// Block retention is tracked via an internal tick serial: every call to Tick() increments
// mTickSerial by 1, and blocks whose age exceeds the keep alive window are evicted.
//
// We use std::deque<FreeBlock> protected by MutexProtected to store recycled blocks.
// Popping from the back (`back()` / `pop_back()`) provides LIFO / MRU allocation order so that
// returned blocks remain warm in CPU cache. Pushing to the back (`push_back()`) naturally keeps
// the oldest blocks at the front (`front()`) for early-terminating eviction in Tick().
// TODO(crbug.com/533386841): Consider using intrusive linked list to avoid additional deque memory.
class MemoryBlockAllocator : public dawn::NonCopyable {
  public:
    explicit MemoryBlockAllocator(size_t blockSize = 16384, uint64_t keepAliveWindow = 60);
    ~MemoryBlockAllocator();

    HeapArray<std::byte> Allocate(size_t minimumSize);
    void Return(std::vector<HeapArray<std::byte>>&& blocks);
    void Tick();
    void TrimMemory();
    size_t GetRecycledBlockCountForTesting();
    size_t GetBlockSize() const;

  private:
    struct FreeBlock {
        uint64_t lastKnownSerial;
        HeapArray<std::byte> block;
    };

    const size_t mBlockSize;
    const uint64_t mKeepAliveWindow;
    MutexProtected<std::deque<FreeBlock>> mFreeList;
    uint64_t mTickSerial = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_MEMORYBLOCKALLOCATOR_H_
