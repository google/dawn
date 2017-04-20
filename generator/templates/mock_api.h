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

#ifndef MOCK_NXT_H
#define MOCK_NXT_H

#include <gmock/gmock.h>
#include <nxt/nxt.h>

class ProcTableAsClass {
    public:
        virtual ~ProcTableAsClass();

        void GetProcTableAndDevice(nxtProcTable* table, nxtDevice* device);

        {% for type in by_category["object"] %}
            {{as_cType(type.name)}} GetNew{{type.name.CamelCase()}}();
        {% endfor %}

        {% for type in by_category["object"] %}
            {% for method in native_methods(type) %}
                virtual {{as_cType(method.return_type.name)}} {{as_MethodSuffix(type.name, method.name)}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) = 0;
            {% endfor %}

        {% endfor %}

    private:
        std::vector<ProcTableAsClass**> selfPtrs;
};

class MockProcTable : public ProcTableAsClass {
    public:
        {% for type in by_category["object"] %}
            {% for method in native_methods(type) %}
                MOCK_METHOD{{len(method.arguments) + 1}}(
                    {{-as_MethodSuffix(type.name, method.name)}},
                    {{as_cType(method.return_type.name)}}(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ));
            {% endfor %}

        {% endfor %}
};

#endif // MOCK_NXT_H
