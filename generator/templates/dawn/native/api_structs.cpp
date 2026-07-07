//* Copyright 2018 The Dawn & Tint Authors
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

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set include_dir = namespace_name.Dirs() %}
{% set namespace = metadata.namespace %}
#include "{{include_dir}}/{{namespace}}_structs_autogen.h"

#include <cstring>
#include <tuple>

#include "src/utils/assert.h"

#if defined(__GNUC__) || defined(__clang__)
// error: 'offsetof' within non-standard-layout type '{{namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

namespace {{native_namespace}} {

    {% set c_prefix = metadata.c_prefix %}
    static_assert(sizeof(ChainedStruct) == sizeof({{c_prefix}}ChainedStruct),
            "sizeof mismatch for ChainedStruct");
    static_assert(alignof(ChainedStruct) == alignof({{c_prefix}}ChainedStruct),
            "alignof mismatch for ChainedStruct");
    static_assert(offsetof(ChainedStruct, nextInChain) == offsetof({{c_prefix}}ChainedStruct, next),
            "offsetof mismatch for ChainedStruct::nextInChain");
    static_assert(offsetof(ChainedStruct, sType) == offsetof({{c_prefix}}ChainedStruct, sType),
            "offsetof mismatch for ChainedStruct::sType");

    //* Special structures that are manually written.
    {% set SpecialStructures = ["string view"] %}

    bool StringView::operator==(const StringView& rhs) const {
        return data == rhs.data && length == rhs.length;
    }

    // NOLINTBEGIN(bugprone-invalid-enum-default-initialization)

