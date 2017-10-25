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

{% macro blinkType(type) -%}
    {%- if type.category == "object" -%}
        NXT{{type.name.CamelCase()}}*
    {%- elif type.category == "enum" or type.category == "bitmask" -%}
        uint32_t
    {%- elif type.category == "natively defined" -%}
        NXT{{type.name.CamelCase()}}
    {%- else -%}
        {{as_cType(type.name)}}
    {%- endif -%}
{%- endmacro %}

{% for other_type in by_category["object"] %}
    #include "NXT{{other_type.name.CamelCase()}}.h"
{% endfor %}

#include "nxt/nxt.h"
#include "platform/wtf/text/StringUTF8Adaptor.h"

namespace blink {

    {% for type in by_category["object"] %}
        {% set Class = "NXT" + type.name.CamelCase() %}
        {{Class}}::{{Class}}({{as_cType(type.name)}} self, Member<NXTState> state)
            : self_(self), state_(state) {
        }
        void {{Class}}::Dispose() {
            {% if type.name.canonical_case() != "device" %}
                state_->GetProcTable()->{{as_varName(type.name, Name("release"))}}(self_);
            {% endif %}
        }

        {% for method in type.methods %}
            {% if method.return_type.name.concatcase() == "void" %}
                {{Class}}*
            {%- else %}
                {{blinkType(method.return_type)}}
            {%- endif -%}
            {{" "}}{{Class}}::{{method.name.camelCase()}}(
                {%- for arg in method.arguments -%}
                    {%- if not loop.first %}, {% endif -%}
                    {%- if arg.annotation == "value" -%}
                        {{blinkType(arg.type)}} {{as_varName(arg.name)}}_
                    {%- elif arg.annotation == "const*" and arg.length == "strlen" -%}
                        String {{as_varName(arg.name)}}_
                    {%- else -%}
                        {%- if arg.type.category == "object" -%}
                            const HeapVector<Member<NXT{{(arg.type.name.CamelCase())}}>>& {{as_varName(arg.name)}}_
                        {%- else -%}
                            const Vector<{{blinkType(arg.type)}}>& {{as_varName(arg.name)}}_
                        {%- endif -%}
                    {%- endif -%}
                {%- endfor -%}
            ) {
                {% for arg in method.arguments %}
                    {% set argName = as_varName(arg.name) %}
                    {% set cType = as_cType(arg.type.name) %}
                    {% if arg.annotation == "value" %}
                        {% if arg.type.category == "object" %}
                            {{cType}} {{argName}} = {{argName}}_->GetNXT();
                        {% else %}
                            {{cType}} {{argName}} = static_cast<{{cType}}>({{argName}}_);
                        {% endif %}
                    {% elif arg.annotation == "const*" %}
                        {% if arg.length == "strlen" %}
                            WTF::StringUTF8Adaptor {{argName}}Adaptor({{argName}}_);
                            std::string {{argName}}String({{argName}}Adaptor.Data(), {{argName}}Adaptor.length());
                            const char* {{argName}} = {{argName}}String.c_str();
                        {% elif arg.type.category == "object" %}
                            //* TODO error on bad length
                            auto {{argName}}Array = std::unique_ptr<{{cType}}[]>(new {{cType}}[{{argName}}_.size()]);
                            for (size_t i = 0; i < {{argName}}_.size(); i++) {
                                {{argName}}Array[i] = {{argName}}_[i]->GetNXT();
                            }
                            const {{cType}}* {{argName}} = &{{argName}}Array[0];
                        {% else %}
                            //* TODO error on bad length
                            const {{cType}}* {{argName}} = {{argName}}_.data();
                        {% endif %}
                    {% endif %}
                {% endfor %}

                {% if method.return_type.name.concatcase() != "void" %}
                    auto result =
                {%- endif %}
                state_->GetProcTable()->{{as_varName(type.name, method.name)}}(self_
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );

                {% if method.return_type.name.concatcase() == "void" %}
                    return this;
                {% else %}
                    // TODO actually return the object given by the call to the procs
                    return new NXT{{method.return_type.name.CamelCase()}}(result, state_);
                {% endif %}
            }
        {% endfor %}

        {% if type.is_builder %}
            {{Class}}* {{Class}}::setErrorCallback(V8NXTBuilderErrorCallback*, unsigned long, unsigned long) {
                //TODO
                return this;
            }
        {% endif %}

        {{as_cType(type.name)}} {{Class}}::GetNXT() {
            return self_;
        }

    {% endfor %}

    NXTDevice* NXTDevice::setErrorCallback(V8NXTDeviceErrorCallback*, unsigned long) {
        return this;
    }

    NXTBuffer* NXTBuffer::mapReadAsync(unsigned int, unsigned int, V8NXTBufferMapReadCallback*, unsigned long) {
        return this;
    }

}
