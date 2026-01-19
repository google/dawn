/*
 * Copyright 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
@Target(AnnotationTarget.FUNCTION, AnnotationTarget.TYPE, AnnotationTarget.VALUE_PARAMETER, AnnotationTarget.PROPERTY)

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
