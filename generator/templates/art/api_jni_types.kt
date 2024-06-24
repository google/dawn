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

{% macro arg_to_jni_type(arg) %}
    {% if arg.length == 'strlen' %}
        jstring
    {% elif arg.length and arg.json_data.get('constant_length') != 1 %}
        {% if arg.type.category in ['bitmask', 'enum', 'function pointer', 'object', 'structure'] %}
            jobjectArray
        {% elif arg.type.name.get() == 'void' %}
            jobject
        {% elif arg.type.name.get() == 'uint32_t' %}
            jintArray
        {% else %}
            {{ unreachable_code() }}
        {% endif %}
    {% else %}
        {{ to_jni_type(arg.type) }}
    {% endif %}
{% endmacro %}

{% macro to_jni_type(type) %}
    {% if type.category in ['function pointer', 'object', 'structure'] %}
        jobject
    {% elif type.category in ['bitmask', 'enum'] %}
        jint
    {% else %}
        {{ jni_primitives[type.name.get()] }}
    {% endif %}
{% endmacro %}
