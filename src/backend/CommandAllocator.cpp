// Copyright 2017 The NXT Authors
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

#include "backend/CommandAllocator.h"

#include "common/Assert.h"
#include "common/Math.h"

#include <algorithm>
#include <climits>
#include <cstdlib>

namespace backend {

    constexpr uint32_t EndOfBlock = UINT_MAX;//std::numeric_limits<uint32_t>::max();
    constexpr uint32_t AdditionalData = UINT_MAX - 1;//std::numeric_limits<uint32_t>::max();

    // TODO(cwallez@chromium.org): figure out a way to have more type safety for the iterator

    CommandIterator::CommandIterator()
        : endOfBlock(EndOfBlock) {
        Reset();
    }

    CommandIterator::~CommandIterator() {
        ASSERT(dataWasDestroyed);

        if (!IsEmpty()) {
            for (auto& block : blocks) {
                free(block.block);
            }
        }
    }

    CommandIterator::CommandIterator(CommandIterator&& other)
        : endOfBlock(EndOfBlock) {
        if (!other.IsEmpty()) {
            blocks = std::move(other.blocks);
            other.Reset();
        }
        other.DataWasDestroyed();
        Reset();
    }

    CommandIterator& CommandIterator::operator=(CommandIterator&& other) {
        if (!other.IsEmpty()) {
            blocks = std::move(other.blocks);
            other.Reset();
        } else {
            blocks.clear();
        }
        other.DataWasDestroyed();
        Reset();
        return *this;
    }

    CommandIterator::CommandIterator(CommandAllocator&& allocator)
        : blocks(allocator.AcquireBlocks()), endOfBlock(EndOfBlock) {
        Reset();
    }

    CommandIterator& CommandIterator::operator=(CommandAllocator&& allocator) {
        blocks = allocator.AcquireBlocks();
        Reset();
        return *this;
    }

    void CommandIterator::Reset() {
        currentBlock = 0;

        if (blocks.empty()) {
            // This will case the first NextCommandId call to try to move to the next
            // block and stop the iteration immediately, without special casing the
            // initialization.
            currentPtr = reinterpret_cast<uint8_t*>(&endOfBlock);
            blocks.emplace_back();
            blocks[0].size = sizeof(endOfBlock);
            blocks[0].block = currentPtr;
        } else {
            currentPtr = AlignPtr(blocks[0].block, alignof(uint32_t));
        }
    }

    void CommandIterator::DataWasDestroyed() {
        dataWasDestroyed = true;
    }

    bool CommandIterator::IsEmpty() const {
        return blocks[0].block == reinterpret_cast<const uint8_t*>(&endOfBlock);
    }

    bool CommandIterator::NextCommandId(uint32_t* commandId) {
        uint8_t* idPtr = AlignPtr(currentPtr, alignof(uint32_t));
        ASSERT(idPtr + sizeof(uint32_t) <= blocks[currentBlock].block + blocks[currentBlock].size);

        uint32_t id = *reinterpret_cast<uint32_t*>(idPtr);

        if (id == EndOfBlock) {
            currentBlock++;
            if (currentBlock >= blocks.size()) {
                Reset();
                return false;
            }
            currentPtr = AlignPtr(blocks[currentBlock].block, alignof(uint32_t));
            return NextCommandId(commandId);
        }

        currentPtr = idPtr + sizeof(uint32_t);
        *commandId = id;
        return true;
    }

    void* CommandIterator::NextCommand(size_t commandSize, size_t commandAlignment) {
        uint8_t* commandPtr = AlignPtr(currentPtr, commandAlignment);
        ASSERT(commandPtr + sizeof(commandSize) <= blocks[currentBlock].block + blocks[currentBlock].size);

        currentPtr = commandPtr + commandSize;
        return commandPtr;
    }

