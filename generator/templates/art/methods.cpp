// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
{% from 'art/kotlin_record_conversion.cpp' import define_kotlin_record_structure, define_kotlin_to_struct_conversion with context %}
{% from 'art/api_jni_types.cpp' import arg_to_jni_type, convert_to_kotlin, jni_signature, to_jni_type with context %}
#include <jni.h>
#include <stdlib.h>
#include <webgpu/webgpu.h>
#include <memory>
#include <string>

#include "JNIClasses.h"
#include "JNIContext.h"
#include "structures.h"

// Converts every method call from the Kotlin-hosted version to the native version, performing
// the necessary JNI calls to fetch data, and converting the data to the correct form.

#define DEFAULT __attribute__((visibility("default")))

namespace dawn::kotlin_api {

jobject toByteBuffer(JNIEnv *env, const void* address, jlong size) {
    if (!address) {
      JNIClasses* classes = JNIClasses::getInstance(env);
      env->ThrowNew(classes->dawnException, "Invalid byte buffer.");
      return nullptr;
    }
    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");

    //* Dawn always uses little endian format, so we pre-convert for the client's convenience.
    jclass byteOrderClass = env->FindClass("java/nio/ByteOrder");
    jobject littleEndian = env->GetStaticObjectField(
            byteOrderClass, env->GetStaticFieldID(byteOrderClass, "LITTLE_ENDIAN",
                                                  "Ljava/nio/ByteOrder;"));

    jobject byteBuffer = env->NewDirectByteBuffer(const_cast<void *>(address), size);

    env->CallObjectMethod(
            byteBuffer, env->GetMethodID(byteBufferClass, "order",
                                         "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;"),
            littleEndian);

    env->DeleteLocalRef(byteOrderClass);
    env->DeleteLocalRef(littleEndian);
    return byteBuffer;
}

{% macro render_method(method, object) %}
    {% set ObjectName = kotlin_name(object) if object else "GPU" %}
    {% set MethodName = kotlin_name(method) %}
    {% set FunctionSuffix = ObjectName + "_" + MethodName %}
    {% set KotlinRecord = FunctionSuffix + "KotlinRecord" %}
    {% set ArgsStruct = FunctionSuffix + "ArgsStruct" %}

    {% set unwrap_objs = (object is not none) %}
    {% set _kotlin_return = kotlin_return(method) %}
    {% set _kotlin_args = kotlin_record_members(method.arguments) | list %}
    //* Define the helper structs to perform most of the conversion.
    {% if _kotlin_args %}
        struct {{KotlinRecord}} {
            {% for arg in _kotlin_args %}
                {{ 'jlong' if (unwrap_objs and arg.type.category == 'object' and not arg.length) else arg_to_jni_type(arg) }} {{ as_varName(arg.name) }};
            {% endfor %}
        };
    {% endif %}
    {% if method.arguments %}
        struct {{ArgsStruct}} {
            {% for arg in method.arguments %}
                {{ as_annotated_cType(arg) }};
            {% endfor %}
        };
    {% endif %}
    {% if _kotlin_args %}

        {{ define_kotlin_to_struct_conversion("ConvertInternal", KotlinRecord, ArgsStruct, method.arguments, unwrap_objects=unwrap_objs)}}
    {% endif %}

