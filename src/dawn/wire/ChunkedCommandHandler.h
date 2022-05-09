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

#ifndef SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_
#define SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_

#include <cstdint>
#include <limits>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"

namespace dawn::wire {

class ChunkedCommandHandler : public CommandHandler {
  public:
    ChunkedCommandHandler();
    ~ChunkedCommandHandler() override;

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override;

  protected:
    enum class ChunkedCommandsResult {
        Passthrough,
        Consumed,
        Error,
    };

    // Returns |true| if the commands were entirely consumed into the chunked command vector
    // and should be handled later once we receive all the command data.
    // Returns |false| if commands should be handled now immediately.
    ChunkedCommandsResult HandleChunkedCommands(const volatile char* commands, size_t size) {
        uint64_t commandSize64 = reinterpret_cast<const volatile CmdHeader*>(commands)->commandSize;

        if (commandSize64 > std::numeric_limits<size_t>::max()) {
            return ChunkedCommandsResult::Error;
        }
        size_t commandSize = static_cast<size_t>(commandSize64);
        if (size < commandSize) {
            return BeginChunkedCommandData(commands, commandSize, size);
        }
        return ChunkedCommandsResult::Passthrough;
    }

  private:
    virtual const volatile char* HandleCommandsImpl(const volatile char* commands, size_t size) = 0;

    ChunkedCommandsResult BeginChunkedCommandData(const volatile char* commands,
                                                  size_t commandSize,
                                                  size_t initialSize);

    size_t mChunkedCommandRemainingSize = 0;
    size_t mChunkedCommandPutOffset = 0;
    std::unique_ptr<char[]> mChunkedCommandData;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDHANDLER_H_
