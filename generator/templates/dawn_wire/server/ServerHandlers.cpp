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
    {% for command in cmd_records["command"] %}
        {% set method = command.derived_method %}
        {% set is_method = method != None %}
        {% set returns = is_method and method.return_type.name.canonical_case() != "void" %}

        {% set Suffix = command.name.CamelCase() %}
        //* The generic command handlers
        bool Server::Handle{{Suffix}}(const volatile char** commands, size_t* size) {
            {{Suffix}}Cmd cmd;
            DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator
                {%- if command.may_have_dawn_object -%}
                    , *this
                {%- endif -%}
            );

            if (deserializeResult == DeserializeResult::FatalError) {
                return false;
            }

            {% if Suffix in server_custom_pre_handler_commands %}
                if (!PreHandle{{Suffix}}(cmd)) {
                    return false;
                }
            {% endif %}

            //* Allocate any result objects
            {%- for member in command.members if member.is_return_value -%}
                {{ assert(member.handle_type) }}
                {% set Type = member.handle_type.name.CamelCase() %}
                {% set name = as_varName(member.name) %}

                auto* {{name}}Data = {{Type}}Objects().Allocate(cmd.{{name}}.id);
                if ({{name}}Data == nullptr) {
                    return false;
                }
                {{name}}Data->generation = cmd.{{name}}.generation;

                //* TODO(crbug.com/dawn/384): This is a hack to make sure that all child objects
                //* are destroyed before their device. The dawn_native device needs to track all child objects so
                //* it can destroy them if the device is destroyed first.
                {% if command.derived_object %}
                    {% set type = command.derived_object %}
                    {% if type.name.get() == "device" %}
                        {{name}}Data->deviceInfo = DeviceObjects().Get(cmd.selfId)->info.get();
                    {% else %}
                        auto* selfData = {{type.name.CamelCase()}}Objects().Get(cmd.selfId);
                        {{name}}Data->deviceInfo = selfData->deviceInfo;
                    {% endif %}
                    if ({{name}}Data->deviceInfo != nullptr) {
                        if (!TrackDeviceChild({{name}}Data->deviceInfo, ObjectType::{{Type}}, cmd.{{name}}.id)) {
                            return false;
                        }
                    }
                {% endif %}
            {% endfor %}

            //* Do command
            bool success = Do{{Suffix}}(
                {%- for member in command.members -%}
                    {%- if member.is_return_value -%}
                        {%- if member.handle_type -%}
                            &{{as_varName(member.name)}}Data->handle //* Pass the handle of the output object to be written by the doer
                        {%- else -%}
                            &cmd.{{as_varName(member.name)}}
                        {%- endif -%}
                    {%- else -%}
                        cmd.{{as_varName(member.name)}}
                    {%- endif -%}
                    {%- if not loop.last -%}, {% endif %}
                {%- endfor -%}
            );

            if (!success) {
                return false;
            }

            {%- for member in command.members if member.is_return_value and member.handle_type -%}
                {% set Type = member.handle_type.name.CamelCase() %}
                {% set name = as_varName(member.name) %}

                {% if Type in server_reverse_lookup_objects %}
                    //* For created objects, store a mapping from them back to their client IDs
                    {{Type}}ObjectIdTable().Store({{name}}Data->handle, cmd.{{name}}.id);
                {% endif %}
            {% endfor %}

            return true;
        }
    {% endfor %}

    const volatile char* Server::HandleCommandsImpl(const volatile char* commands, size_t size) {
        while (size >= sizeof(CmdHeader) + sizeof(WireCmd)) {
            // Start by chunked command handling, if it is done, then it means the whole buffer
            // was consumed by it, so we return a pointer to the end of the commands.
            switch (HandleChunkedCommands(commands, size)) {
                case ChunkedCommandsResult::Consumed:
                    return commands + size;
                case ChunkedCommandsResult::Error:
                    return nullptr;
                case ChunkedCommandsResult::Passthrough:
                    break;
            }

            WireCmd cmdId = *reinterpret_cast<const volatile WireCmd*>(commands + sizeof(CmdHeader));
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
