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

#ifndef DAWNWIRE_SERVER_SERVERBASE_H_
#define DAWNWIRE_SERVER_SERVERBASE_H_

#include "dawn_wire/Wire.h"
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/WireDeserializeAllocator.h"
#include "dawn_wire/server/ObjectStorage.h"

namespace dawn_wire { namespace server {

    class Server;

    struct MapUserdata {
        Server* server;
        ObjectHandle buffer;
        uint32_t requestSerial;
        uint32_t size;
        bool isWrite;
    };

    struct FenceCompletionUserdata {
        Server* server;
        ObjectHandle fence;
        uint64_t value;
    };

    class ServerBase : public ObjectIdResolver {
      public:
        ServerBase(dawnDevice device, const dawnProcTable& procs, CommandSerializer* serializer)
            : mProcs(procs), mSerializer(serializer) {
        }

        virtual ~ServerBase() {
            //* Free all objects when the server is destroyed
            {% for type in by_category["object"] if type.name.canonical_case() != "device" %}
                {
                    std::vector<{{as_cType(type.name)}}> handles = mKnown{{type.name.CamelCase()}}.AcquireAllHandles();
                    for ({{as_cType(type.name)}} handle : handles) {
                        mProcs.{{as_varName(type.name, Name("release"))}}(handle);
                    }
                }
            {% endfor %}
        }

      protected:
        dawnProcTable mProcs;
        CommandSerializer* mSerializer = nullptr;

        WireDeserializeAllocator mAllocator;

        void* GetCmdSpace(size_t size) {
            return mSerializer->GetCmdSpace(size);
        }

        // Implementation of the ObjectIdResolver interface
        {% for type in by_category["object"] %}
            DeserializeResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const final {
                auto data = mKnown{{type.name.CamelCase()}}.Get(id);
                if (data == nullptr) {
                    return DeserializeResult::FatalError;
                }

                *out = data->handle;
                if (data->valid) {
                    return DeserializeResult::Success;
                } else {
                    return DeserializeResult::ErrorObject;
                }
            }

            DeserializeResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const final {
                if (id == 0) {
                    *out = nullptr;
                    return DeserializeResult::Success;
                }

                return GetFromId(id, out);
            }
        {% endfor %}

        //* The list of known IDs for each object type.
        {% for type in by_category["object"] %}
            KnownObjects<{{as_cType(type.name)}}> mKnown{{type.name.CamelCase()}};
        {% endfor %}

        {% for type in by_category["object"] if type.name.CamelCase() in server_reverse_lookup_objects %}
            ObjectIdLookupTable<{{as_cType(type.name)}}> m{{type.name.CamelCase()}}IdTable;
        {% endfor %}
    };

}}  // namespace dawn_wire::server

#endif  // DAWNWIRE_SERVER_SERVERBASE_H_
