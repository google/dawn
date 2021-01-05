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

#ifndef DAWNWIRE_CHUNKEDCOMMANDSERIALIZER_H_
#define DAWNWIRE_CHUNKEDCOMMANDSERIALIZER_H_

#include "common/Alloc.h"
#include "common/Compiler.h"
#include "dawn_wire/Wire.h"
#include "dawn_wire/WireCmd_autogen.h"

#include <algorithm>
#include <cstring>
#include <memory>

namespace dawn_wire {

    class ChunkedCommandSerializer {
      public:
        ChunkedCommandSerializer(CommandSerializer* serializer);

        template <typename Cmd>
        void SerializeCommand(const Cmd& cmd) {
            SerializeCommand(cmd, 0, [](char*) {});
        }

        template <typename Cmd, typename ExtraSizeSerializeFn>
        void SerializeCommand(const Cmd& cmd,
                              size_t extraSize,
                              ExtraSizeSerializeFn&& SerializeExtraSize) {
            SerializeCommandImpl(
                cmd,
                [](const Cmd& cmd, size_t requiredSize, char* allocatedBuffer) {
                    cmd.Serialize(requiredSize, allocatedBuffer);
                },
                extraSize, std::forward<ExtraSizeSerializeFn>(SerializeExtraSize));
        }

        template <typename Cmd>
        void SerializeCommand(const Cmd& cmd, const ObjectIdProvider& objectIdProvider) {
            SerializeCommand(cmd, objectIdProvider, 0, [](char*) {});
        }

        template <typename Cmd, typename ExtraSizeSerializeFn>
        void SerializeCommand(const Cmd& cmd,
                              const ObjectIdProvider& objectIdProvider,
                              size_t extraSize,
                              ExtraSizeSerializeFn&& SerializeExtraSize) {
            SerializeCommandImpl(
                cmd,
                [&objectIdProvider](const Cmd& cmd, size_t requiredSize, char* allocatedBuffer) {
                    cmd.Serialize(requiredSize, allocatedBuffer, objectIdProvider);
                },
                extraSize, std::forward<ExtraSizeSerializeFn>(SerializeExtraSize));
        }

      private:
        template <typename Cmd, typename SerializeCmdFn, typename ExtraSizeSerializeFn>
        void SerializeCommandImpl(const Cmd& cmd,
                                  SerializeCmdFn&& SerializeCmd,
                                  size_t extraSize,
                                  ExtraSizeSerializeFn&& SerializeExtraSize) {
            size_t commandSize = cmd.GetRequiredSize();
            size_t requiredSize = commandSize + extraSize;

            if (requiredSize <= mMaxAllocationSize) {
                char* allocatedBuffer = static_cast<char*>(mSerializer->GetCmdSpace(requiredSize));
                if (allocatedBuffer != nullptr) {
                    SerializeCmd(cmd, requiredSize, allocatedBuffer);
                    SerializeExtraSize(allocatedBuffer + commandSize);
                }
                return;
            }

            auto cmdSpace = std::unique_ptr<char[]>(AllocNoThrow<char>(requiredSize));
            if (!cmdSpace) {
                return;
            }
            SerializeCmd(cmd, requiredSize, cmdSpace.get());
            SerializeExtraSize(cmdSpace.get() + commandSize);
            SerializeChunkedCommand(cmdSpace.get(), requiredSize);
        }

        void SerializeChunkedCommand(const char* allocatedBuffer, size_t remainingSize);

        CommandSerializer* mSerializer;
        size_t mMaxAllocationSize;
    };

}  // namespace dawn_wire

#endif  // DAWNWIRE_CHUNKEDCOMMANDSERIALIZER_H_
