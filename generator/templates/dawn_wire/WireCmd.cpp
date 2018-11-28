//* Copyright 2017 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

#include "dawn_wire/WireCmd.h"

#include "common/Assert.h"

#include <cstring>

//* Helper macros so that the main [de]serialization functions can be written in a generic manner.

//* Outputs an rvalue that's the number of elements a pointer member points to.
{% macro member_length(member, record_accessor) -%}
    {%- if member.length == "constant" -%}
        {{member.constant_length}}
    {%- else -%}
        {{record_accessor}}{{as_varName(member.length.name)}}
    {%- endif -%}
{%- endmacro %}

//* Outputs the type that will be used on the wire for the member
{% macro member_transfer_type(member) -%}
    {%- if member.type.category == "object" -%}
        ObjectId
    {%- elif member.type.category == "structure" -%}
        {{as_cType(member.type.name)}}Transfer
    {%- else -%}
        {{as_cType(member.type.name)}}
    {%- endif -%}
{%- endmacro %}

//* Outputs the size of one element of the type that will be used on the wire for the member
{% macro member_transfer_sizeof(member) -%}
    sizeof({{member_transfer_type(member)}})
{%- endmacro %}

//* Outputs the serialization code to put `in` in `out`
{% macro serialize_member(member, in, out) %}
    {%- if member.type.category == "object" -%}
        {% set Optional = "Optional" if member.optional else "" %}
        {{out}} = provider.Get{{Optional}}Id({{in}});
    {% elif member.type.category == "structure"%}
        {{as_cType(member.type.name)}}Serialize({{in}}, &{{out}}, buffer, provider);
    {%- else -%}
        {{out}} = {{in}};
    {%- endif -%}
{% endmacro %}

//* Outputs the deserialization code to put `in` in `out`
{% macro deserialize_member(member, in, out) %}
    {%- if member.type.category == "object" -%}
        {% set Optional = "Optional" if member.optional else "" %}
        DESERIALIZE_TRY(resolver.Get{{Optional}}FromId({{in}}, &{{out}}));
    {% elif member.type.category == "structure"%}
        DESERIALIZE_TRY({{as_cType(member.type.name)}}Deserialize(&{{out}}, &{{in}}, buffer, size, allocator, resolver));
    {%- else -%}
        {{out}} = {{in}};
    {%- endif -%}
{% endmacro %}

//* The main [de]serialization macro

