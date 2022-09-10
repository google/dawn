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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
{% set namespace = metadata.namespace %}
#ifndef {{DIR}}_{{namespace.upper()}}_STRUCTS_H_
#define {{DIR}}_{{namespace.upper()}}_STRUCTS_H_

{% set api = metadata.api.lower() %}
#include "dawn/{{api}}_cpp.h"
{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/Forward.h"
#include <cmath>

namespace {{native_namespace}} {

{% macro render_cpp_default_value(member) -%}
    {%- if member.annotation in ["*", "const*"] and member.optional or member.default_value == "nullptr" -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "object" and member.optional -%}
        {{" "}}= nullptr
    {%- elif member.type.category in ["enum", "bitmask"] and member.default_value != None -%}
        {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
    {%- elif member.type.category == "native" and member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- elif member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- else -%}
        {{assert(member.default_value == None)}}
    {%- endif -%}
{%- endmacro %}

    struct ChainedStruct {
        ChainedStruct const * nextInChain = nullptr;
        {{namespace}}::SType sType = {{namespace}}::SType::Invalid;
    };

    struct ChainedStructOut {
        ChainedStructOut * nextInChain = nullptr;
        {{namespace}}::SType sType = {{namespace}}::SType::Invalid;
    };

    {% for type in by_category["structure"] %}
        {% if type.chained %}
            {% set chainedStructType = "ChainedStructOut" if type.chained == "out" else "ChainedStruct" %}
            struct {{as_cppType(type.name)}} : {{chainedStructType}} {
                {{as_cppType(type.name)}}() {
                    sType = {{namespace}}::SType::{{type.name.CamelCase()}};
                }
        {% else %}
            struct {{as_cppType(type.name)}} {
        {% endif %}
            {% if type.extensible %}
                {% set chainedStructType = "ChainedStructOut" if type.output else "ChainedStruct const" %}
                {{chainedStructType}} * nextInChain = nullptr;
            {% endif %}
            {% for member in type.members %}
                {% set member_declaration = as_annotated_frontendType(member) + render_cpp_default_value(member) %}
                {% if type.chained and loop.first %}
                    //* Align the first member after ChainedStruct to match the C struct layout.
                    //* It has to be aligned both to its natural and ChainedStruct's alignment.
                    alignas({{namespace}}::{{as_cppType(type.name)}}::kFirstMemberAlignment) {{member_declaration}};
                {% else %}
                    {{member_declaration}};
                {% endif %}
            {% endfor %}

            // Equality operators, mostly for testing. Note that this tests
            // strict pointer-pointer equality if the struct contains member pointers.
            bool operator==(const {{as_cppType(type.name)}}& rhs) const;
        };

    {% endfor %}

    {% for typeDef in by_category["typedef"] if typeDef.type.category == "structure" %}
        using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};
    {% endfor %}

} // namespace {{native_namespace}}

#endif  // {{DIR}}_{{namespace.upper()}}_STRUCTS_H_
