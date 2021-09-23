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

#ifndef WEBGPU_ABSL_FORMAT_H_
#define WEBGPU_ABSL_FORMAT_H_

#include "dawn_native/dawn_platform.h"

#include "absl/strings/str_format.h"

namespace dawn_native {

    {% set skip_types = ["instance", "surface"] %}
    {% set pure_frontend_types = ["command encoder", "compute pass encoder", "render pass encoder", "render bundle encoder"] %}

    //
    // Objects
    //

    class ObjectBase;
    absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
    AbslFormatConvert(const ObjectBase* value,
                    const absl::FormatConversionSpec& spec,
                    absl::FormatSink* s);

    {% for type in by_category["object"] %}
        {% set Base = "" if type.name.canonical_case() in pure_frontend_types else "Base" %}
        {% if type.name.canonical_case() not in skip_types %}
            class {{type.name.CamelCase()}}{{Base}};
            absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
            AbslFormatConvert(const {{type.name.CamelCase()}}{{Base}}* value,
                                const absl::FormatConversionSpec& spec,
                                absl::FormatSink* s);
        {% endif %}
    {% endfor %}

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
}

namespace wgpu {

    //
    // Enums
    //

    {% for type in by_category["enum"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                            const absl::FormatConversionSpec& spec,
                            absl::FormatSink* s);
    {% endfor %}

    //
    // Bitmasks
    //

    {% for type in by_category["bitmask"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                            const absl::FormatConversionSpec& spec,
                            absl::FormatSink* s);
    {% endfor %}

}  // namespace dawn_native

#endif // WEBGPU_ABSL_FORMAT_H_
