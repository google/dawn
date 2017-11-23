//* Copyright 2017 The NXT Authors
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

#ifndef MOCK_NXT_H
#define MOCK_NXT_H

#include <gmock/gmock.h>
#include <nxt/nxt.h>

#include <memory>

// An abstract base class representing a proc table so that API calls can be mocked. Most API calls
// are directly represented by a delete virtual method but others need minimal state tracking to be
// useful as mocks.
class ProcTableAsClass {
    public:
        virtual ~ProcTableAsClass();

        void GetProcTableAndDevice(nxtProcTable* table, nxtDevice* device);

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
                void {{as_MethodSuffix(type.name, Name("set error callback"))}}({{as_cType(type.name)}} self, nxtBuilderErrorCallback callback, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2);
            {% endif %}
        {% endfor %}

        // Stores callback and userdata and calls the On* methods
        void DeviceSetErrorCallback(nxtDevice self, nxtDeviceErrorCallback callback, nxtCallbackUserdata userdata);
        void BufferMapReadAsync(nxtBuffer self, uint32_t start, uint32_t size, nxtBufferMapReadCallback callback, nxtCallbackUserdata userdata);

        // Special cased mockable methods
        virtual void OnDeviceSetErrorCallback(nxtDevice device, nxtDeviceErrorCallback callback, nxtCallbackUserdata userdata) = 0;
        virtual void OnBuilderSetErrorCallback(nxtBufferBuilder builder, nxtBuilderErrorCallback callback, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2) = 0;
        virtual void OnBufferMapReadAsyncCallback(nxtBuffer buffer, uint32_t start, uint32_t size, nxtBufferMapReadCallback callback, nxtCallbackUserdata userdata) = 0;

        // Calls the stored callbacks
        void CallDeviceErrorCallback(nxtDevice device, const char* message);
        void CallBuilderErrorCallback(void* builder , nxtBuilderErrorStatus status, const char* message);
        void CallMapReadCallback(nxtBuffer buffer, nxtBufferMapReadStatus status, const void* data);

        struct Object {
            ProcTableAsClass* procs = nullptr;
            nxtDeviceErrorCallback deviceErrorCallback = nullptr;
            nxtBuilderErrorCallback builderErrorCallback = nullptr;
            nxtBufferMapReadCallback mapReadCallback = nullptr;
            nxtCallbackUserdata userdata1 = 0;
            nxtCallbackUserdata userdata2 = 0;
        };

    private:
        // Remembers the values returned by GetNew* so they can be freed.
        std::vector<std::unique_ptr<Object>> mObjects;
};

class MockProcTable : public ProcTableAsClass {
    public:
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

        MOCK_METHOD3(OnDeviceSetErrorCallback, void(nxtDevice device, nxtDeviceErrorCallback callback, nxtCallbackUserdata userdata));
        MOCK_METHOD4(OnBuilderSetErrorCallback, void(nxtBufferBuilder builder, nxtBuilderErrorCallback callback, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2));
        MOCK_METHOD5(OnBufferMapReadAsyncCallback, void(nxtBuffer buffer, uint32_t start, uint32_t size, nxtBufferMapReadCallback callback, nxtCallbackUserdata userdata));
};

#endif // MOCK_NXT_H
