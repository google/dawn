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

namespace dawn_wire {

    using ObjectId = uint32_t;
    using ObjectSerial = uint32_t;

    enum class DeserializeResult {
        Success,
        FatalError,
        ErrorObject,
    };

    // Interface to allocate more space to deserialize pointed-to data.
    // nullptr is treated as an error.
    class DeserializeAllocator {
        public:
            virtual void* GetSpace(size_t size) = 0;
    };

    // Interface to convert an ID to a server object, if possible.
    // Methods return FatalError if the ID is for a non-existent object, ErrorObject if the
    // object is an error value and Success otherwise.
    class ObjectIdResolver {
        public:
            {% for type in by_category["object"] %}
                virtual DeserializeResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const = 0;
                virtual DeserializeResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const = 0;
            {% endfor %}
    };

    // Interface to convert a client object to its ID for the wiring.
    class ObjectIdProvider {
        public:
            {% for type in by_category["object"] %}
                virtual ObjectId GetId({{as_cType(type.name)}} object) const = 0;
                virtual ObjectId GetOptionalId({{as_cType(type.name)}} object) const = 0;
            {% endfor %}
    };

    //* Enum used as a prefix to each command on the wire format.
    enum class WireCmd : uint32_t {
        {% for type in by_category["object"] %}
            {% for method in type.methods %}
                {{as_MethodSuffix(type.name, method.name)}},
            {% endfor %}
            {{as_MethodSuffix(type.name, Name("destroy"))}},
        {% endfor %}
        BufferMapAsync,
        BufferUpdateMappedDataCmd,
    };

    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}
            {% set Cmd = Suffix + "Cmd" %}

            //* These are "structure" version of the list of arguments to the different Dawn methods.
            //* They provide helpers to serialize/deserialize to/from a buffer.
            struct {{Cmd}} {
                //* From a filled structure, compute how much size will be used in the serialization buffer.
                size_t GetRequiredSize() const;

                //* Serialize the structure and everything it points to into serializeBuffer which must be
                //* big enough to contain all the data (as queried from GetRequiredSize).
                void Serialize(char* serializeBuffer, const ObjectIdProvider& objectIdProvider) const;

                //* Deserializes the structure from a buffer, consuming a maximum of *size bytes. When this
                //* function returns, buffer and size will be updated by the number of bytes consumed to
                //* deserialize the structure. Structures containing pointers will use allocator to get
                //* scratch space to deserialize the pointed-to data.
                //* Deserialize returns:
                //*  - Success if everything went well (yay!)
                //*  - FatalError is something bad happened (buffer too small for example)
                //*  - ErrorObject if one if the deserialized object is an error value, for the implementation
                //*    of the Maybe monad.
                //* If the return value is not FatalError, selfId, resultId and resultSerial (if present) are
                //* filled.
                DeserializeResult Deserialize(const char** buffer, size_t* size, DeserializeAllocator* allocator, const ObjectIdResolver& resolver);

                {{as_cType(type.name)}} self;

                //* Command handlers want to know the object ID in addition to the backing object.
                //* Doesn't need to be filled before Serialize, or GetRequiredSize.
                ObjectId selfId;

                //* Commands creating objects say which ID the created object will be referred as.
                {% if method.return_type.category == "object" %}
                    ObjectId resultId;
                    ObjectSerial resultSerial;
                {% endif %}

                {% for arg in method.arguments %}
                    {{as_annotated_cType(arg)}};
                {% endfor %}
            };
        {% endfor %}

        //* The command structure used when sending that an ID is destroyed.
        {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
        struct {{Suffix}}Cmd {
            WireCmd commandId = WireCmd::{{Suffix}};
            ObjectId objectId;
        };

    {% endfor %}

    //* Enum used as a prefix to each command on the return wire format.
    enum class ReturnWireCmd : uint32_t {
        DeviceErrorCallback,
        {% for type in by_category["object"] if type.is_builder %}
                {{type.name.CamelCase()}}ErrorCallback,
        {% endfor %}
        BufferMapReadAsyncCallback,
        BufferMapWriteAsyncCallback,
    };

    //* Command for the server calling a builder status callback.
    {% for type in by_category["object"] if type.is_builder %}
        struct Return{{type.name.CamelCase()}}ErrorCallbackCmd {
            ReturnWireCmd commandId = ReturnWireCmd::{{type.name.CamelCase()}}ErrorCallback;

            ObjectId builtObjectId;
            ObjectSerial builtObjectSerial;
            uint32_t status;
            size_t messageStrlen;
        };
    {% endfor %}

}  // namespace dawn_wire

#endif // DAWNWIRE_WIRECMD_AUTOGEN_H_
