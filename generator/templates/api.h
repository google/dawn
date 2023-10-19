//* Copyright 2020 The Dawn & Tint Authors
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
//*
//*
{% include 'BSD_LICENSE' %}
{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}
#ifndef {{metadata.api.upper()}}_H_
#define {{metadata.api.upper()}}_H_

{% set API = metadata.c_prefix %}
#if defined({{API}}_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined({{API}}_IMPLEMENTATION)
#            define {{API}}_EXPORT __declspec(dllexport)
#        else
#            define {{API}}_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined({{API}}_IMPLEMENTATION)
#            define {{API}}_EXPORT __attribute__((visibility("default")))
#        else
#            define {{API}}_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined({{API}}_SHARED_LIBRARY)
#    define {{API}}_EXPORT
#endif  // defined({{API}}_SHARED_LIBRARY)

#if !defined({{API}}_OBJECT_ATTRIBUTE)
#define {{API}}_OBJECT_ATTRIBUTE
#endif
#if !defined({{API}}_ENUM_ATTRIBUTE)
#define {{API}}_ENUM_ATTRIBUTE
#endif
#if !defined({{API}}_STRUCTURE_ATTRIBUTE)
#define {{API}}_STRUCTURE_ATTRIBUTE
#endif
#if !defined({{API}}_FUNCTION_ATTRIBUTE)
#define {{API}}_FUNCTION_ATTRIBUTE
#endif
#if !defined({{API}}_NULLABLE)
#define {{API}}_NULLABLE
#endif

#include <stdint.h>
#include <stddef.h>

{% for constant in by_category["constant"] %}
    #define {{API}}_{{constant.name.SNAKE_CASE()}} {{constant.value}}
{% endfor %}

typedef uint32_t {{API}}Flags;
typedef uint32_t {{API}}Bool;

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}} {{API}}_OBJECT_ATTRIBUTE;
{% endfor %}

// Structure forward declarations
{% for type in by_category["structure"] %}
    struct {{as_cType(type.name)}};
{% endfor %}

{% for type in by_category["enum"] + by_category["bitmask"] %}
    typedef enum {{as_cType(type.name)}} {
        {% for value in type.values %}
            {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
        {% endfor %}
        {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
    } {{as_cType(type.name)}} {{API}}_ENUM_ATTRIBUTE;
    {% if type.category == "bitmask" %}
        typedef {{API}}Flags {{as_cType(type.name)}}Flags {{API}}_ENUM_ATTRIBUTE;
    {% endif %}

{% endfor -%}
{% for type in by_category["function pointer"] %}
    typedef {{as_cType(type.return_type.name)}} (*{{as_cType(type.name)}})(
        {%- if type.arguments == [] -%}
            void
        {%- else -%}
            {%- for arg in type.arguments -%}
                {% if not loop.first %}, {% endif %}
                {% if arg.type.category == "structure" %}struct {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        {%- endif -%}
    ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

typedef struct {{API}}ChainedStruct {
    struct {{API}}ChainedStruct const * next;
    {{API}}SType sType;
} {{API}}ChainedStruct {{API}}_STRUCTURE_ATTRIBUTE;

typedef struct {{API}}ChainedStructOut {
    struct {{API}}ChainedStructOut * next;
    {{API}}SType sType;
} {{API}}ChainedStructOut {{API}}_STRUCTURE_ATTRIBUTE;

{% for type in by_category["structure"] %}
    {% for root in type.chain_roots %}
        // Can be chained in {{as_cType(root.name)}}
    {% endfor %}
    typedef struct {{as_cType(type.name)}} {
        {% set Out = "Out" if type.output else "" %}
        {% set const = "const " if not type.output else "" %}
        {% if type.extensible %}
            {{API}}ChainedStruct{{Out}} {{const}}* nextInChain;
        {% endif %}
        {% if type.chained %}
            {{API}}ChainedStruct{{Out}} chain;
        {% endif %}
        {% for member in type.members %}
            {% if member.optional %}
                {{API}}_NULLABLE {{as_annotated_cType(member)}};
            {% else %}
                {{as_annotated_cType(member)}};
            {% endif-%}
        {% endfor %}
    } {{as_cType(type.name)}} {{API}}_STRUCTURE_ATTRIBUTE;

{% endfor %}
{% for typeDef in by_category["typedef"] %}
    // {{as_cType(typeDef.name)}} is deprecated.
    // Use {{as_cType(typeDef.type.name)}} instead.
    typedef {{as_cType(typeDef.type.name)}} {{as_cType(typeDef.name)}};

{% endfor %}
#ifdef __cplusplus
extern "C" {
#endif

#if !defined({{API}}_SKIP_PROCS)

{% for function in by_category["function"] %}
    typedef {{as_cType(function.return_type.name)}} (*{{as_cProc(None, function.name)}})(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Procs of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        typedef {{as_cReturnType(method.return_type)}} (*{{as_cProc(type.name, method.name)}})(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                ,{{" "}}
                {%- if arg.optional %}{{API}}_NULLABLE {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
    {% endfor %}

{% endfor %}

#endif  // !defined({{API}}_SKIP_PROCS)

#if !defined({{API}}_SKIP_DECLARATIONS)

{% for function in by_category["function"] %}
    {{API}}_EXPORT {{as_cType(function.return_type.name)}} {{as_cMethod(None, function.name)}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif -%}
                {%- if arg.optional %}{{API}}_NULLABLE {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        {{API}}_EXPORT {{as_cReturnType(method.return_type)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                ,{{" "}}
                {%- if arg.optional %}{{API}}_NULLABLE {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
    {% endfor %}

{% endfor %}

#endif  // !defined({{API}}_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // {{metadata.api.upper()}}_H_
