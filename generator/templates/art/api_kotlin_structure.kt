//* Copyright 2024 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
package {{ kotlin_package }}
{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_definition with context %}

{% set ns = namespace(has_callbackMode = false) %}
{# Add executor related imports if structure contains a callback function #}
{% if 'callback function' in (kotlin_record_members(structure.members) | map(attribute='type.category')) %}
    import java.util.concurrent.Executor
{% endif %}

public class {{ structure.name.CamelCase() }}
    {%- for member in kotlin_record_members(structure.members) %}
        {% if kotlin_default(member) is not none %} @JvmOverloads constructor{% break %}{% endif %}
    {%- endfor %}(
    {% for member in kotlin_record_members(structure.members) %}
        {% if member.type.category in ['callback function']%}
        {{'     '}}@get:JvmName("get{{member.name.CamelCase()}}Executor")
        {{'     '}}public var {{member.name.camelCase()}}Executor: Executor = Executor(Runnable::run),
        {% endif %}
        {{ kotlin_annotation(member) }} public var {{ member.name.camelCase() }}: {{ kotlin_definition(member) }},
        {% if kotlin_annotation(member) == '@CallbackMode'%}
            {% set ns.has_callbackMode = true %}
        {% endif %}
    {% endfor %}
    {% for structure in chain_children[structure.name.get()] %}
        public var {{ structure.name.camelCase() }}: {{ structure.name.CamelCase() }}? = null,
    {% endfor %}
)
{% set members = kotlin_record_members(structure.members) %}
{# Check if structure contains a callback function #}
{% set callback_member = (members | selectattr("type.category", "equalto", "callback function") | first) %}
{# Add a secondary constructor that accepts an executor and a callback function as parameters #}
{% if callback_member and ns.has_callbackMode %}
    {
        public constructor(
            {{callback_member.name.camelCase()}}Executor: Executor,
            {{callback_member.name.camelCase()}}: {{callback_member.type.name.CamelCase()}}
        ) : this(CallbackMode.AllowSpontaneous, {{callback_member.name.camelCase()}}Executor, {{callback_member.name.camelCase()}})
    }
{% endif %}
