//* This template itself is part of the Dawn source and follows Dawn's license,
//* which is Apache 2.0.
//*
//* The WebGPU native API is a joint project used by Google, Mozilla, and Apple.
//* It was agreed to use a BSD 3-Clause license so that it is GPLv2-compatible.
//*
//* As a result, the template comments using //* at the top of the file are
//* removed during generation such that the resulting file starts with the
//* BSD 3-Clause comment, which is inside BSD_LICENSE as included below.
//*
//* Copyright 2020 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
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
#include <stdbool.h>

{% for constant in by_category["constant"] %}
    #define {{API}}_{{constant.name.SNAKE_CASE()}} {{constant.value}}
{% endfor %}

typedef uint32_t {{API}}Flags;
#define WGPU_BREAKING_CHANGE_BOOL
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
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
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
