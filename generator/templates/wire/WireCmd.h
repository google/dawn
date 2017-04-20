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

#ifndef WIRE_WIRECMD_AUTOGEN_H_
#define WIRE_WIRECMD_AUTOGEN_H_

#include <nxt/nxt.h>

namespace nxt {
namespace wire {

    //* Enum used as a prefix to each command on the wire format.
    enum class WireCmd : uint32_t {
        {% for type in by_category["object"] %}
            {% for method in type.methods %}
                {{as_MethodSuffix(type.name, method.name)}},
            {% endfor %}
            {{as_MethodSuffix(type.name, Name("destroy"))}},
        {% endfor %}
    };

    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}

            //* Structure for the wire format of each of the commands. Parameters passed by value
            //* are embedded directly in the structure. Other parameters are assumed to be in the
            //* memory directly following the structure in the buffer. With value parameters the
            //* structure can compute how much buffer size it needs and where the start of non-value
            //* parameters is in the buffer.
            struct {{Suffix}}Cmd {

                //* Start the structure with the command ID, so that casting to WireCmd gives the ID.
                wire::WireCmd commandId = wire::WireCmd::{{Suffix}};

                uint32_t self;

                //* Commands creating objects say which ID the created object will be referred as.
                {% if method.return_type.category == "object" %}
                    uint32_t resultId;
                {% endif %}

                //* Value types are directly in the command, objects being replaced with their IDs.
                {% for arg in method.arguments if arg.annotation == "value" %}
                    {% if arg.type.category == "object" %}
                        uint32_t {{as_varName(arg.name)}};
                    {% else %}
                        {{as_cType(arg.type.name)}} {{as_varName(arg.name)}};
                    {% endif %}
                {% endfor %}

                //* const char* have their length embedded directly in the command.
                {% for arg in method.arguments if arg.length == "strlen" %}
                    size_t {{as_varName(arg.name)}}Strlen;
                {% endfor %}

                //* The following commands do computation, provided the members for value parameters
                //* have been initialized.

                //* Compute how much buffer memory is required to hold the structure and all its arguments.
                size_t GetRequiredSize() const;

                //* Gets the pointer to the start of the buffer containing a non-value parameter.
                {% for get_arg in method.arguments if get_arg.annotation != "value" %}
                    {% set ArgName = as_varName(get_arg.name) %}
                    uint8_t* GetPtr_{{ArgName}}();
                    const uint8_t* GetPtr_{{ArgName}}() const;
                {% endfor %}
            };
        {% endfor %}

        //* The command structure used when sending that an ID is destroyed.
        {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
        struct {{Suffix}}Cmd {
            WireCmd commandId = WireCmd::{{Suffix}};
            uint32_t objectId;

            size_t GetRequiredSize() const;
        };
    {% endfor %}

    //* Enum used as a prefix to each command on the return wire format.
    enum class ReturnWireCmd : uint32_t {
        DeviceErrorCallback,
    };

}
}

#endif // WIRE_WIRECMD_AUTOGEN_H_
