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

#ifndef SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
#define SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_

#include <algorithm>
#include <cstring>
#include <memory>
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Compiler.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"

namespace dawn::wire {

class ChunkedCommandSerializer {
  public:
    explicit ChunkedCommandSerializer(CommandSerializer* serializer);

    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd) {
        SerializeCommand(cmd, 0, [](SerializeBuffer*) { return WireResult::Success; });
    }

    template <typename Cmd, typename ExtraSizeSerializeFn>
    void SerializeCommand(const Cmd& cmd,
                          size_t extraSize,
                          ExtraSizeSerializeFn&& SerializeExtraSize) {
        SerializeCommandImpl(
            cmd,
            [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            },
            extraSize, std::forward<ExtraSizeSerializeFn>(SerializeExtraSize));
    }

    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd, const ObjectIdProvider& objectIdProvider) {
        SerializeCommand(cmd, objectIdProvider, 0,
                         [](SerializeBuffer*) { return WireResult::Success; });
    }

    template <typename Cmd, typename ExtraSizeSerializeFn>
    void SerializeCommand(const Cmd& cmd,
                          const ObjectIdProvider& objectIdProvider,
                          size_t extraSize,
                          ExtraSizeSerializeFn&& SerializeExtraSize) {
        SerializeCommandImpl(
            cmd,
            [&objectIdProvider](const Cmd& cmd, size_t requiredSize,
                                SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer, objectIdProvider);
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
                SerializeBuffer serializeBuffer(allocatedBuffer, requiredSize);
                WireResult r1 = SerializeCmd(cmd, requiredSize, &serializeBuffer);
                WireResult r2 = SerializeExtraSize(&serializeBuffer);
                if (DAWN_UNLIKELY(r1 != WireResult::Success || r2 != WireResult::Success)) {
                    mSerializer->OnSerializeError();
                }
            }
            return;
        }

        auto cmdSpace = std::unique_ptr<char[]>(AllocNoThrow<char>(requiredSize));
        if (!cmdSpace) {
            return;
        }
        SerializeBuffer serializeBuffer(cmdSpace.get(), requiredSize);
        WireResult r1 = SerializeCmd(cmd, requiredSize, &serializeBuffer);
        WireResult r2 = SerializeExtraSize(&serializeBuffer);
        if (DAWN_UNLIKELY(r1 != WireResult::Success || r2 != WireResult::Success)) {
            mSerializer->OnSerializeError();
            return;
        }
        SerializeChunkedCommand(cmdSpace.get(), requiredSize);
    }

    void SerializeChunkedCommand(const char* allocatedBuffer, size_t remainingSize);

    CommandSerializer* mSerializer;
    size_t mMaxAllocationSize;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
