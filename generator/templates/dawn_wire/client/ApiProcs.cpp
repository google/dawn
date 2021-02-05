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

#include "common/Log.h"
#include "dawn_wire/client/ApiObjects.h"
#include "dawn_wire/client/Client.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace dawn_wire { namespace client {

    //* Outputs an rvalue that's the number of elements a pointer member points to.
    {% macro member_length(member, accessor) -%}
        {%- if member.length == "constant" -%}
            {{member.constant_length}}
        {%- else -%}
            {{accessor}}{{as_varName(member.length.name)}}
        {%- endif -%}
    {%- endmacro %}

    //* Implementation of the client API functions.
    {% for type in by_category["object"] %}
        {% set Type = type.name.CamelCase() %}
        {% set cType = as_cType(type.name) %}

        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}

            {% if Suffix in client_handwritten_commands %}
                static
            {% endif %}
            {{as_cType(method.return_type.name)}} Client{{Suffix}}(
                {{-cType}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto self = reinterpret_cast<{{as_wireType(type)}}>(cSelf);
                {% if Suffix not in client_handwritten_commands %}
                    {{Suffix}}Cmd cmd;

                    //* Create the structure going on the wire on the stack and fill it with the value
                    //* arguments so it can compute its size.
                    cmd.self = cSelf;

                    //* For object creation, store the object ID the client will use for the result.
                    {% if method.return_type.category == "object" %}
                        auto* allocation = self->client->{{method.return_type.name.CamelCase()}}Allocator().New(self->client);
                        cmd.result = ObjectHandle{allocation->object->id, allocation->generation};
                    {% endif %}

                    {% for arg in method.arguments %}
                        cmd.{{as_varName(arg.name)}} = {{as_varName(arg.name)}};
                    {% endfor %}

                    //* Allocate space to send the command and copy the value args over.
                    self->client->SerializeCommand(cmd);

                    {% if method.return_type.category == "object" %}
                        return reinterpret_cast<{{as_cType(method.return_type.name)}}>(allocation->object.get());
                    {% endif %}
                {% else %}
                    return self->{{method.name.CamelCase()}}(
                        {%- for arg in method.arguments -%}
                            {%if not loop.first %}, {% endif %} {{as_varName(arg.name)}}
                        {%- endfor -%});
                {% endif %}
            }
        {% endfor %}

        //* When an object's refcount reaches 0, notify the server side of it and delete it.
        void Client{{as_MethodSuffix(type.name, Name("release"))}}({{cType}} cObj) {
            {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
            obj->refcount --;

            if (obj->refcount > 0) {
                return;
            }

            DestroyObjectCmd cmd;
            cmd.objectType = ObjectType::{{type.name.CamelCase()}};
            cmd.objectId = obj->id;

            obj->client->SerializeCommand(cmd);
            obj->client->{{type.name.CamelCase()}}Allocator().Free(obj);
        }

        void Client{{as_MethodSuffix(type.name, Name("reference"))}}({{cType}} cObj) {
            {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
            obj->refcount ++;
        }
    {% endfor %}

    namespace {
        WGPUInstance ClientCreateInstance(WGPUInstanceDescriptor const* descriptor) {
            UNREACHABLE();
            return nullptr;
        }

        struct ProcEntry {
            WGPUProc proc;
            const char* name;
        };
        static const ProcEntry sProcMap[] = {
            {% for (type, method) in c_methods_sorted_by_name %}
                { reinterpret_cast<WGPUProc>(Client{{as_MethodSuffix(type.name, method.name)}}), "{{as_cMethod(type.name, method.name)}}" },
            {% endfor %}
        };
        static constexpr size_t sProcMapSize = sizeof(sProcMap) / sizeof(sProcMap[0]);
    }  // anonymous namespace

    WGPUProc ClientGetProcAddress(WGPUDevice, const char* procName) {
        if (procName == nullptr) {
            return nullptr;
        }

        const ProcEntry* entry = std::lower_bound(&sProcMap[0], &sProcMap[sProcMapSize], procName,
            [](const ProcEntry &a, const char *b) -> bool {
                return strcmp(a.name, b) < 0;
            }
        );

        if (entry != &sProcMap[sProcMapSize] && strcmp(entry->name, procName) == 0) {
            return entry->proc;
        }

        // Special case the two free-standing functions of the API.
        if (strcmp(procName, "wgpuGetProcAddress") == 0) {
            return reinterpret_cast<WGPUProc>(ClientGetProcAddress);
        }

        if (strcmp(procName, "wgpuCreateInstance") == 0) {
            return reinterpret_cast<WGPUProc>(ClientCreateInstance);
        }

        return nullptr;
    }

    std::vector<const char*> GetProcMapNamesForTesting() {
        std::vector<const char*> result;
        result.reserve(sProcMapSize);
        for (const ProcEntry& entry : sProcMap) {
            result.push_back(entry.name);
        }
        return result;
    }

    static DawnProcTable gProcTable = {
        ClientGetProcAddress,
        ClientCreateInstance,
        {% for type in by_category["object"] %}
            {% for method in c_methods(type) %}
                Client{{as_MethodSuffix(type.name, method.name)}},
            {% endfor %}
        {% endfor %}
    };
    const DawnProcTable& GetProcs() {
        return gProcTable;
    }
}}  // namespace dawn_wire::client
