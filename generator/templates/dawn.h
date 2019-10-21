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

// This temporary header translates all the previous Dawn C API to the webgpu.h
// API so that during a small transition period both headers are supported.

#ifndef DAWN_DAWN_H_
#define DAWN_DAWN_H_

#include "webgpu.h"

#define DAWN_WHOLE_SIZE WGPU_WHOLE_SIZE

{% for type in by_category["object"] %}
    typedef {{as_cType(type.name)}} {{as_cTypeDawn(type.name)}};
    typedef {{as_cType(type.name)}}Impl {{as_cTypeDawn(type.name)}}Impl;
    {% for method in native_methods(type) %}
        typedef {{as_cProc(type.name, method.name)}} {{as_cProcDawn(type.name, method.name)}};
        #define {{as_cMethodDawn(type.name, method.name)}} {{as_cMethod(type.name, method.name)}}
    {% endfor %}
{% endfor %}

{% for type in by_category["enum"] + by_category["bitmask"] %}
    typedef {{as_cType(type.name)}} {{as_cTypeDawn(type.name)}};
    {% if type.category == "bitmask" %}
        typedef {{as_cType(type.name)}}Flags {{as_cTypeDawn(type.name)}}Flags;
    {% endif %}

    {% for value in type.values %}
        #define {{as_cEnumDawn(type.name, value.name)}} {{as_cEnum(type.name, value.name)}}
    {% endfor %}
    #define {{as_cEnumDawn(type.name, Name("force32"))}} {{as_cEnum(type.name, Name("force32"))}}
{% endfor %}

{% for type in by_category["structure"] %}
    typedef {{as_cType(type.name)}} {{as_cTypeDawn(type.name)}};
{% endfor %}

typedef WGPUBufferCreateMappedCallback DawnBufferCreateMappedCallback;
typedef WGPUBufferMapReadCallback DawnBufferMapReadCallback;
typedef WGPUBufferMapWriteCallback DawnBufferMapWriteCallback;
typedef WGPUFenceOnCompletionCallback DawnFenceOnCompletionCallback;
typedef WGPUErrorCallback DawnErrorCallback;

typedef WGPUProc DawnProc;

typedef WGPUProcGetProcAddress DawnProcGetProcAddress;
#define DawnGetProcAddress WGPUGetProcAddress

#endif // DAWN_DAWN_H_
