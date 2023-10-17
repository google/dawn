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

    using {{namespace}}::ChainedStruct;
    using {{namespace}}::ChainedStructOut;

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
            {% if type.has_free_members_function %}
                {{as_cppType(type.name)}}() = default;
                ~{{as_cppType(type.name)}}();
                {{as_cppType(type.name)}}(const {{as_cppType(type.name)}}&) = delete;
                {{as_cppType(type.name)}}& operator=(const {{as_cppType(type.name)}}&) = delete;
                {{as_cppType(type.name)}}({{as_cppType(type.name)}}&&);
                {{as_cppType(type.name)}}& operator=({{as_cppType(type.name)}}&&);

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

    {% for type in by_category["structure"] if type.has_free_members_function %}
        // {{as_cppType(type.name)}}
        void API{{as_MethodSuffix(type.name, Name("free members"))}}({{as_cType(type.name)}});
    {% endfor %}

} // namespace {{native_namespace}}

#endif  // {{DIR}}_{{namespace.upper()}}_STRUCTS_H_
