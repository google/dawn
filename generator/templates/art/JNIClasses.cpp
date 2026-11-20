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

#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(__ANDROID__)
#include <android/set_abort_message.h>
#endif

#include "src/utils/assert.h"
#include "src/utils/log.h"

namespace dawn::kotlin_api {
namespace {

template <typename T = jobject>
class ScopedLocalRef {
  public:
    ScopedLocalRef() = default;
    ScopedLocalRef(JNIEnv* env, T ref) : mEnv(env), mRef(ref) {}
    ~ScopedLocalRef() { Reset(); }

    ScopedLocalRef(const ScopedLocalRef&) = delete;
    ScopedLocalRef& operator=(const ScopedLocalRef&) = delete;

    ScopedLocalRef(ScopedLocalRef&& other) noexcept : mEnv(other.mEnv), mRef(other.Release()) {}
    ScopedLocalRef& operator=(ScopedLocalRef&& other) noexcept {
        if (this != &other) {
            Reset();
            mEnv = other.mEnv;
            mRef = other.Release();
        }
        return *this;
    }

    void Reset(JNIEnv* env = nullptr, T ref = nullptr) {
        if (mRef != nullptr) {
            mEnv->DeleteLocalRef(mRef);
        }
        mEnv = env;
        mRef = ref;
    }

    T Get() const { return mRef; }
    explicit operator bool() const { return mRef != nullptr; }
    T Release() noexcept {
        T ref = mRef;
        mRef = nullptr;
        return ref;
    }

  private:
    JNIEnv* mEnv = nullptr;
    T mRef = nullptr;
};

enum class UncaughtDispatchStatus {
    kHandled,
    kUnhandled,
    kHandlerThrew,
};

struct UncaughtDispatchResult {
    UncaughtDispatchStatus status = UncaughtDispatchStatus::kUnhandled;
    ScopedLocalRef<jthrowable> handlerFailure;
};

bool LogAndClearPendingException(JNIEnv* env) {
    if (!env->ExceptionCheck()) {
        return false;
    }
    env->ExceptionDescribe();
    env->ExceptionClear();
    return true;
}

void LogThrowable(JNIEnv* env, jthrowable throwable) {
    if (throwable != nullptr && env->Throw(throwable) == JNI_OK) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

std::string DescribeThrowable(JNIEnv* env, jmethodID objectToString, jthrowable throwable) {
    auto rawDescription = static_cast<jstring>(env->CallObjectMethod(throwable, objectToString));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return "<Throwable.toString() failed>";
    }
    ScopedLocalRef<jstring> description(env, rawDescription);
    if (!description) {
        return "<Throwable.toString() returned null>";
    }
    const char* utfChars = env->GetStringUTFChars(description.Get(), nullptr);
    if (utfChars == nullptr) {
        LogAndClearPendingException(env);
        return "<GetStringUTFChars failed>";
    }
    std::string result(utfChars);
    env->ReleaseStringUTFChars(description.Get(), utfChars);
    return result;
}

UncaughtDispatchResult TryDispatchUncaughtException(JNIEnv* env,
                                                    const JNIClasses& classes,
                                                    jthrowable pending) {
    jobject rawThread =
        env->CallStaticObjectMethod(classes.javaThread, classes.threadCurrentThread);
    if (LogAndClearPendingException(env)) {
        return {};
    }
    ScopedLocalRef<jobject> thread(env, rawThread);
    if (!thread) {
        return {};
    }

    jobject rawHandler =
        env->CallObjectMethod(thread.Get(), classes.threadGetUncaughtExceptionHandler);
    if (LogAndClearPendingException(env)) {
        return {};
    }
    ScopedLocalRef<jobject> handler(env, rawHandler);

    bool useDefaultHandler = !handler;
    if (!useDefaultHandler) {
        jobject rawThreadGroup = env->CallObjectMethod(thread.Get(), classes.threadGetThreadGroup);
        if (LogAndClearPendingException(env)) {
            return {};
        }
        ScopedLocalRef<jobject> threadGroup(env, rawThreadGroup);
        useDefaultHandler =
            threadGroup && env->IsSameObject(handler.Get(), threadGroup.Get()) == JNI_TRUE;
    }

    if (useDefaultHandler) {
        handler.Reset();
        jobject rawDefaultHandler = env->CallStaticObjectMethod(
            classes.javaThread, classes.threadGetDefaultUncaughtExceptionHandler);
        if (LogAndClearPendingException(env)) {
            return {};
        }
        handler.Reset(env, rawDefaultHandler);
        if (!handler) {
            return {};
        }
    }

    // Invoking uncaughtException() directly bypasses Thread.dispatchUncaughtException(), so
    // Android's uncaughtExceptionPreHandler does not run unless the handler itself invokes it
    // (as Android's RuntimeInit.KillApplicationHandler does via ensureLogging()).
    env->CallVoidMethod(handler.Get(), classes.uncaughtExceptionHandlerUncaughtException,
                        thread.Get(), pending);
    if (jthrowable rawFailure = env->ExceptionOccurred()) {
        env->ExceptionClear();
        return {UncaughtDispatchStatus::kHandlerThrew,
                ScopedLocalRef<jthrowable>(env, rawFailure)};
    }
    return {UncaughtDispatchStatus::kHandled, {}};
}

}  // namespace

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
    javaIllegalArgumentException = findGlobalClass("java/lang/IllegalArgumentException");
    javaIllegalStateException = findGlobalClass("java/lang/IllegalStateException");
    javaUnsupportedOperationException = findGlobalClass("java/lang/UnsupportedOperationException");
    jclass objectClz = findLocalClass("java/lang/Object");
    objectToString = getMethodID(objectClz, "toString", "()Ljava/lang/String;");
    env->DeleteLocalRef(objectClz);
    javaThread = findGlobalClass("java/lang/Thread");
    threadCurrentThread = getStaticMethodID(javaThread, "currentThread", "()Ljava/lang/Thread;");
    threadGetUncaughtExceptionHandler = getMethodID(javaThread, "getUncaughtExceptionHandler", "()Ljava/lang/Thread$UncaughtExceptionHandler;");
    threadGetThreadGroup = getMethodID(javaThread, "getThreadGroup", "()Ljava/lang/ThreadGroup;");
    threadGetDefaultUncaughtExceptionHandler = getStaticMethodID(
        javaThread, "getDefaultUncaughtExceptionHandler",
        "()Ljava/lang/Thread$UncaughtExceptionHandler;");
    jclass uncaughtExceptionHandlerClz = findLocalClass("java/lang/Thread$UncaughtExceptionHandler");
    uncaughtExceptionHandlerUncaughtException = getMethodID(uncaughtExceptionHandlerClz, "uncaughtException", "(Ljava/lang/Thread;Ljava/lang/Throwable;)V");
    env->DeleteLocalRef(uncaughtExceptionHandlerClz);
    jclass parcelFileDescriptorClz = findLocalClass("android/os/ParcelFileDescriptor");
    pfdGetFd = getMethodID(parcelFileDescriptorClz, "getFd", "()I");
    env->DeleteLocalRef(parcelFileDescriptorClz);
}

