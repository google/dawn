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

#ifndef DAWNWIRE_CLIENT_CLIENTBASE_AUTOGEN_H_
#define DAWNWIRE_CLIENT_CLIENTBASE_AUTOGEN_H_

#include "dawn/wire/ChunkedCommandHandler.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects.h"

namespace dawn::wire::client {

    class ClientBase : public ChunkedCommandHandler, public ObjectIdProvider {
      public:
        ClientBase() = default;
        ~ClientBase() override = default;

      private:
        // Implementation of the ObjectIdProvider interface
        {% for type in by_category["object"] %}
            WireResult GetId({{as_cType(type.name)}} object, ObjectId* out) const final {
                ASSERT(out != nullptr);
                if (object == nullptr) {
                    return WireResult::FatalError;
                }
                *out = reinterpret_cast<{{as_wireType(type)}}>(object)->GetWireId();
                return WireResult::Success;
            }
            WireResult GetOptionalId({{as_cType(type.name)}} object, ObjectId* out) const final {
                ASSERT(out != nullptr);
                *out = (object == nullptr ? 0 : reinterpret_cast<{{as_wireType(type)}}>(object)->GetWireId());
                return WireResult::Success;
            }
        {% endfor %}
    };

}  // namespace dawn::wire::client

#endif  // DAWNWIRE_CLIENT_CLIENTBASE_AUTOGEN_H_
