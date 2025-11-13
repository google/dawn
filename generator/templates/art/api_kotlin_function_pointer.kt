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

{% set callbackName = 'on' + function_pointer.name.chunks[:-1] | map('title') | join %}

//* Generating KDocs
{% set all_callback_info = kdocs.callbacks %}
{% set funtion_pointer_info = all_callback_info.get(function_pointer.name.get()) %}
{% set main_doc = funtion_pointer_info.doc if funtion_pointer_info else "" %}
{% set arg_docs_map =  funtion_pointer_info.args if funtion_pointer_info else {} %}

{% set function_pointer_args = function_pointer.arguments[:-1] | list %}
{% if check_if_doc_present(main_doc, "", arg_docs_map, function_pointer_args) == 'True' %}
    {{ generate_kdoc(main_doc, return_str, arg_docs_map, function_pointer_args , line_wrap_prefix = "\n * ") }}

{%- endif %}
public fun interface {{ function_pointer.name.CamelCase() }} {
    @Suppress("INAPPLICABLE_JVM_NAME")  //* Required for @JvmName on global function.
    @JvmName("{{ callbackName }}")  //* Required to access Inline Value Class parameters via JNI.
    public fun {{ callbackName }}(
    {%- for arg in kotlin_record_members(function_pointer.arguments) -%}
        {{ kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_declaration(arg) }},{{ ' ' }}
    {%- endfor -%});
}

{% set args_list = kotlin_record_members(function_pointer.arguments) | list %}

internal class {{ function_pointer.name.CamelCase() }}Runnable(
private val callback: {{ function_pointer.name.CamelCase() }},
{% for arg in args_list %}
    private val {{ as_varName(arg.name) }}: {{ kotlin_declaration(arg) }}{{ ','
    if not loop.last }}
{% endfor %}
) : Runnable {
    override fun run() {
        callback.{{ callbackName }}(
            {%- for arg in args_list -%}
            {{ as_varName(arg.name) }}{{ ', ' if not loop.last }}
            {%- endfor -%}
        )
    }
}