void JNIClasses::DispatchPendingException(JNIEnv* env) const {
    ScopedLocalRef<jthrowable> pending(env, env->ExceptionOccurred());
    if (!pending) {
        return;
    }
    env->ExceptionClear();

    UncaughtDispatchResult dispatch = TryDispatchUncaughtException(env, *this, pending.Get());
    if (dispatch.status == UncaughtDispatchStatus::kHandled) {
        return;
    }

    // If no UncaughtExceptionHandler is configured or dispatching to the handler failed, log the
    // exception and abort natively rather than continuing on a faulted callback worker thread.
    LogThrowable(env, pending.Get());

    std::string abortMessage = "Uncaught exception in WebGPU callback: " +
                               DescribeThrowable(env, objectToString, pending.Get());
    if (dispatch.status == UncaughtDispatchStatus::kHandlerThrew && dispatch.handlerFailure) {
        LogThrowable(env, dispatch.handlerFailure.Get());
        abortMessage +=
            " (UncaughtExceptionHandler threw: " +
            DescribeThrowable(env, objectToString, dispatch.handlerFailure.Get()) + ")";
    }
    dispatch.handlerFailure.Reset();
    pending.Reset();

    dawn::ErrorLog() << abortMessage;
#if defined(__ANDROID__)
    // Write the exact abort line to stderr for the dalvikvm subprocess tests in AsyncHelperTest,
    // which capture stdout/stderr and assert on this string (dawn::ErrorLog writes only to logcat
    // on Android).
    std::fprintf(stderr, "%s\n", abortMessage.c_str());
    std::fflush(stderr);
    android_set_abort_message(abortMessage.c_str());
#endif
    std::abort();
}

}  // namespace dawn::kotlin_api
