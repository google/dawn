//* Copyright 2018 The NXT Authors
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

#include "backend/ValidationUtils_autogen.h"

namespace backend {

    {% for type in by_category["enum"] %}
        bool IsValid{{type.name.CamelCase()}}(nxt::{{as_cppType(type.name)}} value) {
            switch (value) {
                {% for value in type.values %}
                    case nxt::{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}:
                        return true;
                {% endfor %}
                default:
                    return false;
            }
        }

    {% endfor %}

    {% for type in by_category["bitmask"] %}
        bool IsValid{{type.name.CamelCase()}}(nxt::{{as_cppType(type.name)}} value) {
            return (value & static_cast<nxt::{{as_cppType(type.name)}}>(~{{type.full_mask}})) == 0;
        }

    {% endfor %}

} // namespace backend
