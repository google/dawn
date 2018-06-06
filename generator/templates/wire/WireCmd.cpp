//* Copyright 2017 The NXT Authors
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

#include "wire/WireCmd.h"

#include <cstring>

namespace nxt { namespace wire {

    // Macro to simplify error handling, similar to NXT_TRY but for DeserializeResult.
#define DESERIALIZE_TRY(EXPR) \
    { \
        DeserializeResult exprResult = EXPR; \
        if (exprResult != DeserializeResult::Success) { \
            return exprResult; \
        } \
    }

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

    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}
            {% set Cmd = Suffix + "Cmd" %}

            //* Structure for the wire format of each of the commands. Parameters passed by value
            //* are embedded directly in the structure. Other parameters are assumed to be in the
            //* memory directly following the structure in the buffer. With value parameters the
            //* structure can compute how much buffer size it needs and where the start of non-value
            //* parameters is in the buffer.
            struct {{Cmd}}Transfer {
                //* Start the structure with the command ID, so that casting to WireCmd gives the ID.
                wire::WireCmd commandId;

                ObjectId self;

                {% if method.return_type.category == "object" %}
                    ObjectId resultId;
                    ObjectSerial resultSerial;
                {% endif %}

                //* Value types are directly in the command, objects being replaced with their IDs.
                {% for arg in method.arguments if arg.annotation == "value" %}
                    {% if arg.type.category == "object" %}
                        ObjectId {{as_varName(arg.name)}};
                    {% else %}
                        {{as_cType(arg.type.name)}} {{as_varName(arg.name)}};
                    {% endif %}
                {% endfor %}

                //* const char* have their length embedded directly in the command.
                {% for arg in method.arguments if arg.length == "strlen" %}
                    size_t {{as_varName(arg.name)}}Strlen;
                {% endfor %}
            };

            size_t {{Cmd}}::GetRequiredSize() const {
                size_t result = sizeof({{Cmd}}Transfer);

                {% for arg in method.arguments if arg.annotation != "value" %}
                    {% set argName = as_varName(arg.name) %}

                    {% if arg.length == "strlen" %}
                        result += std::strlen({{as_varName(arg.name)}});

                    {% elif arg.length == "constant_one" %}
                        result += sizeof({{as_cType(arg.type.name)}});

                    {% elif arg.type.category == "object" %}
                        result += {{as_varName(arg.length.name)}} * sizeof(ObjectId);

                    {% else %}
                        result += {{as_varName(arg.length.name)}} * sizeof({{as_cType(arg.type.name)}});

                    {% endif %}
                {% endfor %}

                return result;
            }

            void {{Cmd}}::Serialize(char* buffer, const ObjectIdProvider& objectIdProvider) const {
                auto transfer = reinterpret_cast<{{Cmd}}Transfer*>(buffer);
                buffer += sizeof({{Cmd}}Transfer);

                transfer->commandId = wire::WireCmd::{{Suffix}};
                transfer->self = objectIdProvider.GetId(self);

                {% if method.return_type.category == "object" %}
                    transfer->resultId = resultId;
                    transfer->resultSerial = resultSerial;
                {% endif %}

                //* Value types are directly in the command, objects being replaced with their IDs.
                {% for arg in method.arguments if arg.annotation == "value" %}
                    {% set argName = as_varName(arg.name) %}
                    {% if arg.type.category == "object" %}
                        transfer->{{argName}} = objectIdProvider.GetId(this->{{argName}});
                    {% else %}
                        transfer->{{argName}} = this->{{argName}};
                    {% endif %}
                {% endfor %}

                //* const char* have their length embedded directly in the command.
                {% for arg in method.arguments if arg.length == "strlen" %}
                    {% set argName = as_varName(arg.name) %}
                    transfer->{{argName}}Strlen = std::strlen(this->{{argName}});
                {% endfor %}

                //* In the allocated space, write the non-value arguments.
                {% for arg in method.arguments if arg.annotation != "value" %}
                    {% set argName = as_varName(arg.name) %}

                    {% if arg.length == "strlen" %}
                        memcpy(buffer, this->{{argName}}, transfer->{{argName}}Strlen);
                        buffer += transfer->{{argName}}Strlen;

                    {% elif arg.length == "constant_one" %}
                        memcpy(buffer, this->{{argName}}, sizeof(*(this->{{argName}})));
                        buffer += sizeof(*(this->{{argName}}));

                    {% elif arg.type.category == "object" %}
                        {% set argLength = as_varName(arg.length.name) %}
                        auto {{argName}}Storage = reinterpret_cast<ObjectId*>(buffer);
                        for (size_t i = 0; i < {{argLength}}; i++) {
                            {{argName}}Storage[i] = objectIdProvider.GetId(this->{{argName}}[i]);
                        }
                        buffer += sizeof(ObjectId) * {{argLength}};

                    {% else %}
                        {% set argLength = as_varName(arg.length.name) %}
                        memcpy(buffer, this->{{argName}}, {{argLength}} * sizeof(*(this->{{argName}})));
                        buffer += {{argLength}} * sizeof(*(this->{{argName}}));

                    {% endif %}
                {% endfor %}
            }

