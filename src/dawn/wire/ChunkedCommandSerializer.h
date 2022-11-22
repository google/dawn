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
#include <functional>
#include <memory>
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Compiler.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"

namespace dawn::wire {

// Simple command extension struct used when a command needs to serialize additional information
// that is not baked directly into the command already.
struct CommandExtension {
    size_t size;
    std::function<void(char*)> serialize;
};

namespace detail {

inline WireResult SerializeCommandExtension(SerializeBuffer* serializeBuffer) {
    return WireResult::Success;
}

template <typename Extension, typename... Extensions>
WireResult SerializeCommandExtension(SerializeBuffer* serializeBuffer,
                                     Extension&& e,
                                     Extensions&&... es) {
    char* buffer;
    WIRE_TRY(serializeBuffer->NextN(e.size, &buffer));
    e.serialize(buffer);

    WIRE_TRY(SerializeCommandExtension(serializeBuffer, std::forward<Extensions>(es)...));
    return WireResult::Success;
}

}  // namespace detail

class ChunkedCommandSerializer {
  public:
    explicit ChunkedCommandSerializer(CommandSerializer* serializer);

    template <typename Cmd>
    void SerializeCommand(const Cmd& cmd) {
        SerializeCommandImpl(
            cmd, [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            });
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd, CommandExtension&& e, Extensions&&... es) {
        SerializeCommandImpl(
            cmd,
            [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            },
            std::forward<CommandExtension>(e), std::forward<Extensions>(es)...);
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(const Cmd& cmd,
                          const ObjectIdProvider& objectIdProvider,
                          Extensions&&... extensions) {
        SerializeCommandImpl(
            cmd,
            [&objectIdProvider](const Cmd& cmd, size_t requiredSize,
                                SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer, objectIdProvider);
            },
            std::forward<Extensions>(extensions)...);
    }

  private:
    template <typename Cmd, typename SerializeCmdFn, typename... Extensions>
    void SerializeCommandImpl(const Cmd& cmd,
                              SerializeCmdFn&& SerializeCmd,
                              Extensions&&... extensions) {
        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = (Align(extensions.size, kWireBufferAlignment) + ... + commandSize);

        if (requiredSize <= mMaxAllocationSize) {
            char* allocatedBuffer = static_cast<char*>(mSerializer->GetCmdSpace(requiredSize));
            if (allocatedBuffer != nullptr) {
                SerializeBuffer serializeBuffer(allocatedBuffer, requiredSize);
                WireResult rCmd = SerializeCmd(cmd, requiredSize, &serializeBuffer);
                WireResult rExts =
                    detail::SerializeCommandExtension(&serializeBuffer, extensions...);
                if (DAWN_UNLIKELY(rCmd != WireResult::Success || rExts != WireResult::Success)) {
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
        WireResult rCmd = SerializeCmd(cmd, requiredSize, &serializeBuffer);
        WireResult rExts = detail::SerializeCommandExtension(&serializeBuffer, extensions...);
        if (DAWN_UNLIKELY(rCmd != WireResult::Success || rExts != WireResult::Success)) {
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
