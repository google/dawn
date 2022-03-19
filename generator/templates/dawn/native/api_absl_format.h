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

{% set API = metadata.api.upper() %}
#ifndef {{API}}_ABSL_FORMAT_H_
#define {{API}}_ABSL_FORMAT_H_

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include "{{native_dir}}/{{prefix}}_platform.h"

#include "absl/strings/str_format.h"

namespace {{native_namespace}} {

    //
    // Descriptors
    //

    // Only includes structures that have a 'label' member.
    {% for type in by_category["structure"] %}
        {% for member in type.members %}
            {% if member.name.canonical_case() == "label" %}
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                    AbslFormatConvert(const {{as_cppType(type.name)}}* value,
                                      const absl::FormatConversionSpec& spec,
                                      absl::FormatSink* s);
            {% endif %}
        {% endfor %}
    {% endfor %}

    //
    // Compatible with absl::StrFormat (Needs to be disjoint from having a 'label' for now.)
    // Currently uses a hard-coded list to determine which structures are actually supported. If
    // additional structures are added, be sure to update the cpp file's list as well.
    //
    {% for type in by_category["structure"] %}
        {% if type.name.get() in [
             "buffer binding layout",
             "sampler binding layout",
             "texture binding layout",
             "storage texture binding layout"
           ]
        %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
            AbslFormatConvert(const {{as_cppType(type.name)}}& value,
                              const absl::FormatConversionSpec& spec,
                              absl::FormatSink* s);
        {% endif %}
    {% endfor %}

} // namespace {{native_namespace}}

{% set namespace = metadata.namespace %}
namespace {{namespace}} {

    //
    // Enums
    //

    {% for type in by_category["enum"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                          const absl::FormatConversionSpec& spec,
                          absl::FormatSink* s);
    {% endfor %}

    //
    // Bitmasks
    //

    {% for type in by_category["bitmask"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                          const absl::FormatConversionSpec& spec,
                          absl::FormatSink* s);
    {% endfor %}

}  // namespace {{namespace}}

#endif // {{API}}_ABSL_FORMAT_H_
