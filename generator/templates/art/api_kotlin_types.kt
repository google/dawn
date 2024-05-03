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

//* Outputs a declaration for an argument (type RecordMember), including a default defintion if
//* required.
{%- macro kotlin_declaration(arg) -%}
    {%- set type = arg.type %}
    {%- set optional = arg.optional %}
    {%- set default_value = arg.default_value %}
    {%- set no_default = arg.json_data is defined and arg.json_data.get('no_default', false) %}
    {%- if arg.length == 'strlen' -%}
        String{{ '?' if optional or default_value == 'nullptr' }}
        {%- if default_value or optional -%}
            = null
        {%- endif %}
    {%- elif arg.length and arg.constant_length != 1 %}
        {%- if type.category in ['function pointer', 'object', 'structure'] -%}
            Array<{{ type.name.CamelCase() }}> = arrayOf()
        {%- elif type.category in ['bitmask', 'enum'] -%}
            IntArray = intArrayOf()
        {%- elif type.name.get() == 'bool' -%}
            BooleanArray = booleanArrayOf()
        {%- elif type.name.get() == 'float' -%}
            FloatArray = floatArrayOf()
        {%- elif type.name.get() == 'double' -%}
            DoubleArray = doubleArrayOf()
        {%- elif type.name.get() in ['int8_t', 'uint8_t', 'void'] -%}
            ByteArray = byteArrayOf()
        {%- elif type.name.get() in ['int16_t', 'uint16_t'] -%}
            ShortArray = shortArrayOf()
        {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            IntArray = intArrayOf()
        {%- elif type.name.get() in ['int64_t', 'uint64_t', 'size_t'] -%}
            LongArray = longArrayOf()
        {%- else -%}
            {{ unreachable_code() }}
        {% endif %}
    {%- elif type.category in ['function pointer', 'object'] %}
        {{- type.name.CamelCase() }}
        {%- if optional or default_value %}? = null{% endif %}
    {%- elif type.category == 'structure' %}
        {{- type.name.CamelCase() }}{{ '?' if optional }}
        {%- if type.has_basic_constructor and not no_default -%}
            = {{ type.name.CamelCase() }}()
        {%- elif optional -%}
            = null
        {%- endif %}
    {%- elif type.category in ['bitmask', 'enum'] -%}
        Int
        {%- if default_value %}
            {%- for value in type.values if value.name.name == default_value %}
                = {{ type.name.CamelCase() }}.{{ as_ktName(value.name.CamelCase()) }}
            {%- endfor %}
        {%- endif %}
    {%- elif type.name.get() == 'bool' -%}
        Boolean{{ '?' if optional }}{% if default_value %} = {{ default_value }}{% endif %}
    {%- elif type.name.get() == 'float' -%}
        Float{{ '?' if optional }}{% if default_value %} =
        {{ 'Float.NaN' if default_value == 'NAN' else default_value or '0.0f' }}{% endif %}
    {%- elif type.name.get() == 'double' -%}
        Double{{ '?' if optional }}{% if default_value %} =
        {{ 'Double.NaN' if default_value == 'NAN' else default_value or '0.0' }}{% endif %}
    {%- elif type.name.get() in ['int8_t', 'uint8_t'] -%}
        Byte{{ '?' if optional }}{% if default_value %} = {{ default_value }}{% endif %}
    {%- elif type.name.get() in ['int16_t', 'uint16_t'] -%}
        Short{{ '?' if optional }}{% if default_value %} = {{ default_value }}{% endif %}
    {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
        Int
        {%- if default_value -%}
            {%- if default_value is string and default_value.startswith('WGPU_') -%}
                = {{ 'Constants.' + default_value | replace('WGPU_', '') }}
            {%- elif default_value == 'nullptr' -%}
                ? = null
            {%- elif default_value == '0xFFFFFFFF' -%}
                = -0x7FFFFFFF
            {%- else -%}
                = {{ default_value }}
            {%- endif %}
        {% endif %}
    {%- elif type.name.get() in ['int64_t', 'uint64_t', 'size_t'] -%}
        Long
        {%- if default_value %}
            {%- if default_value is string and default_value.startswith('WGPU_') -%}
                = {{ 'Constants.' + default_value | replace('WGPU_', '') }}
            {%- elif default_value == 'nullptr' -%}
                ? = null
            {%- elif default_value == '0xFFFFFFFFFFFFFFFF' -%}
                = -0x7FFFFFFFFFFFFFFF
            {%- else -%}
                = {{ default_value }}
            {%- endif %}
        {% endif %}
    {%- elif type.name.get() == 'void' %}
        {{- 'Long' if arg.annotation == '*' else 'Unit' }}
    {%- else -%}
        {{ unreachable_code() }}
    {%- endif %}
{% endmacro %}
