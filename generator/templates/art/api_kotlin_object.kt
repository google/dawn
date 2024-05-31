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
{% from 'art/api_kotlin_types.kt' import kotlin_type_declaration, kotlin_definition with context %}

class {{ obj.name.CamelCase() }}(val handle: Long) {
    {% for method in obj.methods if include_method(method) %}
        @JvmName("{{ method.name.camelCase() }}") external fun {{ method.name.camelCase() }}(
        //* userdata parameter omitted because Kotlin clients can achieve the same with closures.
        //* length parameters are omitted because Kotlin containers have 'length'.
        //* TODO(b/341923892): rework async methods to use futures.
        {%- for arg in method.arguments if arg.name.get() != 'userdata' and
                not method.arguments | selectattr('length', 'equalto', arg) | first %}
            {{- as_varName(arg.name) }}: {{ kotlin_definition(arg) }},
        {%- endfor -%}):
        {{- kotlin_type_declaration(method.return_type) -}}
        {% if method.name.chunks[0] == 'get' and not method.arguments %}
            //* For the Kotlin getter, strip word 'get' from name and convert the remainder to
            //* camelCase() (lower case first word). E.g. "get foo bar" translated to fooBar.
            {% set name = method.name.chunks[1] + method.name.chunks[2:] | map('title') | join %}
            @get:JvmName("{{ name }}")
            val {{ name }} get() = {{ method.name.camelCase() }}()
        {% endif %}
    {% endfor %}
}
