//* Copyright 2017 The Dawn Authors
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

#ifndef DAWN_DAWN_H_
#define DAWN_DAWN_H_

#include "dawn/dawn_export.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

const uint64_t DAWN_WHOLE_SIZE = 0xffffffffffffffffULL; // UINT64_MAX

typedef uint32_t WGPUFlags;

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}};
{% endfor %}

{% for type in by_category["enum"] + by_category["bitmask"] %}
    typedef enum {{as_cType(type.name)}} {
        {% for value in type.values %}
            {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
        {% endfor %}
        {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
    } {{as_cType(type.name)}};
    {% if type.category == "bitmask" %}
        typedef WGPUFlags {{as_cType(type.name)}}Flags;
    {% endif %}

{% endfor %}

{% for type in by_category["structure"] %}
    typedef struct {{as_cType(type.name)}} {
        {% if type.extensible %}
            void const * nextInChain;
        {% endif %}
        {% for member in type.members %}
            {{as_annotated_cType(member)}};
        {% endfor %}
    } {{as_cType(type.name)}};

{% endfor %}

#ifdef __cplusplus
extern "C" {
#endif

// Custom types depending on the target language
typedef void (*DawnBufferCreateMappedCallback)(DawnBufferMapAsyncStatus status,
                                               DawnCreateBufferMappedResult result,
                                               void* userdata);
typedef void (*DawnBufferMapReadCallback)(DawnBufferMapAsyncStatus status,
                                          const void* data,
                                          uint64_t dataLength,
                                          void* userdata);
typedef void (*DawnBufferMapWriteCallback)(DawnBufferMapAsyncStatus status,
                                           void* data,
                                           uint64_t dataLength,
                                           void* userdata);
typedef void (*DawnFenceOnCompletionCallback)(DawnFenceCompletionStatus status, void* userdata);
typedef void (*DawnErrorCallback)(DawnErrorType type, const char* message, void* userdata);

typedef void (*DawnProc)();

#if !defined(DAWN_SKIP_PROCS)

typedef DawnProc (*DawnProcGetProcAddress)(DawnDevice device, const char* procName);

{% for type in by_category["object"] %}
    // Procs of {{type.name.CamelCase()}}
    {% for method in native_methods(type) %}
        typedef {{as_cType(method.return_type.name)}} (*{{as_cProc(type.name, method.name)}})(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor %}
#endif  // !defined(DAWN_SKIP_PROCS)

#if !defined(DAWN_SKIP_DECLARATIONS)

DAWN_EXPORT DawnProc DawnGetProcAddress(DawnDevice device, const char* procName);

{% for type in by_category["object"] %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in native_methods(type) %}
        DAWN_EXPORT {{as_cType(method.return_type.name)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor %}
#endif  // !defined(DAWN_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DAWN_DAWN_H_