    {% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}
        {% set spanify = not CppType in structure_spanification_blocklist %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% if type.extensible %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                    "offsetof mismatch for {{CppType}}::nextInChain");
        {% endif %}
        {% if type.chained %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, chain) + offsetof(WGPUChainedStruct, next),
                    "offsetof mismatch for {{CppType}}::nextInChain");
            static_assert(offsetof({{CppType}}, sType) == offsetof({{CType}}, chain) + offsetof(WGPUChainedStruct, sType),
                    "offsetof mismatch for {{CppType}}::sType");
        {% endif %}
        {% for member in type.members %}
            {% if spanify and member.is_length %}
                //* Skip as the member is included in the span member.
            {% elif spanify and member.length and member.length != "constant" %}
                // TODO(https://crbug.com/524405497): Support fixed-length spans.
                {% set memberName = member.name.camelCase() %}
                {% set lengthName = member.length.name.camelCase() %}
                using {{CppType}}{{memberName}}Span = decltype(std::declval<{{CppType}}>().{{memberName}});
                {{ assert(member.length.type.name.canonical_case() == "size_t") }}
                static_assert(offsetof({{CppType}}, {{memberName}}) + {{CppType}}{{memberName}}Span::GetOffsetOfSize() == offsetof({{CType}}, {{lengthName}}),
                             "offsetof mismatch for {{CppType}}::{{memberName}}::mSize");
                static_assert(offsetof({{CppType}}, {{memberName}}) + {{CppType}}{{memberName}}Span::GetOffsetOfData() == offsetof({{CType}}, {{memberName}}),
                             "offsetof mismatch for {{CppType}}::{{memberName}}::mData");
            {% else %}
                {% set memberName = member.name.camelCase() %}
                static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                             "offsetof mismatch for {{CppType}}::{{memberName}}");
            {% endif %}
        {% endfor %}

        {% if type.any_member_requires_struct_defaulting %}
            {{CppType}} {{CppType}}::WithTrivialFrontendDefaults() const {
                {{CppType}} copy;
                {% if type.extensible %}
                    copy.nextInChain = nextInChain;
                {% endif %}
                {% if type.chained %}
                    copy.nextInChain = nextInChain;
                    copy.sType = sType;
                {% endif %}
                {% for member in type.members %}
                    {% set memberName = member.name.camelCase() %}
                    {% if spanify and member.is_length %}
                        //* Skip as the member is included in the span member.
                    {% elif member.requires_struct_defaulting %}
                        {% if member.type.category == "structure" %}
                            copy.{{memberName}} = {{memberName}}.WithTrivialFrontendDefaults();
                        {% elif member.type.category == "enum" %}
                            {% set Enum = namespace + "::" + as_cppType(member.type.name) %}
                            copy.{{memberName}} = ({{memberName}} == {{Enum}}::Undefined)
                                ? {{Enum}}::{{as_cppEnum(Name(member.default_value))}}
                                : {{memberName}};
                        {% else %}
                            {{assert(False, "other types do not currently support defaulting")}}
                        {% endif %}
                    {% else %}
                        copy.{{memberName}} = {{memberName}};
                    {% endif %}
                {% endfor %}
                return copy;
            }
        {% endif %}
        bool {{CppType}}::operator==(const {{CppType}}& rhs) const {
            {% if type.extensible or type.chained -%}
                if (nextInChain != rhs.nextInChain) { return false; }
            {% endif %}
            {% for member in type.members if member.type.category != 'callback info' %}
                {% if spanify and member.is_length %}
                    //* Skip as the member is included in the span member.
                {% elif spanify and member.length and member.length != "constant" %}
                    if ({{member.name.camelCase()}}.size() != rhs.{{member.name.camelCase()}}.size()) { return false; }
                    if ({{member.name.camelCase()}}.data() != rhs.{{member.name.camelCase()}}.data()) { return false; }
                {% else %}
                    if ({{member.name.camelCase()}} != rhs.{{member.name.camelCase()}}) { return false; }
                {% endif %}
            {% endfor %}
            return true;
        }

    {% endfor %}
    // NOLINTEND(bugprone-invalid-enum-default-initialization)

    {% for type in by_category["structure"] if type.has_free_members_function %}
        {% set CppType = as_cppType(type.name) %}
        {% set spanify = not CppType in structure_spanification_blocklist %}

        // {{as_cppType(type.name)}}
        {{CppType}}::~{{CppType}}() {
            FreeMembers();
        }

        {{CppType}}::{{CppType}}({{CppType}}&& rhs)
        : {% for member in type.members if (not spanify or not member.is_length)%}
            {%- set memberName = member.name.camelCase() -%}
            {{memberName}}(rhs.{{memberName}}){% if not loop.last %},{{"\n      "}}{% endif %}
        {% endfor -%}
        {
            {% for member in type.members if (not spanify or not member.is_length)%}
                rhs.{{member.name.camelCase()}} = {};
            {% endfor %}
        }

        {{CppType}}& {{CppType}}::operator=({{CppType}}&& rhs) {
            if (&rhs == this) {
                return *this;
            }
            FreeMembers();
            {% for member in type.members if (not spanify or not member.is_length)%}
                this->{{member.name.camelCase()}} = std::move(rhs.{{member.name.camelCase()}});
            {% endfor %}
            {% for member in type.members if (not spanify or not member.is_length)%}
                rhs.{{member.name.camelCase()}} = {};
            {% endfor %}
            return *this;
        }

        void {{CppType}}::FreeMembers() {
            bool needsFreeing = false;
            {%- for member in type.members if member.annotation != 'value' %}
                {% if spanify and member.length != "constant" %}
                    if (!this->{{member.name.camelCase()}}.empty()) { needsFreeing = true; }
                {% else %}
                    if (this->{{member.name.camelCase()}} != nullptr) { needsFreeing = true; }
                {% endif %}
            {%- endfor -%}
            {%- for member in type.members if member.type.name.canonical_case() == 'string view' %}
                if (this->{{member.name.camelCase()}}.data != nullptr) { needsFreeing = true; }
            {%- endfor -%}
            if (needsFreeing) {
                API{{as_MethodSuffix(type.name, Name("free members"))}}(*reinterpret_cast<{{as_cType(type.name)}}*>(this));
            }
        }

    {% endfor %}

} // namespace {{native_namespace}}
