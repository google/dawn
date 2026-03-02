// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/replay/CaptureWalker.h"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "dawn/replay/Capture.h"
#include "dawn/replay/Deserialization.h"

namespace dawn::replay {

namespace {

// These x-macros use DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES which
// is a list of all BindGroupLayoutEntry types to auto generate
// a switch case for each type of BindGroupLayoutEntryType that deserializes
// a capture for that type and converts it to an std::variant entry
// for that type.
#define DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_VALID(NAME)    \
    case schema::BindGroupLayoutEntryType::NAME: {                  \
        schema::BindGroupLayoutEntryType##NAME data;                \
        data.variantType = type;                                    \
        DAWN_TRY(Deserialize(readHead, &data.binding, &data.data)); \
        *out = std::move(data);                                     \
        return {};                                                  \
    }
#define DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_INVALID(NAME, VALUE)
#define DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_GET_MACRO(_1, _2, NAME, ...) NAME
#define DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE(...)                  \
    DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_GET_MACRO(                     \
        __VA_ARGS__, DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_INVALID, \
        DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_VALID)(__VA_ARGS__)

MaybeError Deserialize(ReadHead& readHead, BindGroupLayoutEntryVariant* out) {
    schema::BindGroupLayoutEntryType type;
    DAWN_TRY(Deserialize(readHead, &type));
    switch (type) {
        DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES(DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE)
        default:
            return DAWN_INTERNAL_ERROR("unhandled bind group layout entry type");
    }
}
#undef DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE
#undef DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_GET_MACRO
#undef DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_INVALID
#undef DAWN_REPLAY_BINDGROUPLAYOUT_DESERIALIZE_CASE_VALID

// These x-macros use DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES which
// is a list of all BindGroupLayoutEntry types to auto generate
// a switch case for each type of BindGroupEntryType that deserializes
// a capture for that type and converts it to an std::variant entry
// for that type.
#define DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_VALID(NAME)          \
    case schema::BindGroupLayoutEntryType::NAME: {                  \
        schema::BindGroupEntryType##NAME data;                      \
        data.variantType = type;                                    \
        DAWN_TRY(Deserialize(readHead, &data.binding, &data.data)); \
        *out = std::move(data);                                     \
        return {};                                                  \
    }
#define DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_INVALID(NAME, VALUE)
#define DAWN_REPLAY_BINDGROUP_DESERIALIZE_GET_MACRO(_1, _2, NAME, ...) NAME
#define DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE(...)                  \
    DAWN_REPLAY_BINDGROUP_DESERIALIZE_GET_MACRO(                     \
        __VA_ARGS__, DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_INVALID, \
        DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_VALID)(__VA_ARGS__)

#define DAWN_REPLAY_GEN_BINDGROUP_DESERIALIZE(ENUM_NAME, MEMBERS)              \
    MaybeError Deserialize(ReadHead& readHead, BindGroupEntryVariant* out) {   \
        schema::ENUM_NAME type;                                                \
        DAWN_TRY(Deserialize(readHead, &type));                                \
        switch (type) {                                                        \
            MEMBERS(DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE)                    \
            default:                                                           \
                return DAWN_INTERNAL_ERROR("unhandled bind group entry type"); \
        }                                                                      \
    }

DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES_ENUM(DAWN_REPLAY_GEN_BINDGROUP_DESERIALIZE)

#undef DAWN_REPLAY_GEN_BINDGROUP_DESERIALIZE
#undef DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE
#undef DAWN_REPLAY_BINDGROUP_DESERIALIZE_GET_MACRO
#undef DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_INVALID
#undef DAWN_REPLAY_BINDGROUP_DESERIALIZE_CASE_VALID

MaybeError Deserialize(ReadHead& readHead, BindGroupData* out) {
    DAWN_TRY(Deserialize(readHead, &out->bg));
    out->entries.reserve(out->bg.numEntries);
    for (uint32_t i = 0; i < out->bg.numEntries; ++i) {
        BindGroupEntryVariant entry;
        DAWN_TRY(Deserialize(readHead, &entry));
        out->entries.push_back(std::move(entry));
    }
    return {};
}

MaybeError Deserialize(ReadHead& readHead, BindGroupLayoutData* out) {
    DAWN_TRY(Deserialize(readHead, &out->bgl));
    out->entries.reserve(out->bgl.numEntries);
    for (uint32_t i = 0; i < out->bgl.numEntries; ++i) {
        BindGroupLayoutEntryVariant entry;
        DAWN_TRY(Deserialize(readHead, &entry));
        out->entries.push_back(std::move(entry));
    }
    return {};
}

MaybeError Deserialize(ReadHead& readHead, CommandBufferData* out) {
    out->readHead = &readHead;
    return {};
}

MaybeError Deserialize(ReadHead& readHead, RenderBundleData* out) {
    DAWN_TRY(Deserialize(readHead, &out->bundle));
    out->readHead = &readHead;
    return {};
}

MaybeError DeserializeResourceData(ReadHead& readHead, schema::ObjectType type, ResourceData* out) {
    switch (type) {
#define AS_DESERIALIZE_RESOURCE_DATA_CASE(NAME, TYPE)                                            \
    case schema::ObjectType::NAME: {                                                             \
        if constexpr (!std::is_same_v<TYPE, InvalidData> && !std::is_same_v<TYPE, DeviceData>) { \
            TYPE data;                                                                           \
            DAWN_TRY(Deserialize(readHead, &data));                                              \
            *out = std::move(data);                                                              \
        } else {                                                                                 \
            *out = TYPE{};                                                                       \
        }                                                                                        \
        return {};                                                                               \
    }
        DAWN_REPLAY_RESOURCE_DATA_MAP(AS_DESERIALIZE_RESOURCE_DATA_CASE)
#undef AS_DESERIALIZE_RESOURCE_DATA_CASE
        default:
            return DAWN_INTERNAL_ERROR("unhandled resource type");
    }
}

MaybeError Deserialize(ReadHead& readHead, CreateResourceData* out) {
    DAWN_TRY(Deserialize(readHead, &out->resource));
    return DeserializeResourceData(readHead, out->resource.type, &out->data);
}

template <typename T>
struct RootCommandDataType {
    using Type = T;
};

template <>
struct RootCommandDataType<schema::RootCommandCreateResourceCmdData> {
    using Type = CreateResourceData;
};

#define DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_VALID(NAME) \
    RootCommandDataType<schema::RootCommand##NAME##CmdData>::Type,
#define DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_INVALID(NAME, VALUE)
#define DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_GET_MACRO(_1, _2, NAME, ...) NAME
#define DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE(...)                  \
    DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_GET_MACRO(                \
        __VA_ARGS__, DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_INVALID, \
        DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE_VALID)(__VA_ARGS__)

using RootCommandVariant =
    std::variant<DAWN_REPLAY_ROOT_COMMANDS(DAWN_REPLAY_ROOT_COMMAND_VARIANT_TYPE) std::monostate>;

#define DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE_VALID(NAME)               \
    case schema::RootCommand::NAME: {                                       \
        RootCommandDataType<schema::RootCommand##NAME##CmdData>::Type data; \
        DAWN_TRY(Deserialize(readHead, &data));                             \
        *out = std::move(data);                                             \
        return {};                                                          \
    }
#define DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE_INVALID(NAME, VALUE)
#define DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_GET_MACRO(_1, _2, NAME, ...) NAME
#define DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE(...)                  \
    DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_GET_MACRO(                     \
        __VA_ARGS__, DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE_INVALID, \
        DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE_VALID)(__VA_ARGS__)

MaybeError DeserializeRootCommand(ReadHead& readHead,
                                  schema::RootCommand cmd,
                                  RootCommandVariant* out) {
    switch (cmd) {
        DAWN_REPLAY_ROOT_COMMANDS(DAWN_REPLAY_ROOT_COMMAND_DESERIALIZE_CASE)
        default:
            return DAWN_INTERNAL_ERROR("unhandled root command");
    }
}

}  // anonymous namespace

#define PASS_COMMAND_CASE(NAME)                                                   \
    case schema::CommandBufferCommand::NAME: {                                    \
        schema::CommandBufferCommand##NAME##CmdData data;                         \
        DAWN_TRY(Deserialize(*readHead, &data));                                  \
        DAWN_TRY((*visitor)(data));                                               \
        if constexpr (std::is_same_v<schema::CommandBufferCommand##NAME##CmdData, \
                                     schema::CommandBufferCommandEndCmdData>) {   \
            return {};                                                            \
        }                                                                         \
        break;                                                                    \
    }

#define PROCESS_COMMANDS_FUNC(PASS_NAME, COMMANDS)                                             \
    MaybeError Process##PASS_NAME##Commands(ReadHead* readHead, PASS_NAME##Visitor* visitor) { \
        while (!readHead->IsDone()) {                                                          \
            schema::CommandBufferCommand cmd;                                                  \
            DAWN_TRY(Deserialize(*readHead, &cmd));                                            \
            switch (cmd) {                                                                     \
                COMMANDS(PASS_COMMAND_CASE)                                                    \
                default:                                                                       \
                    return DAWN_INTERNAL_ERROR("unhandled " #PASS_NAME " command");            \
            }                                                                                  \
        }                                                                                      \
        return DAWN_INTERNAL_ERROR("Missing " #PASS_NAME " End command");                      \
    }

PROCESS_COMMANDS_FUNC(ComputePass, DAWN_REPLAY_COMPUTE_PASS_COMMANDS)
PROCESS_COMMANDS_FUNC(RenderPass, DAWN_REPLAY_RENDER_PASS_COMMANDS)
PROCESS_COMMANDS_FUNC(RenderBundle, DAWN_REPLAY_RENDER_BUNDLE_COMMANDS)

#undef PROCESS_COMMANDS_FUNC

MaybeError ProcessEncoderCommands(ReadHead* readHead, EncoderVisitor* visitor) {
    while (!readHead->IsDone()) {
        schema::CommandBufferCommand cmd;
        DAWN_TRY(Deserialize(*readHead, &cmd));

        switch (cmd) {
            case schema::CommandBufferCommand::BeginComputePass: {
                schema::CommandBufferCommandBeginComputePassCmdData data;
                DAWN_TRY(Deserialize(*readHead, &data));
                ComputePassVisitor* subVisitor;
                DAWN_TRY_ASSIGN(subVisitor, visitor->BeginComputePass(data));
                DAWN_TRY(ProcessComputePassCommands(readHead, subVisitor));
                break;
            }
            case schema::CommandBufferCommand::BeginRenderPass: {
                schema::CommandBufferCommandBeginRenderPassCmdData data;
                DAWN_TRY(Deserialize(*readHead, &data));
                RenderPassVisitor* subVisitor;
                DAWN_TRY_ASSIGN(subVisitor, visitor->BeginRenderPass(data));
                DAWN_TRY(ProcessRenderPassCommands(readHead, subVisitor));
                break;
            }

#define ENCODER_NON_CREATION_COMMAND_CASE(NAME)                                   \
    case schema::CommandBufferCommand::NAME: {                                    \
        schema::CommandBufferCommand##NAME##CmdData data;                         \
        DAWN_TRY(Deserialize(*readHead, &data));                                  \
        DAWN_TRY((*visitor)(data));                                               \
        if constexpr (std::is_same_v<schema::CommandBufferCommand##NAME##CmdData, \
                                     schema::CommandBufferCommandEndCmdData>) {   \
            return {};                                                            \
        }                                                                         \
        break;                                                                    \
    }

                DAWN_REPLAY_ENCODER_NON_CREATION_COMMANDS(ENCODER_NON_CREATION_COMMAND_CASE)
#undef ENCODER_NON_CREATION_COMMAND_CASE

            default:
                return DAWN_INTERNAL_ERROR("unhandled encoder command");
        }
    }
    return DAWN_INTERNAL_ERROR("Missing encoder End command");
}

#undef PASS_COMMAND_CASE

MaybeError ResourceVisitor::operator()(const InvalidData& data) {
    return DAWN_INTERNAL_ERROR("Invalid resource data");
}

MaybeError ResourceVisitor::operator()(const DeviceData& data) {
    return DAWN_INTERNAL_ERROR("Device data not expected here");
}

MaybeError ResourceVisitor::operator()(const std::monostate&) {
    return DAWN_INTERNAL_ERROR("Invalid resource data (monostate)");
}

MaybeError RootCommandVisitor::operator()(const std::monostate&) {
    return DAWN_INTERNAL_ERROR("Invalid command (monostate)");
}

CaptureWalker::CaptureWalker(std::unique_ptr<const CaptureImpl> capture)
    : mCapture(std::move(capture)) {}

MaybeError CaptureWalker::Walk(RootCommandVisitor& visitor) {
    auto readHead = mCapture->GetCommandReadHead();
    auto contentReadHead = mCapture->GetContentReadHead();
    visitor.SetContentReadHead(&contentReadHead);

    while (!readHead.IsDone()) {
        schema::RootCommand cmd;
        DAWN_TRY(Deserialize(readHead, &cmd));

        RootCommandVariant v;
        DAWN_TRY(DeserializeRootCommand(readHead, cmd, &v));
        DAWN_TRY(std::visit(visitor, v));
    }

    return {};
}

}  // namespace dawn::replay
