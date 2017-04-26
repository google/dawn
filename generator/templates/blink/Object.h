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
    {%- else -%}
        {{as_cType(type.name)}}
    {%- endif -%}
{%- endmacro %}

{% set Class = "NXT" + type.name.CamelCase() %}

#ifndef {{Class}}_H
#define {{Class}}_H

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/GarbageCollected.h"
#include "platform/wtf/text/WTFString.h"

#include "../NXTState.h"
#include "Forward.h"

namespace blink {

class {{Class}} final :
        public GarbageCollectedFinalized<{{Class}}>,
        public ScriptWrappable {

    WTF_MAKE_NONCOPYABLE({{Class}});
    USING_PRE_FINALIZER({{Class}}, Dispose);
    public:
        DEFINE_INLINE_TRACE() {
            visitor->Trace(state_);
        };

    public:
        DEFINE_WRAPPERTYPEINFO();

    public:
        {{Class}}({{as_cType(type.name)}} self, Member<NXTState> state);
        void Dispose();

        {% for method in type.methods %}
            {% if method.return_type.name.concatcase() == "void" %}
                {{Class}}*
            {%- else %}
                {{blinkType(method.return_type)}}
            {%- endif -%}
            {{" "}}{{method.name.camelCase()}}(
                {%- for arg in method.arguments -%}
                    {%- if not loop.first %}, {% endif -%}
                    {%- if arg.annotation == "value" -%}
                        {{blinkType(arg.type)}} {{as_varName(arg.name)}}
                    {%- elif arg.annotation == "const*" and arg.length == "strlen" -%}
                        String {{as_varName(arg.name)}}
                    {%- else -%}
                        {%- if arg.type.category == "object" -%}
                            const HeapVector<Member<NXT{{(arg.type.name.CamelCase())}}>>& {{as_varName(arg.name)}}
                        {%- else -%}
                            const Vector<{{blinkType(arg.type)}}>& {{as_varName(arg.name)}}
                        {%- endif -%}
                    {%- endif -%}
                {%- endfor -%}
            );
        {% endfor %}

        {{as_cType(type.name)}} GetNXT();

        {% if type.name.canonical_case() == "device" %}
            {% for type in by_category["enum"] + by_category["bitmask"] %}
                {% for value in type.values %}
                    static constexpr uint32_t k{{type.name.CamelCase()}}{{value.name.CamelCase()}} = 0x{{format(value.value, "08X")}};
                {% endfor %}
            {% endfor %}
        {% endif %}

    private:
        {{as_cType(type.name)}} self_ = nullptr;
        Member<NXTState> state_;
};

}

#endif // {{Class}}_H

