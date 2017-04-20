//* Copyright 2017 The NXT Authors
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

#include "Wire.h"
#include "WireCmd.h"

#include <cassert>
#include <vector>

namespace nxt {
namespace wire {

    namespace server {
        //* Stores what the backend knows about the type.
        template<typename T>
        struct ObjectDataBase {
            //* The backend-provided handle to this object.
            T handle;
            //* Used by the error-propagation mechanism to know if this object is an error.
            //* TODO(cwallez@chromium.org): this is doubling the memory usae of
            //* std::vector<ObjectDataBase> consider making it a special marker value in handle instead.
            bool valid;
            //* Whether this object has been allocated, used by the KnownObjects queries
            //* TODO(cwallez@chromium.org): make this an internal bit vector in KnownObjects.
            bool allocated;
        };

        //* Keeps track of the mapping between client IDs and backend objects.
        template<typename T>
        class KnownObjects {
            public:
                using Data = ObjectDataBase<T>;

                KnownObjects() {
                    //* Pre-allocate ID 0 to refer to the null handle.
                    Data nullObject;
                    nullObject.handle = nullptr;
                    nullObject.valid = true;
                    nullObject.allocated = true;
                    known.push_back(nullObject);
                }

                //* Get a backend objects for a given client ID.
                //* Returns nullptr if the ID hasn't previously been allocated.
                Data* Get(uint32_t id) {
                    if (id >= known.size()) {
                        return nullptr;
                    }

                    Data* data = &known[id];

                    if (!data->allocated) {
                        return nullptr;
                    }

                    return data;
                }

                //* Allocates the data for a given ID and returns it.
                //* Returns nullptr if the ID is already allocated, or too far ahead.
                //* Invalidates all the Data*
                Data* Allocate(uint32_t id) {
                    if (id > known.size()) {
                        return nullptr;
                    }

                    Data data;
                    data.allocated = true;
                    data.valid = false;
                    data.handle = nullptr;

                    if (id >= known.size()) {
                        known.push_back(data);
                        return &known.back();
                    }

                    if (known[id].allocated) {
                        return nullptr;
                    }

                    known[id] = data;
                    return &known[id];
                }

                //* Marks an ID as deallocated
                void Free(uint32_t id) {
                    assert(id < known.size());
                    known[id].allocated = false;
                }

            private:
                std::vector<Data> known;
        };

        class Server : public CommandHandler {
            public:
                Server(nxtDevice device, const nxtProcTable& procs) : procs(procs) {
                    //* The client-server knowledge is bootstrapped with device 1.
                    auto* deviceData = knownDevice.Allocate(1);
                    deviceData->handle = device;
                    deviceData->valid = true;
                }

