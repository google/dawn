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

#include "Wire.h"
#include "WireCmd.h"

#include <cstring>
#include <vector>

namespace nxt {
namespace wire {

    //* Client side implementation of the API, will serialize everything to memory to send to the server side.
    namespace client {

        class Device;

        //* All non-Device objects of the client side have:
        //*  - A pointer to the device to get where to serialize commands
        //*  - The external reference count
        //*  - An ID that is used to refer to this object when talking with the server side
        struct ObjectBase {
            ObjectBase(Device* device, uint32_t refcount, uint32_t id)
                :device(device), refcount(refcount), id(id) {
            }

            Device* device;
            uint32_t refcount;
            uint32_t id;
        };

        {% for type in by_category["object"] if not type.name.canonical_case() == "device" %}
            struct {{type.name.CamelCase()}} : ObjectBase {
                using ObjectBase::ObjectBase;
            };
        {% endfor %}

        //* TODO: Remember objects so they can all be destroyed at device destruction.
        template<typename T>
        class ObjectAllocator {
            public:
                ObjectAllocator(Device* device) : device(device) {
                }

                T* New() {
                    return new T(device, 1, GetNewId());
                }
                void Free(T* obj) {
                    FreeId(obj->id);
                    delete obj;
                }

            private:
                uint32_t GetNewId() {
                    if (freeIds.empty()) {
                        return currentId ++;
                    }
                    uint32_t id = freeIds.back();
                    freeIds.pop_back();
                    return id;
                }
                void FreeId(uint32_t id) {
                    freeIds.push_back(id);
                };

                // 0 is an ID reserved to represent nullptr
                uint32_t currentId = 1;
                std::vector<uint32_t> freeIds;
                Device* device;
        };

        //* The client wire uses the global NXT device to store its global data such as the serializer
        //* and the object id allocators.
        class Device : public ObjectBase {
            public:
                Device(CommandSerializer* serializer)
                    : ObjectBase(this, 1, 1),
                    {% for type in by_category["object"] if not type.name.canonical_case() == "device" %}
                        {{type.name.camelCase()}}(this),
                    {% endfor %}
                    serializer(serializer) {
                }

                void* GetCmdSpace(size_t size) {
                    return serializer->GetCmdSpace(size);
                }

                {% for type in by_category["object"] if not type.name.canonical_case() == "device" %}
                    ObjectAllocator<{{type.name.CamelCase()}}> {{type.name.camelCase()}};
                {% endfor %}

            private:
               CommandSerializer* serializer = nullptr;
        };

        //* Implementation of the client API functions.
        {% for type in by_category["object"] %}
            {% set Type = type.name.CamelCase() %}

            {% for method in type.methods %}
                {% set Suffix = as_MethodSuffix(type.name, method.name) %}

                {{as_backendType(method.return_type)}} Client{{Suffix}}(
                    {{-as_backendType(type)}} self
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_backendType(arg)}}
                    {%- endfor -%}
                ) {
                    Device* device = self->device;
                    wire::{{Suffix}}Cmd cmd;

                    //* Create the structure going on the wire on the stack and fill it with the value
                    //* arguments so it can compute its size.
                    {
                        //* Value objects are stored as IDs
                        {% for arg in method.arguments if arg.annotation == "value" %}
                            {% if arg.type.category == "object" %}
                                cmd.{{as_varName(arg.name)}} = {{as_varName(arg.name)}}->id;
                            {% else %}
                                cmd.{{as_varName(arg.name)}} = {{as_varName(arg.name)}};
                            {% endif %}
                        {% endfor %}

                        cmd.self = self->id;

                        //* The length of const char* is considered a value argument.
                        {% for arg in method.arguments if arg.length == "strlen" %}
                            cmd.{{as_varName(arg.name)}}Strlen = strlen({{as_varName(arg.name)}});
                        {% endfor %}
                    }

                    //* Allocate space to send the command and copy the value args over.
                    size_t requiredSize = cmd.GetRequiredSize();
                    auto allocCmd = reinterpret_cast<decltype(cmd)*>(device->GetCmdSpace(requiredSize));
                    *allocCmd = cmd;

                    //* In the allocated space, write the non-value arguments.
                    {% for arg in method.arguments if arg.annotation != "value" %}
                        {% set argName = as_varName(arg.name) %}
                        {% if arg.length == "strlen" %}
                            memcpy(allocCmd->GetPtr_{{argName}}(), {{argName}}, allocCmd->{{argName}}Strlen + 1);
                        {% elif arg.type.category == "object" %}
                            auto {{argName}}Storage = reinterpret_cast<uint32_t*>(allocCmd->GetPtr_{{argName}}());
                            for (size_t i = 0; i < {{as_varName(arg.length.name)}}; i++) {
                                {{argName}}Storage[i] = {{argName}}[i]->id;
                            }
                        {% else %}
                            memcpy(allocCmd->GetPtr_{{argName}}(), {{argName}}, {{as_varName(arg.length.name)}} * sizeof(*{{argName}}));
                        {% endif %}
                    {% endfor %}

                    //* For object creation, store the object ID the client will use for the result.
                    {% if method.return_type.category == "object" %}
                        auto result = self->device->{{method.return_type.name.camelCase()}}.New();
                        allocCmd->resultId = result->id;
                        return result;
                    {% endif %}
                }
            {% endfor %}

            {% if not type.name.canonical_case() == "device" %}
                //* When an object's refcount reaches 0, notify the server side of it and delete it.
                void Client{{as_MethodSuffix(type.name, Name("release"))}}({{Type}}* obj) {
                    obj->refcount --;

                    if (obj->refcount > 0) {
                        return;
                    }

                    wire::{{as_MethodSuffix(type.name, Name("destroy"))}}Cmd cmd;
                    cmd.objectId = obj->id;

                    size_t requiredSize = cmd.GetRequiredSize();
                    auto allocCmd = reinterpret_cast<decltype(cmd)*>(obj->device->GetCmdSpace(requiredSize));
                    *allocCmd = cmd;

                    obj->device->{{type.name.camelCase()}}.Free(obj);
                }

                void Client{{as_MethodSuffix(type.name, Name("reference"))}}({{Type}}* obj) {
                    obj->refcount ++;
                }
            {% endif %}
        {% endfor %}

        void ClientDeviceReference(Device* self) {
        }

        void ClientDeviceRelease(Device* self) {
        }

        nxtProcTable GetProcs() {
            nxtProcTable table;
            {% for type in by_category["object"] %}
                {% for method in native_methods(type) %}
                    table.{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Client{{as_MethodSuffix(type.name, method.name)}});
                {% endfor %}
            {% endfor %}
            return table;
        }

    }

    void NewClientDevice(nxtProcTable* procs, nxtDevice* device, CommandSerializer* serializer) {
        *device = reinterpret_cast<nxtDeviceImpl*>(new client::Device(serializer));
        *procs = client::GetProcs();
    }

}
}
