// Copyright 2017 The Dawn Authors
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

#include "dawn/native/CommandAllocator.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"

namespace dawn::native {

// TODO(cwallez@chromium.org): figure out a way to have more type safety for the iterator

CommandIterator::CommandIterator() {
    Reset();
}

CommandIterator::~CommandIterator() {
    ASSERT(IsEmpty());
}

CommandIterator::CommandIterator(CommandIterator&& other) {
    if (!other.IsEmpty()) {
        mBlocks = std::move(other.mBlocks);
        other.Reset();
    }
    Reset();
}

CommandIterator& CommandIterator::operator=(CommandIterator&& other) {
    ASSERT(IsEmpty());
    if (!other.IsEmpty()) {
        mBlocks = std::move(other.mBlocks);
        other.Reset();
    }
    Reset();
    return *this;
}

CommandIterator::CommandIterator(CommandAllocator allocator) : mBlocks(allocator.AcquireBlocks()) {
    Reset();
}

void CommandIterator::AcquireCommandBlocks(std::vector<CommandAllocator> allocators) {
    ASSERT(IsEmpty());
    mBlocks.clear();
    for (CommandAllocator& allocator : allocators) {
        CommandBlocks blocks = allocator.AcquireBlocks();
        if (!blocks.empty()) {
            mBlocks.reserve(mBlocks.size() + blocks.size());
            for (BlockDef& block : blocks) {
                mBlocks.push_back(std::move(block));
            }
        }
    }
    Reset();
}

bool CommandIterator::NextCommandIdInNewBlock(uint32_t* commandId) {
    mCurrentBlock++;
    if (mCurrentBlock >= mBlocks.size()) {
        Reset();
        *commandId = detail::kEndOfBlock;
        return false;
    }
    mCurrentPtr = AlignPtr(mBlocks[mCurrentBlock].block, alignof(uint32_t));
    return NextCommandId(commandId);
}

void CommandIterator::Reset() {
    mCurrentBlock = 0;

    if (mBlocks.empty()) {
        // This will case the first NextCommandId call to try to move to the next block and stop
        // the iteration immediately, without special casing the initialization.
        mCurrentPtr = reinterpret_cast<uint8_t*>(&mEndOfBlock);
        mBlocks.emplace_back();
        mBlocks[0].size = sizeof(mEndOfBlock);
        mBlocks[0].block = mCurrentPtr;
    } else {
        mCurrentPtr = AlignPtr(mBlocks[0].block, alignof(uint32_t));
    }
}

void CommandIterator::MakeEmptyAsDataWasDestroyed() {
    if (IsEmpty()) {
        return;
    }

    for (BlockDef& block : mBlocks) {
        free(block.block);
    }
    mBlocks.clear();
    Reset();
    ASSERT(IsEmpty());
}

bool CommandIterator::IsEmpty() const {
    return mBlocks[0].block == reinterpret_cast<const uint8_t*>(&mEndOfBlock);
}

// Potential TODO(crbug.com/dawn/835):
//  - Host the size and pointer to next block in the block itself to avoid having an allocation
//    in the vector
//  - Assume T's alignof is, say 64bits, static assert it, and make commandAlignment a constant
//    in Allocate
//  - Be able to optimize allocation to one block, for command buffers expected to live long to
//    avoid cache misses
//  - Better block allocation, maybe have Dawn API to say command buffer is going to have size
//    close to another

CommandAllocator::CommandAllocator() {
    ResetPointers();
}

CommandAllocator::~CommandAllocator() {
    Reset();
}

CommandAllocator::CommandAllocator(CommandAllocator&& other)
    : mBlocks(std::move(other.mBlocks)), mLastAllocationSize(other.mLastAllocationSize) {
    other.mBlocks.clear();
    if (!other.IsEmpty()) {
        mCurrentPtr = other.mCurrentPtr;
        mEndPtr = other.mEndPtr;
    } else {
        ResetPointers();
    }
    other.Reset();
}

CommandAllocator& CommandAllocator::operator=(CommandAllocator&& other) {
    Reset();
    if (!other.IsEmpty()) {
        std::swap(mBlocks, other.mBlocks);
        mLastAllocationSize = other.mLastAllocationSize;
        mCurrentPtr = other.mCurrentPtr;
        mEndPtr = other.mEndPtr;
    }
    other.Reset();
    return *this;
}

void CommandAllocator::Reset() {
    for (BlockDef& block : mBlocks) {
        free(block.block);
    }
    mBlocks.clear();
    mLastAllocationSize = kDefaultBaseAllocationSize;
    ResetPointers();
}

bool CommandAllocator::IsEmpty() const {
    return mCurrentPtr == reinterpret_cast<const uint8_t*>(&mPlaceholderEnum[0]);
}

CommandBlocks&& CommandAllocator::AcquireBlocks() {
    ASSERT(mCurrentPtr != nullptr && mEndPtr != nullptr);
    ASSERT(IsPtrAligned(mCurrentPtr, alignof(uint32_t)));
    ASSERT(mCurrentPtr + sizeof(uint32_t) <= mEndPtr);
    *reinterpret_cast<uint32_t*>(mCurrentPtr) = detail::kEndOfBlock;

    mCurrentPtr = nullptr;
    mEndPtr = nullptr;
    return std::move(mBlocks);
}

uint8_t* CommandAllocator::AllocateInNewBlock(uint32_t commandId,
                                              size_t commandSize,
                                              size_t commandAlignment) {
    // When there is not enough space, we signal the kEndOfBlock, so that the iterator knows
    // to move to the next one. kEndOfBlock on the last block means the end of the commands.
    uint32_t* idAlloc = reinterpret_cast<uint32_t*>(mCurrentPtr);
    *idAlloc = detail::kEndOfBlock;

    // We'll request a block that can contain at least the command ID, the command and an
    // additional ID to contain the kEndOfBlock tag.
    size_t requestedBlockSize = commandSize + kWorstCaseAdditionalSize;

    // The computation of the request could overflow.
    if (DAWN_UNLIKELY(requestedBlockSize <= commandSize)) {
        return nullptr;
    }

    if (DAWN_UNLIKELY(!GetNewBlock(requestedBlockSize))) {
        return nullptr;
    }
    return Allocate(commandId, commandSize, commandAlignment);
}

bool CommandAllocator::GetNewBlock(size_t minimumSize) {
    // Allocate blocks doubling sizes each time, to a maximum of 16k (or at least minimumSize).
    mLastAllocationSize = std::max(minimumSize, std::min(mLastAllocationSize * 2, size_t(16384)));

    uint8_t* block = static_cast<uint8_t*>(malloc(mLastAllocationSize));
    if (DAWN_UNLIKELY(block == nullptr)) {
        return false;
    }

    mBlocks.push_back({mLastAllocationSize, block});
    mCurrentPtr = AlignPtr(block, alignof(uint32_t));
    mEndPtr = block + mLastAllocationSize;
    return true;
}

void CommandAllocator::ResetPointers() {
    mCurrentPtr = reinterpret_cast<uint8_t*>(&mPlaceholderEnum[0]);
    mEndPtr = reinterpret_cast<uint8_t*>(&mPlaceholderEnum[1]);
}

}  // namespace dawn::native
