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

#ifndef MOCK_DAWN_H
#define MOCK_DAWN_H

#include <gmock/gmock.h>
#include <dawn/dawn.h>

#include <memory>

// An abstract base class representing a proc table so that API calls can be mocked. Most API calls
// are directly represented by a delete virtual method but others need minimal state tracking to be
// useful as mocks.
class ProcTableAsClass {
    public:
        virtual ~ProcTableAsClass();

        void GetProcTableAndDevice(dawnProcTable* table, dawnDevice* device);

        // Creates an object that can be returned by a mocked call as in WillOnce(Return(foo)).
        // It returns an object of the write type that isn't equal to any previously returned object.
        // Otherwise some mock expectation could be triggered by two different objects having the same
        // value.
        {% for type in by_category["object"] %}
            {{as_cType(type.name)}} GetNew{{type.name.CamelCase()}}();
        {% endfor %}

        {% for type in by_category["object"] %}
            {% for method in type.methods if len(method.arguments) < 10 %}
                virtual {{as_cType(method.return_type.name)}} {{as_MethodSuffix(type.name, method.name)}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) = 0;
            {% endfor %}
            virtual void {{as_MethodSuffix(type.name, Name("reference"))}}({{as_cType(type.name)}} self) = 0;
            virtual void {{as_MethodSuffix(type.name, Name("release"))}}({{as_cType(type.name)}} self) = 0;

            // Stores callback and userdata and calls OnBuilderSetErrorCallback
            {% if type.is_builder %}
                void {{as_MethodSuffix(type.name, Name("set error callback"))}}({{as_cType(type.name)}} self, dawnBuilderErrorCallback callback, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2);
            {% endif %}
        {% endfor %}

        // Stores callback and userdata and calls the On* methods
        void DeviceSetErrorCallback(dawnDevice self, dawnDeviceErrorCallback callback, dawnCallbackUserdata userdata);
        void BufferMapReadAsync(dawnBuffer self, uint32_t start, uint32_t size, dawnBufferMapReadCallback callback, dawnCallbackUserdata userdata);
        void BufferMapWriteAsync(dawnBuffer self, uint32_t start, uint32_t size, dawnBufferMapWriteCallback callback, dawnCallbackUserdata userdata);
        void FenceOnCompletion(dawnFence self,
                               uint64_t value,
                               dawnFenceOnCompletionCallback callback,
                               dawnCallbackUserdata userdata);

        // Special cased mockable methods
        virtual void OnDeviceSetErrorCallback(dawnDevice device, dawnDeviceErrorCallback callback, dawnCallbackUserdata userdata) = 0;
        virtual void OnBuilderSetErrorCallback(dawnBufferBuilder builder, dawnBuilderErrorCallback callback, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2) = 0;
        virtual void OnBufferMapReadAsyncCallback(dawnBuffer buffer, uint32_t start, uint32_t size, dawnBufferMapReadCallback callback, dawnCallbackUserdata userdata) = 0;
        virtual void OnBufferMapWriteAsyncCallback(dawnBuffer buffer, uint32_t start, uint32_t size, dawnBufferMapWriteCallback callback, dawnCallbackUserdata userdata) = 0;
        virtual void OnFenceOnCompletionCallback(dawnFence fence,
                                                 uint64_t value,
                                                 dawnFenceOnCompletionCallback callback,
                                                 dawnCallbackUserdata userdata) = 0;

        // Calls the stored callbacks
        void CallDeviceErrorCallback(dawnDevice device, const char* message);
        void CallBuilderErrorCallback(void* builder , dawnBuilderErrorStatus status, const char* message);
        void CallMapReadCallback(dawnBuffer buffer, dawnBufferMapAsyncStatus status, const void* data);
        void CallMapWriteCallback(dawnBuffer buffer, dawnBufferMapAsyncStatus status, void* data);
        void CallFenceOnCompletionCallback(dawnFence fence, dawnFenceCompletionStatus status);

        struct Object {
            ProcTableAsClass* procs = nullptr;
            dawnDeviceErrorCallback deviceErrorCallback = nullptr;
            dawnBuilderErrorCallback builderErrorCallback = nullptr;
            dawnBufferMapReadCallback mapReadCallback = nullptr;
            dawnBufferMapWriteCallback mapWriteCallback = nullptr;
            dawnFenceOnCompletionCallback fenceOnCompletionCallback = nullptr;
            dawnCallbackUserdata userdata1 = 0;
            dawnCallbackUserdata userdata2 = 0;
        };

    private:
        // Remembers the values returned by GetNew* so they can be freed.
        std::vector<std::unique_ptr<Object>> mObjects;
};

class MockProcTable : public ProcTableAsClass {
    public:
        MockProcTable();

        {% for type in by_category["object"] %}
            {% for method in type.methods if len(method.arguments) < 10 %}
                MOCK_METHOD{{len(method.arguments) + 1}}(
                    {{-as_MethodSuffix(type.name, method.name)}},
                    {{as_cType(method.return_type.name)}}(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ));
            {% endfor %}

            MOCK_METHOD1({{as_MethodSuffix(type.name, Name("reference"))}}, void({{as_cType(type.name)}} self));
            MOCK_METHOD1({{as_MethodSuffix(type.name, Name("release"))}}, void({{as_cType(type.name)}} self));
        {% endfor %}

        MOCK_METHOD3(OnDeviceSetErrorCallback, void(dawnDevice device, dawnDeviceErrorCallback callback, dawnCallbackUserdata userdata));
        MOCK_METHOD4(OnBuilderSetErrorCallback, void(dawnBufferBuilder builder, dawnBuilderErrorCallback callback, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2));
        MOCK_METHOD5(OnBufferMapReadAsyncCallback, void(dawnBuffer buffer, uint32_t start, uint32_t size, dawnBufferMapReadCallback callback, dawnCallbackUserdata userdata));
        MOCK_METHOD5(OnBufferMapWriteAsyncCallback, void(dawnBuffer buffer, uint32_t start, uint32_t size, dawnBufferMapWriteCallback callback, dawnCallbackUserdata userdata));
        MOCK_METHOD4(OnFenceOnCompletionCallback,
                     void(dawnFence fence,
                          uint64_t value,
                          dawnFenceOnCompletionCallback callback,
                          dawnCallbackUserdata userdata));
};

#endif // MOCK_DAWN_H
