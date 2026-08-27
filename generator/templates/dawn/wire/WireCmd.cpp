//* Copyright 2017 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/wire/WireCmd_autogen.h"

#include "dawn/wire/Wire.h"
#include "dawn/wire/dawn_platform.h"
#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/Numeric.h"
#include "src/utils/assert.h"
#include "src/utils/log.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
// error: 'offsetof' within non-standard-layout type 'wgpu::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

{% from 'dawn/cpp_macros.tmpl' import as_dawnType with context %}

//* Helper macros so that the main [de]serialization functions can be written in a generic manner.

//* Outputs an rvalue that's the number of elements a pointer member points to. Currently, this is
//* using the record_accessor to also check whether we are dealing with a record which stores the
//* length as a part of a Span, or a Transfer struct which stores the length as a field. We could
//* probably make this more explicit, but since it isn't used too frequently at the moment, this
//* is an acceptable tradeoff for now.
{%- macro member_length(member, record_accessor) -%}
    {%- if member.length == "constant" -%}
        {{member.constant_length}}u
    {%- elif record_accessor == "transfer->" -%}
        {{record_accessor}}{{as_varName(member.length.name)}}
    {%- else -%}
        {{record_accessor}}{{as_varName(member.name)}}.size()
    {%- endif -%}
{%- endmacro -%}

//* Outputs the type that will be used on the wire for the member
{%- macro member_transfer_type(type) -%}
    {%- if type.category == "object" -%}
        ObjectId
    {%- elif type.category == "structure" -%}
        {{as_cppType(type.name)}}Transfer
    {%- elif type.name.get() == "bool" -%}
        bool
    {%- elif type.name.get() == "optional bool" -%}
        {{as_cType(type.name)}}
    {%- elif type.category in ["enum", "bitmask"] -%}
        wgpu::{{as_cppType(type.name)}}
    {%- elif as_cType(type.name) == "size_t" -%}
        {{as_cType(types["uint64_t"].name)}}
    {%- elif type.name.canonical_case() == "void" -%}
        std::byte
    {%- else -%}
        {%- do assert(type.is_wire_transparent, 'wire transparent') -%}
        {{as_cType(type.name)}}
    {%- endif -%}
{%- endmacro -%}

//* Outputs the size of one element of the type that will be used on the wire for the member
{%- macro member_transfer_sizeof(type) -%}
    sizeof({{member_transfer_type(type)}})
{%- endmacro -%}

//* Outputs the serialization code to put `in` in `out`
{%- macro serialize_member(type, optional, in, out) -%}
    {%- if type.category == "object" -%}
        {%- set Optional = "Optional" if optional else "" -%}
        WIRE_TRY(provider.Get{{Optional}}Id({{in}}, &{{out}}));
    {%- elif type.category == "structure" -%}
        //* Do not memcpy or we may serialize padding bytes which can leak information across a
        //* trusted boundary.
        {%- set Provider = ", provider" if type.may_have_dawn_object else "" -%}
        WIRE_TRY({{as_cppType(type.name)}}Serialize(FromAPI({{in}}), &{{out}}, buffer{{Provider}}));
    {%- elif not is_wire_serializable(type) -%}
        if ({{in}} != nullptr) return WireResult::FatalError;
    {%- else -%}
        WIRE_TRY(TryAssign({{out}}, {{in}}));
    {%- endif -%}
{%- endmacro -%}

//* Outputs the deserialization code to put `in` in `out`
{%- macro deserialize_member(type, optional, in, out) -%}
    {%- if type.category == "object" -%}
        {%- set Optional = "Optional" if optional else "" -%}
        WIRE_TRY(resolver.Get{{Optional}}FromId({{in}}, &{{out}}));
    {%- elif type.category == "structure" %}
        {% if type.is_wire_transparent %}
            static_assert(sizeof({{out}}) == sizeof({{in}}), "Deserialize memcpy size must match.");
                memcpy(&{{out}}, const_cast<const {{member_transfer_type(type)}}*>(&{{in}}), {{member_transfer_sizeof(type)}});
        {%- else %}
            WIRE_TRY({{as_cppType(type.name)}}Deserialize(FromAPI(&{{out}}), &{{in}}, deserializeBuffer, allocator
                {%- if type.may_have_dawn_object -%}
                    , resolver
                {%- endif -%}
            ));
        {%- endif -%}
    {%- elif type.category == 'callback info' %}
        {{out}} = WGPU_{{type.name.SNAKE_CASE()}}_INIT;
    {%- elif not is_wire_serializable(type) %}
        {{out}} = nullptr;
    {%- else -%}
        WIRE_TRY(TryAssign({{out}}, {{in}}));
    {%- endif -%}
{%- endmacro -%}

