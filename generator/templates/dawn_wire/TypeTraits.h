//* Copyright 2019 The Dawn Authors
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

#ifndef DAWNWIRE_TYPETRAITS_AUTOGEN_H_
#define DAWNWIRE_TYPETRAITS_AUTOGEN_H_

#include <dawn/dawn.h>

//* This file can be removed when WebGPU error handling is implemented
namespace dawn_wire {
    template <typename T>
    struct IsBuilderType {
        static constexpr bool value = false;
    };

    {% for type in by_category["object"] if type.is_builder %}
        template<>
        struct IsBuilderType<{{as_cType(type.name)}}> {
            static constexpr bool value = true;
        };
    {% endfor %}
}

#endif  // DAWNWIRE_TYPETRAITS_AUTOGEN_H_