    void* CommandIterator::NextData(size_t dataSize, size_t dataAlignment) {
        uint32_t id;
        bool hasId = NextCommandId(&id);
        ASSERT(hasId);
        ASSERT(id == AdditionalData);

        return NextCommand(dataSize, dataAlignment);
    }

    // Potential TODO(cwallez@chromium.org):
    //  - Host the size and pointer to next block in the block itself to avoid having an allocation in the vector
    //  - Assume T's alignof is, say 64bits, static assert it, and make commandAlignment a constant in Allocate
    //  - Be able to optimize allocation to one block, for command buffers expected to live long to avoid cache misses
    //  - Better block allocation, maybe have NXT API to say command buffer is going to have size close to another

    CommandAllocator::CommandAllocator()
        : currentPtr(reinterpret_cast<uint8_t*>(&dummyEnum[0])), endPtr(reinterpret_cast<uint8_t*>(&dummyEnum[1])) {
    }

    CommandAllocator::~CommandAllocator() {
        ASSERT(blocks.empty());
    }

    CommandBlocks&& CommandAllocator::AcquireBlocks() {
        ASSERT(currentPtr != nullptr && endPtr != nullptr);
        ASSERT(IsAligned(currentPtr, alignof(uint32_t)));
        ASSERT(currentPtr + sizeof(uint32_t) <= endPtr);
        *reinterpret_cast<uint32_t*>(currentPtr) = EndOfBlock;

        currentPtr = nullptr;
        endPtr = nullptr;
        return std::move(blocks);
    }

    uint8_t* CommandAllocator::Allocate(uint32_t commandId, size_t commandSize, size_t commandAlignment) {
        ASSERT(currentPtr != nullptr);
        ASSERT(endPtr != nullptr);
        ASSERT(commandId != EndOfBlock);

        // It should always be possible to allocate one id, for EndOfBlock tagging,
        ASSERT(IsAligned(currentPtr, alignof(uint32_t)));
        ASSERT(currentPtr + sizeof(uint32_t) <= endPtr);
        uint32_t* idAlloc = reinterpret_cast<uint32_t*>(currentPtr);

        uint8_t* commandAlloc = AlignPtr(currentPtr + sizeof(uint32_t), commandAlignment);
        uint8_t* nextPtr = AlignPtr(commandAlloc + commandSize, alignof(uint32_t));

        // When there is not enough space, we signal the EndOfBlock, so that the iterator nows to
        // move to the next one. EndOfBlock on the last block means the end of the commands.
        if (nextPtr + sizeof(uint32_t) > endPtr) {

            // Even if we are not able to get another block, the list of commands will be well-formed
            // and iterable as this block will be that last one.
            *idAlloc = EndOfBlock;

            // Make sure we have space for current allocation, plus end of block and alignment padding
            // for the first id.
            ASSERT(nextPtr > currentPtr);
            if (!GetNewBlock(static_cast<size_t>(nextPtr - currentPtr) + sizeof(uint32_t) + alignof(uint32_t))) {
                return nullptr;
            }
            return Allocate(commandId, commandSize, commandAlignment);
        }

        *idAlloc = commandId;
        currentPtr = nextPtr;
        return commandAlloc;
    }

    uint8_t* CommandAllocator::AllocateData(size_t commandSize, size_t commandAlignment) {
        return Allocate(AdditionalData, commandSize, commandAlignment);
    }

    bool CommandAllocator::GetNewBlock(size_t minimumSize) {
        // Allocate blocks doubling sizes each time, to a maximum of 16k (or at least minimumSize).
        lastAllocationSize = std::max(minimumSize, std::min(lastAllocationSize * 2, size_t(16384)));

        uint8_t* block = reinterpret_cast<uint8_t*>(malloc(lastAllocationSize));
        if (block == nullptr) {
            return false;
        }

        blocks.push_back({lastAllocationSize, block});
        currentPtr = AlignPtr(block, alignof(uint32_t));
        endPtr = block + lastAllocationSize;
        return true;
    }

}
