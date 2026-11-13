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
{% from 'art/api_jni_types.cpp' import jni_signature with context %}
#include "JNIClasses.h"

#include "src/utils/assert.h"
#include "src/utils/log.h"

namespace dawn::kotlin_api {
JNIClasses* JNIClasses::getInstance(JNIEnv* env) {
  static JNIClasses instance(env);
  return &instance;
}

JNIClasses::JNIClasses(JNIEnv* env) {
    auto check = [env](auto result, const char* name, const char* sig = "") {
        if (!result || env->ExceptionCheck()) {
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
            }
            dawn::ErrorLog() << "JNIClasses: missing " << name << " " << sig;
            DAWN_CHECK(false);
        }
        return result;
    };
    auto findLocalClass = [env, &check](const char* name) -> jclass {
        return check(env->FindClass(name), name);
    };
    auto findGlobalClass = [env, &check, &findLocalClass](const char* name) -> jclass {
        jclass local = findLocalClass(name);
        jclass global = reinterpret_cast<jclass>(env->NewGlobalRef(local));
        env->DeleteLocalRef(local);
        return check(global, name);
    };
    auto getMethodID = [env, &check](jclass clz, const char* name, const char* sig) -> jmethodID {
        return check(env->GetMethodID(clz, name, sig), name, sig);
    };
    auto getStaticMethodID = [env, &check](jclass clz, const char* name, const char* sig) -> jmethodID {
        return check(env->GetStaticMethodID(clz, name, sig), name, sig);
    };
    auto getFieldID = [env, &check](jclass clz, const char* name, const char* sig) -> jfieldID {
        return check(env->GetFieldID(clz, name, sig), name, sig);
    };
    {% for entity in has_kotlin_classes %}
        {% if entity.category != 'structure' %}
            {{ entity.name.camelCase() }} = findGlobalClass("{{ jni_name(entity, entity.category) }}");
            {% if entity.category == 'callback function' %}
                {{ entity.name.camelCase() }}Runnable = findGlobalClass("{{ jni_name(entity) }}Runnable");
            {% elif entity.category == 'object' %}
                {{ entity.name.camelCase() }}Handle = getFieldID({{ entity.name.camelCase() }}, "handle", "J");
                {{ entity.name.camelCase() }}Ctor = getMethodID({{ entity.name.camelCase() }}, "<init>", "(J)V");
            {% endif %}
        {% else %}
            {% set _k2n_members = (kotlin_record_members(entity.members) | list) if entity.needs_k2n else [] %}
            {% set _k2n_children = chain_children[entity.name.get()] if entity.needs_k2n else [] %}
            {% set _has_extra_fields = entity.name.get() in ['origin 2D', 'extent 2D', 'color space dawn'] %}
            {% set _needs_local_clz = (not entity.needs_n2k) and ((_k2n_members | length > 0) or (_k2n_children | length > 0) or _has_extra_fields) %}
            {% if entity.needs_n2k %}
                {{ entity.name.camelCase() }} = findGlobalClass("{{ jni_name(entity, entity.category) }}");
                {{ entity.name.camelCase() }}Ctor = getMethodID({{ entity.name.camelCase() }}, "<init>", "(
                {%- for member in kotlin_record_members(entity.members) %}
                    {{- jni_signature(member) -}}
                {%- endfor -%}
                {%- for structure in chain_children[entity.name.get()] -%}
                    {{- jni_signature({'type': structure}) -}}
                {%- endfor %})V");
            {% elif _needs_local_clz %}
                jclass {{ entity.name.camelCase() }} = findLocalClass("{{ jni_name(entity, entity.category) }}");
            {% endif %}
            {% for member in _k2n_members %}
                {% set prefix = "is" if member.type.name.get() == "bool" else "get" %}
                {{ entity.name.camelCase() }}Get{{ member.name.CamelCase() }} = getMethodID({{ entity.name.camelCase() }}, "{{prefix}}{{member.name.CamelCase()}}", "(){{jni_signature(member)}}");
            {% endfor %}
            {% for child in _k2n_children %}
                {{ entity.name.camelCase() }}Get{{ child.name.CamelCase() }} = getMethodID({{ entity.name.camelCase() }}, "get{{ child.name.CamelCase() }}", "()L{{ jni_name(child) }};");
            {% endfor %}
            {% if entity.name.get() == 'origin 2D' %}
                originX = getFieldID(origin2D, "x", "I");
                originY = getFieldID(origin2D, "y", "I");
            {% elif entity.name.get() == 'extent 2D' %}
                extentWidth = getFieldID(extent2D, "width", "I");
                extentHeight = getFieldID(extent2D, "height", "I");
            {% elif entity.name.get() == 'color space dawn' %}
                colorSpacePrimaries = getFieldID(colorSpaceDawn, "primaries", "I");
                colorSpaceTransfer = getFieldID(colorSpaceDawn, "transfer", "I");
                colorSpaceYCbCrRange = getFieldID(colorSpaceDawn, "yCbCrRange", "I");
                colorSpaceYCbCrMatrix = getFieldID(colorSpaceDawn, "yCbCrMatrix", "I");
            {% endif %}
            {% if _needs_local_clz %}
                env->DeleteLocalRef({{ entity.name.camelCase() }});
            {% endif %}
        {% endif %}
    {% endfor %}
    gpuCallbackRunnable = findGlobalClass("androidx/webgpu/GPURequestCallbackRunnable");
    gpuCallbackErrorTypeRunnable = findGlobalClass("androidx/webgpu/GPURequestCallbackErrorTypeRunnable");
    gpuCallbackVoidRunnable = findGlobalClass("androidx/webgpu/GPURequestCallbackVoidRunnable");
    dawnException = findGlobalClass("androidx/webgpu/DawnException");
    stringClass = findGlobalClass("java/lang/String");
    gpuHardwareBufferExternalTexture = findGlobalClass("androidx/webgpu/GPUHardwareBufferExternalTexture");
    gpuHardwareBufferExternalTextureCtor = getMethodID(gpuHardwareBufferExternalTexture, "<init>", "(JLandroidx/webgpu/GPUExternalTexture;)V");
    gpuHardwareBufferTexture = findGlobalClass("androidx/webgpu/GPUHardwareBufferTexture");
    gpuHardwareBufferTextureCtor = getMethodID(gpuHardwareBufferTexture, "<init>", "(JLandroidx/webgpu/GPUTexture;)V");
    javaIllegalStateException = findGlobalClass("java/lang/IllegalStateException");
    javaUnsupportedOperationException = findGlobalClass("java/lang/UnsupportedOperationException");
    javaThread = findGlobalClass("java/lang/Thread");
    threadCurrentThread = getStaticMethodID(javaThread, "currentThread", "()Ljava/lang/Thread;");
    threadGetUncaughtExceptionHandler = getMethodID(javaThread, "getUncaughtExceptionHandler", "()Ljava/lang/Thread$UncaughtExceptionHandler;");
    jclass uncaughtExceptionHandlerClz = findLocalClass("java/lang/Thread$UncaughtExceptionHandler");
    uncaughtExceptionHandlerUncaughtException = getMethodID(uncaughtExceptionHandlerClz, "uncaughtException", "(Ljava/lang/Thread;Ljava/lang/Throwable;)V");
    env->DeleteLocalRef(uncaughtExceptionHandlerClz);
    jclass parcelFileDescriptorClz = findLocalClass("android/os/ParcelFileDescriptor");
    pfdGetFd = getMethodID(parcelFileDescriptorClz, "getFd", "()I");
    env->DeleteLocalRef(parcelFileDescriptorClz);
}

void JNIClasses::DispatchPendingException(JNIEnv* env) const {
    jthrowable pending = env->ExceptionOccurred();
    if (!pending) {
        return;
    }
    env->ExceptionClear();

    bool dispatched = false;
    jobject thread = env->CallStaticObjectMethod(javaThread, threadCurrentThread);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    } else if (thread != nullptr) {
        jobject handler = env->CallObjectMethod(thread, threadGetUncaughtExceptionHandler);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        } else if (handler != nullptr) {
            env->CallVoidMethod(
                handler, uncaughtExceptionHandlerUncaughtException, thread, pending);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
            } else {
                dispatched = true;
            }
        }
        if (handler != nullptr) {
            env->DeleteLocalRef(handler);
        }
    }

    if (!dispatched) {
        env->Throw(pending);
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    if (thread != nullptr) {
        env->DeleteLocalRef(thread);
    }
    env->DeleteLocalRef(pending);
}

}  // namespace dawn::kotlin_api
