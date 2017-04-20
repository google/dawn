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

#include "mock_nxt.h"

namespace {
    {% for type in by_category["object"] %}
        {% for method in native_methods(type) %}
            {{as_cType(method.return_type.name)}} Forward{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} self
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto tablePtr = reinterpret_cast<ProcTableAsClass**>(self);
                return (*tablePtr)->{{as_MethodSuffix(type.name, method.name)}}(self
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}

    {% endfor %}
}

ProcTableAsClass::~ProcTableAsClass() {
    for (auto ptr : selfPtrs) {
        delete ptr;
    }
}

void ProcTableAsClass::GetProcTableAndDevice(nxtProcTable* table, nxtDevice* device) {
    *device = GetNewDevice();

    {% for type in by_category["object"] %}
        {% for method in native_methods(type) %}
            table->{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Forward{{as_MethodSuffix(type.name, method.name)}});
        {% endfor %}
    {% endfor %}
}

{% for type in by_category["object"] %}
    {{as_cType(type.name)}} ProcTableAsClass::GetNew{{type.name.CamelCase()}}() {
        auto self = new ProcTableAsClass*(this);
        selfPtrs.push_back(self);
        return reinterpret_cast<{{as_cType(type.name)}}>(self);
    }
{% endfor %}
