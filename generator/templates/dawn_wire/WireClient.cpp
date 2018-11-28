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

#include <cstring>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dawn_wire {

    //* Client side implementation of the API, will serialize everything to memory to send to the server side.
    namespace client {

        class Device;

        struct BuilderCallbackData {
            bool Call(dawnBuilderErrorStatus status, const char* message) {
                if (canCall && callback != nullptr) {
                    canCall = true;
                    callback(status, message, userdata1, userdata2);
                    return true;
                }

                return false;
            }

            //* For help with development, prints all builder errors by default.
            dawnBuilderErrorCallback callback = nullptr;
            dawnCallbackUserdata userdata1 = 0;
            dawnCallbackUserdata userdata2 = 0;
            bool canCall = true;
        };

        //* All non-Device objects of the client side have:
        //*  - A pointer to the device to get where to serialize commands
        //*  - The external reference count
        //*  - An ID that is used to refer to this object when talking with the server side
        struct ObjectBase {
            ObjectBase(Device* device, uint32_t refcount, uint32_t id)
                :device(device), refcount(refcount), id(id) {
            }

            Device* device;
            uint32_t refcount;
            uint32_t id;

            BuilderCallbackData builderCallback;
        };

        {% set special_objects = [
            "device",
            "buffer",
        ] %}
        {% for type in by_category["object"] if not type.name.canonical_case() in special_objects %}
            struct {{type.name.CamelCase()}} : ObjectBase {
                using ObjectBase::ObjectBase;
            };
        {% endfor %}

        struct Buffer : ObjectBase {
            using ObjectBase::ObjectBase;

            ~Buffer() {
                //* Callbacks need to be fired in all cases, as they can handle freeing resources
                //* so we call them with "Unknown" status.
                ClearMapRequests(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN);

                if (mappedData) {
                    free(mappedData);
                }
            }

            void ClearMapRequests(dawnBufferMapAsyncStatus status) {
                for (auto& it : requests) {
                    if (it.second.isWrite) {
                        it.second.writeCallback(status, nullptr, it.second.userdata);
                    } else {
                        it.second.readCallback(status, nullptr, it.second.userdata);
                    }
                }
                requests.clear();
            }

            //* We want to defer all the validation to the server, which means we could have multiple
            //* map request in flight at a single time and need to track them separately.
            //* On well-behaved applications, only one request should exist at a single time.
            struct MapRequestData {
                dawnBufferMapReadCallback readCallback = nullptr;
                dawnBufferMapWriteCallback writeCallback = nullptr;
                dawnCallbackUserdata userdata = 0;
                uint32_t size = 0;
                bool isWrite = false;
            };
            std::map<uint32_t, MapRequestData> requests;
            uint32_t requestSerial = 0;

            //* Only one mapped pointer can be active at a time because Unmap clears all the in-flight requests.
            void* mappedData = nullptr;
            size_t mappedDataSize = 0;
            bool isWriteMapped = false;
        };

        //* TODO(cwallez@chromium.org): Do something with objects before they are destroyed ?
        //*  - Call still uncalled builder callbacks
        template<typename T>
        class ObjectAllocator {
            public:
                struct ObjectAndSerial {
                    ObjectAndSerial(std::unique_ptr<T> object, uint32_t serial)
                        : object(std::move(object)), serial(serial) {
                    }
                    std::unique_ptr<T> object;
                    uint32_t serial;
                };

                ObjectAllocator(Device* device) : mDevice(device) {
                    // ID 0 is nullptr
                    mObjects.emplace_back(nullptr, 0);
                }

                ObjectAndSerial* New() {
                    uint32_t id = GetNewId();
                    T* result = new T(mDevice, 1, id);
                    auto object = std::unique_ptr<T>(result);

                    if (id >= mObjects.size()) {
                        ASSERT(id == mObjects.size());
                        mObjects.emplace_back(std::move(object), 0);
                    } else {
                        ASSERT(mObjects[id].object == nullptr);
                        //* TODO(cwallez@chromium.org): investigate if overflows could cause bad things to happen
                        mObjects[id].serial++;
                        mObjects[id].object = std::move(object);
                    }

                    return &mObjects[id];
                }
                void Free(T* obj) {
                    FreeId(obj->id);
                    mObjects[obj->id].object = nullptr;
                }

                T* GetObject(uint32_t id) {
                    if (id >= mObjects.size()) {
                        return nullptr;
                    }
                    return mObjects[id].object.get();
                }

                uint32_t GetSerial(uint32_t id) {
                    if (id >= mObjects.size()) {
                        return 0;
                    }
                    return mObjects[id].serial;
                }

            private:
                uint32_t GetNewId() {
                    if (mFreeIds.empty()) {
                        return mCurrentId ++;
                    }
                    uint32_t id = mFreeIds.back();
                    mFreeIds.pop_back();
                    return id;
                }
                void FreeId(uint32_t id) {
                    mFreeIds.push_back(id);
                }

                // 0 is an ID reserved to represent nullptr
                uint32_t mCurrentId = 1;
                std::vector<uint32_t> mFreeIds;
                std::vector<ObjectAndSerial> mObjects;
                Device* mDevice;
        };

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

        //* Implementation of the client API functions.
        {% for type in by_category["object"] %}
            {% set Type = type.name.CamelCase() %}
            {% set cType = as_cType(type.name) %}

            {% for method in type.methods %}
                {% set Suffix = as_MethodSuffix(type.name, method.name) %}

                {{as_cType(method.return_type.name)}} Client{{Suffix}}(
                    {{-cType}} cSelf
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) {
                    auto self = reinterpret_cast<{{as_wireType(type)}}>(cSelf);
                    Device* device = self->device;
                    {{Suffix}}Cmd cmd;

                    //* Create the structure going on the wire on the stack and fill it with the value
                    //* arguments so it can compute its size.
                    cmd.self = cSelf;

                    //* For object creation, store the object ID the client will use for the result.
                    {% if method.return_type.category == "object" %}
                        auto* allocation = self->device->{{method.return_type.name.camelCase()}}.New();

                        {% if type.is_builder %}
                            //* We are in GetResult, so the callback that should be called is the
                            //* currently set one. Copy it over to the created object and prevent the
                            //* builder from calling the callback on destruction.
                            allocation->object->builderCallback = self->builderCallback;
                            self->builderCallback.canCall = false;
                        {% endif %}

                        cmd.resultId = allocation->object->id;
                        cmd.resultSerial = allocation->serial;
                    {% endif %}

                    {% for arg in method.arguments %}
                        cmd.{{as_varName(arg.name)}} = {{as_varName(arg.name)}};
                    {% endfor %}

                    //* Allocate space to send the command and copy the value args over.
                    size_t requiredSize = cmd.GetRequiredSize();
                    char* allocatedBuffer = static_cast<char*>(device->GetCmdSpace(requiredSize));
                    cmd.Serialize(allocatedBuffer, *device);

                    {% if method.return_type.category == "object" %}
                        return reinterpret_cast<{{as_cType(method.return_type.name)}}>(allocation->object.get());
                    {% endif %}
                }
            {% endfor %}

            {% if type.is_builder %}
                void Client{{as_MethodSuffix(type.name, Name("set error callback"))}}({{cType}} cSelf,
                                                                                      dawnBuilderErrorCallback callback,
                                                                                      dawnCallbackUserdata userdata1,
                                                                                      dawnCallbackUserdata userdata2) {
                    {{Type}}* self = reinterpret_cast<{{Type}}*>(cSelf);
                    self->builderCallback.callback = callback;
                    self->builderCallback.userdata1 = userdata1;
                    self->builderCallback.userdata2 = userdata2;
                }
            {% endif %}

            {% if not type.name.canonical_case() == "device" %}
                //* When an object's refcount reaches 0, notify the server side of it and delete it.
                void Client{{as_MethodSuffix(type.name, Name("release"))}}({{cType}} cObj) {
                    {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
                    obj->refcount --;

                    if (obj->refcount > 0) {
                        return;
                    }

                    obj->builderCallback.Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, "Unknown");

                    {{as_MethodSuffix(type.name, Name("destroy"))}}Cmd cmd;
                    cmd.objectId = obj->id;

                    auto allocCmd = static_cast<decltype(cmd)*>(obj->device->GetCmdSpace(sizeof(cmd)));
                    *allocCmd = cmd;

                    obj->device->{{type.name.camelCase()}}.Free(obj);
                }

                void Client{{as_MethodSuffix(type.name, Name("reference"))}}({{cType}} cObj) {
                    {{Type}}* obj = reinterpret_cast<{{Type}}*>(cObj);
                    obj->refcount ++;
                }
            {% endif %}
        {% endfor %}

        void ClientBufferMapReadAsync(dawnBuffer cBuffer, uint32_t start, uint32_t size, dawnBufferMapReadCallback callback, dawnCallbackUserdata userdata) {
            Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

            uint32_t serial = buffer->requestSerial++;
            ASSERT(buffer->requests.find(serial) == buffer->requests.end());

            Buffer::MapRequestData request;
            request.readCallback = callback;
            request.userdata = userdata;
            request.size = size;
            request.isWrite = false;
            buffer->requests[serial] = request;

            BufferMapAsyncCmd cmd;
            cmd.bufferId = buffer->id;
            cmd.requestSerial = serial;
            cmd.start = start;
            cmd.size = size;
            cmd.isWrite = false;

            auto allocCmd = static_cast<decltype(cmd)*>(buffer->device->GetCmdSpace(sizeof(cmd)));
            *allocCmd = cmd;
        }

        void ClientBufferMapWriteAsync(dawnBuffer cBuffer, uint32_t start, uint32_t size, dawnBufferMapWriteCallback callback, dawnCallbackUserdata userdata) {
            Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

            uint32_t serial = buffer->requestSerial++;
            ASSERT(buffer->requests.find(serial) == buffer->requests.end());

            Buffer::MapRequestData request;
            request.writeCallback = callback;
            request.userdata = userdata;
            request.size = size;
            request.isWrite = true;
            buffer->requests[serial] = request;

            BufferMapAsyncCmd cmd;
            cmd.bufferId = buffer->id;
            cmd.requestSerial = serial;
            cmd.start = start;
            cmd.size = size;
            cmd.isWrite = true;

            auto allocCmd = static_cast<decltype(cmd)*>(buffer->device->GetCmdSpace(sizeof(cmd)));
            *allocCmd = cmd;
        }

        void ProxyClientBufferUnmap(dawnBuffer cBuffer) {
            Buffer* buffer = reinterpret_cast<Buffer*>(cBuffer);

            //* Invalidate the local pointer, and cancel all other in-flight requests that would turn into
            //* errors anyway (you can't double map). This prevents race when the following happens, where
            //* the application code would have unmapped a buffer but still receive a callback:
            //*  - Client -> Server: MapRequest1, Unmap, MapRequest2
            //*  - Server -> Client: Result of MapRequest1
            //*  - Unmap locally on the client
            //*  - Server -> Client: Result of MapRequest2
            if (buffer->mappedData) {

                // If the buffer was mapped for writing, send the update to the data to the server
                if (buffer->isWriteMapped) {
                    BufferUpdateMappedDataCmd cmd;
                    cmd.bufferId = buffer->id;
                    cmd.dataLength = static_cast<uint32_t>(buffer->mappedDataSize);

                    auto allocCmd = static_cast<decltype(cmd)*>(buffer->device->GetCmdSpace(sizeof(cmd)));
                    *allocCmd = cmd;

                    void* dataAlloc = buffer->device->GetCmdSpace(cmd.dataLength);
                    memcpy(dataAlloc, buffer->mappedData, cmd.dataLength);
                }

                free(buffer->mappedData);
                buffer->mappedData = nullptr;
            }
            buffer->ClearMapRequests(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN);

            ClientBufferUnmap(cBuffer);
        }

        void ClientDeviceReference(dawnDevice) {
        }

        void ClientDeviceRelease(dawnDevice) {
        }

        void ClientDeviceSetErrorCallback(dawnDevice cSelf, dawnDeviceErrorCallback callback, dawnCallbackUserdata userdata) {
            Device* self = reinterpret_cast<Device*>(cSelf);
            self->errorCallback = callback;
            self->errorUserdata = userdata;
        }

        // Some commands don't have a custom wire format, but need to be handled manually to update
        // some client-side state tracking. For these we have two functions:
        //  - An autogenerated Client{{suffix}} method that sends the command on the wire
        //  - A manual ProxyClient{{suffix}} method that will be inserted in the proctable instead of
        //    the autogenerated one, and that will have to call Client{{suffix}}
        {% set proxied_commands = ["BufferUnmap"] %}

        dawnProcTable GetProcs() {
            dawnProcTable table;
            {% for type in by_category["object"] %}
                {% for method in native_methods(type) %}
                    {% set suffix = as_MethodSuffix(type.name, method.name) %}
                    {% if suffix in proxied_commands %}
                        table.{{as_varName(type.name, method.name)}} = ProxyClient{{suffix}};
                    {% else %}
                        table.{{as_varName(type.name, method.name)}} = Client{{suffix}};
                    {% endif %}
                {% endfor %}
            {% endfor %}
            return table;
        }

        class Client : public CommandHandler {
            public:
                Client(Device* device) : mDevice(device) {
                }

                const char* HandleCommands(const char* commands, size_t size) override {
                    while (size >= sizeof(ReturnWireCmd)) {
                        ReturnWireCmd cmdId = *reinterpret_cast<const ReturnWireCmd*>(commands);

                        bool success = false;
                        switch (cmdId) {
                            case ReturnWireCmd::DeviceErrorCallback:
                                success = HandleDeviceErrorCallbackCmd(&commands, &size);
                                break;
                            {% for type in by_category["object"] if type.is_builder %}
                                case ReturnWireCmd::{{type.name.CamelCase()}}ErrorCallback:
                                    success = Handle{{type.name.CamelCase()}}ErrorCallbackCmd(&commands, &size);
                                    break;
                            {% endfor %}
                            case ReturnWireCmd::BufferMapReadAsyncCallback:
                                success = HandleBufferMapReadAsyncCallback(&commands, &size);
                                break;
                            case ReturnWireCmd::BufferMapWriteAsyncCallback:
                                success = HandleBufferMapWriteAsyncCallback(&commands, &size);
                                break;
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

            private:
                Device* mDevice = nullptr;

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

                bool HandleDeviceErrorCallbackCmd(const char** commands, size_t* size) {
                    const auto* cmd = GetCommand<ReturnDeviceErrorCallbackCmd>(commands, size);
                    if (cmd == nullptr) {
                        return false;
                    }

                    const char* message = GetData<char>(commands, size, cmd->messageStrlen + 1);
                    if (message == nullptr || message[cmd->messageStrlen] != '\0') {
                        return false;
                    }

                    mDevice->HandleError(message);

                    return true;
                }

                {% for type in by_category["object"] if type.is_builder %}
                    {% set Type = type.name.CamelCase() %}
                    bool Handle{{Type}}ErrorCallbackCmd(const char** commands, size_t* size) {
                        const auto* cmd = GetCommand<Return{{Type}}ErrorCallbackCmd>(commands, size);
                        if (cmd == nullptr) {
                            return false;
                        }

                        const char* message = GetData<char>(commands, size, cmd->messageStrlen + 1);
                        if (message == nullptr || message[cmd->messageStrlen] != '\0') {
                            return false;
                        }

                        auto* builtObject = mDevice->{{type.built_type.name.camelCase()}}.GetObject(cmd->builtObjectId);
                        uint32_t objectSerial = mDevice->{{type.built_type.name.camelCase()}}.GetSerial(cmd->builtObjectId);

                        //* The object might have been deleted or a new object created with the same ID.
                        if (builtObject == nullptr || objectSerial != cmd->builtObjectSerial) {
                            return true;
                        }

                        bool called = builtObject->builderCallback.Call(static_cast<dawnBuilderErrorStatus>(cmd->status), message);

                        // Unhandled builder errors are forwarded to the device
                        if (!called && cmd->status != DAWN_BUILDER_ERROR_STATUS_SUCCESS && cmd->status != DAWN_BUILDER_ERROR_STATUS_UNKNOWN) {
                            mDevice->HandleError(("Unhandled builder error: " + std::string(message)).c_str());
                        }

                        return true;
                    }
                {% endfor %}

                bool HandleBufferMapReadAsyncCallback(const char** commands, size_t* size) {
                    const auto* cmd = GetCommand<ReturnBufferMapReadAsyncCallbackCmd>(commands, size);
                    if (cmd == nullptr) {
                        return false;
                    }

                    //* Unconditionnally get the data from the buffer so that the correct amount of data is
                    //* consumed from the buffer, even when we ignore the command and early out.
                    const char* requestData = nullptr;
                    if (cmd->status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
                        requestData = GetData<char>(commands, size, cmd->dataLength);
                        if (requestData == nullptr) {
                            return false;
                        }
                    }

                    auto* buffer = mDevice->buffer.GetObject(cmd->bufferId);
                    uint32_t bufferSerial = mDevice->buffer.GetSerial(cmd->bufferId);

                    //* The buffer might have been deleted or recreated so this isn't an error.
                    if (buffer == nullptr || bufferSerial != cmd->bufferSerial) {
                        return true;
                    }

                    //* The requests can have been deleted via an Unmap so this isn't an error.
                    auto requestIt = buffer->requests.find(cmd->requestSerial);
                    if (requestIt == buffer->requests.end()) {
                        return true;
                    }

                    //* It is an error for the server to call the read callback when we asked for a map write
                    if (requestIt->second.isWrite) {
                        return false;
                    }

                    auto request = requestIt->second;
                    //* Delete the request before calling the callback otherwise the callback could be fired a
                    //* second time. If, for example, buffer.Unmap() is called inside the callback.
                    buffer->requests.erase(requestIt);

                    //* On success, we copy the data locally because the IPC buffer isn't valid outside of this function
                    if (cmd->status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
                        //* The server didn't send the right amount of data, this is an error and could cause
                        //* the application to crash if we did call the callback.
                        if (request.size != cmd->dataLength) {
                            return false;
                        }

                        ASSERT(requestData != nullptr);

                        if (buffer->mappedData != nullptr) {
                            return false;
                        }

                        buffer->isWriteMapped = false;
                        buffer->mappedDataSize = request.size;
                        buffer->mappedData = malloc(request.size);
                        memcpy(buffer->mappedData, requestData, request.size);

                        request.readCallback(static_cast<dawnBufferMapAsyncStatus>(cmd->status), buffer->mappedData, request.userdata);
                    } else {
                        request.readCallback(static_cast<dawnBufferMapAsyncStatus>(cmd->status), nullptr, request.userdata);
                    }

                    return true;
                }

                bool HandleBufferMapWriteAsyncCallback(const char** commands, size_t* size) {
                    const auto* cmd = GetCommand<ReturnBufferMapWriteAsyncCallbackCmd>(commands, size);
                    if (cmd == nullptr) {
                        return false;
                    }

                    auto* buffer = mDevice->buffer.GetObject(cmd->bufferId);
                    uint32_t bufferSerial = mDevice->buffer.GetSerial(cmd->bufferId);

                    //* The buffer might have been deleted or recreated so this isn't an error.
                    if (buffer == nullptr || bufferSerial != cmd->bufferSerial) {
                        return true;
                    }

                    //* The requests can have been deleted via an Unmap so this isn't an error.
                    auto requestIt = buffer->requests.find(cmd->requestSerial);
                    if (requestIt == buffer->requests.end()) {
                        return true;
                    }

                    //* It is an error for the server to call the write callback when we asked for a map read
                    if (!requestIt->second.isWrite) {
                        return false;
                    }

                    auto request = requestIt->second;
                    //* Delete the request before calling the callback otherwise the callback could be fired a second time. If, for example, buffer.Unmap() is called inside the callback.
                    buffer->requests.erase(requestIt);

                    //* On success, we copy the data locally because the IPC buffer isn't valid outside of this function
                    if (cmd->status == DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS) {
                        if (buffer->mappedData != nullptr) {
                            return false;
                        }

                        buffer->isWriteMapped = true;
                        buffer->mappedDataSize = request.size;
                        buffer->mappedData = malloc(request.size);
                        memset(buffer->mappedData, 0, request.size);

                        request.writeCallback(static_cast<dawnBufferMapAsyncStatus>(cmd->status), buffer->mappedData, request.userdata);
                    } else {
                        request.writeCallback(static_cast<dawnBufferMapAsyncStatus>(cmd->status), nullptr, request.userdata);
                    }

                    return true;
                }
        };

    }

    CommandHandler* NewClientDevice(dawnProcTable* procs, dawnDevice* device, CommandSerializer* serializer) {
        auto clientDevice = new client::Device(serializer);

        *device = reinterpret_cast<dawnDeviceImpl*>(clientDevice);
        *procs = client::GetProcs();

        return new client::Client(clientDevice);
    }

}  // namespace dawn_wire
