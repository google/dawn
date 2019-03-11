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

#include "dawn_wire/client/Client.h"

#include <string>

namespace dawn_wire {
    namespace client {
        {% for type in by_category["object"] if type.is_builder %}
            {% set Type = type.name.CamelCase() %}
            bool Client::Do{{Type}}ErrorCallback({{type.built_type.name.CamelCase()}}* object, uint32_t status, const char* message) {
                //* The object might have been deleted or a new object created with the same ID.
                if (object == nullptr) {
                    return true;
                }
                bool called = object->builderCallback.Call(static_cast<DawnBuilderErrorStatus>(status), message);

                //* Unhandled builder errors are forwarded to the device
                if (!called && status != DAWN_BUILDER_ERROR_STATUS_SUCCESS && status != DAWN_BUILDER_ERROR_STATUS_UNKNOWN) {
                    mDevice->HandleError(("Unhandled builder error: " + std::string(message)).c_str());
                }

                return true;
            }
        {% endfor %}
    }
}
