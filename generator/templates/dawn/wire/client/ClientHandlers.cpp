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
#include "dawn/wire/client/Client.h"

#include <string>

namespace dawn::wire::client {
    {% for command in cmd_records["return command"] %}
        bool Client::Handle{{command.name.CamelCase()}}(DeserializeBuffer* deserializeBuffer) {
            Return{{command.name.CamelCase()}}Cmd cmd;
            WireResult deserializeResult = cmd.Deserialize(deserializeBuffer, &mWireCommandAllocator);

            if (deserializeResult == WireResult::FatalError) {
                return false;
            }

            {% for member in command.members if member.handle_type %}
                {% set Type = member.handle_type.name.CamelCase() %}
                {% set name = as_varName(member.name) %}

                {% if member.type.dict_name == "ObjectHandle" %}
                    {{Type}}* {{name}} = Get<{{Type}}>(cmd.{{name}}.id);
                    if ({{name}} != nullptr && {{name}}->GetWireGeneration() != cmd.{{name}}.generation) {
                        {{name}} = nullptr;
                    }
                {% endif %}
            {% endfor %}

            return Do{{command.name.CamelCase()}}(
                {%- for member in command.members -%}
                    {%- if member.handle_type -%}
                        {{as_varName(member.name)}}
                    {%- else -%}
                        cmd.{{as_varName(member.name)}}
                    {%- endif -%}
                    {%- if not loop.last -%}, {% endif %}
                {%- endfor -%}
            );
        }
    {% endfor %}

    const volatile char* Client::HandleCommandsImpl(const volatile char* commands, size_t size) {
        DeserializeBuffer deserializeBuffer(commands, size);

        while (deserializeBuffer.AvailableSize() >= sizeof(CmdHeader) + sizeof(ReturnWireCmd)) {
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

            ReturnWireCmd cmdId = *static_cast<const volatile ReturnWireCmd*>(static_cast<const volatile void*>(
                deserializeBuffer.Buffer() + sizeof(CmdHeader)));
            bool success = false;
            switch (cmdId) {
                {% for command in cmd_records["return command"] %}
                    {% set Suffix = command.name.CamelCase() %}
                    case ReturnWireCmd::{{Suffix}}:
                        success = Handle{{Suffix}}(&deserializeBuffer);
                        break;
                {% endfor %}
                default:
                    success = false;
            }

            if (!success) {
                return nullptr;
            }
            mWireCommandAllocator.Reset();
        }

        if (deserializeBuffer.AvailableSize() != 0) {
            return nullptr;
        }

        return commands;
    }
}  // namespace dawn::wire::client
