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
{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_definition, generate_simple_kdoc with context %}

{% set ns = namespace(callback_count = 0) %}
//* Generating KDocs
{% set all_structs_info = kdocs.structs %}
{% set struct_info = all_structs_info.get(structure.name.get()) %}
{% set main_doc = struct_info.doc if struct_info else "" %}
{% if main_doc %}
    {{ generate_simple_kdoc(main_doc) }}
{% endif %}
public class {{ kotlin_name(structure) }}
    {%- for member in kotlin_record_members(structure.members) %}
        {% if kotlin_default(member) is not none %} @JvmOverloads constructor{% break %}{% endif %}
    {%- endfor %}(
    {% for member in kotlin_record_members(structure.members) %}
        {% if member.name.camelCase().endswith('Callback') %}
            {% set ns.callback_count = ns.callback_count + 1 %}
        {% endif %}
        //* Generating KDocs
        {% set member_doc = struct_info.members.get(member.name.get(), "") if struct_info and struct_info.members else "" %}
        {% if member_doc %}
            {{ generate_simple_kdoc(member_doc, indent_prefix = "    ", line_wrap_prefix = '\n     * ') }}
        {% endif %}
        {{ kotlin_annotation(member) }} public var {{ member.name.camelCase() }}: {{ kotlin_definition(member) }},
    {% endfor %}
    {% for structure in chain_children[structure.name.get()] %}
        //* Generating KDocs
        {% set chain_struct_doc = all_structs_info.get(structure.name.get()).doc if all_structs_info.get(structure.name.get()) else "" %}
        {% if chain_struct_doc %}
            {{ generate_simple_kdoc(chain_struct_doc, indent_prefix = "    ", line_wrap_prefix = '\n     * ') }}
        {% endif %}
        public var {{ structure.name.camelCase() }}: {{ kotlin_name(structure) }}? = null,
    {% endfor %}
)
{% if ns.callback_count > 1 %}
    {
      {% for member in kotlin_record_members(structure.members) %}
          {% if member.name.camelCase().endswith('Callback') %}
              {% set callback_name = member.name.camelCase() %}
              {% set executor_name = callback_name + 'Executor' %}
              {% set function_name = 'set' + member.name.CamelCase() %}
              public fun {{ function_name }}(
                  {{ executor_name }}: java.util.concurrent.Executor,
                  {{ callback_name }}: {{ kotlin_definition(member) }}
              ): Unit {
                  this.{{ executor_name }} = {{ executor_name }}
                  this.{{ callback_name }} = {{ callback_name }}
              }
          {% endif %}
      {% endfor %}
    }
{% endif %}