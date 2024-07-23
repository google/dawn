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
{% from 'art/api_jni_types.kt' import arg_to_jni_type, convert_to_kotlin, jni_signature, to_jni_type with context %}
#include <jni.h>
#include <stdlib.h>

#include "dawn/webgpu.h"
#include "JNIContext.h"
#include "structures.h"

// Converts every method call from the Kotlin-hosted version to the native version, performing
// the necessary JNI calls to fetch data, and converting the data to the correct form.

#define DEFAULT __attribute__((visibility("default")))

namespace dawn::kotlin_api {

struct UserData {
    JNIEnv *env;
    jobject callback;
};

jobject toByteBuffer(JNIEnv *env, const void* address, jlong size) {
    if (!address) {
        return nullptr;
    }
    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");

    //* Dawn always uses little endian format, so we pre-convert for the client's convenience.
    jclass byteOrderClass = env->FindClass("java/nio/ByteOrder");
    jobject littleEndian = env->NewGlobalRef(env->GetStaticObjectField(
            byteOrderClass, env->GetStaticFieldID(byteOrderClass, "LITTLE_ENDIAN",
                                                  "Ljava/nio/ByteOrder;")));

    jobject byteBuffer = env->NewDirectByteBuffer(const_cast<void *>(address), size);

    env->CallObjectMethod(
            byteBuffer, env->GetMethodID(byteBufferClass, "order",
                                         "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;"),
            littleEndian);
    return byteBuffer;
}

{% macro render_method(method, object) %}
    {% set _kotlin_return = kotlin_return(method) %}
    //*  A JNI-external method is built with the JNI signature expected to match the host Kotlin.
    DEFAULT extern "C"
    {{ arg_to_jni_type(_kotlin_return) }} Java_{{ kotlin_package.replace('.', '_') }}_
            {{- object.name.CamelCase() if object else 'FunctionsKt' -}}
            _{{ method.name.camelCase() }}(JNIEnv *env
                    {{ ', jobject obj' if object else ', jclass clazz' -}}

    //* Make the signature for each argument in turn.
    {% for arg in kotlin_record_members(method.arguments) %},
        {{ arg_to_jni_type(arg) }} _{{ as_varName(arg.name) }}
    {% endfor %}) {

    // * Helper context for the duration of this method call.
    JNIContext c(env);

    //*  A variable is declared for each parameter of the native method.
    {% for arg in method.arguments %}
        {{ as_annotated_cType(arg) }};
    {% endfor %}

    //* Each parameter is converted from the JNI parameter to the expected form of the native
    //* parameter.
    {% for arg in kotlin_record_members(method.arguments) %}
        {% if arg.length == 'strlen' %}
            if (_{{ as_varName(arg.name) }}) {  //* Don't convert null strings.
                {{ as_varName(arg.name) }} = c.GetStringUTFChars(_{{ as_varName(arg.name) }});
            } else {
                {{ as_varName(arg.name) }} = nullptr;
            }
        {% elif arg.constant_length == 1 %}
            //*  Optional structure.
            {% if arg.type.category == 'structure' %}
                if (_{{ as_varName(arg.name) }}) {
                    auto convertedMember = c.Alloc<{{ as_cType(arg.type.name) }}>();
                    ToNative(&c, env, _{{ as_varName(arg.name) }}, convertedMember);
                    {{ as_varName(arg.name) }} = convertedMember;
                } else {
                    {{ as_varName(arg.name) }} = nullptr;
                }
            {% else %}
                {{ unreachable_code() }}
            {% endif %}
        {% elif arg.length %}
            //*  Container types.
            {% if arg.type.name.get() == 'uint32_t' %}
                {{ as_varName(arg.name) }} =
                        reinterpret_cast<const {{ as_cType(arg.type.name) }}*>(
                               c.GetIntArrayElements(_{{ as_varName(arg.name) }}));
                {{ arg.length.name.camelCase() }} =
                       env->GetArrayLength(_{{ as_varName(arg.name) }});
            {% elif arg.type.name.get() == 'void' %}
                {{ as_varName(arg.name) }} =
                        env->GetDirectBufferAddress(_{{ as_varName(arg.name) }});
                {{ arg.length.name.camelCase() }} =
                        env->GetDirectBufferCapacity(_{{ as_varName(arg.name) }});
            {% else %} {
                size_t length = env->GetArrayLength(_{{ as_varName(arg.name) }});
                auto out = c.AllocArray<{{ as_cType(arg.type.name) }}>(length);
                {% if arg.type.category in ['bitmask', 'enum'] %} {
                    jclass memberClass = env->FindClass("{{ jni_name(arg.type) }}");
                    jmethodID getValue = env->GetMethodID(memberClass, "getValue", "()I");
                    for (int idx = 0; idx != length; idx++) {
                        jobject element =
                                env->GetObjectArrayElement(_{{ as_varName(arg.name) }}, idx);
                        out[idx] = static_cast<{{ as_cType(arg.type.name) }}>(
                                env->CallIntMethod(element, getValue));
                    }
                } {% elif arg.type.category == 'object' %} {
                    jclass memberClass = env->FindClass("{{ jni_name(arg.type) }}");
                    jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                    for (int idx = 0; idx != length; idx++) {
                        jobject element =
                                env->GetObjectArrayElement(_{{ as_varName(arg.name) }}, idx);
                        out[idx] = reinterpret_cast<{{ as_cType(arg.type.name) }}>(
                                env->CallLongMethod(element, getHandle));
                    }
                } {% else %}
                    {{ unreachable_code() }}
                {% endif %}
                {{ as_varName(arg.name) }} = out;
                {{ arg.length.name.camelCase() }} = length;
            } {% endif %}

        //*  Single value types.
        {% elif arg.type.category == 'object' %}
            if (_{{ as_varName(arg.name) }}) {
                jclass memberClass = env->FindClass("{{ jni_name(arg.type) }}");
                jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                {{ as_varName(arg.name) }} =
                        reinterpret_cast<{{ as_cType(arg.type.name) }}>(
                                env->CallLongMethod(_{{ as_varName(arg.name) }}, getHandle));
            } else {
                {{ as_varName(arg.name) }} = nullptr;
            }
        {% elif arg.type.name.get() in ['int32_t', 'size_t', 'uint32_t', 'uint64_t'] or arg.type.category in ['bitmask', 'enum'] %}
            {{ as_varName(arg.name) }} =
                    static_cast<{{ as_cType(arg.type.name) }}>(_{{ as_varName(arg.name) }});
        {% elif arg.type.name.get() in ['float', 'int'] %}
            {{ as_varName(arg.name) }} = _{{ as_varName(arg.name) }};

        {% elif arg.type.category == 'function pointer' %} {
            //* Function pointers themselves require each argument converting.
            //* A custom native callback is generated to wrap the Kotlin callback.
            {{ as_varName(arg.name) }} = [](
                {%- for callbackArg in arg.type.arguments %}
                    {{ as_annotated_cType(callbackArg) }}{{ ',' if not loop.last }}
                {%- endfor %}) {
                UserData* userData1 = static_cast<UserData *>(userdata);
                JNIEnv *env = userData1->env;
                if (env->ExceptionCheck()) {
                    return;
                }

                {%- for callbackArg in kotlin_record_members(arg.type.arguments) -%}
                    {{ convert_to_kotlin(callbackArg.name.camelCase(),
                                         '_' + callbackArg.name.camelCase(),
                                         'input->' + callbackArg.length.name.camelCase() if callbackArg.length.name,
                                         callbackArg) }}
                {% endfor %}

                //* Get the client (Kotlin) callback so we can call it.
                jmethodID callbackMethod = env->GetMethodID(
                        env->FindClass("{{ jni_name(arg.type) }}"), "callback", "(
                    {%- for callbackArg in kotlin_record_members(arg.type.arguments) -%}
                        {{- jni_signature(callbackArg) -}}
                    {%- endfor %})V");

                //* Call the callback with all converted parameters.
                env->CallVoidMethod(userData1->callback, callbackMethod
                {%- for callbackArg in kotlin_record_members(arg.type.arguments) %}
                    ,_{{ callbackArg.name.camelCase() }}
                {%- endfor %});
            };
            //* TODO(b/330293719): free associated resources.
            userdata = new UserData(
                    {.env = env, .callback = env->NewGlobalRef(_{{ as_varName(arg.name) }})});
        } {% else %}
            {{ unreachable_code() }}
        {% endif %}
    {% endfor %}

    {% if object %}
        jclass memberClass = env->FindClass("{{ jni_name(object) }}");
        jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
        auto handle =
                reinterpret_cast<{{ as_cType(object.name) }}>(env->CallLongMethod(obj, getHandle));
    {% endif %}

    //* Actually invoke the native version of the method.
    {% if _kotlin_return.length == 'size_t' %}
        //* Methods that return containers are converted from two-call to single call, and the
        //* return type is switched to a Kotlin container.
        size_t size = wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%},
                //* The replaced output parameter is set to nullptr on the first call.
                {{ 'nullptr' if arg.annotation == '*' else as_varName(arg.name) -}}
            {% endfor %}
        );
        //* Allocate the native container
        auto returnAllocation = std::make_unique<{{ as_cType(_kotlin_return.type.name) }}[]>(size);
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
        //* Second call completes the native container
        wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%}
                {{- ', ' if object or not loop.first -}}
                {{- 'returnAllocation.get()' if arg == _kotlin_return else as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
    {% else %}
        {% if _kotlin_return.annotation == '*' %}
            //* Make a native container to accept the data output via parameter.
            {{ as_cType(_kotlin_return.type.name) }} out;
            {{ _kotlin_return.name.get() }} = &out;
        {% endif %}
        {{ 'auto result =' if _kotlin_return.type.name.get() != 'void' }}
        {% if object %}
            wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
        {% else %}
            wgpu{{ method.name.CamelCase() }}(
        {% endif %}
            {% for arg in method.arguments -%}
                {{- ',' if object or not loop.first }}{{ as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return {{ '0' if method.return_type.name.get() != 'void' }};
        }
    {% endif %}
    {% if _kotlin_return.type.name.get() != 'void' %}
        {{ convert_to_kotlin(_kotlin_return.name.get() if _kotlin_return.annotation == '*' else 'result',
                             'result_kt',
                             'size' if _kotlin_return.type.name.get() in ['void const *', 'void *'] or _kotlin_return.length == 'size_t',
                             _kotlin_return) }}
        return result_kt;
    {% endif %}
} {% endmacro %}

{% for obj in by_category['object'] %}
    {% for method in obj.methods if include_method(method) %}
        {{ render_method(method, obj) }}
    {% endfor %}

    //* Every object gets a Release method, to supply a Kotlin AutoCloseable.
    extern "C"
    JNIEXPORT void JNICALL
    Java_{{ kotlin_package.replace('.', '_') }}_{{ obj.name.CamelCase() }}_close(
            JNIEnv *env, jobject obj) {
        jclass clz = env->FindClass("{{ jni_name(obj) }}");
        const {{ as_cType(obj.name) }} handle = reinterpret_cast<{{ as_cType(obj.name) }}>(
                env->CallLongMethod(obj, env->GetMethodID(clz, "getHandle", "()J")));
        wgpu{{ obj.name.CamelCase() }}Release(handle);
    }
{% endfor %}

//* Global functions don't have an associated class.
{% for function in by_category['function'] if include_method(function) %}
    {{ render_method(function, None) }}
{% endfor %}

}  // namespace dawn::kotlin_api
