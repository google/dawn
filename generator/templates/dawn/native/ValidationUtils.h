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

#ifndef BACKEND_VALIDATIONUTILS_H_
#define BACKEND_VALIDATIONUTILS_H_

{% set api = metadata.api.lower() %}
#include "dawn/{{api}}_cpp.h"

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/Error.h"

namespace {{native_namespace}} {

    // Helper functions to check the value of enums and bitmasks
    {% for type in by_category["enum"] + by_category["bitmask"] %}
        {% set namespace = metadata.namespace %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value);
    {% endfor %}

} // namespace {{native_namespace}}

#endif  // BACKEND_VALIDATIONUTILS_H_