//* Methods are very similar to structures that have one member corresponding to each arguments.
//* This macro takes advantage of the similarity to output [de]serialization code for a record
//* that is either a structure or a method, with some special cases for each.
{% macro write_serialization_methods(name, members, as_method=None, as_struct=None, is_return_command=False) %}
    {% set Return = "Return" if is_return_command else "" %}
    {% set is_method = as_method != None %}
    {% set is_struct = as_struct != None %}

    //* Structure for the wire format of each of the records. Members that are values
    //* are embedded directly in the structure. Other members are assumed to be in the
    //* memory directly following the structure in the buffer.
    struct {{name}}Transfer {
        {% if is_method %}
            //* Start the transfer structure with the command ID, so that casting to WireCmd gives the ID.
            {{Return}}WireCmd commandId;

            //* Methods always have an implicit "self" argument.
            ObjectId self;

            //* Methods that return objects need to declare to the server which ID will be used for the
            //* return value.
            {% if as_method.return_type.category == "object" %}
                ObjectId resultId;
                ObjectSerial resultSerial;
            {% endif %}
        {% endif %}

        //* Value types are directly in the command, objects being replaced with their IDs.
        {% for member in members if member.annotation == "value" %}
            {{member_transfer_type(member)}} {{as_varName(member.name)}};
        {% endfor %}

        //* const char* have their length embedded directly in the command.
        {% for member in members if member.length == "strlen" %}
            size_t {{as_varName(member.name)}}Strlen;
        {% endfor %}
    };

    //* Returns the required transfer size for `record` in addition to the transfer structure.
    size_t {{name}}GetExtraRequiredSize(const {{name}}& record) {
        DAWN_UNUSED(record);

        size_t result = 0;

        //* Special handling of const char* that have their length embedded directly in the command
        {% for member in members if member.length == "strlen" %}
            result += std::strlen(record.{{as_varName(member.name)}});
        {% endfor %}

        //* Gather how much space will be needed for pointer members.
        {% for member in members if member.annotation != "value" and member.length != "strlen" %}
            {
                size_t memberLength = {{member_length(member, "record.")}};
                result += memberLength * {{member_transfer_sizeof(member)}};

                //* Structures might contain more pointers so we need to add their extra size as well.
                {% if member.type.category == "structure" %}
                    for (size_t i = 0; i < memberLength; ++i) {
                        result += {{as_cType(member.type.name)}}GetExtraRequiredSize(record.{{as_varName(member.name)}}[i]);
                    }
                {% endif %}
            }
        {% endfor %}

        return result;
    }
    // GetExtraRequiredSize isn't used for structures that are value members of other structures
    // because we assume they cannot contain pointers themselves.
    DAWN_UNUSED_FUNC({{name}}GetExtraRequiredSize);

    //* Serializes `record` into `transfer`, using `buffer` to get more space for pointed-to data
    //* and `provider` to serialize objects.
    void {{name}}Serialize(const {{name}}& record, {{name}}Transfer* transfer,
                           char* buffer, const ObjectIdProvider& provider) {
        DAWN_UNUSED(provider);
        DAWN_UNUSED(buffer);

        //* Handle special transfer members of methods.
        {% if is_method %}
            {% if as_method.return_type.category == "object" %}
                transfer->resultId = record.resultId;
                transfer->resultSerial = record.resultSerial;
            {% endif %}

            transfer->commandId = {{Return}}WireCmd::{{name}};
            transfer->self = provider.GetId(record.self);
        {% endif %}

        //* Value types are directly in the transfer record, objects being replaced with their IDs.
        {% for member in members if member.annotation == "value" %}
            {% set memberName = as_varName(member.name) %}
            {{serialize_member(member, "record." + memberName, "transfer->" + memberName)}}
        {% endfor %}

        //* Special handling of const char* that have their length embedded directly in the command
        {% for member in members if member.length == "strlen" %}
            {% set memberName = as_varName(member.name) %}
            transfer->{{memberName}}Strlen = std::strlen(record.{{memberName}});

            memcpy(buffer, record.{{memberName}}, transfer->{{memberName}}Strlen);
            buffer += transfer->{{memberName}}Strlen;
        {% endfor %}

        //* Allocate space and write the non-value arguments in it.
        {% for member in members if member.annotation != "value" and member.length != "strlen" %}
            {% set memberName = as_varName(member.name) %}
            {
                size_t memberLength = {{member_length(member, "record.")}};
                auto memberBuffer = reinterpret_cast<{{member_transfer_type(member)}}*>(buffer);

                buffer += memberLength * {{member_transfer_sizeof(member)}};
                for (size_t i = 0; i < memberLength; ++i) {
                    {{serialize_member(member, "record." + memberName + "[i]", "memberBuffer[i]" )}}
                }
            }
        {% endfor %}
    }

    //* Deserializes `transfer` into `record` getting more serialized data from `buffer` and `size`
    //* if needed, using `allocator` to store pointed-to values and `resolver` to translate object
    //* Ids to actual objects.
    DeserializeResult {{name}}Deserialize({{name}}* record, const {{name}}Transfer* transfer,
                                          const char** buffer, size_t* size, DeserializeAllocator* allocator, const ObjectIdResolver& resolver) {
        DAWN_UNUSED(allocator);
        DAWN_UNUSED(resolver);
        DAWN_UNUSED(buffer);
        DAWN_UNUSED(size);

        //* Handle special transfer members for methods
        {% if is_method %}
            {% if as_method.return_type.category == "object" %}
                record->resultId = transfer->resultId;
                record->resultSerial = transfer->resultSerial;
            {% endif %}

            ASSERT(transfer->commandId == {{Return}}WireCmd::{{name}});

            record->selfId = transfer->self;
            //* This conversion is done after the copying of result* and selfId: Deserialize
            //* guarantees they are filled even if there is an ID for an error object for the
            //* Maybe monad mechanism.
            DESERIALIZE_TRY(resolver.GetFromId(record->selfId, &record->self));

            //* The object resolver returns a success even if the object is null because the
            //* frontend is reponsible to validate that (null objects sometimes have special
            //* meanings). However it is never valid to call a method on a null object so we
            //* can error out in that case.
            if (record->self == nullptr) {
                return DeserializeResult::FatalError;
            }
        {% endif %}

        {% if is_struct and as_struct.extensible %}
            record->nextInChain = nullptr;
        {% endif %}

        //* Value types are directly in the transfer record, objects being replaced with their IDs.
        {% for member in members if member.annotation == "value" %}
            {% set memberName = as_varName(member.name) %}
            {{deserialize_member(member, "transfer->" + memberName, "record->" + memberName)}}
        {% endfor %}

        //* Special handling of const char* that have their length embedded directly in the command
        {% for member in members if member.length == "strlen" %}
            {% set memberName = as_varName(member.name) %}
            {
                size_t stringLength = transfer->{{memberName}}Strlen;
                const char* stringInBuffer = nullptr;
                DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, stringLength, &stringInBuffer));

                char* copiedString = nullptr;
                DESERIALIZE_TRY(GetSpace(allocator, stringLength + 1, &copiedString));
                memcpy(copiedString, stringInBuffer, stringLength);
                copiedString[stringLength] = '\0';
                record->{{memberName}} = copiedString;
            }
        {% endfor %}

        //* Get extra buffer data, and copy pointed to values in extra allocated space.
        {% for member in members if member.annotation != "value" and member.length != "strlen" %}
            {% set memberName = as_varName(member.name) %}
            {
                size_t memberLength = {{member_length(member, "record->")}};
                auto memberBuffer = reinterpret_cast<const {{member_transfer_type(member)}}*>(buffer);
                DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, memberLength, &memberBuffer));

                {{as_cType(member.type.name)}}* copiedMembers = nullptr;
                DESERIALIZE_TRY(GetSpace(allocator, memberLength, &copiedMembers));
                record->{{memberName}} = copiedMembers;

                for (size_t i = 0; i < memberLength; ++i) {
                    {{deserialize_member(member, "memberBuffer[i]", "copiedMembers[i]")}}
                }
            }
        {% endfor %}

        return DeserializeResult::Success;
    }
{% endmacro %}

