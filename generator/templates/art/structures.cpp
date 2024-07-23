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
{% from 'art/api_jni_types.kt' import convert_to_kotlin, jni_signature with context %}

#include "structures.h"

#include <jni.h>

#include "dawn/common/Assert.h"
#include "dawn/webgpu.h"
#include "JNIContext.h"

// Converts Kotlin objects representing Dawn structures into native structures that can be passed
// into the native Dawn API.

namespace dawn::kotlin_api {

// Special-case noop handling of the two callback info that are part of other structures.
// TODO(352710628) support converting callback info.
void ToNative(JNIContext* c, JNIEnv* env, jobject obj, WGPUDeviceLostCallbackInfo* info) {
    *info = {};
}
void ToNative(JNIContext* c, JNIEnv* env, jobject obj, WGPUUncapturedErrorCallbackInfo* info) {
    *info = {};
}

jobject ToKotlin(JNIEnv *env, const WGPUDeviceLostCallbackInfo* input) {
}

jobject ToKotlin(JNIEnv *env, const WGPUUncapturedErrorCallbackInfo* input) {
}

{% for structure in by_category['structure'] if include_structure(structure) %}
    //* Native -> Kotlin converter.
    //* TODO(b/354411474): Filter the structures for which to add a ToKotlin conversion.
    jobject ToKotlin(JNIEnv *env, const {{ as_cType(structure.name) }}* input) {
        if (!input) {
            return nullptr;
        }
        //* Make a new Kotlin object to receive a copy of the structure.
        jclass clz = env->FindClass("{{ jni_name(structure) }}");

        //* JNI signature needs to be built using the same logic used in the Kotlin structure spec.
        jmethodID ctor = env->GetMethodID(clz, "<init>", "(
        {%- for member in kotlin_record_members(structure.members) %}
            {{- jni_signature(member) -}}
        {%- endfor -%}

        {%- for structure in chain_children[structure.name.get()] -%}
            {{- jni_signature({'type': structure}) -}}
        {%- endfor %})V");

        //* Each field converted using the individual value converter.
        {% for member in kotlin_record_members(structure.members) %}
            {{ convert_to_kotlin('input->' + member.name.camelCase(), member.name.camelCase(),
                                 'input->' + member.length.name.camelCase() if member.length.name,
                                 member) }}
        {% endfor %}

        //* Allow conversion of every child structure.
        {% for structure in chain_children[structure.name.get()] %}
            jobject {{ structure.name.camelCase() }} = nullptr;
        {% endfor %}

        //* Walk the chain to find and convert (recursively) all child structures.
        {% if chain_children[structure.name.get()] %}
            for (const WGPUChainedStruct* child = input->nextInChain;
                    child != nullptr; child = child->next) {
                switch (child->sType) {
                    {% for structure in chain_children[structure.name.get()] %}
                        case WGPUSType_{{ structure.name.CamelCase() }}:
                            {{ structure.name.camelCase() }} = ToKotlin(env,
                                    reinterpret_cast<const {{ as_cType(structure.name) }}*>(child));
                            break;
                    {% endfor %}
                        default:
                            DAWN_UNREACHABLE();
                }
            }
        {% endif %}

        //* Now all the fields are converted, invoke the constructor.
        jobject converted = env->NewObject(clz, ctor
        {%- for member in kotlin_record_members(structure.members) %}
            ,{{ member.name.camelCase() }}
        {% endfor %}
        {%- for structure in chain_children[structure.name.get()] -%}
            ,{{ structure.name.camelCase() }}
        {%- endfor %});
        return converted;
    }