//* The main [de]serialization macro
//* Methods are very similar to structures that have one member corresponding to each arguments.
//* This macro takes advantage of the similarity to output [de]serialization code for a record
//* that is either a structure or a method, with some special cases for each.
{%- macro write_record_serialization_helpers(record, name, members, is_cmd=False, is_return_command=False) -%}
    {%- set Return = "Return" if is_return_command else "" -%}
    {%- set Cmd = "Cmd" if is_cmd else "" -%}
    {%- set RecordName = Return + name + Cmd -%}
    {%- set Inherits = " : CmdHeader" if is_cmd else "" %}
    {%- set TransferStructName = Return + name + "Transfer" -%}

    //* Structure for the wire format of each of the records. Members that are values
    //* are embedded directly in the structure. Other members are assumed to be in the
    //* memory directly following the structure in the buffer.
    struct {{TransferStructName}}{{Inherits}} {
        static_assert({{[is_cmd, record.extensible, record.chained].count(True)}} <= 1,
                      "Record must be at most one of is_cmd, extensible, and chained.");
        {% if record.extensible %}
            bool hasNextInChain;
        {% elif record.chained %}
            ChainedStructTransfer chain;
        {% endif %}

        {% for member in members %}
            {% if not is_wire_serializable(member.type) %}
                {% continue %}
            {% endif %}
            //* Value types are directly in the command, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {{member_transfer_type(member.type)}} {{as_varName(member.name)}};
                {% continue %}
            {% endif %}
            //* Optional members additionally come with a boolean to indicate whether they were set.
            {% if member.optional and member.type.category != "object" %}
                WGPUBool has_{{as_varName(member.name)}};
            {% endif %}
        {% endfor %}

        {{TransferStructName}}() = default;
        {{TransferStructName}}(const {{TransferStructName}}&) = default;
        {{TransferStructName}}({{TransferStructName}}&&) = default;

        //* Volatile constructors and assignment operators are never expected to be called at
        //* runtime when handling wire commands. They exist solely to satisfy C++20 iterator and
        //* std::span requirements (e.g. std::indirectly_readable) when constructing views over
        //* volatile shared memory buffers.
        [[noreturn]] {{TransferStructName}}(const volatile {{TransferStructName}}& other) {
            DAWN_UNREACHABLE();
        }
        [[noreturn]] {{TransferStructName}}(volatile {{TransferStructName}}&& other) {
            DAWN_UNREACHABLE();
        }
        [[noreturn]] {{TransferStructName}}(const volatile {{TransferStructName}}&& other) {
            DAWN_UNREACHABLE();
        }
        [[noreturn]] {{TransferStructName}}& operator=(const volatile {{TransferStructName}}& other) {
            DAWN_UNREACHABLE();
        }
    };

    {% if is_cmd %}
        static_assert(offsetof({{TransferStructName}}, commandSize) == 0);
    {% endif -%}

    {% if record.chained %}
        static_assert(offsetof({{TransferStructName}}, chain) == 0);
    {% endif %}

    //* Returns the required transfer size for `record` in addition to the transfer structure.
    [[maybe_unused]] size_t {{Return}}{{name}}GetExtraRequiredSize([[maybe_unused]] const {{RecordName}}& record) {
        size_t result = 0;

        //* Gather how much space will be needed for the extension chain.
        {% if record.extensible %}
            {% set ChainedType = "ChainedStructOut" if record.output else "ChainedStruct" %}
            const {{ChainedType}}* next = record.nextInChain;
            while (next != nullptr) {
                switch (next->sType) {
                    {% for extension in record.extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CppType = as_cppType(extension.name) %}
                        case wgpu::SType::{{extension.name.CamelCase()}}: {
                            const auto& typedStruct = *reinterpret_cast<{{CppType}} const *>(next);
                            result += WireAlignSizeof<{{CppType}}Transfer>();
                            result += {{CppType}}GetExtraRequiredSize(typedStruct);
                            break;
                        }
                    {% endfor %}
                    default: {
                        result += WireAlignSizeof<DawnInjectedInvalidSTypeTransfer>();
                        break;
                    }
                }
                next = next->nextInChain;
            }
        {% endif %}
        //* Gather space needed for pointer members.
        {% for member in members %}
            {%- set memberName = as_varName(member.name) -%}
            //* Skip size computation if we are skipping serialization.
            {% if member.skip_serialize %}
                // [Skipped serialization for {{ memberName }}, it needs to be filled with a CommandExtension by the caller.]
                {% continue %}
            {% endif %}
            //* Normal handling for pointer members and structs.
            {% if member.annotation != "value" %}
                {% if member.type.category != "object" and member.optional %}
                    {% if member.length and member.length != "constant" %}
                        if (!record.{{as_varName(member.name)}}.empty())
                    {% elif member.length == "constant" and member.constant_length != 1 %}
                        if (record.{{as_varName(member.name)}}.data() != nullptr)
                    {% else %}
                        if (record.{{as_varName(member.name)}} != nullptr)
                    {% endif %}
                {% endif %}
                {
                    {% do assert(member.annotation != "const*const*", "const*const* not valid here") %}
                    auto memberLength = {{member_length(member, "record.")}};
                    auto size = WireAlignSizeofN<{{member_transfer_type(member.type)}}>(checked_cast<size_t>(memberLength));
                    DAWN_ASSERT(size);
                    result += *size;
                    //* Structures might contain more pointers so we need to add their extra size as well.
                    {% if member.type.category == "structure" %}
                        for (decltype(memberLength) i = 0; i < memberLength; ++i) {
                            {% do assert(member.annotation == "const*" or member.annotation == "*", "unhandled annotation: " + member.annotation)%}
                            result += {{as_cppType(member.type.name)}}GetExtraRequiredSize(FromAPI(record.{{as_varName(member.name)}}[i]));
                        }
                    {% endif %}
                }
            {% elif member.type.category == "structure" %}
                result += {{as_cppType(member.type.name)}}GetExtraRequiredSize(FromAPI(record.{{as_varName(member.name)}}));
            {% endif %}
        {% endfor %}
        return result;
    }
    // GetExtraRequiredSize isn't used for structures that are value members of other structures
    // because we assume they cannot contain pointers themselves.
    DAWN_UNUSED_FUNC({{Return}}{{name}}GetExtraRequiredSize);

    //* Serializes `record` into `transfer`, using `buffer` to get more space for pointed-to data
    //* and `provider` to serialize objects.
    [[maybe_unused]] WireResult {{Return}}{{name}}Serialize(
        const {{RecordName}}& record,
        volatile {{TransferStructName}}* transfer,
        [[maybe_unused]] SerializeBuffer* buffer
        {%- if record.may_have_dawn_object -%}
            , const ObjectIdProvider& provider
        {%- endif -%}
    ) {
        //* Handle special transfer members of methods.
        {% if is_cmd %}
            transfer->commandId = WireCmd::{{Return}}{{name}};
        {% endif %}

        {% if record.extensible %}
            {% set ChainedType = "ChainedStructOut" if record.output else "ChainedStruct" %}
            const {{ChainedType}}* next = record.nextInChain;
            transfer->hasNextInChain = false;
            while (next != nullptr) {
                transfer->hasNextInChain = true;
                switch (next->sType) {
                    {% for extension in record.extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CppType = as_cppType(extension.name) %}
                        case wgpu::SType::{{extension.name.CamelCase()}}: {
                            volatile {{CppType}}Transfer* chainTransfer;
                            WIRE_TRY(buffer->Next(&chainTransfer));
                            chainTransfer->chain.sType = next->sType;
                            chainTransfer->chain.hasNext = next->nextInChain != nullptr;

                            WIRE_TRY({{CppType}}Serialize(*reinterpret_cast<{{CppType}} const*>(next), chainTransfer, buffer, provider));
                            break;
                        }
                    {% endfor %}
                    default: {
                        // Invalid enum. Serialize just the invalid sType for validation purposes.
                        ::dawn::WarningLog() << "Unknown sType " << static_cast<uint32_t>(next->sType) << " discarded.";

                        volatile DawnInjectedInvalidSTypeTransfer* chainTransfer;
                        WIRE_TRY(buffer->Next(&chainTransfer));
                        chainTransfer->chain.sType = wgpu::SType::DawnInjectedInvalidSType;
                        chainTransfer->chain.hasNext = next->nextInChain != nullptr;
                        chainTransfer->invalidSType = next->sType;
                        break;
                    }
                }
                next = next->nextInChain;
            }
        {% endif %}
        {% if record.chained %}
            //* Should be set by the root descriptor's call to SerializeChainedStruct.
            DAWN_ASSERT(transfer->chain.sType == wgpu::SType::{{as_cppEnum(record.name)}});
            DAWN_ASSERT(transfer->chain.hasNext == (record.nextInChain != nullptr));
        {% endif %}

        //* Iterate members, sorted in reverse on "attribute" so that "value" types are serialized first.
        //* Note this is important because some array pointer members rely on another "value" for their
        //* "length", but order is not always given.
        {% for member in members | sort(reverse=true, attribute="annotation") %}
            {% set memberName = as_varName(member.name) %}
            //* Skip serialization for callback infos.
            {% if member.type.category == 'callback info' %}
                {% continue %}
            {% endif %}
            //* Value types are directly in the transfer record, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {% if member.is_length %}
                    //* Skipped serializing length {{ memberName }} from record as it will be serialized when we handle the member.
                    {% continue %}
                {% endif %}
                {{serialize_member(member.type, member.optional, "record." + memberName, "transfer->" + memberName)}}
                {% continue %}
            {% endif %}
            //* Allocate space and write the non-value arguments in it.
            {% do assert(member.annotation != "const*const*") %}
            {% if member.type.category != "object" and member.optional %}
                {% if member.length and member.length != "constant" %}
                    bool has_{{memberName}} = !record.{{memberName}}.empty();
                {% elif member.length == "constant" and member.constant_length != 1 %}
                    bool has_{{memberName}} = record.{{memberName}}.data() != nullptr;
                {% else %}
                    bool has_{{memberName}} = record.{{memberName}} != nullptr;
                {% endif %}
                transfer->has_{{memberName}} = has_{{memberName}};
                if (has_{{memberName}}) {
            {% else %}
                {
            {% endif %}
                auto memberLength = {{member_length(member, "record.")}};
                {% if member.length != "constant" %}
                    {{serialize_member(member.length.type, false, "memberLength", "transfer->" + as_varName(member.length.name))}}
                {% endif %}
                {% if member.skip_serialize %}
                    }
                    {% continue %}
                {% endif %}
                Span<volatile {{member_transfer_type(member.type)}}> memberBuffer;
                if (!std::in_range<size_t>(memberLength)) {
                    return WireResult::FatalError;
                }
                WIRE_TRY(buffer->NextN(checked_cast<size_t>(memberLength), &memberBuffer));

                {% if member.constant_length == 1 %}
                    {% if member.type.is_wire_transparent %}
                        SpanAsWritableBytes(memberBuffer).CopyFrom(ByteSpanFromRef(*record.{{memberName}}));
                    {% else %}
                        {{serialize_member(member.type, member.array_element_optional, "*record." + memberName, "memberBuffer[0]")}}
                    {% endif %}
                {% else %}
                    {% if member.type.is_wire_transparent %}
                        if (memberLength != 0) {
                            SpanAsWritableBytes(memberBuffer).CopyFrom(SpanAsBytes(record.{{memberName}}));
                        }
                    {% else %}
                        for (auto [i, member] : Enumerate(memberBuffer)) {
                            {{serialize_member(member.type, member.array_element_optional, "record." + memberName + "[i]", "member")}}
                        }
                    {% endif %}
                {% endif %}
            }
        {% endfor %}

        return WireResult::Success;
    }
    DAWN_UNUSED_FUNC({{Return}}{{name}}Serialize);

    //* Deserializes `transfer` into `record` getting more serialized data from `buffer` and `size`
    //* if needed, using `allocator` to store pointed-to values and `resolver` to translate object
    //* Ids to actual objects.
    [[maybe_unused]] WireResult {{Return}}{{name}}Deserialize(
        {{RecordName}}* record,
        const volatile {{TransferStructName}}* transfer,
        DeserializeBuffer* deserializeBuffer,
        [[maybe_unused]] DeserializeAllocator* allocator
        {%- if record.may_have_dawn_object -%}
            , const ObjectIdResolver& resolver
        {%- endif -%}) {
        {% if is_cmd %}
            DAWN_ASSERT(transfer->commandId == WireCmd::{{Return}}{{name}});
        {% endif %}
        {% if record.derived_method %}
            record->selfId = transfer->self;
        {% endif %}

        {% if record.extensible %}
            {% set ChainedType = "ChainedStructOut" if record.output else "ChainedStruct const" %}
            {{ChainedType}}** outChainNext = &record->nextInChain;
            bool hasNext = transfer->hasNextInChain;
            while (hasNext) {
                const volatile ChainedStructTransfer* header;
                WIRE_TRY(deserializeBuffer->Peek(&header));
                wgpu::SType sType = header->sType;
                hasNext = header->hasNext;

                switch (sType) {
                    //* All extensible types need to be able to handle deserializing the invalid
                    //* sType struct.
                    {% set extensions = record.extensions if record.output else record.extensions + [types['dawn injected invalid s type']] %}
                    {% for extension in extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CppType = as_cppType(extension.name) %}
                        case wgpu::SType::{{as_cppEnum(extension.name)}}: {
                            const volatile {{CppType}}Transfer* chainTransfer;
                            WIRE_TRY(deserializeBuffer->Read(&chainTransfer));

                            {{CppType}}* typedOutStruct;
                            WIRE_TRY(GetSpace(allocator, &typedOutStruct));
                            typedOutStruct->sType = sType;
                            typedOutStruct->nextInChain = nullptr;
                            WIRE_TRY({{CppType}}Deserialize(typedOutStruct, chainTransfer,
                                                          deserializeBuffer, allocator, resolver));
                            *outChainNext = typedOutStruct;
                            outChainNext = &typedOutStruct->nextInChain;
                            break;
                        }
                    {% endfor %}
                    default: {
                        //* For invalid sTypes, it's a fatal error since this implies a compromised
                        //* or corrupt client.
                        return WireResult::FatalError;
                    }
                }
            }
            *outChainNext = nullptr;
        {% endif %}
        {% if record.chained %}
            //* Should be set by the root descriptor's call to DeserializeChainedStruct.
            //* Don't check |record->nextInChain| matches because it is not set until the
            //* next iteration inside DeserializeChainedStruct.
            DAWN_ASSERT(record->sType == wgpu::SType::{{record.name.CamelCase()}});
            DAWN_ASSERT(record->nextInChain == nullptr);
        {% endif %}

        //* Iterate members, sorted in reverse on "attribute" so that "value" types are serialized first.
        //* Note this is important because some array pointer members rely on another "value" for their
        //* "length", but order is not always given.
        {% for member in members | sort(reverse=true, attribute="annotation") %}
            {% set memberName = as_varName(member.name) %}
            //* Value types are directly in the transfer record, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {% if member.is_length %}
                    //* Skipped deserializing length {{ memberName }} as it is included in the span.
                    {% continue %}
                {% endif %}
                {{deserialize_member(member.type, member.optional, "transfer->" + memberName, "record->" + memberName)}}
                {% continue %}
            {% endif %}
            //* Get extra buffer data, and copy pointed to values in extra allocated space. Note that
            //* currently, there is an implicit restriction that "skip_serialize" members must not be be
            //* a "value" type, and that they are the last non-"value" type specified in the list in
            //* dawn_wire.json.
            {% do assert(member.annotation != "const*const*") %}
            {% if member.type.category != "object" and member.optional %}
                //* Non-constant length optional members use length=0 to denote they aren't present.
                //* Otherwise we could have length=N and has_member=false, causing reads from an
                //* uninitialized pointer.
                {% do assert(member.length == "constant") %}
                bool has_{{memberName}} = transfer->has_{{memberName}};
                record->{{memberName}} = {};
                if (has_{{memberName}}) {
            {% else %}
                {
            {% endif %}
                auto memberLength = {{member_length(member, "transfer->")}};
                if (!std::in_range<size_t>(memberLength)) {
                    return WireResult::FatalError;
                }
                Span<const volatile {{member_transfer_type(member.type)}}> memberBuffer;
                WIRE_TRY(deserializeBuffer->ReadN(checked_cast<size_t>(memberLength), &memberBuffer));

                {% if member.constant_length == 1 %}
                    {% if is_wire_data_only(member) %}
                        record->{{memberName}} =
                            const_cast<const {{member_transfer_type(member.type)}}*>(memberBuffer.data());
                    {% else %}
                        {{as_dawnType(member.type)}}* copiedMember;
                        WIRE_TRY(GetSpace(allocator, &copiedMember));
                        record->{{memberName}} = copiedMember;

                        {% if member.type.is_wire_transparent %}
                            if (!memberBuffer.empty()) {
                                ByteSpanFromRef(*copiedMember).CopyFrom(SpanAsBytes(memberBuffer));
                            }
                        {% else %}
                            {{deserialize_member(member.type, member.array_element_optional, "memberBuffer[0]", "*copiedMember")}}
                        {% endif %}
                    {% endif %}
                {% else %}
                    //* For data-only members (e.g. "data" in WriteBuffer and WriteTexture), they are
                    //* not security sensitive so we can directly refer the data inside the transfer
                    //* buffer in dawn_native. For other members, as prevention of TOCTOU attacks is an
                    //* important feature of the wire, we must make sure every single value returned to
                    //* dawn_native must be a copy of what's in the wire.
                    {% if is_wire_data_only(member) %}
                        {% do assert(member.annotation == "const*") %}
                        record->{{memberName}} = memberBuffer;
                    {% else %}
                        {% if member.length == "constant" %}
                            Span<{{as_dawnType(member.type)}}, {{member.constant_length}}> copiedMembers;
                            WIRE_TRY(GetSpace(allocator, &copiedMembers));
                        {% else %}
                            Span<{{as_dawnType(member.type)}}> copiedMembers;
                            WIRE_TRY(GetSpace(allocator, memberBuffer.size(), &copiedMembers));
                        {% endif %}
                        record->{{memberName}} = copiedMembers;

                        {% if member.type.is_wire_transparent %}
                            if (!memberBuffer.empty()) {
                                SpanAsWritableBytes(copiedMembers).CopyFrom(SpanAsBytes(memberBuffer));
                            }
                        {% else %}
                            for (auto [i, member] : Enumerate(memberBuffer)) {
                                {{deserialize_member(member.type, member.array_element_optional, "member", "copiedMembers[i]" )}}
                            }
                        {% endif %}
                    {% endif %}
                {% endif %}
            }
        {% endfor %}

        return WireResult::Success;
    }
    DAWN_UNUSED_FUNC({{Return}}{{name}}Deserialize);
{%- endmacro -%}

{%- macro write_command_serialization_methods(command, is_return) -%}
    {% set Return = "Return" if is_return else "" %}
    {% set Name = Return + command.name.CamelCase() %}
    {% set Cmd = Name + "Cmd" %}
    {% set TransferStructName = Name + "Transfer" %}
    size_t {{Cmd}}::GetRequiredSize() const {
        return WireAlignSizeof<{{TransferStructName}}>() + {{Name}}GetExtraRequiredSize(*this);
    }

    {% if command.may_have_dawn_object %}
        WireResult {{Cmd}}::Serialize(
            size_t commandSize,
            SerializeBuffer* serializeBuffer,
            const ObjectIdProvider& provider) const {
            volatile {{TransferStructName}}* transfer;
            WIRE_TRY(serializeBuffer->Next(&transfer));
            transfer->commandSize = commandSize;
            return ({{Name}}Serialize(*this, transfer, serializeBuffer, provider));
        }
        WireResult {{Cmd}}::Serialize(size_t commandSize, SerializeBuffer* serializeBuffer) const {
            ErrorObjectIdProvider provider;
            return Serialize(commandSize, serializeBuffer, provider);
        }

        WireResult {{Cmd}}::Deserialize(
            DeserializeBuffer* deserializeBuffer,
            DeserializeAllocator* allocator,
            const ObjectIdResolver& resolver) {
            const volatile {{TransferStructName}}* transfer;
            WIRE_TRY(deserializeBuffer->Read(&transfer));
            return {{Name}}Deserialize(this, transfer, deserializeBuffer, allocator, resolver);
        }
        WireResult {{Cmd}}::Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator) {
            ErrorObjectIdResolver resolver;
            return Deserialize(deserializeBuffer, allocator, resolver);
        }
    {% else %}
        WireResult {{Cmd}}::Serialize(size_t commandSize, SerializeBuffer* serializeBuffer) const {
            volatile {{TransferStructName}}* transfer;
            WIRE_TRY(serializeBuffer->Next(&transfer));
            transfer->commandSize = commandSize;
            return ({{Name}}Serialize(*this, transfer, serializeBuffer));
        }
        WireResult {{Cmd}}::Serialize(
            size_t commandSize,
            SerializeBuffer* serializeBuffer,
            const ObjectIdProvider&) const {
            return Serialize(commandSize, serializeBuffer);
        }

        WireResult {{Cmd}}::Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator) {
            const volatile {{TransferStructName}}* transfer;
            WIRE_TRY(deserializeBuffer->Read(&transfer));
            return {{Name}}Deserialize(this, transfer, deserializeBuffer, allocator);
        }
        WireResult {{Cmd}}::Deserialize(
            DeserializeBuffer* deserializeBuffer,
            DeserializeAllocator* allocator,
            const ObjectIdResolver&) {
            return Deserialize(deserializeBuffer, allocator);
        }
    {% endif %}
{% endmacro %}

