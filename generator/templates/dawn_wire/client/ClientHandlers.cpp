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
#include "dawn_wire/client/Client.h"

#include <string>

namespace dawn_wire { namespace client {
    {% for type in by_category["object"] if type.is_builder %}
        {% set Type = type.name.CamelCase() %}
        bool Client::Handle{{Type}}ErrorCallback(const char** commands, size_t* size) {
            Return{{Type}}ErrorCallbackCmd cmd;
            DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator);

            if (deserializeResult == DeserializeResult::FatalError) {
                return false;
            }

            DAWN_ASSERT(cmd.message != nullptr);

            auto* builtObject = mDevice->GetClient()->{{type.built_type.name.CamelCase()}}Allocator().GetObject(cmd.builtObject.id);
            uint32_t objectSerial = mDevice->GetClient()->{{type.built_type.name.CamelCase()}}Allocator().GetSerial(cmd.builtObject.id);

            //* The object might have been deleted or a new object created with the same ID.
            if (builtObject == nullptr || objectSerial != cmd.builtObject.serial) {
                return true;
            }

            bool called = builtObject->builderCallback.Call(static_cast<dawnBuilderErrorStatus>(cmd.status), cmd.message);

            // Unhandled builder errors are forwarded to the device
            if (!called && cmd.status != DAWN_BUILDER_ERROR_STATUS_SUCCESS && cmd.status != DAWN_BUILDER_ERROR_STATUS_UNKNOWN) {
                mDevice->HandleError(("Unhandled builder error: " + std::string(cmd.message)).c_str());
            }

            return true;
        }
    {% endfor %}

    const char* Client::HandleCommands(const char* commands, size_t size) {
        while (size >= sizeof(ReturnWireCmd)) {
            ReturnWireCmd cmdId = *reinterpret_cast<const ReturnWireCmd*>(commands);

            bool success = false;
            switch (cmdId) {
                {% for command in cmd_records["return command"] %}
                    {% set Suffix = command.name.CamelCase() %}
                    case ReturnWireCmd::{{Suffix}}:
                        success = Handle{{Suffix}}(&commands, &size);
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
}}  // namespace dawn_wire::client
