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

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
#ifndef MOCK_{{API}}_H
#define MOCK_{{API}}_H

{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include "dawn/{{prefix}}_proc_table.h"
#include "dawn/{{api}}.h"
#include <gmock/gmock.h>

#include <memory>

// An abstract base class representing a proc table so that API calls can be mocked. Most API calls
// are directly represented by a delete virtual method but others need minimal state tracking to be
// useful as mocks.
class ProcTableAsClass {
    public:
        virtual ~ProcTableAsClass();

        void GetProcTable({{Prefix}}ProcTable* table);

        // Creates an object that can be returned by a mocked call as in WillOnce(Return(foo)).
        // It returns an object of the write type that isn't equal to any previously returned object.
        // Otherwise some mock expectation could be triggered by two different objects having the same
        // value.
        {% for type in by_category["object"] %}
            {{as_cType(type.name)}} GetNew{{type.name.CamelCase()}}();
        {% endfor %}

        {% for type in by_category["object"] %}
            {% for method in type.methods if not has_callback_arguments(method) %}
                virtual {{as_cType(method.return_type.name)}} {{as_MethodSuffix(type.name, method.name)}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) = 0;
            {% endfor %}

            virtual void {{as_MethodSuffix(type.name, Name("reference"))}}({{as_cType(type.name)}} self) = 0;
            virtual void {{as_MethodSuffix(type.name, Name("release"))}}({{as_cType(type.name)}} self) = 0;

            {% for method in type.methods if has_callback_arguments(method) %}
                {% set Suffix = as_MethodSuffix(type.name, method.name) %}
                //* Stores callback and userdata and calls the On* method.
                {{as_cType(method.return_type.name)}} {{Suffix}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                );
                //* The virtual function to call after saving the callback and userdata in the proc.
                //* This function can be mocked.
                virtual {{as_cType(method.return_type.name)}} On{{Suffix}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) = 0;

                //* Calls the stored callback.
                {% for callback_arg in method.arguments if callback_arg.type.category == 'function pointer' %}
                    void Call{{as_MethodSuffix(type.name, method.name)}}Callback(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in callback_arg.type.arguments -%}
                            {%- if not loop.last -%}, {{as_annotated_cType(arg)}}{%- endif -%}
                        {%- endfor -%}
                    );
                {% endfor %}
            {% endfor %}
        {% endfor %}

        struct Object {
            ProcTableAsClass* procs = nullptr;
            {% for type in by_category["object"] %}
                {% for method in type.methods if has_callback_arguments(method) %}
                    {% for callback_arg in method.arguments if callback_arg.type.category == 'function pointer' %}
                        {{as_cType(callback_arg.type.name)}} m{{as_MethodSuffix(type.name, method.name)}}Callback = nullptr;
                    {% endfor %}
                {% endfor %}
            {% endfor %}
            void* userdata = 0;
        };

    private:
        // Remembers the values returned by GetNew* so they can be freed.
        std::vector<std::unique_ptr<Object>> mObjects;
};

class MockProcTable : public ProcTableAsClass {
    public:
        MockProcTable();
        ~MockProcTable() override;

        void IgnoreAllReleaseCalls();

        {% for type in by_category["object"] %}
            {% for method in type.methods if not has_callback_arguments(method) %}
                MOCK_METHOD({{as_cType(method.return_type.name)}},{{" "}}
                    {{-as_MethodSuffix(type.name, method.name)}}, (
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ), (override));
            {% endfor %}

            MOCK_METHOD(void, {{as_MethodSuffix(type.name, Name("reference"))}}, ({{as_cType(type.name)}} self), (override));
            MOCK_METHOD(void, {{as_MethodSuffix(type.name, Name("release"))}}, ({{as_cType(type.name)}} self), (override));

            {% for method in type.methods if has_callback_arguments(method) %}
                MOCK_METHOD({{as_cType(method.return_type.name)}},{{" "-}}
                    On{{as_MethodSuffix(type.name, method.name)}}, (
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ), (override));
            {% endfor %}
        {% endfor %}
};

#endif  // MOCK_{{API}}_H
