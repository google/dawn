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

#ifndef BACKEND_VALIDATIONUTILS_H_
#define BACKEND_VALIDATIONUTILS_H_

#include "nxt/nxtcpp.h"

namespace backend {

    // Helper functions to check the value of enums and bitmasks
    {% for type in by_category["enum"] + by_category["bitmask"] %}
        bool IsValid{{type.name.CamelCase()}}(nxt::{{as_cppType(type.name)}} value);
    {% endfor %}

} // namespace backend

#endif  // BACKEND_VALIDATIONUTILS_H_
