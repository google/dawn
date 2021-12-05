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

{% set Prefix = metadata.proc_table_prefix %}
#ifndef DAWN_{{Prefix.upper()}}_PROC_TABLE_H_
#define DAWN_{{Prefix.upper()}}_PROC_TABLE_H_

#include "dawn/{{metadata.api.lower()}}.h"

// Note: Often allocated as a static global. Do not add a complex constructor.
typedef struct {{Prefix}}ProcTable {
    {% for function in by_category["function"] %}
        {{as_cProc(None, function.name)}} {{as_varName(function.name)}};
    {% endfor %}

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_cProc(type.name, method.name)}} {{as_varName(type.name, method.name)}};
        {% endfor %}

    {% endfor %}
} {{Prefix}}ProcTable;

#endif  // DAWN_{{Prefix.upper()}}_PROC_TABLE_H_
