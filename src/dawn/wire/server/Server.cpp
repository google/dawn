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

#include "dawn/wire/server/Server.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire::server {

CallbackUserdata::CallbackUserdata(Server* server, const std::shared_ptr<bool>& serverIsAlive)
    : server(server), serverIsAlive(serverIsAlive) {}

Server::Server(const DawnProcTable& procs,
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

    ObjectData<WGPUTexture>* data = TextureObjects().Allocate(ObjectHandle{id, generation});
    if (data == nullptr) {
        return false;
    }

    data->handle = texture;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.textureReference(texture);

    return true;
}

bool Server::InjectSwapChain(WGPUSwapChain swapchain,
                             uint32_t id,
                             uint32_t generation,
                             uint32_t deviceId,
                             uint32_t deviceGeneration) {
    ASSERT(swapchain != nullptr);
    ObjectData<WGPUDevice>* device = DeviceObjects().Get(deviceId);
    if (device == nullptr || device->generation != deviceGeneration) {
        return false;
    }

    ObjectData<WGPUSwapChain>* data = SwapChainObjects().Allocate(ObjectHandle{id, generation});
    if (data == nullptr) {
        return false;
    }

    data->handle = swapchain;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.swapChainReference(swapchain);

    return true;
}

bool Server::InjectDevice(WGPUDevice device, uint32_t id, uint32_t generation) {
    ASSERT(device != nullptr);
    ObjectData<WGPUDevice>* data = DeviceObjects().Allocate(ObjectHandle{id, generation});
    if (data == nullptr) {
        return false;
    }

    data->handle = device;
    data->generation = generation;
    data->state = AllocationState::Allocated;
    data->info->server = this;
    data->info->self = ObjectHandle{id, generation};

    // The device is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.deviceReference(device);

    // Set callbacks to forward errors to the client.
    SetForwardingDeviceCallbacks(data);
    return true;
}

bool Server::InjectInstance(WGPUInstance instance, uint32_t id, uint32_t generation) {
    ASSERT(instance != nullptr);
    ObjectData<WGPUInstance>* data = InstanceObjects().Allocate(ObjectHandle{id, generation});
    if (data == nullptr) {
        return false;
    }

    data->handle = instance;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The instance is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.instanceReference(instance);

    return true;
}

WGPUDevice Server::GetDevice(uint32_t id, uint32_t generation) {
    ObjectData<WGPUDevice>* data = DeviceObjects().Get(id);
    if (data == nullptr || data->generation != generation) {
        return nullptr;
    }
    return data->handle;
}

bool Server::IsDeviceKnown(WGPUDevice device) const {
    return DeviceObjects().IsKnown(device);
}

void Server::SetForwardingDeviceCallbacks(ObjectData<WGPUDevice>* deviceObject) {
    // Note: these callbacks are manually inlined here since they do not acquire and
    // free their userdata. Also unlike other callbacks, these are cleared and unset when
    // the server is destroyed, so we don't need to check if the server is still alive
    // inside them.
    // Also, the device is special-cased in Server::DoDestroyObject to call
    // ClearDeviceCallbacks. This ensures that callbacks will not fire after |deviceObject|
    // is freed.
    mProcs.deviceSetUncapturedErrorCallback(
        deviceObject->handle,
        [](WGPUErrorType type, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnUncapturedError(info->self, type, message);
        },
        deviceObject->info.get());
    // Set callback to post warning and other infomation to client.
    // Almost the same with UncapturedError.
    mProcs.deviceSetLoggingCallback(
        deviceObject->handle,
        [](WGPULoggingType type, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnLogging(info->self, type, message);
        },
        deviceObject->info.get());
    mProcs.deviceSetDeviceLostCallback(
        deviceObject->handle,
        [](WGPUDeviceLostReason reason, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnDeviceLost(info->self, reason, message);
        },
        deviceObject->info.get());
}

void Server::ClearDeviceCallbacks(WGPUDevice device) {
    // Un-set the error and lost callbacks since we cannot forward them
    // after the server has been destroyed.
    mProcs.deviceSetUncapturedErrorCallback(device, nullptr, nullptr);
    mProcs.deviceSetLoggingCallback(device, nullptr, nullptr);
    mProcs.deviceSetDeviceLostCallback(device, nullptr, nullptr);
}

}  // namespace dawn::wire::server
