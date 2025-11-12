// Copyright 2024 The Dawn & Tint Authors
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
{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_definition, generate_simple_kdoc with context %}

//* Generating KDocs
{% set all_structs_info = kdocs.structs %}
{% set struct_info = all_structs_info.get(structure.name.get()) %}
{% set main_doc = struct_info.doc if struct_info else "" %}
{% if main_doc %}
    {{ generate_simple_kdoc(main_doc) }}
{% endif %}
public class {{ structure.name.CamelCase() }}
    {%- for member in kotlin_record_members(structure.members) %}
        {% if kotlin_default(member) is not none %} @JvmOverloads constructor{% break %}{% endif %}
    {%- endfor %}(
    {% for member in kotlin_record_members(structure.members) %}
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
        public var {{ structure.name.camelCase() }}: {{ structure.name.CamelCase() }}? = null,
    {% endfor %}
)
