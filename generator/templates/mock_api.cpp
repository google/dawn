//* Copyright 2017 The Dawn & Tint Authors
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

{% set api = metadata.api.lower() %}
#include "dawn/common/Log.h"
#include "mock_{{api}}.h"

using namespace testing;

namespace {
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_cType(method.return_type.name)}} Forward{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} self
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
                return object->procs->{{as_CppMethodSuffix(type.name, method.name)}}(self
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}

    {% endfor %}
}

ProcTableAsClass::~ProcTableAsClass() {
}

{% set Prefix = metadata.proc_table_prefix %}
void ProcTableAsClass::GetProcTable({{Prefix}}ProcTable* table) {
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            table->{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Forward{{as_MethodSuffix(type.name, method.name)}});
        {% endfor %}
    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        table->{{as_varName(type.name, Name("free members"))}} = []({{as_cType(type.name)}} {{as_varName(type.name)}}) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No mock available for {{as_varName(type.name, Name('free members'))}}";
            }
        };
    {% endfor %}
}

//* Generate the older Call*Callback if there is no Future call equivalent.
//* Includes:
//*   - setLoggingCallback
{% set LegacyCallbackFunctions = ['set logging callback'] %}

//* Manually implemented mock functions due to incompatibility.
{% set ManuallyMockedFunctions = ['set device lost callback', 'set uncaptured error callback'] %}