    void ToNative(JNIContext* c, JNIEnv* env, jobject obj, {{ as_cType(structure.name) }}* converted) {
        jclass clz = env->FindClass("{{ jni_name(structure) }}");

        //* Convert each member in turn from corresponding members of the Kotlin object obtained via
        //* JNI calls
        {% for member in kotlin_record_members(structure.members) %} {
            jmethodID method = env->GetMethodID(
                    clz, "get{{ member.name.CamelCase() }}", "(){{ jni_signature(member) }}");
            {% if member.length == 'strlen' %}
                jobject mObj = env->CallObjectMethod(obj, method);
                if (mObj) {
                    converted->{{ member.name.camelCase() }} =
                            c->GetStringUTFChars(reinterpret_cast<jstring>(mObj));
                } else {
                    converted->{{ member.name.camelCase() }} = nullptr;
                }
            {% elif member.constant_length == 1 %}
                {% if member.type.category == 'structure' %}
                    //* Convert optional structure if present.
                    jobject mObj = env->CallObjectMethod(obj, method);
                    if (mObj) {
                        auto convertedMember = c->Alloc<{{ as_cType(member.type.name) }}>();
                        ToNative(c, env, mObj, convertedMember);
                        converted->{{ member.name.camelCase() }} = convertedMember;
                    }
                {% elif member.type.name.get() == 'void' %}
                    converted->{{ member.name.camelCase() }} =
                            reinterpret_cast<void*>(env->CallLongMethod(obj, method));
                {% else %}
                    {{ unreachable_code() }}
                {% endif %}

            {% elif member.length %}
                {% if member.constant_length %}
                    {{ unreachable_code() }}
                {% endif %}
                //* Convert container, including the length field.
                {% if member.type.name.get() == 'uint32_t' %} {
                    //* This container type is represented in Kotlin as a primitive array.
                    jintArray array = static_cast<jintArray>(env->CallObjectMethod(obj, method));
                    converted->{{ member.name.camelCase() }} =
                            reinterpret_cast<const {{ as_cType(member.type.name) }}*>(
                                   c->GetIntArrayElements(array));
                    converted->{{ member.length.name.camelCase() }} = env->GetArrayLength(array);
                }
                {% else %} {
                    //* These container types are represented in Kotlin as arrays of objects.
                    auto in = static_cast<jobjectArray>(env->CallObjectMethod(obj, method));
                    size_t length = env->GetArrayLength(in);
                    auto out = c->AllocArray<{{ as_cType(member.type.name) }}>(length);
                    {% if member.type.category in ['bitmask', 'enum'] %} {
                        jclass memberClass = env->FindClass("{{ jni_name(member.type) }}");
                        jmethodID getValue = env->GetMethodID(memberClass, "getValue", "()I");
                        for (int idx = 0; idx != length; idx++) {
                            jobject element = env->GetObjectArrayElement(in, idx);
                            out[idx] = static_cast<{{ as_cType(member.type.name) }}>(
                                    env->CallIntMethod(element, getValue));
                        }
                    }
                    {% elif member.type.category == 'object' %} {
                        jclass memberClass = env->FindClass("{{ jni_name(member.type) }}");
                        jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                        for (int idx = 0; idx != length; idx++) {
                            jobject element = env->GetObjectArrayElement(in, idx);
                            out[idx] = reinterpret_cast<{{ as_cType(member.type.name) }}>(
                                    env->CallLongMethod(element, getHandle));
                        }
                    }
                    {% elif member.type.category == 'structure' %}
                        for (int idx = 0; idx != length; idx++) {
                            ToNative(c, env, env->GetObjectArrayElement(in, idx), out + idx);
                        }
                    {% else %}
                        {{ unreachable_code() }}
                    {% endif %}
                    converted->{{ member.name.camelCase() }} = out;
                    converted->{{ member.length.name.camelCase() }} = length;
                }
                {% endif %}

            //* From here members are single values.
            {% elif member.type.category == 'object' %}
                jobject mObj = env->CallObjectMethod(obj, method);
                if (mObj) {
                    jclass memberClass = env->FindClass("{{ jni_name(member.type) }}");
                    jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                    converted->{{ member.name.camelCase() }} =
                            reinterpret_cast<{{ as_cType(member.type.name) }}>(
                                    env->CallLongMethod(mObj, getHandle));
                } else {
                    converted->{{ member.name.camelCase() }} = nullptr;
                }
            {% else %}
                {% if member.type.category == 'structure' or member.type.category == 'callback info' %}
                    //* Mandatory structure.
                    ToNative(c, env, env->CallObjectMethod(obj, method),
                            &converted->{{ member.name.camelCase() }});
                {% elif member.type.name.get() == 'void *' %}
                    converted->{{ member.name.camelCase() }} =
                        reinterpret_cast<void*>(static_cast<intptr_t>(env->CallLongMethod(obj, method)));
                {% else %}
                    converted->{{ member.name.camelCase() }} =
                            static_cast<{{ as_cType(member.type.name) }}>(
                    {% if member.type.name.get() == 'bool' %}
                        env->CallBooleanMethod
                    {% elif member.type.name.get() == 'uint16_t' %}
                        env->CallShortMethod
                    {% elif member.type.name.get() in ['int', 'int32_t', 'uint32_t']
                            or member.type.category in ['bitmask', 'enum'] %}
                        env->CallIntMethod
                    {% elif member.type.name.get() == 'float' %}
                        env->CallFloatMethod
                    {% elif member.type.name.get() in ['size_t', 'uint64_t'] %}
                        env->CallLongMethod
                    {% elif member.type.name.get() == 'double' %}
                        env->CallDoubleMethod
                    {% else %}
                        {{ unreachable_code() }}
                    {% endif %} (obj, method));
                {% endif %}
            {% endif %}
        } {% endfor %}

        //* Set up the chain type and links for child objects.
        {% if structure.chained %}
            converted->chain = {.sType = WGPUSType_{{ structure.name.CamelCase() }}};
        {% endif %}

        {% for child in chain_children[structure.name.get()] %} {
            jobject child = env->CallObjectMethod(obj,
                    env->GetMethodID(clz, "get{{ child.name.CamelCase() }}",
                            "()L{{ jni_name(child) }};"));
            if (child) {
                auto out = c->Alloc<{{ as_cType(child.name) }}>();
                ToNative(c, env, child, out);
                out->chain.next = converted->nextInChain;
                converted->nextInChain = &out->chain;
            }
        }
        {% endfor %}
    }
{% endfor %}

}  // namespace dawn::kotlin_api
