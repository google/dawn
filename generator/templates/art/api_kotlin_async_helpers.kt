// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package {{ kotlin_package }}

{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition, check_if_doc_present, generate_kdoc with context %}

{% macro analyze_callback(callback_arg, output_ns) %}
    {% set output_ns.status_arg = none %}
    {% set output_ns.message_arg = none %}
    {% set output_ns.payload_arg = none %}
    {% for arg in kotlin_record_members(callback_arg.type.arguments) %}
        {% if arg.name.get() == 'status' %}
            {% set output_ns.status_arg = arg %}
        {% elif arg.name.get() == 'message' %}
            {% set output_ns.message_arg = arg %}
        {% else %}
            {% set output_ns.payload_arg = arg %}
        {% endif %}
    {% endfor %}
{% endmacro %}

{% set all_objects = kdocs.objects %}
{% macro async_wrapper(obj, method, callback_arg) %}
    {% set ns = namespace() %}
    {{ analyze_callback(callback_arg, ns) }}

    //* Generating KDocs
    {% set object_info = all_objects.get(obj.name.get()) %}
    {% set method_info = object_info.methods.get(method.name.snake_case()) if object_info else None %}
    {% if not method_info %}
        {% set method_info = object_info.methods.get(method.name.camelCase()) if object_info else None %}
    {% endif %}
    {% set method_doc = method_info.doc if method_info and method_info.doc else "" %}
    {% set return_doc = method_info.returns_doc if method_info and method_info.returns_doc else "" %}
    {% set args_doc = method_info.args if method_info else {} %}
    {% set method_args = [] %}
    {% for arg in method.arguments %}
        {% if arg.name.get() != 'callback info' %}
            {% do method_args.append(arg) %}
        {% endif %}
    {% endfor %}
    {% if check_if_doc_present(method_doc, return_doc, args_doc, method_args) != 'False' %}
        {{ generate_kdoc(method_doc, return_doc, args_doc, method_args, line_wrap_prefix = "\n * ") }}

    {%- endif %}
    //* The wrapped method has executor and callback function stripped out (the wrapper supplies
    //* those so the client doesn't have to).
    {% set exception_name = (ns.status_arg.name.chunks[:-1] if len(ns.status_arg.name.chunks) > 1 else ['web', 'gpu']) | map('title') | join + 'Exception' %}
    @Throws({{ exception_name}}::class{% if ns.payload_arg and ns.payload_arg.type.name.get() == 'error type' %}, WebGpuRuntimeException::class{% endif %})
    public suspend fun {{ method.name.camelCase() | replace('Async', 'AndAwait') }}(
        {%- for arg in kotlin_record_members(method.arguments) if not (
            arg.type.category == 'callback function' or
            (arg.type.category == 'kotlin type' and arg.type.name.get() == 'java.util.concurrent.Executor')
        ) %}
            {{ kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},
        {%- endfor %}): {{ kotlin_annotation(ns.payload_arg) + ' ' + kotlin_declaration(ns.payload_arg, true) if ns.payload_arg else 'Unit' -}}
        = awaitGPURequest { callback ->
            {{ method.name.camelCase() }}(
            {%- for arg in kotlin_record_members(method.arguments) %}
                {%- if arg.type.category == 'kotlin type' and arg.type.name.get() == 'java.util.concurrent.Executor' -%}
                    Executor(Runnable::run),
                {%- elif arg.name.get() == callback_arg.name.get() -%}
                    callback
                {%- else -%}
                    {{ as_varName(arg.name) }},
                {%- endif %}
            {%- endfor -%}
          )
    }
{% endmacro %}