            DeserializeResult {{Cmd}}::Deserialize(const char** buffer, size_t* size, DeserializeAllocator* allocator, const ObjectIdResolver& resolver) {
                (void) allocator;

                const {{Cmd}}Transfer* transfer = nullptr;
                DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, 1, &transfer));

                selfId = transfer->self;
                {% if method.return_type.category == "object" %}
                    resultId = transfer->resultId;
                    resultSerial = transfer->resultSerial;
                {% endif %}

                DESERIALIZE_TRY(resolver.GetFromId(selfId, &self));

                {% for arg in method.arguments if arg.annotation == "value" %}
                    {% set argName = as_varName(arg.name) %}
                    {% if arg.type.category == "object" %}
                        DESERIALIZE_TRY(resolver.GetFromId(transfer->{{argName}}, &(this->{{argName}})));
                    {% else %}
                        this->{{argName}} = transfer->{{argName}};
                    {% endif %}
                {% endfor %}

                {% for arg in method.arguments if arg.annotation != "value" %}
                    {% set argName = as_varName(arg.name) %}
                    {% if arg.length == "strlen" %}
                        {
                            size_t stringLength = transfer->{{argName}}Strlen;
                            const char* stringInBuffer = nullptr;
                            DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, stringLength, &stringInBuffer));

                            char* copiedString = nullptr;
                            DESERIALIZE_TRY(GetSpace(allocator, stringLength + 1, &copiedString));
                            memcpy(copiedString, stringInBuffer, stringLength);
                            copiedString[stringLength] = '\0';
                            this->{{argName}} = copiedString;
                        }
                    {% elif arg.length == "constant_one" %}
                        {
                            const {{as_cType(arg.type.name)}}* argInBuffer = nullptr;
                            DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, 1, &argInBuffer));

                            {{as_cType(arg.type.name)}}* copiedArg = nullptr;
                            DESERIALIZE_TRY(GetSpace(allocator, 1, &copiedArg));
                            memcpy(copiedArg, argInBuffer, sizeof(*{{argName}}));
                            this->{{argName}} = copiedArg;
                        }
                    {% elif arg.type.category == "object" %}
                        {% set argLength = as_varName(arg.length.name) %}
                        {
                            const ObjectId* idsInBuffer = nullptr;
                            DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, {{argLength}}, &idsInBuffer));

                            {{as_cType(arg.type.name)}}* copiedObjects = nullptr;
                            DESERIALIZE_TRY(GetSpace(allocator, {{argLength}}, &copiedObjects));
                            for (size_t i = 0; i < {{argLength}}; i++) {
                                DESERIALIZE_TRY(resolver.GetFromId(idsInBuffer[i], &copiedObjects[i]));
                            }
                            this->{{argName}} = copiedObjects;
                        }
                    {% else %}
                        {% set argLength = as_varName(arg.length.name) %}
                        {
                            const {{as_cType(arg.type.name)}}* argInBuffer = nullptr;
                            DESERIALIZE_TRY(GetPtrFromBuffer(buffer, size, {{argLength}}, &argInBuffer));

                            {{as_cType(arg.type.name)}}* copiedArg = nullptr;
                            DESERIALIZE_TRY(GetSpace(allocator, {{argLength}}, &copiedArg))
                            memcpy(copiedArg, argInBuffer, {{argLength}} * sizeof(*{{argName}}));
                            this->{{argName}} = copiedArg;
                        }
                    {% endif %}
                {% endfor %}

                return DeserializeResult::Success;
            }
        {% endfor %}
    {% endfor %}

}}  // namespace nxt::wire
