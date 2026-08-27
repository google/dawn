// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
#define SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_

#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>

#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Compiler.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Math.h"
#include "src/utils/heap_array.h"
#include "src/utils/span.h"

namespace dawn::wire {

// Simple command extension struct used when a command needs to serialize additional information
// that is not baked directly into the command already.
template <auto Member>
struct CommandExtension {
    static_assert(Member != nullptr, "CommandExtension member pointer cannot be null.");

    size_t size = 0;
    std::function<void(Span<volatile std::byte>)> serialize = {};
};

namespace detail {

template <typename Cmd>
inline WireResult SerializeCommandExtension(Cmd& cmd, SerializeBuffer* serializeBuffer) {
    return WireResult::Success;
}

template <typename Cmd, auto Member, typename... Extensions>
WireResult SerializeCommandExtension(Cmd& cmd,
                                     SerializeBuffer* serializeBuffer,
                                     const CommandExtension<Member>& e,
                                     Extensions&&... es) {
    Span<volatile std::byte> buffer;
    WIRE_TRY(serializeBuffer->NextN(e.size, &buffer));
    e.serialize(buffer);

    // SAFETY: This Span is NEVER supposed to be read/serialized, only its size is used during Cmd
    // serialization so that the deserializer sees the correct length.
    DAWN_UNSAFE_BUFFERS(cmd.*Member =
                            Span<const std::byte>(static_cast<const std::byte*>(nullptr), e.size));

    WIRE_TRY(SerializeCommandExtension(cmd, serializeBuffer, std::forward<Extensions>(es)...));
    return WireResult::Success;
}

}  // namespace detail

class ChunkedCommandSerializer {
  public:
    explicit ChunkedCommandSerializer(CommandSerializer* serializer);

    // This utility function is intended only for disconnect situations where we want the serializer
    // to appear to keep working even though we are no longer serializing and flushing commands.
    void SetCommandSerializerForDisconnect(CommandSerializer* serializer);

    template <typename Cmd>
    void SerializeCommand(Cmd&& cmd) {
        SerializeCommandImpl(std::forward<Cmd>(cmd), [](const Cmd& cmd, size_t requiredSize,
                                                        SerializeBuffer* serializeBuffer) {
            return cmd.Serialize(requiredSize, serializeBuffer);
        });
    }

    template <typename Cmd, typename Extension, typename... Extensions>
        requires(!std::is_base_of_v<ObjectIdProvider, std::decay_t<Extension>>)
    void SerializeCommand(Cmd&& cmd, Extension&& e, Extensions&&... es) {
        SerializeCommandImpl(
            std::forward<Cmd>(cmd),
            [](const Cmd& cmd, size_t requiredSize, SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer);
            },
            std::forward<Extension>(e), std::forward<Extensions>(es)...);
    }

    template <typename Cmd, typename... Extensions>
    void SerializeCommand(Cmd&& cmd,
                          const ObjectIdProvider& objectIdProvider,
                          Extensions&&... extensions) {
        SerializeCommandImpl(
            std::forward<Cmd>(cmd),
            [&objectIdProvider](const Cmd& cmd, size_t requiredSize,
                                SerializeBuffer* serializeBuffer) {
                return cmd.Serialize(requiredSize, serializeBuffer, objectIdProvider);
            },
            std::forward<Extensions>(extensions)...);
    }

    template <typename Cmd, typename... Args>
    void SerializeCommand(Cmd&, Args&&...) = delete;

    void Flush();

  private:
    template <typename Cmd, typename SerializeCmdFn, typename... Extensions>
    void SerializeCommandImpl(Cmd&& cmd,
                              SerializeCmdFn&& SerializeCmd,
                              Extensions&&... extensions) {
        size_t commandSize = cmd.GetRequiredSize();
        size_t requiredSize = (Align(extensions.size, kWireBufferAlignment) + ... + commandSize);

        if (requiredSize <= mMaxAllocationSize) {
            std::optional<std::span<volatile std::byte>> cmdSpace =
                mSerializer->GetCommandSpace(requiredSize);
            if (cmdSpace) {
                const auto [cmdBuffer, extBuffer] =
                    Span<volatile std::byte>(*cmdSpace).SplitAt(commandSize);

                // We must serialize the extensions first since this also updates the command's
                // extension members with the appropriate sizes for the extension members.
                SerializeBuffer extSerializeBuffer(extBuffer);
                WireResult rExts =
                    detail::SerializeCommandExtension(cmd, &extSerializeBuffer, extensions...);

                // Now that the command's extension members have been updated, we can serialise the
                // command.
                SerializeBuffer cmdSerializeBuffer(cmdBuffer);
                WireResult rCmd = SerializeCmd(cmd, requiredSize, &cmdSerializeBuffer);
                if (rCmd != WireResult::Success || rExts != WireResult::Success) [[unlikely]] {
                    mSerializer->OnSerializeError();
                }
            }
            return;
        }

        // Allocate as zero-initialized because padding won't get initialized during command
        // serialization (and this whole buffer is sent raw to the other end of the wire).
        HeapArray<std::byte> cmdSpace(requiredSize);
        const auto [cmdBuffer, extBuffer] = Span<std::byte>(cmdSpace).SplitAt(commandSize);

        // We must serialize the extensions first since this also updates the command's extension
        // members with the appropriate sizes for the extension members.
        SerializeBuffer extSerializeBuffer(extBuffer);
        WireResult rExts =
            detail::SerializeCommandExtension(cmd, &extSerializeBuffer, extensions...);

        // Now that the command's extension members have been updated, we can serialise the command.
        SerializeBuffer cmdSerializeBuffer(cmdBuffer);
        WireResult rCmd = SerializeCmd(cmd, requiredSize, &cmdSerializeBuffer);
        if (rCmd != WireResult::Success || rExts != WireResult::Success) [[unlikely]] {
            mSerializer->OnSerializeError();
            return;
        }
        SerializeChunkedCommand(cmdSpace);
    }

    void SerializeChunkedCommand(Span<const std::byte> allocatedBuffer);

    raw_ptr<CommandSerializer> mSerializer;
    size_t mMaxAllocationSize;
    std::atomic<uint64_t> mNextChunkedCommandId = 0;
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_CHUNKEDCOMMANDSERIALIZER_H_
