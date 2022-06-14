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

#ifndef DAWNWIRE_WIRECMD_AUTOGEN_H_
#define DAWNWIRE_WIRECMD_AUTOGEN_H_

#include "dawn/webgpu.h"

#include "dawn/wire/BufferConsumer.h"
#include "dawn/wire/ObjectType_autogen.h"
#include "dawn/wire/ObjectHandle.h"
#include "dawn/wire/WireResult.h"

namespace dawn::wire {

    // Interface to allocate more space to deserialize pointed-to data.
    // nullptr is treated as an error.
    class DeserializeAllocator {
        public:
            virtual void* GetSpace(size_t size) = 0;
    };

    // Interface to convert an ID to a server object, if possible.
    // Methods return FatalError if the ID is for a non-existent object and Success otherwise.
    class ObjectIdResolver {
        public:
            {% for type in by_category["object"] %}
                virtual WireResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const = 0;
                virtual WireResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const = 0;
            {% endfor %}
    };

    // Interface to convert a client object to its ID for the wiring.
    class ObjectIdProvider {
        public:
            {% for type in by_category["object"] %}
                virtual WireResult GetId({{as_cType(type.name)}} object, ObjectId* out) const = 0;
                virtual WireResult GetOptionalId({{as_cType(type.name)}} object, ObjectId* out) const = 0;
            {% endfor %}
    };

    //* Enum used as a prefix to each command on the wire format.
    enum class WireCmd : uint32_t {
        {% for command in cmd_records["command"] %}
            {{command.name.CamelCase()}},
        {% endfor %}
    };

    //* Enum used as a prefix to each command on the return wire format.
    enum class ReturnWireCmd : uint32_t {
        {% for command in cmd_records["return command"] %}
            {{command.name.CamelCase()}},
        {% endfor %}
    };

    struct CmdHeader {
        uint64_t commandSize;
    };

{% macro write_command_struct(command, is_return_command) %}
    {% set Return = "Return" if is_return_command else "" %}
    {% set Cmd = command.name.CamelCase() + "Cmd" %}
    struct {{Return}}{{Cmd}} {
        //* From a filled structure, compute how much size will be used in the serialization buffer.
        size_t GetRequiredSize() const;

        //* Serialize the structure and everything it points to into serializeBuffer which must be
        //* big enough to contain all the data (as queried from GetRequiredSize).
        WireResult Serialize(size_t commandSize, SerializeBuffer* serializeBuffer, const ObjectIdProvider& objectIdProvider) const;
        // Override which produces a FatalError if any object is used.
        WireResult Serialize(size_t commandSize, SerializeBuffer* serializeBuffer) const;

        //* Deserializes the structure from a buffer, consuming a maximum of *size bytes. When this
        //* function returns, buffer and size will be updated by the number of bytes consumed to
        //* deserialize the structure. Structures containing pointers will use allocator to get
        //* scratch space to deserialize the pointed-to data.
        //* Deserialize returns:
        //*  - Success if everything went well (yay!)
        //*  - FatalError is something bad happened (buffer too small for example)
        WireResult Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator, const ObjectIdResolver& resolver);
        // Override which produces a FatalError if any object is used.
        WireResult Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator);

        {% if command.derived_method %}
            //* Command handlers want to know the object ID in addition to the backing object.
            //* Doesn't need to be filled before Serialize, or GetRequiredSize.
            ObjectId selfId;
        {% endif %}

        {% for member in command.members %}
            {{as_annotated_cType(member)}};
        {% endfor %}
    };
{% endmacro %}

    {% for command in cmd_records["command"] %}
        {{write_command_struct(command, False)}}
    {% endfor %}

    {% for command in cmd_records["return command"] %}
        {{write_command_struct(command, True)}}
    {% endfor %}

}  // namespace dawn::wire

#endif // DAWNWIRE_WIRECMD_AUTOGEN_H_