namespace dawn::wire {
namespace {

// Allocates enough space from allocator to countain T[count] and return it in out.
// Return FatalError if the allocator couldn't allocate the memory.
// Always writes to |out| on success.
template <typename T>
WireResult GetSpace(DeserializeAllocator* allocator, size_t count, Span<T>* out) {
    // Because we use this function extensively when `count` == 1, we can optimize the
    // size computations a bit more for those cases via constexpr version of the
    // alignment computation.
    constexpr size_t kSizeofT = WireAlignSizeof<T>();
    size_t size = 0;
    if (count == 1) {
      size = kSizeofT;
    } else {
      auto sizeN = WireAlignSizeofN<T>(count);
      // A size of 0 indicates an overflow, so return an error.
      if (!sizeN) {
        return WireResult::FatalError;
      }
      size = *sizeN;
    }

    auto span = allocator->TryGetSpace(size);
    if (!span) {
        return WireResult::FatalError;
    }

    // SAFETY: Size and alignment is checked above.
    *out = DAWN_UNSAFE_BUFFERS(Span<T>(reinterpret_cast<T*>(span->data()), count));
    return WireResult::Success;
}

template <typename T, size_t N>
WireResult GetSpace(DeserializeAllocator* allocator, Span<T, N>* out) {
    Span<T> dynamicSpan;
    WIRE_TRY(GetSpace(allocator, N, &dynamicSpan));
    *out = dynamicSpan;
    return WireResult::Success;
}

template <typename T>
WireResult GetSpace(DeserializeAllocator* allocator, T** out) {
    Span<T> span;
    WIRE_TRY(GetSpace(allocator, 1, &span));
    *out = span.data();
    return WireResult::Success;
}

template <typename Dst, typename Src>
constexpr WireResult TryAssign(Dst& dst, const Src& src) {
    using CleanDst = std::remove_cvref_t<Dst>;
    using CleanSrc = std::remove_cvref_t<Src>;
    if constexpr (std::is_same_v<CleanDst, CleanSrc>) {
        dst = src;
        return WireResult::Success;
    } else if constexpr (std::integral<CleanDst> && std::integral<CleanSrc>) {
        if (!std::in_range<CleanDst>(src)) {
            return WireResult::FatalError;
        }
        dst = dawn::checked_cast<CleanDst>(src);
        return WireResult::Success;
    } else if constexpr ((std::is_same_v<CleanDst, bool> && std::is_same_v<CleanSrc, wgpu::Bool>) ||
                         (std::is_same_v<CleanDst, wgpu::Bool> && std::is_same_v<CleanSrc, bool>) ||
                         (std::is_same_v<CleanDst, WGPUOptionalBool> &&
                          std::is_same_v<CleanSrc, wgpu::OptionalBool>) ||
                         (std::is_same_v<CleanDst, wgpu::OptionalBool> &&
                          std::is_same_v<CleanSrc, WGPUOptionalBool>)) {
        // A static_cast is necessary here because:
        // 1. `wgpu::Bool` is a wrapper struct around an integral value while the wire transfer
        //    type uses native `bool`.
        // 2. `wgpu::OptionalBool` is a scoped enum class while the wire transfer type uses the C
        //    enum `WGPUOptionalBool`.
        dst = static_cast<CleanDst>(src);
        return WireResult::Success;
    } else {
        return WireResult::FatalError;
    }
}

struct ChainedStructTransfer {
    wgpu::SType sType;
    bool hasNext;