namespace dawn_wire {

    // Macro to simplify error handling, similar to DAWN_TRY but for DeserializeResult.
#define DESERIALIZE_TRY(EXPR) \
    { \
        DeserializeResult exprResult = EXPR; \
        if (exprResult != DeserializeResult::Success) { \
            return exprResult; \
        } \
    }

    namespace {

        // Consumes from (buffer, size) enough memory to contain T[count] and return it in data.
        // Returns FatalError if not enough memory was available
        template <typename T>
        DeserializeResult GetPtrFromBuffer(const char** buffer, size_t* size, size_t count, const T** data) {
            // TODO(cwallez@chromium.org): For robustness we would need to handle overflows here.
            size_t totalSize = sizeof(T) * count;
            if (totalSize > *size) {
                return DeserializeResult::FatalError;
            }

            *data = reinterpret_cast<const T*>(*buffer);
            *buffer += totalSize;
            *size -= totalSize;

            return DeserializeResult::Success;
        }

        // Allocates enough space from allocator to countain T[count] and return it in out.
        // Return FatalError if the allocator couldn't allocate the memory.
        template <typename T>
        DeserializeResult GetSpace(DeserializeAllocator* allocator, size_t count, T** out) {
            // TODO(cwallez@chromium.org): For robustness we would need to handle overflows here.
            size_t totalSize = sizeof(T) * count;
            *out = static_cast<T*>(allocator->GetSpace(totalSize));
            if (*out == nullptr) {
                return DeserializeResult::FatalError;
            }

            return DeserializeResult::Success;
        }

        //* Output structure [de]serialization first because it is used by methods.
        {% for type in by_category["structure"] %}
            {% set name = as_cType(type.name) %}
            {{write_serialization_methods(name, type.members, as_struct=type)}}
        {% endfor %}

        // * Output [de]serialization helpers for methods
        {% for type in by_category["object"] %}
            {% for method in type.methods %}
                {% set name = as_MethodSuffix(type.name, method.name) %}

                using {{name}} = {{name}}Cmd;
                {{write_serialization_methods(name, method.arguments, as_method=method)}}
            {% endfor %}
        {% endfor %}
    }  // anonymous namespace

    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set name = as_MethodSuffix(type.name, method.name) %}
            {% set Cmd = name + "Cmd" %}

            size_t {{Cmd}}::GetRequiredSize() const {
                return sizeof({{name}}Transfer) + {{name}}GetExtraRequiredSize(*this);
            }

            void {{Cmd}}::Serialize(char* buffer, const ObjectIdProvider& objectIdProvider) const {
                auto transfer = reinterpret_cast<{{name}}Transfer*>(buffer);
                buffer += sizeof({{name}}Transfer);

                {{name}}Serialize(*this, transfer, buffer, objectIdProvider);
            }

            DeserializeResult {{Cmd}}::Deserialize(const char** buffer, size_t* size, DeserializeAllocator* allocator, const ObjectIdResolver& resolver) {
                const {{name}}Transfer* transfer = nullptr;
                DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, 1, &transfer));

                return {{name}}Deserialize(this, transfer, buffer, size, allocator, resolver);
            }
        {% endfor %}
    {% endfor %}

}  // namespace dawn_wire
