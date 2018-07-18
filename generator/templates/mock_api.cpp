//* Copyright 2017 The Dawn Authors
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

#include "mock_nxt.h"

namespace {
    {% for type in by_category["object"] %}
        {% for method in native_methods(type) if len(method.arguments) < 10 %}
            {{as_cType(method.return_type.name)}} Forward{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} self
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
                return object->procs->{{as_MethodSuffix(type.name, method.name)}}(self
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

void ProcTableAsClass::GetProcTableAndDevice(nxtProcTable* table, nxtDevice* device) {
    *device = GetNewDevice();

    {% for type in by_category["object"] %}
        {% for method in native_methods(type) if len(method.arguments) < 10 %}
            table->{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Forward{{as_MethodSuffix(type.name, method.name)}});
        {% endfor %}
    {% endfor %}
}

void ProcTableAsClass::DeviceSetErrorCallback(nxtDevice self, nxtDeviceErrorCallback callback, nxtCallbackUserdata userdata) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
    object->deviceErrorCallback = callback;
    object->userdata1 = userdata;

    OnDeviceSetErrorCallback(self, callback, userdata);
}

void ProcTableAsClass::BufferMapReadAsync(nxtBuffer self, uint32_t start, uint32_t size, nxtBufferMapReadCallback callback, nxtCallbackUserdata userdata) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
    object->mapReadCallback = callback;
    object->userdata1 = userdata;

    OnBufferMapReadAsyncCallback(self, start, size, callback, userdata);
}

void ProcTableAsClass::BufferMapWriteAsync(nxtBuffer self, uint32_t start, uint32_t size, nxtBufferMapWriteCallback callback, nxtCallbackUserdata userdata) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
    object->mapWriteCallback = callback;
    object->userdata1 = userdata;

    OnBufferMapWriteAsyncCallback(self, start, size, callback, userdata);
}

void ProcTableAsClass::CallDeviceErrorCallback(nxtDevice device, const char* message) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->deviceErrorCallback(message, object->userdata1);
}
void ProcTableAsClass::CallBuilderErrorCallback(void* builder , nxtBuilderErrorStatus status, const char* message) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(builder);
    object->builderErrorCallback(status, message, object->userdata1, object->userdata2);
}
void ProcTableAsClass::CallMapReadCallback(nxtBuffer buffer, nxtBufferMapAsyncStatus status, const void* data) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(buffer);
    object->mapReadCallback(status, data, object->userdata1);
}

void ProcTableAsClass::CallMapWriteCallback(nxtBuffer buffer, nxtBufferMapAsyncStatus status, void* data) {
    auto object = reinterpret_cast<ProcTableAsClass::Object*>(buffer);
    object->mapWriteCallback(status, data, object->userdata1);
}

{% for type in by_category["object"] if type.is_builder %}
    void ProcTableAsClass::{{as_MethodSuffix(type.name, Name("set error callback"))}}({{as_cType(type.name)}} self, nxtBuilderErrorCallback callback, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2) {
        auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
        object->builderErrorCallback = callback;
        object->userdata1 = userdata1;
        object->userdata2 = userdata2;

        OnBuilderSetErrorCallback(reinterpret_cast<nxtBufferBuilder>(self), callback, userdata1, userdata2);
    }
{% endfor %}

{% for type in by_category["object"] %}
    {{as_cType(type.name)}} ProcTableAsClass::GetNew{{type.name.CamelCase()}}() {
        mObjects.emplace_back(new Object);
        mObjects.back()->procs = this;
        return reinterpret_cast<{{as_cType(type.name)}}>(mObjects.back().get());
    }
{% endfor %}
