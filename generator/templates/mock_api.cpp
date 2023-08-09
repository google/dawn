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

{% set api = metadata.api.lower() %}
#include "dawn/common/Log.h"
#include "mock_{{api}}.h"

using namespace testing;

namespace {
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_cType(method.return_type.name)}} Forward{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} self
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
                return object->procs->{{as_MethodSuffix(type.name, method.name)}}(self
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}

    {% endfor %}
}

ProcTableAsClass::~ProcTableAsClass() {
}

{% set Prefix = metadata.proc_table_prefix %}
void ProcTableAsClass::GetProcTable({{Prefix}}ProcTable* table) {
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            table->{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Forward{{as_MethodSuffix(type.name, method.name)}});
        {% endfor %}
    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        table->{{as_varName(type.name, Name("free members"))}} = []({{as_cType(type.name)}} {{as_varName(type.name)}}) {
            dawn::WarningLog() << "No mock available for {{as_varName(type.name, Name('free members'))}}";
        };
    {% endfor %}
}

{% for type in by_category["object"] %}
    {% for method in type.methods if has_callback_arguments(method) %}
        {% set Suffix = as_MethodSuffix(type.name, method.name) %}

        {{as_cType(method.return_type.name)}} ProcTableAsClass::{{Suffix}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
            {% for callback_arg in method.arguments if callback_arg.type.category == 'function pointer' %}
                object->m{{as_MethodSuffix(type.name, method.name)}}Callback = {{as_varName(callback_arg.name)}};
            {% endfor %}
            object->userdata = userdata;
            return On{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }

        {% for callback_arg in method.arguments if callback_arg.type.category == 'function pointer' %}
            void ProcTableAsClass::Call{{Suffix}}Callback(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in callback_arg.type.arguments -%}
                    {%- if not loop.last -%}, {{as_annotated_cType(arg)}}{%- endif -%}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                object->m{{Suffix}}Callback(
                    {%- for arg in callback_arg.type.arguments -%}
                        {%- if not loop.last -%}{{as_varName(arg.name)}}, {% endif -%}
                    {%- endfor -%}
                    object->userdata);
            }
        {% endfor %}
    {% endfor %}
{% endfor %}

{% for type in by_category["object"] %}
    {{as_cType(type.name)}} ProcTableAsClass::GetNew{{type.name.CamelCase()}}() {
        mObjects.emplace_back(new Object);
        mObjects.back()->procs = this;
        return reinterpret_cast<{{as_cType(type.name)}}>(mObjects.back().get());
    }
{% endfor %}

MockProcTable::MockProcTable() = default;

MockProcTable::~MockProcTable() = default;

void MockProcTable::IgnoreAllReleaseCalls() {
    {% for type in by_category["object"] %}
        EXPECT_CALL(*this, {{as_MethodSuffix(type.name, Name("release"))}}(_)).Times(AnyNumber());
    {% endfor %}
}
