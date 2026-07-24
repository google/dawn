//* Copyright 2026 The Dawn & Tint Authors
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

//* TODO(https://crbug.com/526537254): It might be possible to merge this with the
//* api_structs.h in native once we finish migrating both native and wire.

{% from 'dawn/cpp_macros.tmpl' import wgpu_string_members with context %}

{% set namespace = metadata.namespace %}
#ifndef DAWNWIRE_CLIENT_{{namespace.upper()}}_STRUCTS_AUTOGEN_H_
#define DAWNWIRE_CLIENT_{{namespace.upper()}}_STRUCTS_AUTOGEN_H_

#include "absl/strings/string_view.h"
{% set api = metadata.api.lower() %}
{% set CAPI = metadata.c_prefix %}
#include "dawn/{{api}}_cpp.h"
#include "src/utils/span.h"

#include <cmath>
#include <optional>
#include <string_view>

namespace dawn::wire::client {

{% macro render_cpp_default_value(member, forced_default_value="") -%}
    //* Apply default values over undefined when dealing with the Dawn
    //* native header to avoid needing to do the additional step of
    //* resolving trivial defaults internally.
    {%- if forced_default_value -%}
        {{" "}}= {{forced_default_value}}
    {%- elif member.annotation in ["*", "const*", "const*const*"] or member.default_value == "nullptr" -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "object" and member.optional -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "callback info" -%}
        {{" "}}= {{CAPI}}_{{member.name.SNAKE_CASE()}}_INIT
    {%- elif member.type.category == "enum" -%}
        {%- if member.default_value != None -%}
            {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
        {%- elif member.type.hasUndefined -%}
            {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name("undefined"))}}
        {%- else -%}
            {{" "}}= {}
        {%- endif -%}
    {%- elif member.type.category == "bitmask" -%}
        {%- if member.default_value != None -%}
            {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
        {%- else -%}
            //* Bitmask types should currently always default to "none" if not
            //* explicitly set.
            {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name("none"))}}
        {%- endif -%}
    {%- elif member.type.category == "native" and member.default_value != None -%}
        //* Check to see if the default value is a known constant.
        {%- set constant = find_by_name(by_category["constant"], member.default_value) -%}
        {%- if constant -%}
            {{" "}}= {{namespace}}::k{{constant.name.CamelCase()}}
        {%- else -%}
            {{" "}}= {{member.default_value}}
        {%- endif -%}
    {%- elif member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- else -%}
        {{assert(member.default_value == None)}}
    {%- endif -%}
{%- endmacro %}

{% for type in by_category["object"] %}
    {% if type.name.CamelCase() in client_special_objects %}
        class {{type.name.CamelCase()}};
    {% else %}
        struct {{type.name.CamelCase()}};
    {% endif %}
{% endfor %}

    using {{namespace}}::ChainedStruct;
    using {{namespace}}::ChainedStructOut;

    //* Special structures that are manually written.
    {% set SpecialStructures = ["string view"] %}

    struct StringView {
        char const * data = nullptr;
        size_t length = WGPU_STRLEN;

        {{wgpu_string_members("StringView")}}

        #ifndef ABSL_USES_STD_STRING_VIEW
        // NOLINTNEXTLINE(google-explicit-constructor)
        operator absl::string_view() const {
            if (this->length == wgpu::kStrlen) {
                if (IsUndefined()) {
                    return {};
                }
                return {this->data};
            }
            return {this->data, this->length};
        }
        #endif
    };

    // NOLINTBEGIN(bugprone-invalid-enum-default-initialization)

    {% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
        {% set CppType = as_cppType(type.name) %}
        {% set spanify = true %}
        {% if type.chained %}
            {% set chainedStructType = "ChainedStructOut" if type.chained == "out" else "ChainedStruct" %}
            struct {{CppType}} : {{chainedStructType}} {
                {{CppType}}() {
                    sType = {{namespace}}::SType::{{type.name.CamelCase()}};
                }
        {% else %}
            struct {{CppType}} {
                {% if type.has_free_members_function %}
                    {{CppType}}() = default;
                {% endif %}
        {% endif %}
            {% if type.has_free_members_function %}
                ~{{CppType}}();
                {{CppType}}(const {{CppType}}&) = delete;
                {{CppType}}& operator=(const {{CppType}}&) = delete;
                {{CppType}}({{CppType}}&&);
                {{CppType}}& operator=({{CppType}}&&);

            {% endif %}
            {% if type.extensible %}
                {% set chainedStructType = "ChainedStructOut" if type.output else "ChainedStruct const" %}
                {{chainedStructType}} * nextInChain = nullptr;
            {% endif %}
            {% for member in type.members %}
                //* Align the first member after ChainedStruct to match the C struct layout.
                //* It has to be aligned both to its natural and ChainedStruct's alignment.
                {% if type.chained and loop.first %}
                    alignas({{namespace}}::{{CppType}}::kFirstMemberAlignment)
                {% endif %}

                {% if type.name.get() == "bind group layout entry" %}
                    {% if member.name.canonical_case() == "buffer" %}
                        {% set forced_default_value = "{ nullptr, wgpu::BufferBindingType::BindingNotUsed, false, 0 }" %}
                    {% elif member.name.canonical_case() == "sampler" %}
                        {% set forced_default_value = "{ nullptr, wgpu::SamplerBindingType::BindingNotUsed }" %}
                    {% elif member.name.canonical_case() == "texture" %}
                        {% set forced_default_value = "{ nullptr, wgpu::TextureSampleType::BindingNotUsed, wgpu::TextureViewDimension::e2D, false }" %}
                    {% elif member.name.canonical_case() == "storage texture" %}
                        {% set forced_default_value = "{ nullptr, wgpu::StorageTextureAccess::BindingNotUsed, wgpu::TextureFormat::Undefined, wgpu::TextureViewDimension::e2D }" %}
                    {% endif %}
                {% endif %}

                {% if spanify and member.is_length %}
                    //* Skip as it's included in the span just below.
                {% elif spanify and member.length and member.length != "constant" %}
                    // TODO(https://crbug.com/524405497): Support fixed-length spans.
                    {% if member.type.name.canonical_case() == "void" %}
                        {% set element_type = "const std::byte" %}
                    {% else %}
                        {% set element_type = "std::remove_pointer_t<" + decorate(as_wire_clientType(member.type), member) + ">" %}
                    {% endif %}
                    {% set index_type = member.length.type.name.canonical_case() %}
                    ityp::span<{{index_type}}, {{element_type}}> {{as_varName(member.name)}};
                {% else %}
                    {{as_annotated_wire_clientType(member)}} {{render_cpp_default_value(member, forced_default_value)}};
                {% endif %}
            {% endfor %}

            {% if type.has_free_members_function %}
              private:
                inline void FreeMembers();
            {% endif %}
        };

    {% endfor %}
    // NOLINTEND(bugprone-invalid-enum-default-initialization)

    {% for typeDef in by_category["typedef"] if typeDef.type.category == "structure" %}
        using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        // {{as_cppType(type.name)}}
        void APIFreeMembers({{as_cType(type.name)}});
    {% endfor %}

} // namespace dawn::wire::client

#endif  // DAWNWIRE_CLIENT_{{namespace.upper()}}_STRUCTS_AUTOGEN_H_
