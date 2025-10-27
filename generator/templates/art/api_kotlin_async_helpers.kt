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

import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine
{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition with context %}

{% macro async_wrapper(obj, method, callback_arg) %}
    {% set return_name = callback_arg.type.name.chunks[:-1] | map('title') | join + 'Return' %}
    {% set result_args = kotlin_record_members(callback_arg.type.arguments) | list %}

    //* We make a return class to receive the callback's (possibly multiple) return values.
    public class {{ return_name }}(
        {% for arg in result_args %}
            {{ kotlin_annotation(arg) }} public val {{ as_varName(arg.name) }}: {{ kotlin_declaration(arg) }}{{ ',' if not loop.last }}
        {% endfor %}) {
        //* Required for destructuring declarations. These come for free in a 'data' class but
        //* we don't make it a data class because that can cause binary compatibility issues.
        {% for arg in result_args %}
            public operator fun component{{ loop.index }}() : {{ kotlin_declaration(arg) }} =
                {{- as_varName(arg.name) }}
        {% endfor %}
    }

    //* The wrapped method has executor and callback function stripped out (the wrapper supplies
    //* those so the client doesn't have to).
    public suspend fun {{ obj.name.CamelCase() }}.{{ method.name.camelCase() }}(
        {%- for arg in kotlin_record_members(method.arguments) if not (
            arg.type.category == 'callback function' or
            (arg.type.category == 'kotlin type' and arg.type.name.get() == 'java.util.concurrent.Executor')
        ) %}
            {{ kotlin_annotation(arg) }}  {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},
        {%- endfor %}): {{ return_name }} = suspendCoroutine {
        {{ method.name.camelCase() }}(
            {%- for arg in kotlin_record_members(method.arguments) %}
                {%- if arg.type.category == 'kotlin type' and arg.type.name.get() == 'java.util.concurrent.Executor' -%}
                    Executor(Runnable::run),
                {%- elif arg.name.get() == callback_arg.name.get() %}{
                    {%- for arg in kotlin_record_members(callback_arg.type.arguments) %}
                        {{- as_varName(arg.name) }},
                    {%- endfor %} -> it.resume({{ return_name }}(
                        //* We make an instance of the callback parameters -> return type wrapper.
                        {%- for arg in result_args %}
                            {{- as_varName(arg.name) }},
                        {%- endfor %}
                    ))},
                {%- else -%}
                    {{- as_varName(arg.name) }},
                {%- endif %}
            {%- endfor %})
    }
{% endmacro %}

//* Every method that is identified as using callbacks is given a helper method that wraps the
//* call with a suspend function.
{% for obj in by_category['object'] %}
    {%- for method in obj.methods if include_method(obj, method) %}
        {%- for arg in kotlin_record_members(method.arguments) %}
            {% if arg.type.category == 'callback function' %}
                {{ async_wrapper(obj, method, arg) }}
                {{ continue }}
            {% endif %}
        {% endfor %}
    {% endfor %}
{% endfor %}

