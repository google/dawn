//* Copyright 2021 The Dawn Authors
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
{% set native_namespace = Name(metadata.native_namespace).snake_case() %}
{% set native_dir = impl_dir + native_namespace %}
{% set api = metadata.api.lower() %}
#include "{{native_dir}}/{{api}}_absl_format_autogen.h"

#include "{{native_dir}}/ObjectType_autogen.h"

namespace {{native_namespace}} {

    //
    // Descriptors
    //

    {% for type in by_category["structure"] %}
        {% for member in type.members %}
            {% if member.name.canonical_case() == "label" %}
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                AbslFormatConvert(const {{as_cppType(type.name)}}* value,
                                    const absl::FormatConversionSpec& spec,
                                    absl::FormatSink* s) {
                    if (value == nullptr) {
                        s->Append("[null]");
                        return {true};
                    }
                    s->Append("[{{as_cppType(type.name)}}");
                    if (value->label != nullptr) {
                        s->Append(absl::StrFormat(" \"%s\"", value->label));
                    }
                    s->Append("]");
                    return {true};
                }
            {% endif %}
        {% endfor %}
    {% endfor %}

}  // namespace {{native_namespace}}

{% set namespace = metadata.namespace %}
namespace {{namespace}} {

    //
    // Enums
    //

    {% for type in by_category["enum"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                            const absl::FormatConversionSpec& spec,
                            absl::FormatSink* s) {
            s->Append("{{as_cppType(type.name)}}::");
            switch (value) {
            {% for value in type.values %}
                case {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}:
                s->Append("{{as_cppEnum(value.name)}}");
                break;
            {% endfor %}
                default:
                s->Append(absl::StrFormat("%x", static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value)));
            }
            return {true};
        }
    {% endfor %}

    //
    // Bitmasks
    //

    {% for type in by_category["bitmask"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                            const absl::FormatConversionSpec& spec,
                            absl::FormatSink* s) {
            s->Append("{{as_cppType(type.name)}}::");
            if (!static_cast<bool>(value)) {
                {% for value in type.values if value.value == 0 %}
                    // 0 is often explicitly declared as None.
                    s->Append("{{as_cppEnum(value.name)}}");
                {% else %}
                    s->Append(absl::StrFormat("{{as_cppType(type.name)}}::%x", 0));
                {% endfor %}
                return {true};
            }

            bool moreThanOneBit = !HasZeroOrOneBits(value);
            if (moreThanOneBit) {
                s->Append("(");
            }

            bool first = true;
            {% for value in type.values if value.value != 0 %}
                if (value & {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}) {
                    if (!first) {
                        s->Append("|");
                    }
                    first = false;
                    s->Append("{{as_cppEnum(value.name)}}");
                    value &= ~{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}};
                }
            {% endfor %}

            if (static_cast<bool>(value)) {
                if (!first) {
                    s->Append("|");
                }
                s->Append(absl::StrFormat("{{as_cppType(type.name)}}::%x", static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value)));
            }

            if (moreThanOneBit) {
                s->Append(")");
            }

            return {true};
        }
    {% endfor %}

}  // namespace {{namespace}}
