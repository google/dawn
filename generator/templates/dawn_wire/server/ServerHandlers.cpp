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

#include "common/Assert.h"
#include "dawn_wire/server/Server.h"

namespace dawn_wire { namespace server {
    {% for type in by_category["object"] %}
        {% for method in type.methods %}
            {% set Suffix = as_MethodSuffix(type.name, method.name) %}
            {% if Suffix not in client_side_commands %}
                //* The generic command handlers

                bool Server::Handle{{Suffix}}(const char** commands, size_t* size) {
                    {{Suffix}}Cmd cmd;
                    DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator, *this);

                    if (deserializeResult == DeserializeResult::FatalError) {
                        return false;
                    }

                    {% if Suffix in server_custom_pre_handler_commands %}
                        if (!PreHandle{{Suffix}}(cmd)) {
                            return false;
                        }
                    {% endif %}

                    //* Unpack 'self'
                    auto* selfData = {{type.name.CamelCase()}}Objects().Get(cmd.selfId);
                    ASSERT(selfData != nullptr);

                    //* In all cases allocate the object data as it will be refered-to by the client.
                    {% set return_type = method.return_type %}
                    {% set returns = return_type.name.canonical_case() != "void" %}
                    {% if returns %}
                        {% set Type = method.return_type.name.CamelCase() %}
                        auto* resultData = {{Type}}Objects().Allocate(cmd.result.id);
                        if (resultData == nullptr) {
                            return false;
                        }
                        resultData->serial = cmd.result.serial;

                        {% if type.is_builder %}
                            selfData->builtObject = cmd.result;
                        {% endif %}
                    {% endif %}

                    //* After the data is allocated, apply the argument error propagation mechanism
                    if (deserializeResult == DeserializeResult::ErrorObject) {
                        {% if type.is_builder %}
                            selfData->valid = false;
                            //* If we are in GetResult, fake an error callback
                            {% if returns %}
                                On{{type.name.CamelCase()}}Error(DAWN_BUILDER_ERROR_STATUS_ERROR, "Maybe monad", cmd.selfId, selfData->serial);
                            {% endif %}
                        {% endif %}
                        return true;
                    }

                    {% if returns %}
                        auto result ={{" "}}
                    {%- endif %}
                    mProcs.{{as_varName(type.name, method.name)}}(cmd.self
                        {%- for arg in method.arguments -%}
                            , cmd.{{as_varName(arg.name)}}
                        {%- endfor -%}
                    );

                    {% if Suffix in server_custom_post_handler_commands %}
                        if (!PostHandle{{Suffix}}(cmd)) {
                            return false;
                        }
                    {% endif %}

                    {% if returns %}
                        resultData->handle = result;
                        resultData->valid = result != nullptr;

                        {% if return_type.name.CamelCase() in server_reverse_lookup_objects %}
                            //* For created objects, store a mapping from them back to their client IDs
                            if (result) {
                                {{return_type.name.CamelCase()}}ObjectIdTable().Store(result, cmd.result.id);
                            }
                        {% endif %}

                        //* builders remember the ID of the object they built so that they can send it
                        //* in the callback to the client.
                        {% if return_type.is_builder %}
                            if (result != nullptr) {
                                uint64_t userdata1 = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
                                uint64_t userdata2 = (uint64_t(resultData->serial) << uint64_t(32)) + cmd.result.id;
                                mProcs.{{as_varName(return_type.name, Name("set error callback"))}}(result, Forward{{return_type.name.CamelCase()}}ToClient, userdata1, userdata2);
                            }
                        {% endif %}
                    {% endif %}

                    return true;
                }
            {% endif %}
        {% endfor %}
    {% endfor %}

    bool Server::HandleDestroyObject(const char** commands, size_t* size) {
        DestroyObjectCmd cmd;
        DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

        if (deserializeResult == DeserializeResult::FatalError) {
            return false;
        }

        ObjectId objectId = cmd.objectId;
        //* ID 0 are reserved for nullptr and cannot be destroyed.
        if (objectId == 0) {
            return false;
        }

        switch (cmd.objectType) {
            {% for type in by_category["object"] %}
                {% set ObjectType = type.name.CamelCase() %}
                case ObjectType::{{ObjectType}}: {
                    {% if ObjectType == "Device" %}
                        //* Freeing the device has to be done out of band.
                        return false;
                    {% else %}
                        auto* data = {{type.name.CamelCase()}}Objects().Get(objectId);
                        if (data == nullptr) {
                            return false;
                        }
                        {% if type.name.CamelCase() in server_reverse_lookup_objects %}
                            {{type.name.CamelCase()}}ObjectIdTable().Remove(data->handle);
                        {% endif %}

                        if (data->handle != nullptr) {
                            mProcs.{{as_varName(type.name, Name("release"))}}(data->handle);
                        }

                        {{type.name.CamelCase()}}Objects().Free(objectId);
                        return true;
                    {% endif %}
                }
            {% endfor %}
            default:
                UNREACHABLE();
        }
    }

    const char* Server::HandleCommands(const char* commands, size_t size) {
        mProcs.deviceTick(DeviceObjects().Get(1)->handle);

        while (size >= sizeof(WireCmd)) {
            WireCmd cmdId = *reinterpret_cast<const WireCmd*>(commands);

            bool success = false;
            switch (cmdId) {
                {% for command in cmd_records["command"] %}
                    case WireCmd::{{command.name.CamelCase()}}:
                        success = Handle{{command.name.CamelCase()}}(&commands, &size);
                        break;
                {% endfor %}
                default:
                    success = false;
            }

            if (!success) {
                return nullptr;
            }
            mAllocator.Reset();
        }

        if (size != 0) {
            return nullptr;
        }

        return commands;
    }

}}  // namespace dawn_wire::server
