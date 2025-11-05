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

import {{ kotlin_package }}.helper.DawnException
import dalvik.annotation.optimization.FastNative
import java.nio.ByteBuffer
import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlinx.coroutines.suspendCancellableCoroutine

{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition, check_if_doc_present, generate_kdoc, generate_simple_kdoc with context %}
{% from 'art/api_kotlin_async_helpers.kt' import async_wrapper with context %}

//* Generating KDocs
{% set all_objects_info = kdocs.objects%}
{% set object_info = all_objects_info.get(obj.name.get()) %}
{% set doc_str = object_info.doc if object_info else "" %}
{% if doc_str | trim %}
    {{ generate_simple_kdoc(doc_str) }}
{% endif %}
public class {{ kotlin_name(obj) }}(public val handle: Long): AutoCloseable {
    {% set all_method_info = object_info.methods if object_info else {} %}
    {% for method in obj.methods if include_method(obj, method) %}
        //* Generating KDocs
        {% set method_info = all_method_info.get(method.name.snake_case()) %}
        {% set main_doc = method_info.doc if method_info else "" %}
        {% set return_doc = method_info.returns_doc if method_info else "" %}
        {% set arg_docs_map = method_info.args if method_info else {} %}
        {% set method_args = kotlin_record_members(method.arguments) | list %}
        {% if check_if_doc_present(main_doc, return_doc, arg_docs_map, method_args) == 'True' %}
        {{ generate_kdoc(main_doc, return_doc, arg_docs_map, method_args, "\n     * ", indent_prefix = "    ") }}

        {%- endif %}
        @FastNative
        @JvmName("{{ method.name.camelCase() }}")
        {% for arg in kotlin_record_members(method.arguments) %}
            {% if kotlin_default(arg) is not none %}
                @JvmOverloads
            {% break %}{% endif %}
        {% endfor %}
        {{ kotlin_annotation(kotlin_return(method)) }} public external fun {{ method.name.camelCase() }}(
        //* TODO(b/341923892): rework async methods to use futures.
        {%- for arg in kotlin_record_members(method.arguments) %}
            {{- kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},{{ ' ' }}
        {%- endfor -%}): {{ kotlin_declaration(kotlin_return(method)) }}

        {% if method.name.chunks[0] == 'get' and not method.arguments %}
            //* For the Kotlin getter, strip word 'get' from name and convert the remainder to
            //* camelCase() (lower case first word). E.g. "get foo bar" translated to fooBar.
            {% set name = method.name.chunks[1] + method.name.chunks[2:] | map('title') | join %}
            @get:JvmName("{{ name }}")
            public val {{ name }}: {{ kotlin_declaration(kotlin_return(method)) }} get() = {{ method.name.camelCase() }}()

        {% endif %}

        //* Every method that is identified as using callbacks is given a helper method that wraps the
        //* call with a suspend function.
        {%- for arg in kotlin_record_members(method.arguments) %}
            {% if arg.type.category == 'callback function' %}
                {{- async_wrapper(obj, method, arg) -}}
                {{ continue }}
            {% endif %}
        {%- endfor -%}

    {% endfor %}
    external override fun close()

    //* By default, the equals() function implements referential equality.
    //* see: https://kotlinlang.org/docs/equality.html#structural-equality
    //* A structural comparison of the wrapper object is equivalent to a referential comparison of
    //* the wrapped object.
    override fun equals(other: Any?): Boolean =
        other is {{ kotlin_name(obj) }} && other.handle == handle
    override fun hashCode(): Int = handle.hashCode()
}
