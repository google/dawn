// Copyright 2019 The Dawn Authors
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

#include "dawn_wire/server/Server.h"
#include "dawn_wire/WireServer.h"

namespace dawn_wire { namespace server {

    Server::Server(WGPUDevice device,
                   const DawnProcTable& procs,
                   CommandSerializer* serializer,
                   MemoryTransferService* memoryTransferService)
        : mSerializer(serializer),
          mProcs(procs),
          mMemoryTransferService(memoryTransferService),
          mIsAlive(std::make_shared<bool>(true)) {
        if (mMemoryTransferService == nullptr) {
            // If a MemoryTransferService is not provided, fallback to inline memory.
            mOwnedMemoryTransferService = CreateInlineMemoryTransferService();
            mMemoryTransferService = mOwnedMemoryTransferService.get();
        }

        // For the deprecated initialization path:
        // The client-server knowledge is bootstrapped with device 1, generation 0.
        if (device != nullptr) {
            bool success = InjectDevice(device, 1, 0);
            ASSERT(success);
        }
    }

    Server::~Server() {
        // Un-set the error and lost callbacks since we cannot forward them
        // after the server has been destroyed.
        for (WGPUDevice device : DeviceObjects().GetAllHandles()) {
            ClearDeviceCallbacks(device);
        }
        DestroyAllObjects(mProcs);
    }

    bool Server::InjectTexture(WGPUTexture texture,
                               uint32_t id,
                               uint32_t generation,
                               uint32_t deviceId,
                               uint32_t deviceGeneration) {
        ASSERT(texture != nullptr);
        ObjectData<WGPUDevice>* device = DeviceObjects().Get(deviceId);
        if (device == nullptr || device->generation != deviceGeneration) {
            return false;
        }

        ObjectData<WGPUTexture>* data = TextureObjects().Allocate(id);
        if (data == nullptr) {
            return false;
        }

        data->handle = texture;
        data->generation = generation;
        data->allocated = true;
        data->deviceInfo = device->info.get();

        if (!TrackDeviceChild(data->deviceInfo, ObjectType::Texture, id)) {
            return false;
        }

        // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
        // message from the client. Add a reference to counterbalance the eventual release.
        mProcs.textureReference(texture);

        return true;
    }

    bool Server::InjectDevice(WGPUDevice device, uint32_t id, uint32_t generation) {
        ASSERT(device != nullptr);
        ObjectData<WGPUDevice>* data = DeviceObjects().Allocate(id);
        if (data == nullptr) {
            return false;
        }

        data->handle = device;
        data->generation = generation;
        data->allocated = true;
        data->info->server = this;
        data->info->self = ObjectHandle{id, generation};

        // The device is externally owned so it shouldn't be destroyed when we receive a destroy
        // message from the client. Add a reference to counterbalance the eventual release.
        mProcs.deviceReference(device);

        // Set callbacks to forward errors to the client.
        // Note: these callbacks are manually inlined here since they do not acquire and
        // free their userdata. Also unlike other callbacks, these are cleared and unset when
        // the server is destroyed, so we don't need to check if the server is still alive
        // inside them.
        mProcs.deviceSetUncapturedErrorCallback(
            device,
            [](WGPUErrorType type, const char* message, void* userdata) {
                DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
                info->server->OnUncapturedError(info->self, type, message);
            },
            data->info.get());
        mProcs.deviceSetDeviceLostCallback(
            device,
            [](const char* message, void* userdata) {
                DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
                info->server->OnDeviceLost(info->self, message);
            },
            data->info.get());

        return true;
    }

    WGPUDevice Server::GetDevice(uint32_t id, uint32_t generation) {
        ObjectData<WGPUDevice>* data = DeviceObjects().Get(id);
        if (data == nullptr || data->generation != generation) {
            return nullptr;
        }
        return data->handle;
    }

    void Server::ClearDeviceCallbacks(WGPUDevice device) {
        // Un-set the error and lost callbacks since we cannot forward them
        // after the server has been destroyed.
        mProcs.deviceSetUncapturedErrorCallback(device, nullptr, nullptr);
        mProcs.deviceSetDeviceLostCallback(device, nullptr, nullptr);
    }

    bool TrackDeviceChild(DeviceInfo* info, ObjectType type, ObjectId id) {
        auto it = info->childObjectTypesAndIds.insert(PackObjectTypeAndId(type, id));
        if (!it.second) {
            // An object of this type and id already exists.
            return false;
        }
        return true;
    }

    bool UntrackDeviceChild(DeviceInfo* info, ObjectType type, ObjectId id) {
        auto& children = info->childObjectTypesAndIds;
        auto it = children.find(PackObjectTypeAndId(type, id));
        if (it == children.end()) {
            // An object of this type and id was already deleted.
            return false;
        }
        children.erase(it);
        return true;
    }

}}  // namespace dawn_wire::server
