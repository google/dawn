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
@file:JvmName("Functions")

package {{ kotlin_package }}

import dalvik.annotation.optimization.FastNative

public object GPU {
    {% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition, check_if_doc_present, generate_kdoc with context %}

    {% set all_functions_info = kdocs.functions %}
    {% for function in by_category['function'] if include_method(None, function) %}
        {% set _kotlin_return = kotlin_return(function) %}
        //* Generating KDocs
        {% set function_info = all_functions_info.get(function.name.get()) %}
        {% set main_doc = function_info.doc if function_info else "" %}
        {% set return_doc = function_info.returns_doc if function_info else "" %}
        {% set arg_docs_map = function_info.args if function_info else {} %}
        {% set function_args = function.arguments | list %}
        {% if check_if_doc_present(main_doc, return_doc, arg_docs_map, function_args) == 'True' %}
            {{ generate_kdoc(main_doc, return_doc, arg_docs_map, function_args , line_wrap_prefix = "\n * ") }}

        {%- endif %}
        @FastNative
        {% if function.returns and function.returns.type.name.canonical_case() == 'status' %}
            @Throws({{"WebGpuException::class"}})
        {% endif %}
        {{ kotlin_annotation(_kotlin_return) if _kotlin_return else '' }} public external fun {{ function.name.camelCase() }}(
            {%- for arg in function.arguments -%}
                {{- kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},{{' '}}
            {%- endfor %}): {{ kotlin_declaration(_kotlin_return) if _kotlin_return else 'Unit' }}
{% endfor %}}