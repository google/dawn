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
    {% for type in by_category["object"] if type.is_builder%}
        void Server::Forward{{type.name.CamelCase()}}ToClient(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2) {
            auto server = reinterpret_cast<Server*>(static_cast<uintptr_t>(userdata1));
            uint32_t id = userdata2 & 0xFFFFFFFFu;
            uint32_t serial = userdata2 >> uint64_t(32);
            server->On{{type.name.CamelCase()}}Error(status, message, id, serial);
        }
    {% endfor %}

    {% for type in by_category["object"] if type.is_builder%}
        {% set Type = type.name.CamelCase() %}
        void Server::On{{Type}}Error(dawnBuilderErrorStatus status, const char* message, uint32_t id, uint32_t serial) {
            auto* builder = {{Type}}Objects().Get(id);

            if (builder == nullptr || builder->serial != serial) {
                return;
            }

            if (status != DAWN_BUILDER_ERROR_STATUS_SUCCESS) {
                builder->valid = false;
            }

            if (status != DAWN_BUILDER_ERROR_STATUS_UNKNOWN) {
                //* Unknown is the only status that can be returned without a call to GetResult
                //* so we are guaranteed to have created an object.
                ASSERT(builder->builtObject.id != 0);

                Return{{Type}}ErrorCallbackCmd cmd;
                cmd.builtObject = builder->builtObject;
                cmd.status = status;
                cmd.message = message;

                size_t requiredSize = cmd.GetRequiredSize();
                char* allocatedBuffer = static_cast<char*>(GetCmdSpace(requiredSize));
                cmd.Serialize(allocatedBuffer);
            }
        }
    {% endfor %}

}}  // namespace dawn_wire::server
