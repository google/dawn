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

#include "dawncpp.h"

namespace dawn {

    {% for type in by_category["enum"] + by_category["bitmask"] %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% for value in type.values %}
            static_assert(static_cast<uint32_t>({{CppType}}::{{as_cppEnum(value.name)}}) == {{as_cEnum(type.name, value.name)}}, "value mismatch for {{CppType}}::{{as_cppEnum(value.name)}}");
        {% endfor %}

    {% endfor %}


    {% for type in by_category["structure"] %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% if type.extensible %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                    "offsetof mismatch for {{CppType}}::nextInChain");
        {% endif %}
        {% for member in type.members %}
            {% set memberName = member.name.camelCase() %}
            static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                    "offsetof mismatch for {{CppType}}::{{memberName}}");
        {% endfor %}

    {% endfor %}

    {% for type in by_category["object"] %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% macro render_cpp_method_declaration(type, method) %}
            {% set CppType = as_cppType(type.name) %}
            {% if method.return_type.name.concatcase() == "void" and type.is_builder -%}
                {{CppType}} const&
            {%- else -%}
                {{as_cppType(method.return_type.name)}}
            {%- endif -%}
            {{" "}}{{CppType}}::{{method.name.CamelCase()}}(
                {%- for arg in method.arguments -%}
                    {%- if not loop.first %}, {% endif -%}
                    {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                        {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}
                    {%- else -%}
                        {{as_annotated_cppType(arg)}}
                    {%- endif -%}
                {%- endfor -%}
            ) const
        {%- endmacro %}

        {% macro render_cpp_to_c_method_call(type, method) -%}
            {{as_cMethod(type.name, method.name)}}(Get()
                {%- for arg in method.arguments -%},{{" "}}
                    {%- if arg.annotation == "value" -%}
                        {%- if arg.type.category == "object" -%}
                            {{as_varName(arg.name)}}.Get()
                        {%- elif arg.type.category == "enum" or arg.type.category == "bitmask" -%}
                            static_cast<{{as_cType(arg.type.name)}}>({{as_varName(arg.name)}})
                        {%- elif arg.type.category in ["native", "natively defined"] -%}
                            {{as_varName(arg.name)}}
                        {%- else -%}
                            UNHANDLED
                        {%- endif -%}
                    {%- else -%}
                        reinterpret_cast<{{decorate("", as_cType(arg.type.name), arg)}}>({{as_varName(arg.name)}})
                    {%- endif -%}
                {%- endfor -%}
            )
        {%- endmacro %}

        {% for method in native_methods(type) %}
            {{render_cpp_method_declaration(type, method)}} {
                {% if method.return_type.name.concatcase() == "void" %}
                    {{render_cpp_to_c_method_call(type, method)}};
                    {% if type.is_builder %}
                        return *this;
                    {% endif %}
                {% else %}
                    auto result = {{render_cpp_to_c_method_call(type, method)}};
                    {% if method.return_type.category == "native" %} 
                        return result;
                    {% elif method.return_type.category == "object" %}
                        return {{as_cppType(method.return_type.name)}}::Acquire(result);
                    {% else %}
                        return static_cast<{{as_cppType(method.return_type.name)}}>(result);
                    {% endif%}
                {% endif %}
            }
        {% endfor %}
        void {{CppType}}::DawnReference({{CType}} handle) {
            if (handle != nullptr) {
                {{as_cMethod(type.name, Name("reference"))}}(handle);
            }
        }
        void {{CppType}}::DawnRelease({{CType}} handle) {
            if (handle != nullptr) {
                {{as_cMethod(type.name, Name("release"))}}(handle);
            }
        }

    {% endfor %}

}
