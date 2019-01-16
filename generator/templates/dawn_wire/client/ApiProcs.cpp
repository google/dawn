//* Copyright 2019 The Dawn Authors
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

#include "dawn_wire/client/ApiObjects.h"
#include "dawn_wire/client/ApiProcs_autogen.h"
#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device_autogen.h"

namespace dawn_wire { namespace client {
    //* Implementation of the client API functions.
    {% for type in by_category["object"] %}
        {% set Type = type.name.CamelCase() %}
        {% set cType = as_cType(type.name) %}

        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}
            {% if Suffix not in client_side_commands %}
                {{as_cType(method.return_type.name)}} Client{{Suffix}}(
                    {{-cType}} cSelf
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) {
                    auto self = reinterpret_cast<{{as_wireType(type)}}>(cSelf);
                    Device* device = self->device;
                    {{Suffix}}Cmd cmd;

                    //* Create the structure going on the wire on the stack and fill it with the value
                    //* arguments so it can compute its size.
                    cmd.self = cSelf;

                    //* For object creation, store the object ID the client will use for the result.
                    {% if method.return_type.category == "object" %}
                        auto* allocation = self->device->{{method.return_type.name.camelCase()}}.New();

                        {% if type.is_builder %}
                            //* We are in GetResult, so the callback that should be called is the
                            //* currently set one. Copy it over to the created object and prevent the
                            //* builder from calling the callback on destruction.
                            allocation->object->builderCallback = self->builderCallback;
                            self->builderCallback.canCall = false;
                        {% endif %}

                        cmd.result = ObjectHandle{allocation->object->id, allocation->serial};
                    {% endif %}

                    {% for arg in method.arguments %}
                        cmd.{{as_varName(arg.name)}} = {{as_varName(arg.name)}};
                    {% endfor %}

                    //* Allocate space to send the command and copy the value args over.
                    size_t requiredSize = cmd.GetRequiredSize();
                    char* allocatedBuffer = static_cast<char*>(device->GetCmdSpace(requiredSize));
                    cmd.Serialize(allocatedBuffer, *device);

                    {% if method.return_type.category == "object" %}
                        return reinterpret_cast<{{as_cType(method.return_type.name)}}>(allocation->object.get());
                    {% endif %}
                }
            {% endif %}
        {% endfor %}

        {% if type.is_builder %}
            void Client{{as_MethodSuffix(type.name, Name("set error callback"))}}({{cType}} cSelf,
                                                                                  dawnBuilderErrorCallback callback,
                                                                                  dawnCallbackUserdata userdata1,
                                                                                  dawnCallbackUserdata userdata2) {
                {{Type}}* self = reinterpret_cast<{{Type}}*>(cSelf);
                self->builderCallback.callback = callback;
                self->builderCallback.userdata1 = userdata1;
                self->builderCallback.userdata2 = userdata2;
            }
        {% endif %}

        {% if not type.name.canonical_case() == "device" %}
            //* When an object's refcount reaches 0, notify the server side of it and delete it.
            void Client{{as_MethodSuffix(type.name, Name("release"))}}({{cType}} cObj) {
                {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
                obj->refcount --;

                if (obj->refcount > 0) {
                    return;
                }

                obj->builderCallback.Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, "Unknown");

                DestroyObjectCmd cmd;
                cmd.objectType = ObjectType::{{type.name.CamelCase()}};
                cmd.objectId = obj->id;

                size_t requiredSize = cmd.GetRequiredSize();
                char* allocatedBuffer = static_cast<char*>(obj->device->GetCmdSpace(requiredSize));
                cmd.Serialize(allocatedBuffer);

                obj->device->{{type.name.camelCase()}}.Free(obj);
            }

            void Client{{as_MethodSuffix(type.name, Name("reference"))}}({{cType}} cObj) {
                {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
                obj->refcount ++;
            }
        {% endif %}
    {% endfor %}

    //* Some commands don't have a custom wire format, but need to be handled manually to update
    //* some client-side state tracking. For these we have two functions:
    //*  - An autogenerated Client{{suffix}} method that sends the command on the wire
    //*  - A manual ProxyClient{{suffix}} method that will be inserted in the proctable instead of
    //*    the autogenerated one, and that will have to call Client{{suffix}}
    dawnProcTable GetProcs() {
        dawnProcTable table;
        {% for type in by_category["object"] %}
            {% for method in native_methods(type) %}
                {% set suffix = as_MethodSuffix(type.name, method.name) %}
                {% if suffix in client_proxied_commands %}
                    table.{{as_varName(type.name, method.name)}} = ProxyClient{{suffix}};
                {% else %}
                    table.{{as_varName(type.name, method.name)}} = Client{{suffix}};
                {% endif %}
            {% endfor %}
        {% endfor %}
        return table;
    }
}}  // namespace dawn_wire::client
