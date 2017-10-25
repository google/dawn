//* Copyright 2017 The NXT Authors
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

#ifndef NXTForward_H
#define NXTForward_H

namespace blink {

    {% for other_type in by_category["object"] %}
        class NXT{{other_type.name.CamelCase()}};
    {% endfor %}

    class V8NXTDeviceErrorCallback;
    using NXTDeviceErrorCallback = V8NXTDeviceErrorCallback*;

    class V8NXTBuilderErrorCallback;
    using NXTBuilderErrorCallback = V8NXTBuilderErrorCallback*;

    class V8NXTBufferMapReadCallback;
    using NXTBufferMapReadCallback = V8NXTBufferMapReadCallback*;

    using NXTCallbackUserdata = uint64_t;
}

struct nxtProcTable_s;
typedef struct nxtProcTable_s nxtProcTable;

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}};
{% endfor %}

#endif //NXTForward_H