    ChainedStructTransfer() = default;
    ChainedStructTransfer(const ChainedStructTransfer&) = default;
    ChainedStructTransfer(ChainedStructTransfer&&) = default;

    //* Volatile constructors and assignment operators are never expected to be called at
    //* runtime when handling wire commands. They exist solely to satisfy C++20 iterator and
    //* std::span requirements (e.g. std::indirectly_readable) when constructing views over
    //* volatile shared memory buffers.
    [[noreturn]] ChainedStructTransfer(const volatile ChainedStructTransfer& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] ChainedStructTransfer(volatile ChainedStructTransfer&& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] ChainedStructTransfer(const volatile ChainedStructTransfer&& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] ChainedStructTransfer& operator=(const volatile ChainedStructTransfer& other) {
        DAWN_UNREACHABLE();
    }
};

//* Structs that need special handling for [de]serialization code generation.
{% set SpecialSerializeStructs = ["string view", "dawn injected invalid s type", "dawn WGSL blocklist"] %}

// Manually define serialization and deserialization for StringView because
// it has a special encoding where:
//  { .data = nullptr, .length = WGPU_STRLEN }  --> nil
//  { .data = non-null, .length = WGPU_STRLEN } --> null-terminated, use strlen
//  { .data = ..., .length = 0 }             --> ""
//  { .data = ..., .length > 0 }             --> string of size `length`
struct StringViewTransfer {
    bool has_data;
    uint64_t length;

