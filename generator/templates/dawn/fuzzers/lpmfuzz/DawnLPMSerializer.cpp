// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/fuzzers/lpmfuzz/DawnLPMConstants_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMFuzzer.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMObjectStore.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializerCustom.h"
#include "dawn/webgpu.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/ObjectHandle.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireResult.h"
#include "dawn/wire/client/ApiObjects_autogen.h"

namespace dawn::wire {

//* Outputs an rvalue that's the number of elements a pointer member points to.
{% macro member_length(member, proto_accessor) -%}
    {%- if member.length == "constant" -%}
        {{member.constant_length}}u
    {%- else -%}
        {{proto_accessor}}().size()
    {%- endif -%}
{%- endmacro %}

//* Outputs the type that will be used on the wire for the member
{% macro member_type(member) -%}
    {{ assert(as_cType(member.type.name) != "size_t") }}
    {{as_cType(member.type.name)}}
{%- endmacro %}

//* Outputs the conversion code to put `in` in `out`
{% macro convert_member(member, in, out, in_access="") %}
    {% if member.type in by_category["structure"] %}
        {{ convert_structure(member, in, out, in_access) }}
    {% elif member.type in by_category["bitmask"] %}
        {{ convert_bitmask(member, in, out, in_access) }}
    {% elif member.type in by_category["enum"] %}
        {{ convert_enum(member, in, out, in_access) }}
    {% elif member.type in by_category["object"] %}
        {{ convert_object(member, in, out, in_access) }}
    {% elif member.type.name.get() == "ObjectId" %}
        {{ convert_objectid(member, in, out, access) }}
    {% elif member.type.name.get() == "ObjectHandle" %}
        {{ convert_objecthandle(member, in, out, in_access) }}
    {% else %}
        {{out}} = {{in}}({{in_access}});
    {% endif %}
{% endmacro %}

//* Helper functions for converting protobufs to specific types
{% macro convert_enum(member, in, out, in_access) %}
    {{out}} = static_cast<{{ member_type(member) }}>(
                {{in}}({{in_access}})
    );
{% endmacro %}

{% macro convert_object(member, in, out, in_access) -%}
    {{ out }} = reinterpret_cast<{{ as_cType(member.type.name)  }}>(
        objectStores[ObjectType::{{ member.type.name.CamelCase() }}].Lookup(
            static_cast<ObjectId>(
                {{in}}({{in_access}})
            )
        )
    );
{%- endmacro %}

{% macro convert_objectid(member, in, out, in_access) -%}
    {{ out }} = objectStores[ObjectType::{{ member.id_type.name.CamelCase() }}].Lookup(
        static_cast<ObjectId>(
            {{in}}({{ in_access}})
        )
    );
{%- endmacro %}

{% macro convert_objecthandle(member, in, out, in_access) -%}
    if (objectStores[ObjectType::{{ member.handle_type.name.CamelCase() }}].Size() < DawnLPMFuzzer::k{{ member.handle_type.name.CamelCase() }}Limit) {
        {{ out }} = objectStores[ObjectType::{{ member.handle_type.name.CamelCase() }}].ReserveHandle();
    } else {
        // Return failure in this case to guide the fuzzer away from generating too many
        // objects of this type
        return WireResult::FatalError;
    }
{%- endmacro %}

{% macro convert_bitmask(member, in, out, in_access) -%}
    {{ out }} = 0;
    for (size_t bm = 0; bm < static_cast<size_t>({{ in }}().size()); bm++) {
        {{ out }} |=
            static_cast<{{ member_type(member) }}>(
                {{ in }}(bm)
            );
    }
{%- endmacro %}

{% macro convert_structure(member, in, out, in_access) -%}
    // Serializing a Structure Recursively
    WIRE_TRY({{member_type(member)}}ProtoConvert({{in}}({{in_access}}), &{{out}}, serializeBuffer, objectStores));
{%- endmacro %}

{% macro write_record_conversion_helpers(record, name, members, is_cmd) %}
    {% set overrides = cmd_records["lpm_info"]["overrides"]  %}
    {% set overrides_key = record.name.canonical_case()  %}
    {% set name = record.name.CamelCase() %}
    {% set Cmd = "Cmd" if is_cmd else "" %}
    {% set WGPU = "WGPU" if not is_cmd else "" %}