    //*  A JNI-external method is built with the JNI signature expected to match the host Kotlin.
    //*  Object methods bind to the top-level static native on {{ kotlin_name(object) }}Kt, receiving the handle as a jlong.
    DEFAULT extern "C"
    {{ arg_to_jni_type(_kotlin_return) }}
    {% if object %}
        Java_{{ kotlin_package.replace('.', '_') }}_{{ kotlin_name(object) }}Kt_native{{ method.name.CamelCase() }}
                (JNIEnv *env, jclass clazz, jlong _handle
    {%- else %}
        Java_{{ kotlin_package.replace('.', '_') }}_{{ FunctionSuffix }}
                (JNIEnv *env, jclass clazz
    {%- endif -%}

    //* Make the signature for each argument in turn.
    {%- for arg in _kotlin_args %},
        {{ 'jlong' if (unwrap_objs and arg.type.category == 'object' and not arg.length) else arg_to_jni_type(arg) }} _{{ as_varName(arg.name) }}
    {%- endfor %}) {

    // * Helper context for the duration of this method call.
    {% if _kotlin_return and (_kotlin_return.type.category == 'object' or (_kotlin_return.length == 'size_t' and (_kotlin_return.type.category == 'structure' or _kotlin_return.type.name.get() == 'char'))) %}
        JNIClasses* classes = JNIClasses::getInstance(env);
    {% endif %}
    {% if _kotlin_args or (_kotlin_return and _kotlin_return.length == 'size_t') %}
        JNIContext c(env);
    {% endif %}

    {% if _kotlin_args %}
        //* Perform the conversion of arguments.
        {{KotlinRecord}} kotlinRecord;
        {% for arg in _kotlin_args %}
            kotlinRecord.{{ as_varName(arg.name) }} = _{{ as_varName(arg.name) }};
        {% endfor %}
        {{ArgsStruct}} args;
        ConvertInternal(&c, kotlinRecord, &args);
    {% elif method.arguments %}
        {{ArgsStruct}} args = {};
    {% endif %}

    {% if object %}
        auto handle = reinterpret_cast<{{ as_cType(object.name) }}>(_handle);
    {% endif %}

    //* Actually invoke the native version of the method.
    {% if _kotlin_return and _kotlin_return.length == 'size_t' %}
        //* Methods that return containers are converted from two-call to single call, and the
        //* return type is switched to a Kotlin container.
        size_t size = wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%},
                //* The replaced output parameter is set to nullptr on the first call.
                {{ 'nullptr' if arg.annotation == '*' else "args." + as_varName(arg.name) -}}
            {% endfor %}
        );
        //* Allocate the native container
        args.{{ as_varName(_kotlin_return.name) }} = c.AllocArray<{{ as_cType(_kotlin_return.type.name) }}>(size);
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
        //* Second call completes the native container
        wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%}
                , {{- "args." + as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
    {% else %}
        {% if _kotlin_return and _kotlin_return.annotation == '*' %}
            //* Make a native container to accept the data output via parameter.
            {{ as_cType(_kotlin_return.type.name) }} out = {};
            args.{{ as_varName(_kotlin_return.name) }} = &out;
        {% endif %}
        {{ 'auto result =' if (_kotlin_return and _kotlin_return.annotation != '*') or (method.returns and method.returns.type.name.canonical_case() == 'status') }}
        {% if object %}
            wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
        {% else %}
            wgpu{{ method.name.CamelCase() }}(
        {% endif %}
            {% for arg in method.arguments -%}
                {{- ',' if object or not loop.first }}args.{{ as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return{{ ' 0' if _kotlin_return }};
        }
        {% if method.returns and method.returns.type.name.canonical_case() == 'status' %}
            if (result != WGPUStatus_Success) {
                jclass exClass = env->FindClass("androidx/webgpu/WebGpuException");
                jmethodID exConstructor =
                    env->GetMethodID(exClass, "<init>", "(Ljava/lang/String;I)V");
                std::string message = "Method {{ ObjectName }}.{{ MethodName }} failed.";
                jstring jmessage = env->NewStringUTF(message.c_str());
                jobject exception = env->NewObject(exClass, exConstructor, jmessage, result);
                env->Throw(static_cast<jthrowable>(exception));
                return{{ ' 0' if _kotlin_return }};
            }
        {% endif %}
    {% endif %}
    {% if object and object.name.get() == 'device' and _kotlin_args %}
        // Note: We intentionally only register recurring callbacks for WGPUDevice, as it is
        // the only object in Dawn with a terminal lifecycle event (device lost callback)
        // that guarantees safe cleanup. Instance does not have recurring callbacks in dawn.json.
        RegisterDeviceCallbacks(handle, c.recurringCallbacks);
        c.recurringCallbacks.clear();
    {% endif %}
    {% if _kotlin_return %}
        {% if _kotlin_return.type.name.get() in ['void const *', 'void *'] %}
            size_t size = args.size;
            {% if object and as_cType(object.name) == 'WGPUBuffer' %}
                if (args.size == WGPU_WHOLE_MAP_SIZE) {
                  uint64_t totalBufferSize = wgpuBufferGetSize(handle);
                  size = static_cast<size_t>(totalBufferSize) - args.offset;
                }
            {% endif %}
        {% endif %}
        {% if object and method.name.get() == 'create device' and _kotlin_args %}
            if (result != nullptr) {
                // Transfer callbacks to the newly created device.
                RegisterDeviceCallbacks(result, c.recurringCallbacks);
            }
            c.recurringCallbacks.clear();
        {% endif %}
        {{ convert_to_kotlin("args." + as_varName(_kotlin_return.name) if _kotlin_return.annotation == '*' else 'result',
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
    //* The handle is unwrapped on the Kotlin side and passed to the top-level static native on {{ kotlin_name(obj) }}Kt.
    DEFAULT extern "C"
    void
    Java_{{ kotlin_package.replace('.', '_') }}_{{ kotlin_name(obj) }}Kt_nativeClose(
            JNIEnv *env, jclass clazz, jlong _handle) {
        wgpu{{ obj.name.CamelCase() }}Release(reinterpret_cast<{{ as_cType(obj.name) }}>(_handle));
    }
{% endfor %}

//* Global functions don't have an associated class.
{% for function in by_category['function'] if include_method(function) %}
    {{ render_method(function, None) }}
{% endfor %}

}  // namespace dawn::kotlin_api