    StringViewTransfer() = default;
    StringViewTransfer(const StringViewTransfer&) = default;
    StringViewTransfer(StringViewTransfer&&) = default;

    //* Volatile constructors and assignment operators are never expected to be called at
    //* runtime when handling wire commands. They exist solely to satisfy C++20 iterator and
    //* std::span requirements (e.g. std::indirectly_readable) when constructing views over
    //* volatile shared memory buffers.
    [[noreturn]] StringViewTransfer(const volatile StringViewTransfer& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] StringViewTransfer(volatile StringViewTransfer&& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] StringViewTransfer(const volatile StringViewTransfer&& other) {
        DAWN_UNREACHABLE();
    }
    [[noreturn]] StringViewTransfer& operator=(const volatile StringViewTransfer& other) {
        DAWN_UNREACHABLE();
    }
};

size_t StringViewGetExtraRequiredSize(const StringView& record) {
    size_t size = record.length;
    if (size == WGPU_STRLEN) {
        // This is a null-terminated string, or it's nil.
        size = record.data ? std::strlen(record.data) : 0;
    }
    return Align(size, kWireBufferAlignment);
}

WireResult StringViewSerialize(
    const StringView& record,
    volatile StringViewTransfer* transfer,
    SerializeBuffer* buffer) {

    bool has_data = record.data != nullptr;
    uint64_t length = record.length;
    transfer->has_data = has_data;

    if (!has_data) {
        // The StringView is either empty or nil. Wire needs to be bitness-independent, so we use
        // UINT64_MAX as the sentinel (instead of WGPU_STRLEN, which is SIZE_MAX).
        if (length == 0) {
            transfer->length = 0;
        } else if (length == WGPU_STRLEN) {
            transfer->length = UINT64_MAX;
        } else {
            DAWN_ASSERT(false);
        }
        return WireResult::Success;
    }
    if (length == WGPU_STRLEN) {
        length = std::strlen(record.data);
    }
    if (length > 0) {
        Span<volatile char> memberBuffer;
        if (!std::in_range<size_t>(length)) {
            return WireResult::FatalError;
        }
        WIRE_TRY(buffer->NextN(checked_cast<size_t>(length), &memberBuffer));
        // TODO(https://crbug.com/524406299): Use Span::CopyFrom.
        // TODO(https://crbug.com/528027992): Spanify the record members.
        std::ranges::copy(record.data, record.data + length, memberBuffer.begin());
    }
    transfer->length = length;
    return WireResult::Success;
}

WireResult StringViewDeserialize(
    StringView* record,
    const volatile StringViewTransfer* transfer,
    DeserializeBuffer* deserializeBuffer,
    DeserializeAllocator* allocator) {

    bool has_data = transfer->has_data;
    uint64_t length = transfer->length;

    if (!has_data) {
        record->data = nullptr;
        // The StringView is either empty or nil. Wire needs to be bitness-independent, so we use
        // UINT64_MAX as the sentinel (instead of WGPU_STRLEN, which is SIZE_MAX).
        if (length == 0) {
            record->length = 0;
            return WireResult::Success;
        } else if (length == UINT64_MAX) {
            record->length = WGPU_STRLEN;
            return WireResult::Success;
        } else {
            // Invalid - string with size but no data.
            return WireResult::FatalError;
        }
    }
    if (length == 0) {
        record->data = "";
        record->length = 0;
        return WireResult::Success;
    }

    if (length > WGPU_STRLEN) {
        return WireResult::FatalError;
    }
    size_t stringLength = static_cast<size_t>(length);
    Span<const volatile char> stringInBuffer;
    WIRE_TRY(deserializeBuffer->ReadN(stringLength, &stringInBuffer));

    Span<char> copiedString;
    WIRE_TRY(GetSpace(allocator, stringLength, &copiedString));
    // TODO(https://crbug.com/524406299): Use Span::CopyFrom.
    std::ranges::copy(stringInBuffer, copiedString.begin());

    record->data = copiedString.data();
    record->length = stringLength;
    return WireResult::Success;
}

//* Force generation of de[serialization] methods for DawnInjectedInvalidSType early.
{% set type = types["dawn injected invalid s type"] %}
{%- set name = as_cppType(type.name) -%}
{{write_record_serialization_helpers(type, name, type.members, is_cmd=False)}}

//* Output structure [de]serialization first because it is used by commands.
{% for type in by_category["structure"] %}
    {%- set name = as_cppType(type.name) -%}
    {% if type.name.CamelCase() not in client_side_structures and type.name.get() not in SpecialSerializeStructs -%}
        {{write_record_serialization_helpers(type, name, type.members, is_cmd=False)}}
    {% endif %}
{% endfor %}

//* Generate the list of sTypes that we need to handle.
{% set sTypes = [] %}
{% for sType in types["s type"].values %}
    {% if not sType.valid %}
        {% continue %}
    {% elif sType.name.CamelCase() in client_side_structures %}
        {% continue %}
    {% endif %}
    {% do sTypes.append(sType) %}
{% endfor %}

//* Output [de]serialization helpers for special commands
{% for command in cmd_records["special command"] %}
    {%- set name = command.name.CamelCase() -%}
    {{write_record_serialization_helpers(command, name, command.members, is_cmd=True)}}
{% endfor %}

//* Output [de]serialization helpers for commands
{% for command in cmd_records["command"] %}
    {%- set name = command.name.CamelCase() -%}
    {{write_record_serialization_helpers(command, name, command.members, is_cmd=True)}}
{% endfor %}

//* Output [de]serialization helpers for return commands
{% for command in cmd_records["return command"] %}
    {%- set name = command.name.CamelCase() -%}
    {{write_record_serialization_helpers(command, name, command.members,
                                         is_cmd=True, is_return_command=True)}}
{% endfor %}

// Implementation of ObjectIdResolver that always errors.
// Used when the generator adds a provider argument because of a chained
// struct, but in practice, a chained struct in that location is invalid.
class ErrorObjectIdResolver final : public ObjectIdResolver {
    public:
    {% for type in by_category["object"] %}
      WireResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const override {
          return WireResult::FatalError;
      }
      WireResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const override {
          return WireResult::FatalError;
      }
    {% endfor %}
};

// Implementation of ObjectIdProvider that always errors.
// Used when the generator adds a provider argument because of a chained
// struct, but in practice, a chained struct in that location is invalid.
class ErrorObjectIdProvider final : public ObjectIdProvider {
    public:
    {% for type in by_category["object"] %}
      WireResult GetId({{as_cType(type.name)}} object, volatile ObjectId* out) const override {
          return WireResult::FatalError;
      }
      WireResult GetOptionalId({{as_cType(type.name)}} object, volatile ObjectId* out) const override {
          return WireResult::FatalError;
      }
    {% endfor %}
};

}  // anonymous namespace

{% for command in cmd_records["special command"] -%}
    {{write_command_serialization_methods(command, False)}}
{% endfor %}

{% for command in cmd_records["command"] -%}
    {{write_command_serialization_methods(command, False)}}
{% endfor %}

{% for command in cmd_records["return command"] -%}
    {{write_command_serialization_methods(command, True)}}
{% endfor %}

}  // namespace dawn::wire
