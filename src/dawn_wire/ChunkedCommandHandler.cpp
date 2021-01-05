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

#include "dawn_wire/ChunkedCommandHandler.h"

#include "common/Alloc.h"

#include <algorithm>
#include <cstring>

namespace dawn_wire {

    ChunkedCommandHandler::~ChunkedCommandHandler() = default;

    const volatile char* ChunkedCommandHandler::HandleCommands(const volatile char* commands,
                                                               size_t size) {
        if (mChunkedCommandRemainingSize > 0) {
            // If there is a chunked command in flight, append the command data.
            // We append at most |mChunkedCommandRemainingSize| which is enough to finish the
            // in-flight chunked command, and then pass the rest along to a second call to
            // |HandleCommandsImpl|.
            size_t chunkSize = std::min(size, mChunkedCommandRemainingSize);

            memcpy(mChunkedCommandData.get() + mChunkedCommandPutOffset,
                   const_cast<const char*>(commands), chunkSize);
            mChunkedCommandPutOffset += chunkSize;
            mChunkedCommandRemainingSize -= chunkSize;

            commands += chunkSize;
            size -= chunkSize;

            if (mChunkedCommandRemainingSize == 0) {
                // Once the chunked command is complete, pass the data to the command handler
                // implemenation.
                auto chunkedCommandData = std::move(mChunkedCommandData);
                if (HandleCommandsImpl(chunkedCommandData.get(), mChunkedCommandPutOffset) ==
                    nullptr) {
                    // |HandleCommandsImpl| returns nullptr on error. Forward any errors
                    // out.
                    return nullptr;
                }
            }
        }

        return HandleCommandsImpl(commands, size);
    }

    ChunkedCommandHandler::ChunkedCommandsResult ChunkedCommandHandler::BeginChunkedCommandData(
        const volatile char* commands,
        size_t commandSize,
        size_t initialSize) {
        ASSERT(!mChunkedCommandData);

        // Reserve space for all the command data we're expecting, and copy the initial data
        // to the start of the memory.
        mChunkedCommandData.reset(AllocNoThrow<char>(commandSize));
        if (!mChunkedCommandData) {
            return ChunkedCommandsResult::Error;
        }

        memcpy(mChunkedCommandData.get(), const_cast<const char*>(commands), initialSize);
        mChunkedCommandPutOffset = initialSize;
        mChunkedCommandRemainingSize = commandSize - initialSize;

        return ChunkedCommandsResult::Consumed;
    }

}  // namespace dawn_wire