                const uint8_t* HandleCommands(const uint8_t* commands, size_t size) override {
                    while (size > sizeof(WireCmd)) {
                        WireCmd cmdId = *reinterpret_cast<const WireCmd*>(commands);

                        bool success = false;
                        switch (cmdId) {
                            {% for type in by_category["object"] %}
                                {% for method in type.methods %}
                                    {% set Suffix = as_MethodSuffix(type.name, method.name) %}
                                    case WireCmd::{{Suffix}}:
                                        success = Handle{{Suffix}}(&commands, &size);
                                        break;
                                {% endfor %}
                                {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
                                case WireCmd::{{Suffix}}:
                                    success = Handle{{Suffix}}(&commands, &size);
                                    break;
                            {% endfor %}

                            default:
                                success = false;
                        }

                        if (!success) {
                            return nullptr;
                        }
                    }

                    if (size != 0) {
                        return nullptr;
                    }

                    return commands;
                }

                void OnSynchronousError() override {
                    gotError = true;
                }

            private:
                nxtProcTable procs;
                bool gotError = false;

                //* The list of known IDs for each object type.
                {% for type in by_category["object"] %}
                    KnownObjects<{{as_cType(type.name)}}> known{{type.name.CamelCase()}};
                {% endfor %}

                //* Helper function for the getting of the command data in command handlers.
                //* Checks there is enough data left, updates the buffer / size and returns
                //* the command (or nullptr for an error).
                template<typename T>
                const T* GetCommand(const uint8_t** commands, size_t* size) {
                    if (*size < sizeof(T)) {
                        return nullptr;
                    }

                    const T* cmd = reinterpret_cast<const T*>(*commands);

                    size_t cmdSize = cmd->GetRequiredSize();
                    if (*size < cmdSize) {
                        return nullptr;
                    }

                    *commands += cmdSize;
                    *size -= cmdSize;

                    return cmd;
                }

                //* Implementation of the command handlers
                {% for type in by_category["object"] %}
                    {% for method in type.methods %}
                        {% set Suffix = as_MethodSuffix(type.name, method.name) %}

                        //* The generic command handlers

                        bool Handle{{Suffix}}(const uint8_t** commands, size_t* size) {
                            //* Get command ptr, and check it fits in the buffer.
                            const auto* cmd = GetCommand<{{Suffix}}Cmd>(commands, size);
                            if (cmd == nullptr) {
                                return false;
                            }

                            //* While unpacking arguments, if any of them is an error, valid will be set to false.
                            bool valid = true;

                            //* Unpack 'self'
                            {% set Type = type.name.CamelCase() %}
                            {{as_cType(type.name)}} self;
                            auto* selfData = known{{Type}}.Get(cmd->self);
                            {
                                if (selfData == nullptr) {
                                    return false;
                                }
                                valid = valid && selfData->valid;
                                self = selfData->handle;
                            }

                            //* Unpack value objects from IDs.
                            {% for arg in method.arguments if arg.annotation == "value" and arg.type.category == "object" %}
                                {% set Type = arg.type.name.CamelCase() %}
                                {{as_cType(arg.type.name)}} arg_{{as_varName(arg.name)}};
                                {
                                    auto* data = known{{Type}}.Get(cmd->{{as_varName(arg.name)}});
                                    if (data == nullptr) {
                                        return false;
                                    }
                                    valid = valid && data->valid;
                                    arg_{{as_varName(arg.name)}} = data->handle;
                                }
                            {% endfor %}

                            //* Unpack pointer arguments
                            {% for arg in method.arguments if arg.annotation != "value" %}
                                {% set argName = as_varName(arg.name) %}
                                const {{as_cType(arg.type.name)}}* arg_{{argName}};
                                {% if arg.length == "strlen" %}
                                    //* Unpack strings, checking they are null-terminated.
                                    arg_{{argName}} = reinterpret_cast<const {{as_cType(arg.type.name)}}*>(cmd->GetPtr_{{argName}}());
                                    if (arg_{{argName}}[cmd->{{argName}}Strlen] != 0) {
                                        return false;
                                    }
                                {% elif arg.type.category == "object" %}
                                    //* Unpack arrays of objects.
                                    //* TODO(cwallez@chromium.org) do not allocate when there are few objects.
                                    std::vector<{{as_cType(arg.type.name)}}> {{argName}}Storage(cmd->{{as_varName(arg.length.name)}});
                                    auto {{argName}}Ids = reinterpret_cast<const uint32_t*>(cmd->GetPtr_{{argName}}());
                                    for (size_t i = 0; i < cmd->{{as_varName(arg.length.name)}}; i++) {
                                        {% set Type = arg.type.name.CamelCase() %}
                                        auto* data = known{{Type}}.Get({{argName}}Ids[i]);
                                        if (data == nullptr) {
                                            return false;
                                        }
                                        {{argName}}Storage[i] = data->handle;
                                        valid = valid && data->valid;
                                    }
                                    arg_{{argName}} = {{argName}}Storage.data();
                                {% else %}
                                    //* For anything else, just get the pointer.
                                    arg_{{argName}} = reinterpret_cast<const {{as_cType(arg.type.name)}}*>(cmd->GetPtr_{{argName}}());
                                {% endif %}
                            {% endfor %}

                            //* At that point all the data has been upacked in cmd->* or arg_*

                            //* In all cases allocate the object data as it will be refered-to by the client.
                            {% set returns = method.return_type.name.canonical_case() != "void" %}
                            {% if returns %}
                                {% set Type = method.return_type.name.CamelCase() %}
                                auto* resultData = known{{Type}}.Allocate(cmd->resultId);
                                if (resultData == nullptr) {
                                    return false;
                                }
                            {% endif %}

                            //* After the data is allocated, apply the argument error propagation mechanism
                            if (!valid) {
                                return true;
                            }

                            {% if returns -%}
                                auto result =
                            {%- endif -%}
                            procs.{{as_varName(type.name, method.name)}}(self
                                {%- for arg in method.arguments -%}
                                    {% if arg.annotation == "value" and arg.type.category != "object" %}
                                        , cmd->{{as_varName(arg.name)}}
                                    {% else %}
                                        , arg_{{as_varName(arg.name)}}
                                    {% endif %}
                                {%- endfor -%}
                            );

                            {% if returns %}
                                resultData->handle = result;
                                resultData->valid = result != nullptr;
                            {% endif %}

                            if (gotError) {
                                {% if type.is_builder %}
                                    //* Get the data again, has been invalidated by the call to
                                    //* known.Allocate
                                    known{{type.name.CamelCase()}}.Get(cmd->self)->valid = false;
                                {% endif %}
                                gotError = false;
                            }

                            return true;
                        }
                    {% endfor %}

                    //* Handlers for the destruction of objects: clients do the tracking of the
                    //* reference / release and only send destroy on refcount = 0.
                    {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
                    bool Handle{{Suffix}}(const uint8_t** commands, size_t* size) {
                        const auto* cmd = GetCommand<{{Suffix}}Cmd>(commands, size);
                        if (cmd == nullptr) {
                            return false;
                        }

                        //* ID 0 are reserved for nullptr and cannot be destroyed.
                        if (cmd->objectId == 0) {
                            return false;
                        }

                        auto* data = known{{type.name.CamelCase()}}.Get(cmd->objectId);
                        if (data == nullptr) {
                            return false;
                        }

                        if (data->valid) {
                            procs.{{as_varName(type.name, Name("release"))}}(data->handle);
                        }

                        known{{type.name.CamelCase()}}.Free(cmd->objectId);
                        return true;
                    }
                {% endfor %}
        };

    }

    CommandHandler* CreateCommandHandler(nxtDevice device, const nxtProcTable& procs) {
        return new server::Server(device, procs);
    }

}
}
