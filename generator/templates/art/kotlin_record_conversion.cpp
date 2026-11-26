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
{% from 'art/api_jni_types.cpp' import arg_to_jni_type, convert_to_kotlin, jni_signature with context %}

{% macro define_kotlin_record_structure(struct_name, members) %}
    struct {{struct_name}} {
        {% for member in kotlin_record_members(members) %}
            {% if not member.kotlin_only %}
                //* HACK: Hardcode that ANativeWindow is a jlong instead of an actual pointer. Instead
                //* of this, we should have manually written method that directly creates the
                //* wgpu::Surface from the Java Surface.
                {% if member.name.get() == "window" and member.type.name.get() == "void *"%}
                    jlong {{ as_varName(member.name) }};
                {% else %}
                    {{ arg_to_jni_type(member) }} {{ as_varName(member.name) }};
                {% endif %}
            {% endif %}
        {% endfor%}
    };
{% endmacro %}

{% macro define_kotlin_to_struct_conversion(function_name, kotlin_name, struct_name, members, is_structure_converter=False, unwrap_objects=False) %}
    //* `c` is nullptr when no member needs a JNIContext (see member_needs_context).
    inline void {{function_name}}(JNIContext* c, JNIEnv* env, const {{kotlin_name}}& inStruct, {{struct_name}}* outStruct) {
        {% set ns = namespace(needs_classes=false) %}
        {% for member in kotlin_record_members(members) if not member.kotlin_only %}
            {% if member.type.category == 'object' and ((member.length and member.length != 'constant') or (not member.length and not unwrap_objects)) %}
                {% set ns.needs_classes = true %}
            {% endif %}
        {% endfor %}
        {% if ns.needs_classes %}
            JNIClasses* classes = JNIClasses::getInstance(env);
        {% endif %}
        *outStruct = {};

        {% for member in kotlin_record_members(members) %}
            {% if not member.kotlin_only %}
            {
                {% if member.type.category == 'callback function' %}
                    if (inStruct.{{ as_varName(member.name) }})
                    {
                        auto& callbackInfo = outStruct->{{as_varName(member.name)}}Info;
                        callbackInfo = {};

                        //* Check if the member's corresponding callback info has a callbackMode.
                        {% set callbackInfoType = find_by_name(by_category["callback info"], member.type.name.get() ~ " info") %}
                        {% do assert(callbackInfoType, "callbackInfoType must exist but was not found") %}
                        {% if find_by_name(callbackInfoType.members, 'mode') %}
                            callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
                        {% endif %}

                        //* The callback functions require each argument converting.
                        //* A custom native callback is generated to wrap the Kotlin callback.
                        callbackInfo.callback = [](
                            {%- for callbackArg in member.type.arguments %}
                                {{- as_annotated_cType(callbackArg) }}{{ ', ' if not loop.last }}
                            {%- endfor -%}
                            //* Callback functions do not specify user data params in dawn.json.
                            //* However, the C API always supplements two parameters with the names
                            //* below.
                            , void* userdata1, void* userdata2) {
                                {% set userdata = 'userdata1' %}
                            //* User data is used to carry the JNI context (env) for use by the
                            //* callback.
                            ScopedJVMAttachment jvmAttachment(
                                static_cast<UserData*>({{ userdata }})->jvm);
                            {% if member.type.name.get() in ['uncaptured error callback', 'dawn load cache data callback', 'dawn store cache data callback'] %}
                            std::shared_ptr<UserData> userData1 = static_cast<UserData *>({{ userdata }})->shared_from_this();
                            {% else %}
                            std::unique_ptr<UserData> userData1{static_cast<UserData *>({{ userdata }})};
                            {% endif %}
                            {% if member.type.name.get() == 'request device callback' %}
                            // This implicitly relies on the descriptor (which contains the error/device loss callbacks)
                            // being processed before the callback parameter in wgpuAdapterRequestDevice's arguments list.
                            // Since ConvertInternal processes arguments in order, c->recurringCallbacks will be correctly populated before we reach here.
                            if (status == WGPURequestDeviceStatus_Success && device != nullptr) {
                                RegisterDeviceCallbacks(device, userData1->recurringCallbacks);
                                userData1->recurringCallbacks.clear();
                            }
                            {% endif %}
                            {% if member.type.name.get() == 'device lost callback' %}
                            if (device != nullptr) {
                                // `device` is `WGPUDevice const *`, so we must dereference it to get the handle.
                                FreeDeviceCallbacks(*device);
                            }
                            {% endif %}
                            JNIEnv* env = jvmAttachment.GetEnv();
                            if (!env || env->ExceptionCheck()) {
                                return;
                            }
                            JNIClasses* classes = JNIClasses::getInstance(env);

                            {# Determine the type of Runnable we are building (Structure vs Generic)     #}

                            {%- set info = namespace(mode='standard', class='gpuCallbackRunnable', sig='', has_err=false) -%}
                            {%- set vars = namespace(status='0', msg=none, load=none, type=none) -%}

                            {# Detect ErrorType #}
                            {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                {%- if arg.type.name.get() == 'error type' %}
                                    {% set info.has_err = true %}
                                {% endif -%}
                            {%- endfor -%}

                            {# Determine Initial Mode/Signature #}
                            {%- if is_structure_converter -%}
                                {%- set info.mode = 'structure' -%}
                                {%- set info.class = member.type.name.camelCase() + 'Runnable' -%}
                                {%- set sb = namespace(v='(L' + jni_name(member.type) + ';') -%}
                                {%- for arg in kotlin_record_members(member.type.arguments) %}
                                    {% set sb.v = sb.v + jni_signature(arg) %}
                                {% endfor -%}
                                {%- set info.sig = sb.v + ')V' -%}
                            {%- elif info.has_err -%}
                                {%- set info.mode = 'error' -%}
                                {%- set info.class = 'gpuCallbackErrorTypeRunnable' -%}
                                {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;IILjava/lang/String;)V' -%}
                            {%- else -%}
                                {# Default to Standard, might verify to Void later #}
                                {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;ILjava/lang/String;Ljava/lang/Object;)V' -%}
                            {%- endif -%}

                            {# Generate the C++ variable conversions (e.g. _status, _message) #}
                            {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                {{ convert_to_kotlin(arg.name.camelCase(), '_' + arg.name.camelCase(), 'input->' + arg.length.name.camelCase() if arg.length and arg.length != 'constant' else (arg.constant_length | string if arg.length == 'constant' and arg.constant_length != 1 else None), arg) }}
                                {%- set vName = '_' + arg.name.camelCase() -%}
                                {%- set aName = arg.name.get() -%}
                                {%- if aName == 'status' %}
                                    {% set vars.status = vName %}
                                {%- elif aName == 'message' %}
                                    {% set vars.msg = vName %}
                                {%- elif aName == 'type' %}
                                    {% set vars.type = vName %}
                                {%- else %}
                                    {% set vars.load = vName %}
                                {%- endif -%}
                            {%- endfor -%}


                            {# Handle missing strings for Generics #}
                            {%- if info.mode != 'structure' -%}
                                {%- if vars.msg is none -%}
                                    jstring _empty_msg = env->NewStringUTF("");
                                    {%- set vars.msg = '_empty_msg' -%}
                                {%- endif -%}
                                {# If Standard mode but no payload found -> Switch to Void #}
                                {%- if info.mode == 'standard' and vars.load is none -%}
                                    {%- set info.mode = 'void' -%}
                                    {%- set info.class = 'gpuCallbackVoidRunnable' -%}
                                    {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;ILjava/lang/String;)V' -%}
                                {%- endif -%}
                            {%- endif -%}


                            jclass executorClass = env->FindClass("java/util/concurrent/Executor");
                            jmethodID executeMethodID = env->GetMethodID(executorClass, "execute", "(Ljava/lang/Runnable;)V");
                            jmethodID runCtor = env->GetMethodID(classes->{{ info.class }}, "<init>", "{{ info.sig }}");

                            jobject runnable = env->NewObject(classes->{{ info.class }}, runCtor, userData1->callback
                            {%- if info.mode == 'structure' -%}
                                {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                    {%- if member.type.category != 'kotlin type' %}
                                        , _{{ arg.name.camelCase() }}
                                    {% endif -%}
                                {%- endfor -%}
                            {%- elif info.mode == 'error' -%}
                                , {{ vars.status }}, {{ vars.type }}, {{ vars.msg }}
                                {%- elif info.mode == 'void' -%}
                                , {{ vars.status }}, {{ vars.msg }}
                            {%- else -%}
                                , {{ vars.status }}, {{ vars.msg }}, {{ vars.load }}
                            {%- endif -%}
                            );

                            env->CallVoidMethod(userData1->executor, executeMethodID, runnable);
                            //* An exception thrown by a direct executor on a newly attached native
                            //* worker thread has no outer Java frame; forward it to the thread's
                            //* uncaught-exception handler before detaching. On an already-attached
                            //* caller thread, leave it pending so it propagates to the Kotlin caller.
                            if (jvmAttachment.NeedsDetach() && env->ExceptionCheck()) {
                                classes->DispatchPendingException(env);
                            }
                        };
                        {% if member.type.name.get() in ['uncaptured error callback', 'dawn load cache data callback', 'dawn store cache data callback'] %}
                        auto newUserDataShared = std::make_shared<UserData>();
                        newUserDataShared->callback = env->NewGlobalRef(inStruct.{{member.name.camelCase()}});
                        newUserDataShared->executor = env->NewGlobalRef(inStruct.{{as_varName(member.name)}}Executor);
                        newUserDataShared->jvm = JNIContext::GetJavaVM(env);
                        c->recurringCallbacks.push_back(newUserDataShared);
                        callbackInfo.{{ userdata }} = newUserDataShared.get();
                        {% else %}
                        UserData* newUserData = new UserData();
                        newUserData->callback = env->NewGlobalRef(inStruct.{{member.name.camelCase()}});
                        newUserData->executor = env->NewGlobalRef(inStruct.{{as_varName(member.name)}}Executor);
                        newUserData->jvm = JNIContext::GetJavaVM(env);
                        {% if member.type.name.get() == 'request device callback' %}
                        newUserData->recurringCallbacks = std::move(c->recurringCallbacks);
                        {% endif %}
                        callbackInfo.{{ userdata }} = newUserData;
                        {% endif %}
                    }
                    {% if member.type.name.get() == 'device lost callback' %}
                    else {
                        auto& callbackInfo = outStruct->{{as_varName(member.name)}}Info;
                        callbackInfo = {};
                        {% if find_by_name(callbackInfoType.members, 'mode') %}
                            callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
                        {% endif %}
                        callbackInfo.callback = [](
                            {%- for callbackArg in member.type.arguments %}
                                {{- as_annotated_cType(callbackArg) }}{{ ', ' if not loop.last }}
                            {%- endfor -%}
                            , void* userdata1, void* userdata2) {
                            if (device != nullptr) {
                                // `device` is `WGPUDevice const *`, so we must dereference it to get the handle.
                                FreeDeviceCallbacks(*device);
                            }
                        };
                    }
                    {% endif %}
                {% elif member.type.category != 'kotlin type' %}
                    auto& in = inStruct.{{member.name.camelCase()}};
                    auto& out = outStruct->{{member.name.camelCase()}};
                {% endif %}
                {% if member.constant_length == 1 %}
                    {% if member.type.category == 'structure' %}
                        if (in != nullptr) {
                            auto converted = c->Alloc<{{as_cType(member.type.name)}}>();
                            ToNative(c, env, in, converted);
                            if (env->ExceptionCheck()) {
                                return;
                            }
                            out = converted;
                        }
                    {% elif member.type.name.get() == 'void' %}
                        out = reinterpret_cast<void*>(in);
                    {% else %}
                        {{ unreachable_code() }}
                    {% endif %}
                {% elif member.length %}
                    {% if member.length != 'constant' %}
                        auto& outLength = outStruct->{{member.length.name.camelCase()}};
                        //* Convert container, including the length field.
                        //* *outStruct = {} already zero-initializes (out, outLength) to
                        //* (nullptr, 0), so empty containers skip JNI allocation on hot paths
                        //* such as setBindGroup dynamicOffsets. Empty SPIR-V code is invalid
                        //* regardless of pointer value (ShaderModuleBase copies the span into an
                        //* empty std::vector whose data() is nullptr).
                        {% if member.type.name.get() in ['int', 'int32_t', 'uint32_t'] or member.type.category == 'enum' %}
                            static_assert(sizeof({{ as_cType(member.type.name) }}) == sizeof(jint));
                            jsize length = in ? env->GetArrayLength(in) : 0;
                            if (length > 0) {
                                auto* array = c->AllocArray<{{ as_cType(member.type.name) }}>(length, /*zeroInit=*/false);
                                if (!array) {
                                    return;
                                }
                                env->GetIntArrayRegion(in, 0, length, reinterpret_cast<jint*>(array));
                                {% if member.type.category == 'enum' %}
                                    //* Clamp negative jint elements to *_Force32 without loading them
                                    //* as an unscoped C WGPU enum first (avoids -fsanitize=enum UB).
                                    for (jsize i = 0; i < length; ++i) {
                                        jint value;
                                        std::memcpy(&value, &array[i], sizeof(value));
                                        if (value < 0) {
                                            array[i] = {{as_cEnum(member.type.name, Name("force32"))}};
                                        }
                                    }
                                {% endif %}
                                out = array;
                                outLength = length;
                            }
                        {% elif member.type.name.get() == 'void' %}
                            void* address = in ? env->GetDirectBufferAddress(in) : nullptr;
                            jlong capacity = in ? env->GetDirectBufferCapacity(in) : 0;
                            // GetDirectBufferCapacity returns -1 for a non-direct buffer on API 31+.
                            // On API 24-30 it returns the buffer size while GetDirectBufferAddress
                            // returns nullptr. Checking both rejects non-direct buffers on every API
                            // level and still accepts 0-byte direct buffers.
                            if (capacity < 0 || (capacity > 0 && !address)) {
                                env->ThrowNew(JNIClasses::getInstance(env)->javaIllegalArgumentException,
                                              "Direct ByteBuffer required");
                                return;
                            }
                            if (capacity > 0) {
                                out = address;
                                outLength = static_cast<size_t>(capacity);
                            }
                        {% elif member.type.name.get() == 'char' %}
                            jsize length = in ? env->GetArrayLength(in) : 0;
                            if (length > 0) {
                                auto array = c->AllocArray<const char*>(length);
                                if (!array) {
                                    return;
                                }
                                for (int idx = 0; idx != length; idx++) {
                                    jstring element = static_cast<jstring>(env->GetObjectArrayElement(in, idx));
                                    array[idx] = c->GetStringUTFChars(element);
                                }
                                out = array;
                                outLength = length;
                            }
                        {% else %}
                            //* These container types are represented in Kotlin as arrays of objects.
                            jsize length = in ? env->GetArrayLength(in) : 0;
                            if (length > 0) {
                                auto array = c->AllocArray<{{ as_cType(member.type.name) }}>(length);
                                if (!array) {
                                    return;
                                }
                                out = array;
                                outLength = length;

                                {% if member.type.category == 'object' %}
                                    jfieldID handleField = classes->{{ member.type.name.camelCase() }}Handle;
                                    for (int idx = 0; idx != length; idx++) {
                                        jobject element = env->GetObjectArrayElement(in, idx);
                                        array[idx] = element ? reinterpret_cast<{{ as_cType(member.type.name) }}>(
                                                env->GetLongField(element, handleField)) : nullptr;
                                        env->DeleteLocalRef(element);
                                    }
                                {% elif member.type.category == 'structure' %}
                                    for (int idx = 0; idx != length; idx++) {
                                        jobject element = env->GetObjectArrayElement(in, idx);
                                        ToNative(c, env, element, &array[idx]);
                                        env->DeleteLocalRef(element);
                                        if (env->ExceptionCheck()) {
                                            return;
                                        }
                                    }
                                {% else %}
                                    {{ unreachable_code() }}
                                {% endif %}
                            }
                        {% endif %}
                    {% else %}
                        {% if member.type.name.get() == 'float' %}
                            jsize arrayLength = env->GetArrayLength(static_cast<jarray>(in));
                            if (arrayLength == {{ member.constant_length }}) {
                                auto array = c->AllocArray<float>({{ member.constant_length }}, /*zeroInit=*/false);
                                if (!array) {
                                    return;
                                }
                                env->GetFloatArrayRegion(static_cast<jfloatArray>(in), 0, {{ member.constant_length }}, array);
                                out = array;
                            } else {
                                // If the array length doesn't match the expected constant length,
                                // do not proceed to avoid potential crashes or uninitialized data.
                                // The out variable remains in its default-initialized state (e.g., nullptr).
                                return;
                            }
                        {% else %}
                            {{ unreachable_code('Unsupported constant array type: ' ~ member.type.name.get()) }}
                        {% endif %}
                    {% endif %}
                //* From here members are single values.
                {% elif member.type.category == 'object' %}
                    {% if unwrap_objects %}
                        //* Handle was already unwrapped on the Kotlin side. 0 maps to nullptr.
                        out = reinterpret_cast<{{as_cType(member.type.name)}}>(in);
                    {% else %}
                        if (in != nullptr) {
                            out = reinterpret_cast<{{as_cType(member.type.name)}}>(
                                    env->GetLongField(in, classes->{{ member.type.name.camelCase() }}Handle));
                        } else {
                            out = nullptr;
                        }
                    {% endif %}
                {% elif member.type.category == 'callback function' %}
                    //* Do nothing for this case.
                {% elif member.type.category == 'structure' %}
                    //* Mandatory structure.
                    ToNative(c, env, in, &out);
                    if (env->ExceptionCheck()) {
                        return;
                    }
                {% elif member.name.get() == "window" and member.type.name.get() == "void *" %}
                    //* HACK: Hardcode that ANativeWindow is a jlong instead of an actual pointer. Instead
                    //* of this, we should have manually written method that directly creates the
                    //* wgpu::Surface from the Java Surface.
                    out = reinterpret_cast<{{as_cType(member.type.name)}}>(static_cast<uintptr_t>(in));
                {% elif member.type.category == "enum" %}
                    //* C WGPU enums are unscoped with values in [0, 0x7FFFFFFF]. Negative jint
                    //* values are UB under -fsanitize=enum, so map them to *_Force32 (0x7FFFFFFF),
                    //* which is in range and never a valid enumerator.
                    out = in < 0 ? {{as_cEnum(member.type.name, Name("force32"))}}
                                 : static_cast<{{as_cType(member.type.name)}}>(in);
                {% elif member.type.category in ["native", "bitmask"] %}
                    out = static_cast<{{as_cType(member.type.name)}}>(in);
                {% elif member.type.category != 'kotlin type' %}
                    {{ unreachable_code() }}
                {% endif %}
            }
            {% endif %}
        {% endfor -%}
    }
{% endmacro %}
