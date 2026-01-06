//* Copyright 2025 The Dawn & Tint Authors
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
{% from 'art/api_kotlin_types.kt' import kotlin_declaration, generate_simple_kdoc with context %}
package {{ kotlin_package }}
{% set all_constants_info = kdocs.constants %}

public object Constants {
    /**
     * -1 to max int is resolved at compile time
     */
    private const val UINT32_MAX: Int = -1

    /**
     * -1L to max long is resolved at compile time
     */
    private const val UINT64_MAX: Long = -1L
    private const val SIZE_MAX = UINT64_MAX
    {% for constant in by_category['constant'] %}
        //* Generating KDocs
        {% set constant_doc = all_constants_info.get(constant.name.get()) %}
        {% if constant_doc %}

            {{ generate_simple_kdoc(constant_doc, indent_prefix = "    ", line_wrap_prefix = "\n     * ") }}
        {% endif %}
        public const val {{ as_ktName(constant.name.SNAKE_CASE() ) }}:{{ ' ' }}
        {{- kotlin_declaration(constant) }} =
        {%- if constant.value == 'NAN' %}
            {% if constant.type.name.get() == 'float' -%}   {{ ' ' }}Float.NaN
            {% elif constant.type.name.get() == 'double' %} {{ ' ' }}Double.NaN
            {% else %} {{ assert(false) }}
            {% endif %}
        {%- else %} {{ constant.value }}
        {% endif %}
    {% endfor %}
}
