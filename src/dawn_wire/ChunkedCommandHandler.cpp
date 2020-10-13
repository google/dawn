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
            mChunkedCommandData.insert(mChunkedCommandData.end(), commands, commands + chunkSize);

            commands += chunkSize;
            mChunkedCommandRemainingSize -= chunkSize;
            size -= chunkSize;

            if (mChunkedCommandRemainingSize == 0) {
                // Once the chunked command is complete, pass the data to the command handler
                // implemenation.
                const char* chunkedCommands = mChunkedCommandData.data();
                size_t chunkedSize = mChunkedCommandData.size();
                if (HandleCommandsImpl(chunkedCommands, chunkedSize) == nullptr) {
                    // |HandleCommandsImpl| returns nullptr on error. Forward any errors
                    // out.
                    return nullptr;
                }
                mChunkedCommandData.clear();
            }
        }

        return HandleCommandsImpl(commands, size);
    }

    void ChunkedCommandHandler::BeginChunkedCommandData(const volatile char* commands,
                                                        size_t commandSize,
                                                        size_t initialSize) {
        ASSERT(mChunkedCommandData.empty());

        // Reserve space for all the command data we're expecting, and append the initial data
        // to the end of the vector.
        mChunkedCommandRemainingSize = commandSize - initialSize;
        mChunkedCommandData.reserve(commandSize);
        mChunkedCommandData.insert(mChunkedCommandData.end(), commands, commands + initialSize);
    }

}  // namespace dawn_wire
