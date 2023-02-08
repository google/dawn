// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMFuzzer.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/webgpu.h"
#include "dawn/wire/client/Client.h"


namespace dawn::wire {

void SerializedData(const fuzzing::Program& program, dawn::wire::ChunkedCommandSerializer serializer) {
    DawnLPMObjectIdProvider provider;

    for (const fuzzing::Command& command : program.commands()) {
        switch (command.command_case()) {

            {% for command in cmd_records["command"] %}
            case fuzzing::Command::k{{command.name.CamelCase()}}: {
                {{ command.name.CamelCase() }}Cmd {{ 'cmd' }};
                // TODO(chromium:1374747): Populate command buffer with serialized code from generated
                // protobuf structures. Currently, this will nullptr-deref constantly.
                memset(&{{ 'cmd' }}, 0, sizeof({{ command.name.CamelCase() }}Cmd));
                serializer.SerializeCommand(cmd, provider);
                break;
            }
            {% endfor %}
            default: {
                break;
            }
        }
    }
}

} // namespace dawn::wire
