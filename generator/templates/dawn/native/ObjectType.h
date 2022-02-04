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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_OBJECTTPYE_AUTOGEN_H_
#define {{DIR}}_OBJECTTPYE_AUTOGEN_H_

#include "dawn/common/ityp_array.h"

#include <cstdint>

{% set native_namespace = namespace_name.namespace_case() %}
namespace {{native_namespace}} {

    enum class ObjectType : uint32_t {
        {% for type in by_category["object"] %}
            {{type.name.CamelCase()}},
        {% endfor %}
    };

    template <typename T>
    using PerObjectType = ityp::array<ObjectType, T, {{len(by_category["object"])}}>;

    const char* ObjectTypeAsString(ObjectType type);

} // namespace {{native_namespace}}


#endif  // {{DIR}}_OBJECTTPYE_AUTOGEN_H_
