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

#ifndef DAWNWIRE_CLIENT_DEVICE_AUTOGEN_H_
#define DAWNWIRE_CLIENT_DEVICE_AUTOGEN_H_

#include <dawn/dawn.h>

#include "dawn_wire/Wire.h"
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/client/ApiObjects.h"
#include "dawn_wire/client/ObjectAllocator.h"

namespace dawn_wire { namespace client {
    //* The client wire uses the global Dawn device to store its global data such as the serializer
    //* and the object id allocators.
    class Device : public ObjectBase, public ObjectIdProvider {
        public:
            Device(CommandSerializer* serializer)
                : ObjectBase(this, 1, 1),
                {% for type in by_category["object"] if not type.name.canonical_case() == "device" %}
                    {{type.name.camelCase()}}(this),
                {% endfor %}
                mSerializer(serializer) {
            }

            void* GetCmdSpace(size_t size) {
                return mSerializer->GetCmdSpace(size);
            }

            {% for type in by_category["object"] if not type.name.canonical_case() == "device" %}
                ObjectAllocator<{{type.name.CamelCase()}}> {{type.name.camelCase()}};
            {% endfor %}

            // Implementation of the ObjectIdProvider interface
            {% for type in by_category["object"] %}
                ObjectId GetId({{as_cType(type.name)}} object) const final {
                    return reinterpret_cast<{{as_wireType(type)}}>(object)->id;
                }
                ObjectId GetOptionalId({{as_cType(type.name)}} object) const final {
                    if (object == nullptr) {
                        return 0;
                    }
                    return GetId(object);
                }
            {% endfor %}

            void HandleError(const char* message) {
                if (errorCallback) {
                    errorCallback(message, errorUserdata);
                }
            }

            dawnDeviceErrorCallback errorCallback = nullptr;
            dawnCallbackUserdata errorUserdata;

        private:
            CommandSerializer* mSerializer = nullptr;
    };
}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_DEVICE_AUTOGEN_H_
