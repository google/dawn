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
#include <jni.h>
#include "dawn/webgpu.h"
#include "structures.h"

// Converts Kotlin objects representing Dawn structures into native structures that can be passed
// into the native Dawn API.

namespace dawn::kotlin_api {

{% for structure in by_category['structure'] %}
    void Convert(JNIEnv *env, jobject obj, {{ as_cType(structure.name) }}* converted) {
        jclass clz = env->FindClass("{{ jni_name(structure) }}");

        //* Convert each member in turn from corresponding members of the Kotlin object obtained via
        //* JNI calls
        {% for member in structure.members if include_structure_member(structure, member) %}
            {% if member.length == 'strlen' %} {
                jobject mObj = env->CallObjectMethod(obj,
                        env->GetMethodID(clz, "get{{ member.name.CamelCase() }}",
                                "()Ljava/lang/String;"));
                if (mObj) {
                    //* TODO(b/330293719): free associated resources.
                    converted->{{ member.name.camelCase() }} =
                            env->GetStringUTFChars(reinterpret_cast<jstring>(mObj), 0);
                }
            }
            {% elif member.constant_length == 1 %} {
                {% if member.type.category == 'structure' %}
                    //* Convert optional structure if present.
                    jobject mObj = env->CallObjectMethod(obj,
                            env->GetMethodID(clz, "get{{ member.name.CamelCase() }}",
                                    "()L{{ jni_name(member.type) }};"));
                    if (mObj) {
                        //* TODO(b/330293719): free associated resources.
                        auto convertedMember = new {{ as_cType(member.type.name) }}();
                        Convert(env, mObj, convertedMember);
                        converted->{{ member.name.camelCase() }} = convertedMember;
                    }
                {% elif member.type.name.get() == 'void' %}
                    converted->{{ member.name.camelCase() }} = reinterpret_cast<void*>(
                            env->CallLongMethod(obj, env->GetMethodID(
                                    clz, "get{{ member.name.CamelCase() }}", "()J")));
                {% else %}
                    {{ unreachable_code() }}
                {% endif %}
            }

            {% elif member.length %}
                {% if member.constant_length %}
                    {{ unreachable_code() }}
                {% endif %}
                //* Convert container, including the length field.
                {% if member.type.name.get() == 'uint32_t' %} {
                    //* This container type is represented in Kotlin as a primitive array.
                    jintArray array = static_cast<jintArray>(env->CallObjectMethod(obj,
                            env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()[I")));
                    //* TODO(b/330293719): free associated resources.
                    converted->{{ member.name.camelCase() }} =
                            reinterpret_cast<{{ as_cType(member.type.name) }}*>(
                                   env->GetIntArrayElements(array, 0));
                    converted->{{ member.length.name.camelCase() }} = env->GetArrayLength(array);
                }
                {% else %} {
                    //* These container types are represented in Kotlin as arrays of objects.
                    jmethodID getMethod = env->GetMethodID(clz,
                            "get{{ member.name.CamelCase() }}", "()[L{{ jni_name(member.type) }};");
                    auto in = static_cast<jobjectArray>(env->CallObjectMethod(obj, getMethod));
                    size_t length = env->GetArrayLength(in);
                    //* TODO(b/330293719): free associated resources.
                    auto out = new {{ as_cType(member.type.name) }}[length]();
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
                            Convert(env, env->GetObjectArrayElement(in, idx), out + idx);
                        }
                    {% else %}
                        {{ unreachable_code() }}
                    {% endif %}
                    converted->{{ member.name.camelCase() }} = out;
                    converted->{{ member.length.name.camelCase() }} = length;
                }
                {% endif %}

            //* From here members are single values.
            {% elif member.type.category == 'object' %} {
                jobject mObj = env->CallObjectMethod(obj, env->GetMethodID(clz,
                        "get{{ member.name.CamelCase() }}", "()L{{ jni_name(member.type) }};"));
                if (mObj) {
                    jclass memberClass = env->FindClass("{{ jni_name(member.type) }}");
                    jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                    converted->{{ member.name.camelCase() }} =
                            reinterpret_cast<{{ as_cType(member.type.name) }}>(
                                    env->CallLongMethod(mObj, getHandle));
                }
            }
            {% elif member.type.category == 'structure' or member.type.category == 'callback info' %}
                //* Mandatory structure.
                Convert(env, env->CallObjectMethod(obj,
                        env->GetMethodID(clz, "get{{ member.name.CamelCase() }}",
                                "()L{{ jni_name(member.type) }};")),
                                &converted->{{ member.name.camelCase() }});
            {% elif member.type.name.get() == 'bool' %}
                converted->{{ member.name.camelCase() }} = env->CallBooleanMethod(obj,
                        env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()Z"));
            {% elif member.type.name.get() == 'uint16_t' %}
                converted->{{ member.name.camelCase() }} =
                        static_cast<{{ as_cType(member.type.name) }}>(env->CallShortMethod(obj,
                                env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()S")));
            {% elif member.type.name.get() in ['int', 'int32_t', 'uint32_t']
                    or member.type.category in ['bitmask', 'enum'] %}
                converted->{{ member.name.camelCase() }} =
                        static_cast<{{ as_cType(member.type.name) }}>(env->CallIntMethod(obj,
                                env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()I")));
            {% elif member.type.name.get() == 'float' %}
                converted->{{ member.name.camelCase() }} = env->CallFloatMethod(obj,
                        env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()F"));
            {% elif member.type.name.get() in ['size_t', 'uint64_t'] %}
                converted->{{ member.name.camelCase() }} =
                        static_cast<{{ as_cType(member.type.name) }}>(env->CallLongMethod(obj,
                                env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()J")));
            {% elif member.type.name.get() == 'double' %}
                converted->{{ member.name.camelCase() }} = env->CallDoubleMethod(obj,
                        env->GetMethodID(clz, "get{{ member.name.CamelCase() }}", "()D"));
            {% else %}
                {{ unreachable_code() }}
            {% endif %}
        {% endfor %}

        //* Set up the chain type and links for child objects.
        {% if structure.chained %}
            converted->chain = {.sType = WGPUSType_{{ structure.name.CamelCase() }}};
        {% endif %}

        {% for child in chain_children[structure.name.get()] %} {
            jobject child = env->CallObjectMethod(obj,
                    env->GetMethodID(clz, "get{{ child.name.CamelCase() }}",
                            "()L{{ jni_name(child) }};"));
            if (child) {
                //* TODO(b/330293719): free associated resources.
                auto out = new {{ as_cType(child.name) }}();
                Convert(env, child, out);
                out->chain.next = converted->nextInChain;
                converted->nextInChain = &out->chain;
            }
        }
        {% endfor %}
    }
{% endfor %}

}  // namespace dawn::kotlin_api
