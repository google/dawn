//* Copyright 2018 The Dawn Authors
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
#include "{{native_dir}}/ValidationUtils_autogen.h"

namespace {{native_namespace}} {

    {% set namespace = metadata.namespace %}
    {% for type in by_category["enum"] %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value) {
            switch (value) {
                {% for value in type.values if value.valid %}
                    case {{namespace}}::{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}:
                        return {};
                {% endfor %}
                default:
                    return DAWN_VALIDATION_ERROR("Value %i is invalid for {{as_cType(type.name)}}.", static_cast<uint32_t>(value));
            }
        }

    {% endfor %}

    {% for type in by_category["bitmask"] %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value) {
            if ((value & static_cast<{{namespace}}::{{as_cppType(type.name)}}>(~{{type.full_mask}})) == 0) {
                return {};
            }
            return DAWN_VALIDATION_ERROR("Value %i is invalid for {{as_cType(type.name)}}.", static_cast<uint32_t>(value));
        }

    {% endfor %}

} // namespace {{native_namespace}}
