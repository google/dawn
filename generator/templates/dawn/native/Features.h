// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_FEATURES_AUTOGEN_H_
#define {{DIR}}_FEATURES_AUTOGEN_H_

#include "dawn/common/ityp_array.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class Feature {
  {% for enum in types["feature name"].values if enum.valid %}
    {{as_cppEnum(enum.name)}},
  {% endfor %}
  InvalidEnum,
};

template<>
struct EnumCount<Feature> {
    {% set counter = namespace(value = 0) %}
    {% for enum in types["feature name"].values if enum.valid -%}
        {% set counter.value = counter.value + 1 %}
    {% endfor %}
    static constexpr uint32_t value = {{counter.value}};
};

}  // namespace dawn::native

#endif  // {{DIR}}_FEATURES_AUTOGEN_H_
