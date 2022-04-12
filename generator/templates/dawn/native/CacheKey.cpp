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

//
// Cache key serializers for wgpu structures used in caching.
//
{% macro render_serializer(member) %}
    {%- set name = member.name.camelCase() -%}
    {% if member.length == None %}
        key->Record(t.{{name}});
    {% elif member.length == "strlen" %}
        key->RecordIterable(t.{{name}}, strlen(t.{{name}}));
    {% else %}
        key->RecordIterable(t.{{name}}, t.{{member.length.name.camelCase()}});
    {% endif %}
{% endmacro %}

{# Helper macro to render serializers. Should be used in a call block to provide additional custom
   handling when necessary. The optional `omit` field can be used to omit fields that are either
   handled in the custom code, or unnecessary in the serialized output.
   Example:
       {% call render_cache_key_serializer("struct name", omits=["omit field"]) %}
           // Custom C++ code to handle special types/members that are hard to generate code for
       {% endcall %}
#}
{% macro render_cache_key_serializer(json_type, omits=[]) %}
    {%- set cpp_type = types[json_type].name.CamelCase() -%}
    template <>
    void CacheKeySerializer<{{cpp_type}}>::Serialize(CacheKey* key, const {{cpp_type}}& t) {
    {{ caller() }}
    {% for member in types[json_type].members %}
        {%- if not member.name.get() in omits %}
                {{render_serializer(member)}}
        {%- endif %}
    {% endfor %}
    }
{% endmacro %}

{% call render_cache_key_serializer("adapter properties") %}
{% endcall %}

{% call render_cache_key_serializer("dawn cache device descriptor") %}
{% endcall %}

} // namespace {{native_namespace}}