    WireResult {{WGPU}}{{name}}ProtoConvert(fuzzing::{{ name }} proto_record, {{WGPU}}{{ name }}{{ Cmd }} const *record, SerializeBuffer* serializeBuffer, PerObjectType<DawnLPMObjectStore> &objectStores) {

        {{WGPU}}{{ name }}{{ Cmd }} *mutable_record = const_cast<{{WGPU}}{{ name }}{{ Cmd }} *>(record);

        //* Some commands don't set any members.
        DAWN_UNUSED(mutable_record);

        //* Clear the entire structure to make optional handling simpler.
        memset(mutable_record, 0, sizeof({{WGPU}}{{ name }}{{ Cmd }}));

        //* Pass by Value handling. This mirrors WireCmd with some differences between
        //* convert_member and serialize_member
        {% for member in members if member.annotation == "value" if not member.skip_serialize %}
            {% set memberName = as_varName(member.name) %}
            {% set protoMember = as_protobufMemberName(member.name) %}

            {% if member.optional %}
                if ( proto_record.has_{{ protoMember }}() ) {
            {% else %}
                {
            {% endif %}
                //* Major WireCmd Divergence: Some member values are hardcoded in dawn_lpm.json
                {% if overrides_key in overrides and
                   member.name.canonical_case() in overrides[overrides_key] %}
                    mutable_record->{{ memberName }} =
                        {{- overrides[overrides_key][member.name.canonical_case()] }};
                {%- elif member.type.category == "function pointer" or
                    member.type.name.get() == "void *" -%}
                    mutable_record->{{ memberName }} = nullptr;
                {% else %}
                    {{ convert_member(member, 'proto_record.' + protoMember, "mutable_record->" + memberName) }}
                {% endif %}
                }
        {% endfor %}

        //* Chained structures are currently not supported.
        {% if record.extensible %}
            mutable_record->nextInChain = nullptr;
        {% endif %}

        //* TODO(1374747): Create a string type that can be either
        //* random bytes or the fixed entrypoint name.
        {% for member in members if member.length == "strlen" %}
            {% set memberName = as_varName(member.name) %}
            {
                mutable_record->{{ memberName }} = "main";
            }
        {% endfor %}

        //* Pass by Pointer handling. This mirrors WireCmd with some divergences when handling
        //* byte arrays.
        {% for member in members if member.annotation != "value" and member.length != "strlen" and not member.skip_serialize %}
            {% set memberName = as_varName(member.name) %}
            {% set protoMember = as_protobufMemberName(member.name) %}
            {% set protoAccess = "i" if member.length != "constant" or member.constant_length > 1 else "" %}

            //* Major WireCmd Divergence: DawnLPM handles raw byte arrays uniquely
            //* as they don't lead to new coverage, lead to OOMs when allocated with
            //* an arbitrary size, and are difficult to work with in protobuf.
            {% if member.type.name.get() == 'uint8_t' %}
                {
                const size_t kDataBufferLength = 128;
                auto memberLength = kDataBufferLength;

                {{member_type(member)}}* memberBuffer;
                WIRE_TRY(serializeBuffer->NextN(memberLength, &memberBuffer));
                memset(memberBuffer, 0, kDataBufferLength);
                mutable_record->{{ memberName }} = memberBuffer;

                {% if member.length != "constant" -%}
                    mutable_record->{{ member.length.name.camelCase() }} = memberLength;
                {%- endif %}
                }
            {% else %}
                {% set is_fixed_array = true if (member.length == "constant" and member.constant_length > 1) else false %}

                {% if member.optional and not is_fixed_array %}
                    if ( proto_record.has_{{ protoMember }}() ) {
                {% else %}
                    {
                {% endif %}
                    auto memberLength = static_cast<unsigned int>({{member_length(member, "proto_record." + protoMember)}});

                    //* Needed for the edge cases in "external texture descriptor"
                    //* where we want to fuzzer to fill the fixed-length float arrays
                    //* with values, but the length of the protobuf buffer might not
                    //* be large enough for "src transfer function parameters".
                    {% if is_fixed_array %}
                        memberLength = std::min(memberLength,  static_cast<unsigned int>({{"proto_record." + protoMember}}().size()));
                    {% endif %}

                    {{member_type(member)}}* memberBuffer;
                    WIRE_TRY(serializeBuffer->NextN(memberLength, &memberBuffer));

                    for (decltype(memberLength) i = 0; i < memberLength; ++i) {
                        {{convert_member(member, "proto_record." + protoMember, "memberBuffer[i]", protoAccess )}}
                    }

                    mutable_record->{{ memberName }} = memberBuffer;

                    //* Major WireCmd Divergence: Within the serializer the length member is
                    //* set by using record.length. Here we aren't receiving any data
                    //* and set it to the number of protobuf objects in proto_record.
                    {% if member.length != "constant" -%}
                        mutable_record->{{ member.length.name.camelCase() }} = memberLength;
                    {%- endif %}
                    }
            {% endif %}
        {% endfor %}

        return WireResult::Success;
    }
{% endmacro %}

//* Output structure conversion first because it is used by commands.
{% for type in by_category["structure"] %}
    {% set name = as_cType(type.name) %}
    {% if type.name.CamelCase() not in client_side_structures %}
        {{ write_record_conversion_helpers(type, name, type.members, False) }}
    {% endif %}
{% endfor %}

//* Output command conversion functions.
{% for command in cmd_records["cpp_generated_commands"] %}
    {% set name = command.name.CamelCase() %}
    {{ write_record_conversion_helpers(command, name, command.members, True) }}
{% endfor %}

WireResult SerializedData(const fuzzing::Program& program, dawn::wire::ChunkedCommandSerializer serializer) {
    DawnLPMObjectIdProvider provider;
    PerObjectType<DawnLPMObjectStore> objectStores;

    // Allocate a scoped buffer allocation
    const size_t kMaxSerializeBufferSize = 65536;
    std::unique_ptr<char[]> allocatedBuffer(
        new char[kMaxSerializeBufferSize]()
    );

    for (const fuzzing::Command& command : program.commands()) {
        switch (command.command_case()) {

            {% for command in cmd_records["cpp_generated_commands"] %}
            {% set name = command.name.CamelCase() %}
            case fuzzing::Command::k{{name}}: {
                SerializeBuffer serializeBuffer(allocatedBuffer.get(), kMaxSerializeBufferSize);
                {{ name }}Cmd *cmd = nullptr;
                WIRE_TRY(serializeBuffer.Next(&cmd));

                WIRE_TRY({{name}}ProtoConvert(command.{{ command.name.concatcase() }}(), cmd, &serializeBuffer, objectStores));

                serializer.SerializeCommand(*cmd, provider);
                break;
            }
            {% endfor %}
            default: {
                GetCustomSerializedData(command, serializer, objectStores, provider);
                break;
            }
        }
    }

    return WireResult::Success;
}

} // namespace dawn::wire
