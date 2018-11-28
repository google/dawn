//* Copyright 2017 The Dawn Authors
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

#include "dawn_wire/Wire.h"
#include "dawn_wire/WireCmd.h"

#include "common/Assert.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace dawn_wire {

    namespace server {
        class Server;

        struct MapUserdata {
            Server* server;
            uint32_t bufferId;
            uint32_t bufferSerial;
            uint32_t requestSerial;
            uint32_t size;
            bool isWrite;
        };

        //* Stores what the backend knows about the type.
        template<typename T>
        struct ObjectDataBase {
            //* The backend-provided handle and serial to this object.
            T handle;
            uint32_t serial = 0;

            //* Built object ID and serial, needed to send to the client along with builder error callbacks
            //* TODO(cwallez@chromium.org) only have this for builder T
            uint32_t builtObjectId = 0;
            uint32_t builtObjectSerial = 0;

            //* Used by the error-propagation mechanism to know if this object is an error.
            //* TODO(cwallez@chromium.org): this is doubling the memory usage of
            //* std::vector<ObjectDataBase> consider making it a special marker value in handle instead.
            bool valid;
            //* Whether this object has been allocated, used by the KnownObjects queries
            //* TODO(cwallez@chromium.org): make this an internal bit vector in KnownObjects.
            bool allocated;

            //* TODO(cwallez@chromium.org): this is only useful for buffers
            void* mappedData = nullptr;
            size_t mappedDataSize = 0;
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
                    mKnown.push_back(nullObject);
                }

                //* Get a backend objects for a given client ID.
                //* Returns nullptr if the ID hasn't previously been allocated.
                const Data* Get(uint32_t id) const {
                    if (id >= mKnown.size()) {
                        return nullptr;
                    }

                    const Data* data = &mKnown[id];

                    if (!data->allocated) {
                        return nullptr;
                    }

                    return data;
                }
                Data* Get(uint32_t id) {
                    if (id >= mKnown.size()) {
                        return nullptr;
                    }

                    Data* data = &mKnown[id];

                    if (!data->allocated) {
                        return nullptr;
                    }

                    return data;
                }

                //* Allocates the data for a given ID and returns it.
                //* Returns nullptr if the ID is already allocated, or too far ahead.
                //* Invalidates all the Data*
                Data* Allocate(uint32_t id) {
                    if (id > mKnown.size()) {
                        return nullptr;
                    }

                    Data data;
                    data.allocated = true;
                    data.valid = false;
                    data.handle = nullptr;

                    if (id >= mKnown.size()) {
                        mKnown.push_back(data);
                        return &mKnown.back();
                    }

                    if (mKnown[id].allocated) {
                        return nullptr;
                    }

                    mKnown[id] = data;
                    return &mKnown[id];
                }

                //* Marks an ID as deallocated
                void Free(uint32_t id) {
                    ASSERT(id < mKnown.size());
                    mKnown[id].allocated = false;
                }

            private:
                std::vector<Data> mKnown;
        };

        void ForwardDeviceErrorToServer(const char* message, dawnCallbackUserdata userdata);

        {% for type in by_category["object"] if type.is_builder%}
            void Forward{{type.name.CamelCase()}}ToClient(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2);
        {% endfor %}

        void ForwardBufferMapReadAsync(dawnBufferMapAsyncStatus status, const void* ptr, dawnCallbackUserdata userdata);
        void ForwardBufferMapWriteAsync(dawnBufferMapAsyncStatus status, void* ptr, dawnCallbackUserdata userdata);

        // A really really simple implementation of the DeserializeAllocator. It's main feature
        // is that it has some inline storage so as to avoid allocations for the majority of
        // commands.
        class ServerAllocator : public DeserializeAllocator {
            public:
                ServerAllocator() {
                    Reset();
                }

                ~ServerAllocator() {
                    Reset();
                }

                void* GetSpace(size_t size) override {
                    // Return space in the current buffer if possible first.
                    if (mRemainingSize >= size) {
                        char* buffer = mCurrentBuffer;
                        mCurrentBuffer += size;
                        mRemainingSize -= size;
                        return buffer;
                    }

                    // Otherwise allocate a new buffer and try again.
                    size_t allocationSize = std::max(size, size_t(2048));
                    char* allocation = static_cast<char*>(malloc(allocationSize));
                    if (allocation == nullptr) {
                        return nullptr;
                    }

                    mAllocations.push_back(allocation);
                    mCurrentBuffer = allocation;
                    mRemainingSize = allocationSize;
                    return GetSpace(size);
                }

                void Reset() {
                    for (auto allocation : mAllocations) {
                        free(allocation);
                    }
                    mAllocations.clear();

                    // The initial buffer is the inline buffer so that some allocations can be skipped
                    mCurrentBuffer = mStaticBuffer;
                    mRemainingSize = sizeof(mStaticBuffer);
                }

            private:
                size_t mRemainingSize = 0;
                char* mCurrentBuffer = nullptr;
                char mStaticBuffer[2048];
                std::vector<char*> mAllocations;
        };

        class Server : public CommandHandler, public ObjectIdResolver {
            public:
                Server(dawnDevice device, const dawnProcTable& procs, CommandSerializer* serializer)
                    : mProcs(procs), mSerializer(serializer) {
                    //* The client-server knowledge is bootstrapped with device 1.
                    auto* deviceData = mKnownDevice.Allocate(1);
                    deviceData->handle = device;
                    deviceData->valid = true;

                    auto userdata = static_cast<dawnCallbackUserdata>(reinterpret_cast<intptr_t>(this));
                    procs.deviceSetErrorCallback(device, ForwardDeviceErrorToServer, userdata);
                }

                void OnDeviceError(const char* message) {
                    ReturnDeviceErrorCallbackCmd cmd;
                    cmd.messageStrlen = std::strlen(message);

                    auto allocCmd = static_cast<ReturnDeviceErrorCallbackCmd*>(GetCmdSpace(sizeof(cmd)));
                    *allocCmd = cmd;

                    char* messageAlloc = static_cast<char*>(GetCmdSpace(cmd.messageStrlen + 1));
                    strcpy(messageAlloc, message);
                }

                {% for type in by_category["object"] if type.is_builder%}
                    {% set Type = type.name.CamelCase() %}
                    void On{{Type}}Error(dawnBuilderErrorStatus status, const char* message, uint32_t id, uint32_t serial) {
                        auto* builder = mKnown{{Type}}.Get(id);

                        if (builder == nullptr || builder->serial != serial) {
                            return;
                        }

                        if (status != DAWN_BUILDER_ERROR_STATUS_SUCCESS) {
                            builder->valid = false;
                        }

                        if (status != DAWN_BUILDER_ERROR_STATUS_UNKNOWN) {
                            //* Unknown is the only status that can be returned without a call to GetResult
                            //* so we are guaranteed to have created an object.
                            ASSERT(builder->builtObjectId != 0);

                            Return{{Type}}ErrorCallbackCmd cmd;
                            cmd.builtObjectId = builder->builtObjectId;
                            cmd.builtObjectSerial = builder->builtObjectSerial;
                            cmd.status = status;
                            cmd.messageStrlen = std::strlen(message);

                            auto allocCmd = static_cast<Return{{Type}}ErrorCallbackCmd*>(GetCmdSpace(sizeof(cmd)));
                            *allocCmd = cmd;
                            char* messageAlloc = static_cast<char*>(GetCmdSpace(strlen(message) + 1));
                            strcpy(messageAlloc, message);
                        }
                    }
                {% endfor %}

                void OnMapReadAsyncCallback(dawnBufferMapAsyncStatus status, const void* ptr, MapUserdata* data) {
                    ReturnBufferMapReadAsyncCallbackCmd cmd;
                    cmd.bufferId = data->bufferId;
                    cmd.bufferSerial = data->bufferSerial;
                    cmd.requestSerial = data->requestSerial;
                    cmd.status = status;
                    cmd.dataLength = 0;

                    auto allocCmd = static_cast<ReturnBufferMapReadAsyncCallbackCmd*>(GetCmdSpace(sizeof(cmd)));
                    *allocCmd = cmd;

                    if (status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
                        allocCmd->dataLength = data->size;

                        void* dataAlloc = GetCmdSpace(data->size);
                        memcpy(dataAlloc, ptr, data->size);
                    }

                    delete data;
                }

                void OnMapWriteAsyncCallback(dawnBufferMapAsyncStatus status, void* ptr, MapUserdata* data) {
                    ReturnBufferMapWriteAsyncCallbackCmd cmd;
                    cmd.bufferId = data->bufferId;
                    cmd.bufferSerial = data->bufferSerial;
                    cmd.requestSerial = data->requestSerial;
                    cmd.status = status;

                    auto allocCmd = static_cast<ReturnBufferMapWriteAsyncCallbackCmd*>(GetCmdSpace(sizeof(cmd)));
                    *allocCmd = cmd;

                    if (status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
                        auto* selfData = mKnownBuffer.Get(data->bufferId);
                        ASSERT(selfData != nullptr);

                        selfData->mappedData = ptr;
                        selfData->mappedDataSize = data->size;
                    }

                    delete data;
                }

                const char* HandleCommands(const char* commands, size_t size) override {
                    mProcs.deviceTick(mKnownDevice.Get(1)->handle);

                    while (size >= sizeof(WireCmd)) {
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
                            case WireCmd::BufferMapAsync:
                                success = HandleBufferMapAsync(&commands, &size);
                                break;
                            case WireCmd::BufferUpdateMappedDataCmd:
                                success = HandleBufferUpdateMappedData(&commands, &size);
                                break;

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

            private:
                dawnProcTable mProcs;
                CommandSerializer* mSerializer = nullptr;

                ServerAllocator mAllocator;

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

                //* Helper function for the getting of the command data in command handlers.
                //* Checks there is enough data left, updates the buffer / size and returns
                //* the command (or nullptr for an error).
                template <typename T>
                static const T* GetData(const char** buffer, size_t* size, size_t count) {
                    // TODO(cwallez@chromium.org): Check for overflow
                    size_t totalSize = count * sizeof(T);
                    if (*size < totalSize) {
                        return nullptr;
                    }

                    const T* data = reinterpret_cast<const T*>(*buffer);

                    *buffer += totalSize;
                    *size -= totalSize;

                    return data;
                }
                template <typename T>
                static const T* GetCommand(const char** commands, size_t* size) {
                    return GetData<T>(commands, size, 1);
                }

                {% set custom_pre_handler_commands = ["BufferUnmap"] %}

                bool PreHandleBufferUnmap(const BufferUnmapCmd& cmd) {
                    auto* selfData = mKnownBuffer.Get(cmd.selfId);
                    ASSERT(selfData != nullptr);

                    selfData->mappedData = nullptr;

                    return true;
                }

                //* Implementation of the command handlers
                {% for type in by_category["object"] %}
                    {% for method in type.methods %}
                        {% set Suffix = as_MethodSuffix(type.name, method.name) %}

                        //* The generic command handlers

                        bool Handle{{Suffix}}(const char** commands, size_t* size) {
                            {{Suffix}}Cmd cmd;
                            DeserializeResult deserializeResult = cmd.Deserialize(commands, size, &mAllocator, *this);

                            if (deserializeResult == DeserializeResult::FatalError) {
                                return false;
                            }

                            {% if Suffix in custom_pre_handler_commands %}
                                if (!PreHandle{{Suffix}}(cmd)) {
                                    return false;
                                }
                            {% endif %}

                            //* Unpack 'self'
                            auto* selfData = mKnown{{type.name.CamelCase()}}.Get(cmd.selfId);
                            ASSERT(selfData != nullptr);

                            //* In all cases allocate the object data as it will be refered-to by the client.
                            {% set return_type = method.return_type %}
                            {% set returns = return_type.name.canonical_case() != "void" %}
                            {% if returns %}
                                {% set Type = method.return_type.name.CamelCase() %}
                                auto* resultData = mKnown{{Type}}.Allocate(cmd.resultId);
                                if (resultData == nullptr) {
                                    return false;
                                }
                                resultData->serial = cmd.resultSerial;

                                {% if type.is_builder %}
                                    selfData->builtObjectId = cmd.resultId;
                                    selfData->builtObjectSerial = cmd.resultSerial;
                                {% endif %}
                            {% endif %}

                            //* After the data is allocated, apply the argument error propagation mechanism
                            if (deserializeResult == DeserializeResult::ErrorObject) {
                                {% if type.is_builder %}
                                    selfData->valid = false;
                                    //* If we are in GetResult, fake an error callback
                                    {% if returns %}
                                        On{{type.name.CamelCase()}}Error(DAWN_BUILDER_ERROR_STATUS_ERROR, "Maybe monad", cmd.selfId, selfData->serial);
                                    {% endif %}
                                {% endif %}
                                return true;
                            }

                            {% if returns %}
                                auto result ={{" "}}
                            {%- endif %}
                            mProcs.{{as_varName(type.name, method.name)}}(cmd.self
                                {%- for arg in method.arguments -%}
                                    , cmd.{{as_varName(arg.name)}}
                                {%- endfor -%}
                            );

                            {% if returns %}
                                resultData->handle = result;
                                resultData->valid = result != nullptr;

                                //* builders remember the ID of the object they built so that they can send it
                                //* in the callback to the client.
                                {% if return_type.is_builder %}
                                    if (result != nullptr) {
                                        uint64_t userdata1 = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
                                        uint64_t userdata2 = (uint64_t(resultData->serial) << uint64_t(32)) + cmd.resultId;
                                        mProcs.{{as_varName(return_type.name, Name("set error callback"))}}(result, Forward{{return_type.name.CamelCase()}}ToClient, userdata1, userdata2);
                                    }
                                {% endif %}
                            {% endif %}

                            return true;
                        }
                    {% endfor %}

                    //* Handlers for the destruction of objects: clients do the tracking of the
                    //* reference / release and only send destroy on refcount = 0.
                    {% set Suffix = as_MethodSuffix(type.name, Name("destroy")) %}
                    bool Handle{{Suffix}}(const char** commands, size_t* size) {
                        const auto* cmd = GetCommand<{{Suffix}}Cmd>(commands, size);
                        if (cmd == nullptr) {
                            return false;
                        }

                        ObjectId objectId = cmd->objectId;

                        //* ID 0 are reserved for nullptr and cannot be destroyed.
                        if (objectId == 0) {
                            return false;
                        }

                        auto* data = mKnown{{type.name.CamelCase()}}.Get(objectId);
                        if (data == nullptr) {
                            return false;
                        }

                        if (data->valid) {
                            mProcs.{{as_varName(type.name, Name("release"))}}(data->handle);
                        }

                        mKnown{{type.name.CamelCase()}}.Free(objectId);
                        return true;
                    }
                {% endfor %}

                bool HandleBufferMapAsync(const char** commands, size_t* size) {
                    //* These requests are just forwarded to the buffer, with userdata containing what the client
                    //* will require in the return command.
                    const auto* cmd = GetCommand<BufferMapAsyncCmd>(commands, size);
                    if (cmd == nullptr) {
                        return false;
                    }

                    ObjectId bufferId = cmd->bufferId;
                    uint32_t requestSerial = cmd->requestSerial;
                    uint32_t requestSize = cmd->size;
                    uint32_t requestStart = cmd->start;
                    bool isWrite = cmd->isWrite;

                    //* The null object isn't valid as `self`
                    if (bufferId == 0) {
                        return false;
                    }

                    auto* buffer = mKnownBuffer.Get(bufferId);
                    if (buffer == nullptr) {
                        return false;
                    }

                    auto* data = new MapUserdata;
                    data->server = this;
                    data->bufferId = bufferId;
                    data->bufferSerial = buffer->serial;
                    data->requestSerial = requestSerial;
                    data->size = requestSize;
                    data->isWrite = isWrite;

                    auto userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data));

                    if (!buffer->valid) {
                        //* Fake the buffer returning a failure, data will be freed in this call.
                        if (isWrite) {
                            ForwardBufferMapWriteAsync(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
                        } else {
                            ForwardBufferMapReadAsync(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata);
                        }
                        return true;
                    }

                    if (isWrite) {
                        mProcs.bufferMapWriteAsync(buffer->handle, requestStart, requestSize, ForwardBufferMapWriteAsync, userdata);
                    } else {
                        mProcs.bufferMapReadAsync(buffer->handle, requestStart, requestSize, ForwardBufferMapReadAsync, userdata);
                    }

                    return true;
                }

                bool HandleBufferUpdateMappedData(const char** commands, size_t* size) {
                    const auto* cmd = GetCommand<BufferUpdateMappedDataCmd>(commands, size);
                    if (cmd == nullptr) {
                        return false;
                    }

                    ObjectId bufferId = cmd->bufferId;
                    size_t dataLength = cmd->dataLength;

                    //* The null object isn't valid as `self`
                    if (bufferId == 0) {
                        return false;
                    }

                    auto* buffer = mKnownBuffer.Get(bufferId);
                    if (buffer == nullptr || !buffer->valid || buffer->mappedData == nullptr ||
                        buffer->mappedDataSize != dataLength) {
                        return false;
                    }

                    const char* data = GetData<char>(commands, size, dataLength);
                    if (data == nullptr) {
                        return false;
                    }

                    memcpy(buffer->mappedData, data, dataLength);

                    return true;
                }
        };

        void ForwardDeviceErrorToServer(const char* message, dawnCallbackUserdata userdata) {
            auto server = reinterpret_cast<Server*>(static_cast<intptr_t>(userdata));
            server->OnDeviceError(message);
        }

        {% for type in by_category["object"] if type.is_builder%}
            void Forward{{type.name.CamelCase()}}ToClient(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2) {
                auto server = reinterpret_cast<Server*>(static_cast<uintptr_t>(userdata1));
                uint32_t id = userdata2 & 0xFFFFFFFFu;
                uint32_t serial = userdata2 >> uint64_t(32);
                server->On{{type.name.CamelCase()}}Error(status, message, id, serial);
            }
        {% endfor %}

        void ForwardBufferMapReadAsync(dawnBufferMapAsyncStatus status, const void* ptr, dawnCallbackUserdata userdata) {
            auto data = reinterpret_cast<MapUserdata*>(static_cast<uintptr_t>(userdata));
            data->server->OnMapReadAsyncCallback(status, ptr, data);
        }

        void ForwardBufferMapWriteAsync(dawnBufferMapAsyncStatus status, void* ptr, dawnCallbackUserdata userdata) {
            auto data = reinterpret_cast<MapUserdata*>(static_cast<uintptr_t>(userdata));
            data->server->OnMapWriteAsyncCallback(status, ptr, data);
        }
    }

    CommandHandler* NewServerCommandHandler(dawnDevice device, const dawnProcTable& procs, CommandSerializer* serializer) {
        return new server::Server(device, procs, serializer);
    }

}  // namespace dawn_wire
