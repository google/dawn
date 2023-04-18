// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_BUMP_ALLOCATOR_H_
#define SRC_TINT_UTILS_BUMP_ALLOCATOR_H_

#include <array>
#include <cstring>
#include <utility>

#include "src/tint/utils/bitcast.h"
#include "src/tint/utils/math.h"

namespace tint::utils {

constexpr size_t kBlockSize = 64 * 1024;

/// A allocator for chunks of memory. The memory is owned by the BumpAllocator. When the
/// BumpAllocator is freed all of the allocated memory is freed.
class BumpAllocator {
    /// Block is linked list of memory blocks.
    /// Blocks are allocated out of heap memory.
    struct Block {
        uint8_t data[kBlockSize];
        Block* next;
    };

  public:
    /// Constructor
    BumpAllocator() = default;

    /// Move constructor
    /// @param rhs the BumpAllocator to move
    BumpAllocator(BumpAllocator&& rhs) { std::swap(data, rhs.data); }

    /// Move assignment operator
    /// @param rhs the BumpAllocator to move
    /// @return this BumpAllocator
    BumpAllocator& operator=(BumpAllocator&& rhs) {
        if (this != &rhs) {
            Reset();
            std::swap(data, rhs.data);
        }
        return *this;
    }

    /// Destructor
    ~BumpAllocator() { Reset(); }

    /// Allocates @p size_in_bytes from the current block, or from a newly allocated block if the
    /// current block is full.
    /// @param size_in_bytes the number of bytes to allocate
    /// @returns the pointer to the allocated memory or |nullptr| if the memory can not be allocated
    char* Allocate(size_t size_in_bytes) {
        auto& block = data.block;
        if (block.current_offset + size_in_bytes > kBlockSize) {
            // Allocate a new block from the heap
            auto* prev_block = block.current;
            block.current = new Block;
            if (!block.current) {
                return nullptr;  // out of memory
            }
            block.current->next = nullptr;
            block.current_offset = 0;
            if (prev_block) {
                prev_block->next = block.current;
            } else {
                block.root = block.current;
            }
        }

        auto* base = &block.current->data[0];
        auto* ptr = reinterpret_cast<char*>(base + block.current_offset);
        block.current_offset += size_in_bytes;
        data.count++;
        return ptr;
    }

    /// Frees all allocations from the allocator.
    void Reset() {
        auto* block = data.block.root;
        while (block != nullptr) {
            auto* next = block->next;
            delete block;
            block = next;
        }
        data = {};
    }

    /// @returns the total number of allocations
    size_t Count() const { return data.count; }

  private:
    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;

    struct {
        struct {
            /// The root block of the block linked list
            Block* root = nullptr;
            /// The current (end) block of the blocked linked list.
            /// New allocations come from this block
            Block* current = nullptr;
            /// The byte offset in #current for the next allocation.
            /// Initialized with kBlockSize so that the first allocation triggers a block
            /// allocation.
            size_t current_offset = kBlockSize;
        } block;

        size_t count = 0;
    } data;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_BUMP_ALLOCATOR_H_
