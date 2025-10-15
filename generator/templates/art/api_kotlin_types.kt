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

{%- macro kotlin_declaration(arg) -%}
    {%- set type = arg.type %}
    {%- set optional = arg.optional %}
    {%- set default_value = arg.default_value %}
    {%- if arg == None -%}
        Unit
    {%- elif arg.type.name.get() == 'string view' -%}
        String{{ '?' if optional }}
    {%- elif type.name.get() == 'void' %}
        {{- assert(arg.length and arg.constant_length != 1) -}}  {# void with length is binary data #}
        java.nio.ByteBuffer
    {%- elif arg.length and arg.length != 'constant' %}
        {# * annotation can mean an array, e.g. an output argument #}
        {%- if type.category in ['callback function', 'callback info', 'function pointer', 'object', 'structure'] -%}
            Array<{{ type.name.CamelCase() }}>
        {%- elif type.category in ['bitmask', 'enum'] or type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            IntArray
        {%- else -%}
            {{ unreachable_code() }}
        {% endif %}
    {%- elif type.category in ['callback function', 'function pointer', 'object'] %}
        {{- type.name.CamelCase() }}
        {%- if optional or default_value %}?{% endif %}
    {%- elif type.category == 'structure' or type.category == 'callback info' %}
        {{- type.name.CamelCase() }}{{ '?' if optional }}
    {%- elif type.category in ['bitmask', 'enum'] -%}
        Int
    {%- elif type.name.get() == 'bool' -%}
        Boolean{{ '?' if optional }}
    {%- elif type.name.get() in ['void *', 'void const *'] %}
        //* Hack: void* for a return value is a ByteBuffer.
        {% if not arg.name %}
            ByteBuffer
        {% else %}
            Long
        {% endif %}
    {%- elif type.category == 'native' -%}
        {%- if type.name.get() == 'float' -%}
            Float
        {%- elif type.name.get() == 'double' -%}
            Double
        {%- elif type.name.get() in ['int8_t', 'uint8_t'] -%}
            Byte
        {%- elif type.name.get() in ['int16_t', 'uint16_t'] -%}
            Short
        {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            Int
        {%- elif type.name.get() in ['int64_t', 'uint64_t', 'size_t'] -%}
            Long
        {%- else -%}
            {{ unreachable_code('Unsupported native type: ' + type.name.get()) }}
        {%- endif -%}
        {%- if optional -%}
            {{ unreachable_code('Optional natives not supported: ' + type.name.get()) }}
        {%- endif -%}
    {%- else -%}
        {{ unreachable_code('Unsupported type: ' + type.name.get()) }}
    {%- endif %}
{% endmacro %}

{% macro kotlin_definition(arg) -%}
    {{- kotlin_declaration(arg) -}}
    {%- if kotlin_default(arg) is not none %} = {{ kotlin_default(arg) }}{% endif -%}
{%- endmacro %}

{% macro kotlin_annotation(arg) -%}
    {%- if arg != None -%}
        {% set type = arg.type %}
        {% if type.category in ['bitmask', 'enum'] -%}
            @{{ type.name.CamelCase() -}}
        {% endif -%}
    {% endif -%}
{% endmacro %}
