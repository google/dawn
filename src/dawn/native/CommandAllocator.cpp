// Copyright 2017 The Dawn & Tint Authors
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

#include "src/dawn/native/CommandAllocator.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <new>
#include <utility>

#include "src/dawn/common/Math.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::native {

CommandIterator::CommandIterator() {
    Reset();
}

CommandIterator::~CommandIterator() {
    DAWN_ASSERT(IsEmpty());
}

CommandIterator::CommandIterator(CommandIterator&& other) {
    if (!other.IsEmpty()) {
        mBlocks = std::move(other.mBlocks);
        other.Reset();
    }
    Reset();
}

CommandIterator& CommandIterator::operator=(CommandIterator&& other) {
    DAWN_ASSERT(IsEmpty());
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
    DAWN_ASSERT(IsEmpty());
    mBlocks.clear();

    size_t totalBlocksCount = 0;
    for (CommandAllocator& allocator : allocators) {
        totalBlocksCount += allocator.GetCommandBlocksCount();
    }

    mBlocks.reserve(totalBlocksCount);
    for (CommandAllocator& allocator : allocators) {
        CommandBlocks blocks = allocator.AcquireBlocks();
        if (!blocks.empty()) {
            for (BlockDef& block : blocks) {
                mBlocks.push_back(std::move(block));
            }
        }
    }
    Reset();
}

std::optional<uint32_t> CommandIterator::NextCommandIdInNewBlock() {
    mCurrentBlockIndex++;
    if (mCurrentBlockIndex >= mBlocks.size()) {
        Reset();
        return std::nullopt;
    }
    mCurrentBlock = mBlocks[mCurrentBlockIndex];
    DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));
    return NextCommandId();
}

void CommandIterator::Reset() {
    mCurrentBlockIndex = 0;

    if (mBlocks.empty()) {
        // This will case the first NextCommandId call to try to move to the next block and stop
        // the iteration immediately, without special casing the initialization.
        mCurrentBlock = ByteSpanFromRef(mEndOfBlock);
    } else {
        mCurrentBlock = mBlocks[0];
    }
    DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));
}

void CommandIterator::MakeEmptyAsDataWasDestroyed() {
    if (IsEmpty()) {
        return;
    }

    mCurrentBlock = ByteSpanFromRef(mEndOfBlock);
    mBlocks.clear();
    Reset();
    DAWN_ASSERT(IsEmpty());
}

bool CommandIterator::IsEmpty() const {
    return mBlocks.empty();
}

// Potential TODO(crbug.com/dawn/835):
//  - Host the size and pointer to next block in the block itself to avoid having an allocation
//    in the vector
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
        mCurrentBlock = other.mCurrentBlock;
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
        mCurrentBlock = other.mCurrentBlock;
    }
    other.Reset();
    return *this;
}

void CommandAllocator::Reset() {
    ResetPointers();
    mBlocks.clear();
    mLastAllocationSize = kDefaultBaseAllocationSize;
}

bool CommandAllocator::IsEmpty() const {
    return mCurrentBlock.data() == mPlaceholderSpace.data();
}

size_t CommandAllocator::GetCommandBlocksCount() const {
    return mBlocks.size();
}

CommandBlocks&& CommandAllocator::AcquireBlocks() {
    DAWN_ASSERT(!mCurrentBlock.empty());
    DAWN_ASSERT(IsPtrAligned(mCurrentBlock.data(), kMaxAllocatedCommandAlignment));
    DAWN_ASSERT(mCurrentBlock.size() >= sizeof(uint32_t));
    *reinterpret_cast<uint32_t*>(mCurrentBlock.data()) = detail::kEndOfBlock;

    mCurrentBlock = {};
    return std::move(mBlocks);
}

Span<std::byte> CommandAllocator::AllocateInNewBlock(uint32_t commandId, size_t commandSize) {
    // When there is not enough space, we signal the kEndOfBlock, so that the iterator knows
    // to move to the next one. kEndOfBlock on the last block means the end of the commands.
    *reinterpret_cast<uint32_t*>(mCurrentBlock.data()) = detail::kEndOfBlock;

    // We'll request a block that can contain at least the command ID, the command and an
    // additional ID to contain the kEndOfBlock tag.
    size_t requestedBlockSize = commandSize + kWorstCaseAdditionalSize;

    // The computation of the request could overflow.
    if (requestedBlockSize <= commandSize) [[unlikely]] {
        return {};
    }

    AppendNewBlock(requestedBlockSize);
    return Allocate(commandId, commandSize);
}

void CommandAllocator::AppendNewBlock(size_t minimumSize) {
    // Allocate blocks doubling sizes each time, to a maximum of 16k (or at least minimumSize).
    mLastAllocationSize = std::max(minimumSize, std::min(mLastAllocationSize * 2, size_t(16384)));

    // SAFETY: This is a pool allocation that will be initialized when it's suballocated.
    auto block = DAWN_UNSAFE_BUFFERS(HeapArray<std::byte>::Uninit(mLastAllocationSize));
    DAWN_ASSERT(IsPtrAligned(block.data(), kMaxAllocatedCommandAlignment));

    mCurrentBlock = block;
    mBlocks.push_back(std::move(block));
}

void CommandAllocator::ResetPointers() {
    mCurrentBlock = ByteSpanFromRef(mPlaceholderSpace);
}

}  // namespace dawn::native
