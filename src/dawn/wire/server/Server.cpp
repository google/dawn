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

WireResult Server::InjectTexture(WGPUTexture texture,
                                 uint32_t id,
                                 uint32_t generation,
                                 uint32_t deviceId,
                                 uint32_t deviceGeneration) {
    ASSERT(texture != nullptr);
    Known<WGPUDevice> device;
    WIRE_TRY(DeviceObjects().Get(deviceId, &device));
    if (device->generation != deviceGeneration) {
        return WireResult::FatalError;
    }

    Known<WGPUTexture> data;
    WIRE_TRY(TextureObjects().Allocate(&data, ObjectHandle{id, generation}));

    data->handle = texture;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.textureReference(texture);

    return WireResult::Success;
}

WireResult Server::InjectSwapChain(WGPUSwapChain swapchain,
                                   uint32_t id,
                                   uint32_t generation,
                                   uint32_t deviceId,
                                   uint32_t deviceGeneration) {
    ASSERT(swapchain != nullptr);
    Known<WGPUDevice> device;
    WIRE_TRY(DeviceObjects().Get(deviceId, &device));
    if (device->generation != deviceGeneration) {
        return WireResult::FatalError;
    }

    Known<WGPUSwapChain> data;
    WIRE_TRY(SwapChainObjects().Allocate(&data, ObjectHandle{id, generation}));

    data->handle = swapchain;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The texture is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.swapChainReference(swapchain);

    return WireResult::Success;
}

WireResult Server::InjectDevice(WGPUDevice device, uint32_t id, uint32_t generation) {
    ASSERT(device != nullptr);
    Known<WGPUDevice> data;
    WIRE_TRY(DeviceObjects().Allocate(&data, ObjectHandle{id, generation}));

    data->handle = device;
    data->generation = generation;
    data->state = AllocationState::Allocated;
    data->info->server = this;
    data->info->self = data.AsHandle();

    // The device is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.deviceReference(device);

    // Set callbacks to forward errors to the client.
    SetForwardingDeviceCallbacks(data);
    return WireResult::Success;
}

WireResult Server::InjectInstance(WGPUInstance instance, uint32_t id, uint32_t generation) {
    ASSERT(instance != nullptr);
    Known<WGPUInstance> data;
    WIRE_TRY(InstanceObjects().Allocate(&data, ObjectHandle{id, generation}));

    data->handle = instance;
    data->generation = generation;
    data->state = AllocationState::Allocated;

    // The instance is externally owned so it shouldn't be destroyed when we receive a destroy
    // message from the client. Add a reference to counterbalance the eventual release.
    mProcs.instanceReference(instance);

    return WireResult::Success;
}

WGPUDevice Server::GetDevice(uint32_t id, uint32_t generation) {
    Known<WGPUDevice> device;
    if (DeviceObjects().Get(id, &device) != WireResult::Success ||
        device->generation != generation) {
        return nullptr;
    }
    return device->handle;
}

bool Server::IsDeviceKnown(WGPUDevice device) const {
    return DeviceObjects().IsKnown(device);
}

void Server::SetForwardingDeviceCallbacks(Known<WGPUDevice> device) {
    // Note: these callbacks are manually inlined here since they do not acquire and
    // free their userdata. Also unlike other callbacks, these are cleared and unset when
    // the server is destroyed, so we don't need to check if the server is still alive
    // inside them.
    // Also, the device is special-cased in Server::DoDestroyObject to call
    // ClearDeviceCallbacks. This ensures that callbacks will not fire after |deviceObject|
    // is freed.
    mProcs.deviceSetUncapturedErrorCallback(
        device->handle,
        [](WGPUErrorType type, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnUncapturedError(info->self, type, message);
        },
        device->info.get());
    // Set callback to post warning and other infomation to client.
    // Almost the same with UncapturedError.
    mProcs.deviceSetLoggingCallback(
        device->handle,
        [](WGPULoggingType type, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnLogging(info->self, type, message);
        },
        device->info.get());
    mProcs.deviceSetDeviceLostCallback(
        device->handle,
        [](WGPUDeviceLostReason reason, const char* message, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnDeviceLost(info->self, reason, message);
        },
        device->info.get());
}

void Server::ClearDeviceCallbacks(WGPUDevice device) {
    // Un-set the error and lost callbacks since we cannot forward them
    // after the server has been destroyed.
    mProcs.deviceSetUncapturedErrorCallback(device, nullptr, nullptr);
    mProcs.deviceSetLoggingCallback(device, nullptr, nullptr);
    mProcs.deviceSetDeviceLostCallback(device, nullptr, nullptr);
}

}  // namespace dawn::wire::server
