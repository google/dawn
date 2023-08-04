//* Copyright 2022 The Dawn Authors
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

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include "{{native_dir}}/CacheKey.h"
#include "{{native_dir}}/{{prefix}}_platform.h"

#include <cstring>

namespace {{native_namespace}} {

//*
//* Streaming readers for wgpu structures.
//*
{% macro render_reader(member) %}
    {%- set name = member.name.camelCase() -%}
    DAWN_TRY(StreamOut(source, &t->{{name}}));
{% endmacro %}

//*
//* Streaming writers for wgpu structures.
//*
{% macro render_writer(member) %}
    {%- set name = member.name.camelCase() -%}
    {% if member.length == None %}
        StreamIn(sink, t.{{name}});
    {% elif member.length == "strlen" %}
        StreamIn(sink, Iterable(t.{{name}}, strlen(t.{{name}})));
    {% else %}
        StreamIn(sink, Iterable(t.{{name}}, t.{{member.length.name.camelCase()}}));
    {% endif %}
{% endmacro %}

{# Helper macro to render readers and writers. Should be used in a call block to provide additional custom
   handling when necessary. The optional `omit` field can be used to omit fields that are either
   handled in the custom code, or unnecessary in the serialized output.
   Example:
       {% call render_streaming_impl("struct name", writer=true, reader=false, omits=["omit field"]) %}
           // Custom C++ code to handle special types/members that are hard to generate code for
       {% endcall %}
   One day we should probably make the generator smart enough to generate everything it can
   instead of manually adding streaming implementations here.
#}
{% macro render_streaming_impl(json_type, writer, reader, omits=[]) %}
    {%- set cpp_type = types[json_type].name.CamelCase() -%}
    {% if reader %}
        template <>
        MaybeError stream::Stream<{{cpp_type}}>::Read(stream::Source* source, {{cpp_type}}* t) {
        {{ caller() }}
        {% for member in types[json_type].members %}
            {% if not member.name.get() in omits %}
                    {{render_reader(member)}}
            {% endif %}
        {% endfor %}
            return {};
        }
    {% endif %}
    {% if writer %}
        template <>
        void stream::Stream<{{cpp_type}}>::Write(stream::Sink* sink, const {{cpp_type}}& t) {
        {{ caller() }}
        {% for member in types[json_type].members %}
            {% if not member.name.get() in omits %}
                    {{render_writer(member)}}
            {% endif %}
        {% endfor %}
        }
    {% endif %}
{% endmacro %}

// Custom stream operator for special bool type.
{% set BoolCppType = metadata.namespace + "::" + as_cppType(types["bool"].name) %}
template <>
void stream::Stream<{{BoolCppType}}>::Write(stream::Sink* sink, const {{BoolCppType}}& t) {
    StreamIn(sink, static_cast<bool>(t));
}

{% call render_streaming_impl("adapter properties", true, false) %}
{% endcall %}

{% call render_streaming_impl("dawn cache device descriptor", true, false) %}
{% endcall %}

{% call render_streaming_impl("extent 3D", true, true) %}
{% endcall %}

} // namespace {{native_namespace}}
