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

#include "wire/WireCmd_autogen.h"

namespace nxt {
namespace wire {

    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}

            size_t {{Suffix}}Cmd::GetRequiredSize() const {
                size_t result = sizeof(*this);

                {% for arg in method.arguments if arg.annotation != "value" %}
                    {% if arg.length == "strlen" %}
                        result += {{as_varName(arg.name)}}Strlen + 1;
                    {% elif arg.type.category == "object" %}
                        result += {{as_varName(arg.length.name)}} * sizeof(uint32_t);
                    {% else %}
                        result += {{as_varName(arg.length.name)}} * sizeof({{as_cType(arg.type.name)}});
                    {% endif %}
                {% endfor %}

                return result;
            }

            {% for const in ["", "const"] %}
                {% for get_arg in method.arguments if get_arg.annotation != "value" %}

                    {{const}} uint8_t* {{Suffix}}Cmd::GetPtr_{{as_varName(get_arg.name)}}() {{const}} {
                        //* Start counting after the current structure
                        {{const}} uint8_t* ptr = reinterpret_cast<{{const}} uint8_t*>(this + 1);

                        //* Increment the pointer until we find the 'arg' then return early.
                        //* This will mean some of the code will be unreachable but there is no
                        //* "break" in Jinja2.
                        {% for arg in method.arguments if arg.annotation != "value" %}
                            {% if get_arg == arg %}
                                return ptr;
                            {% endif %}
                            {% if arg.length == "strlen" %}
                                ptr += {{as_varName(arg.name)}}Strlen + 1;
                            {% elif arg.type.category == "object" %}
                                ptr += {{as_varName(arg.length.name)}} * sizeof(uint32_t);
                            {% else %}
                                ptr += {{as_varName(arg.length.name)}} * sizeof({{as_cType(arg.type.name)}});
                            {% endif %}
                        {% endfor %}
                    }

                {% endfor %}
            {% endfor %}
        {% endfor %}

        {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
        size_t {{Suffix}}Cmd::GetRequiredSize() const {
            return sizeof(*this);
        }
    {% endfor %}

}
}
