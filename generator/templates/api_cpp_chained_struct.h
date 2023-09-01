//* Copyright 2023 The Dawn Authors
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
{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}
#ifndef {{API}}_CPP_CHAINED_STRUCT_H_
#define {{API}}_CPP_CHAINED_STRUCT_H_

#include <cstddef>
#include <cstdint>

// This header file declares the ChainedStruct structures separately from the {{metadata.api}}
// headers so that dependencies can directly extend structures without including the larger header
// which exposes capabilities that may require correctly set proc tables.
namespace {{metadata.namespace}} {

    enum class SType : uint32_t;

    struct ChainedStruct {
        ChainedStruct const * nextInChain = nullptr;
        SType sType = SType(0u);
    };

    struct ChainedStructOut {
        ChainedStructOut * nextInChain = nullptr;
        SType sType = SType(0u);
    };

}  // namespace {{metadata.namespace}}}

#endif // {{API}}_CPP_CHAINED_STRUCT_H_