{% for type in by_category["object"] %}
    {% for method in type.methods if method.name.get() not in ManuallyMockedFunctions %}
        {% set Suffix = as_CppMethodSuffix(type.name, method.name) %}
        {% if has_callback_arguments(method) %}
            {{as_cType(method.return_type.name)}} ProcTableAsClass::{{Suffix}}(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                {% for arg in method.arguments if arg.type.category == 'function pointer' %}
                    object->m{{Suffix + arg.name.CamelCase()}} = {{as_varName(arg.name)}};
                    object->m{{Suffix}}Userdata = userdata;
                {% endfor %}

                {% if method.name.get() == 'pop error scope' %}
                    //* Currently special casing popErrorScope since it has an old callback type.
                    On{{Suffix}}({{-as_varName(type.name)}}, {.nextInChain = nullptr, .mode = WGPUCallbackMode_AllowProcessEvents, .callback = nullptr, .oldCallback = oldCallback, .userdata = userdata});
                {% elif method.name.get() not in LegacyCallbackFunctions %}
                    On{{Suffix}}(
                        {{-as_varName(type.name)}}
                        {%- for arg in method.arguments if arg.type.category != 'function pointer' and arg.type.name.get() != 'void *' -%}
                            , {{as_varName(arg.name)}}
                        {%- endfor -%}
                        , {.nextInChain = nullptr, .mode = WGPUCallbackMode_AllowProcessEvents
                        {%- for arg in method.arguments if arg.type.category == 'function pointer' -%}
                            , .{{as_varName(arg.name)}} = {{as_varName(arg.name)}}
                        {%- endfor -%}
                        , .userdata = userdata});
                {% else %}
                    On{{Suffix}}(
                        {{-as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_varName(arg.name)}}
                        {%- endfor -%}
                    );
                {% endif %}
            }
        {% elif has_callback_info(method) %}
            {{as_cType(method.return_type.name)}} ProcTableAsClass::{{Suffix}}(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                {% for arg in method.arguments %}
                    {% if arg.name.get() == 'callback info' %}
                        {% for callback in types[arg.type.name.get()].members if callback.type.category == 'function pointer' %}
                            object->m{{Suffix + callback.name.CamelCase()}} = {{as_varName(arg.name)}}.{{as_varName(callback.name)}};
                            object->m{{Suffix}}Userdata = {{as_varName(arg.name)}}.userdata;
                        {% endfor %}
                    {% endif %}
                {% endfor %}

                On{{Suffix}}(
                    {{-as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
                return {mNextFutureID++};
            }
        {% elif has_callbackInfoStruct(method) %}
            {{as_cType(method.return_type.name)}} ProcTableAsClass::{{Suffix}}(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                object->m{{Suffix}}Callback = callbackInfo.callback;
                object->m{{Suffix}}Userdata1 = callbackInfo.userdata1;
                object->m{{Suffix}}Userdata2 = callbackInfo.userdata2;

                On{{Suffix}}(
                    {{-as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
                return {mNextFutureID++};
            }
            {% set CallbackInfoType = (method.arguments|last).type %}
            {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
            void ProcTableAsClass::Call{{Suffix}}Callback(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in CallbackType.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                object->m{{Suffix}}Callback(
                    {%- for arg in CallbackType.arguments -%}
                        {{as_varName(arg.name)}}{{", "}}
                    {%- endfor -%}
                    object->m{{Suffix}}Userdata1, object->m{{Suffix}}Userdata2);
            }
        {% endif %}
    {% endfor %}

    {% for method in type.methods if has_callback_info(method) or method.name.get() in LegacyCallbackFunctions %}
        {% set Suffix = as_CppMethodSuffix(type.name, method.name) %}
        {% for arg in method.arguments %}
            {% if arg.name.get() == 'callback info' %}
                {% for callback in types[arg.type.name.get()].members if callback.type.category == 'function pointer' %}
                    void ProcTableAsClass::Call{{Suffix + callback.name.CamelCase()}}(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in callback.type.arguments -%}
                            {%- if not loop.last -%}, {{as_annotated_cType(arg)}}{%- endif -%}
                        {%- endfor -%}
                    ) {
                        ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                        object->m{{Suffix + callback.name.CamelCase()}}(
                            {%- for arg in callback.type.arguments -%}
                                {%- if not loop.last -%}{{as_varName(arg.name)}}, {% endif -%}
                            {%- endfor -%}
                            object->m{{Suffix}}Userdata);
                    }
                {% endfor %}
            {% elif arg.type.category == 'function pointer' %}
                void ProcTableAsClass::Call{{Suffix + arg.name.CamelCase()}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in arg.type.arguments -%}
                        {%- if not loop.last -%}, {{as_annotated_cType(arg)}}{%- endif -%}
                    {%- endfor -%}
                ) {
                    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                    object->m{{Suffix + arg.name.CamelCase()}}(
                        {%- for arg in arg.type.arguments -%}
                            {%- if not loop.last -%}{{as_varName(arg.name)}}, {% endif -%}
                        {%- endfor -%}
                        object->m{{Suffix}}Userdata);
                }
            {% endif %}
        {% endfor %}
    {% endfor %}
{% endfor %}

// Manually implement some callback helpers for testing.
void ProcTableAsClass::DeviceSetDeviceLostCallback(WGPUDevice device,
                                                   WGPUDeviceLostCallback callback,
                                                   void* userdata) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mDeviceLostCallback = [](WGPUDevice const*, WGPUDeviceLostReason reason,
                                     char const* message, void* callback, void* userdata) {
        if (callback == nullptr) {
            return;
        }
        auto cb = reinterpret_cast<WGPUDeviceLostCallback>(callback);
        cb(reason, message, userdata);
    };
    object->mDeviceLostUserdata1 = reinterpret_cast<void*>(callback);
    object->mDeviceLostUserdata2 = userdata;

    OnDeviceSetDeviceLostCallback(device, callback, userdata);
}
void ProcTableAsClass::CallDeviceSetDeviceLostCallbackCallback(WGPUDevice device,
                                                               WGPUDeviceLostReason reason,
                                                               char const* message) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mDeviceLostCallback(&device, reason, message, object->mDeviceLostUserdata1,
                                object->mDeviceLostUserdata2);
}
void ProcTableAsClass::DeviceSetUncapturedErrorCallback(WGPUDevice device,
                                                        WGPUErrorCallback callback,
                                                        void* userdata) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mUncapturedErrorCallback = [](WGPUDevice const*, WGPUErrorType type,
                                          char const* message, void* callback, void* userdata) {
        if (callback == nullptr) {
            return;
        }
        auto cb = reinterpret_cast<WGPUErrorCallback>(callback);
        cb(type, message, userdata);
    };
    object->mUncapturedErrorUserdata1 = reinterpret_cast<void*>(callback);
    object->mUncapturedErrorUserdata2 = userdata;

    OnDeviceSetUncapturedErrorCallback(device, callback, userdata);
}
void ProcTableAsClass::CallDeviceSetUncapturedErrorCallbackCallback(WGPUDevice device,
                                                                    WGPUErrorType type,
                                                                    char const* message) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mUncapturedErrorCallback(&device, type, message, object->mUncapturedErrorUserdata1,
                                     object->mUncapturedErrorUserdata2);
}

{% for type in by_category["object"] %}
    {{as_cType(type.name)}} ProcTableAsClass::GetNew{{type.name.CamelCase()}}() {
        mObjects.emplace_back(new Object);
        mObjects.back()->procs = this;
        return reinterpret_cast<{{as_cType(type.name)}}>(mObjects.back().get());
    }
{% endfor %}

MockProcTable::MockProcTable() = default;

MockProcTable::~MockProcTable() = default;

void MockProcTable::IgnoreAllReleaseCalls() {
    {% for type in by_category["object"] %}
        EXPECT_CALL(*this, {{as_CppMethodSuffix(type.name, Name("release"))}}(_)).Times(AnyNumber());
    {% endfor %}
}
