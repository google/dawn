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

import androidx.annotation.IntDef
import androidx.annotation.RestrictTo
import kotlin.annotation.AnnotationRetention
import kotlin.annotation.Retention
import kotlin.annotation.Target
{% from 'art/api_kotlin_types.kt' import generate_simple_kdoc with context %}

@Retention(AnnotationRetention.SOURCE)
@RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
@IntDef(
    {% if enum.category == 'bitmask' %}
        flag = true,
    {% endif %}
    value = [
        {% for value in enum.values %}
            {{ enum.name.CamelCase() }}.{{ as_ktName(value.name.CamelCase()) }},
        {% endfor %}
    ]
)
@Target(AnnotationTarget.FUNCTION, AnnotationTarget.TYPE, AnnotationTarget.VALUE_PARAMETER)

//* Generating KDocs
{% set file_docs = kdocs.bitflags if enum.category == 'bitmask' else kdocs.enums %}
{% set docstring = file_docs.get(enum.name.get(), {}).doc %}
{% if docstring %}
    {{ generate_simple_kdoc(docstring) }}
{% endif %}
public annotation class {{ enum.name.CamelCase() }} {
    public companion object {
        //* Generating KDocs
        {% set enum_doc = file_docs.get(enum.name.get(), {}) %}
        {% for value in enum.values %}
            {% set value_docstring = enum_doc.get('entries', {}).get(value.name.snake_case()) %}
            {% if value_docstring %}

                {{ generate_simple_kdoc(value_docstring, indent_prefix = "        ", line_wrap_prefix = "\n         * ") }}
            {% endif %}
            public const val {{ as_ktName(value.name.CamelCase()) }}: Int = {{ '{:#010x}'.format(value.value) }}
        {% endfor %}
        internal val names: Map<Int, String> = mapOf(
            {% for value in enum.values %}
                {{ '{:#010x}'.format(value.value) }} to "{{ as_ktName(value.name.CamelCase()) }}",
            {% endfor %}
        )
        public fun toString(@{{ enum.name.CamelCase() }} value: Int): String = names[value] ?: value.toString()
    }
}
