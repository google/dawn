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

#ifndef NXT_H
#define NXT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}};
{% endfor %}

{% for type in by_category["enum"] + by_category["bitmask"] %}
    typedef enum {
        {% for value in type.values %}
            {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
        {% endfor %}
        {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
    } {{as_cType(type.name)}};

{% endfor %}

{% for type in by_category["structure"] %}
    typedef struct {
        {% if type.extensible %}
            const void* nextInChain;
        {% endif %}
        {% for member in type.members %}
            {{as_annotated_cType(member)}};
        {% endfor %}
    } {{as_cType(type.name)}};
{% endfor %}

// Custom types depending on the target language
typedef uint64_t nxtCallbackUserdata;
typedef void (*nxtDeviceErrorCallback)(const char* message, nxtCallbackUserdata userdata);
typedef void (*nxtBuilderErrorCallback)(nxtBuilderErrorStatus status, const char* message, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2);
typedef void (*nxtBufferMapReadCallback)(nxtBufferMapAsyncStatus status, const void* data, nxtCallbackUserdata userdata);
typedef void (*nxtBufferMapWriteCallback)(nxtBufferMapAsyncStatus status, void* data, nxtCallbackUserdata userdata);

#ifdef __cplusplus
extern "C" {
#endif

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

struct nxtProcTable_s {
    {% for type in by_category["object"] %}
        {% for method in native_methods(type) %}
            {{as_cProc(type.name, method.name)}} {{as_varName(type.name, method.name)}};
        {% endfor %}

    {% endfor %}
};
typedef struct nxtProcTable_s nxtProcTable;

// Stuff below is for convenience and will forward calls to a static nxtProcTable.

// Set which nxtProcTable will be used
void nxtSetProcs(const nxtProcTable* procs);

{% for type in by_category["object"] %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in native_methods(type) %}
        {{as_cType(method.return_type.name)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor %}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NXT_H
