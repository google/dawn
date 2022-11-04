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

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {
    {% for command in cmd_records["command"] %}
        {% set method = command.derived_method %}
        {% set is_method = method != None %}
        {% set returns = is_method and method.return_type.name.canonical_case() != "void" %}

        {% set Suffix = command.name.CamelCase() %}
        //* The generic command handlers
        bool Server::Handle{{Suffix}}(DeserializeBuffer* deserializeBuffer) {
            {{Suffix}}Cmd cmd;
            WireResult deserializeResult = cmd.Deserialize(deserializeBuffer, &mAllocator
                {%- if command.may_have_dawn_object -%}
                    , *this
                {%- endif -%}
            );

            if (deserializeResult == WireResult::FatalError) {
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

                auto* {{name}}Data = {{Type}}Objects().Allocate(cmd.{{name}});
                if ({{name}}Data == nullptr) {
                    return false;
                }
                {{name}}Data->generation = cmd.{{name}}.generation;
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
        DeserializeBuffer deserializeBuffer(commands, size);

        while (deserializeBuffer.AvailableSize() >= sizeof(CmdHeader) + sizeof(WireCmd)) {
            // Start by chunked command handling, if it is done, then it means the whole buffer
            // was consumed by it, so we return a pointer to the end of the commands.
            switch (HandleChunkedCommands(deserializeBuffer.Buffer(), deserializeBuffer.AvailableSize())) {
                case ChunkedCommandsResult::Consumed:
                    return commands + size;
                case ChunkedCommandsResult::Error:
                    return nullptr;
                case ChunkedCommandsResult::Passthrough:
                    break;
            }

            WireCmd cmdId = *static_cast<const volatile WireCmd*>(static_cast<const volatile void*>(
                deserializeBuffer.Buffer() + sizeof(CmdHeader)));
            bool success = false;
            switch (cmdId) {
                {% for command in cmd_records["command"] %}
                    case WireCmd::{{command.name.CamelCase()}}:
                        success = Handle{{command.name.CamelCase()}}(&deserializeBuffer);
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

        if (deserializeBuffer.AvailableSize() != 0) {
            return nullptr;
        }

        return commands;
    }

}  // namespace dawn::wire::server
