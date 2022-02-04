//* Copyright 2020 The Dawn Authors
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
#include "{{native_dir}}/ObjectType_autogen.h"

namespace {{native_namespace}} {

    const char* ObjectTypeAsString(ObjectType type) {
        switch (type) {
            {% for type in by_category["object"] %}
                case ObjectType::{{type.name.CamelCase()}}:
                    return "{{type.name.CamelCase()}}";
            {% endfor %}
            default:
                UNREACHABLE();
        }
    }

} // namespace {{native_namespace}}
