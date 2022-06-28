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

#include "dawn/wire/client/Client.h"

#include "dawn/common/Compiler.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

namespace {

class NoopCommandSerializer final : public CommandSerializer {
  public:
    static NoopCommandSerializer* GetInstance() {
        static NoopCommandSerializer gNoopCommandSerializer;
        return &gNoopCommandSerializer;
    }

    ~NoopCommandSerializer() override = default;

    size_t GetMaximumAllocationSize() const final { return 0; }
    void* GetCmdSpace(size_t size) final { return nullptr; }
    bool Flush() final { return false; }
};

}  // anonymous namespace

Client::Client(CommandSerializer* serializer, MemoryTransferService* memoryTransferService)
    : ClientBase(), mSerializer(serializer), mMemoryTransferService(memoryTransferService) {
    if (mMemoryTransferService == nullptr) {
        // If a MemoryTransferService is not provided, fall back to inline memory.
        mOwnedMemoryTransferService = CreateInlineMemoryTransferService();
        mMemoryTransferService = mOwnedMemoryTransferService.get();
    }
}

Client::~Client() {
    DestroyAllObjects();
}

void Client::DestroyAllObjects() {
    // Free all devices first since they may hold references to other objects
    // like the default queue. The Device destructor releases the default queue,
    // which would be invalid if the queue was already freed.
    while (!mObjects[ObjectType::Device].empty()) {
        ObjectBase* object = mObjects[ObjectType::Device].head()->value();

        DestroyObjectCmd cmd;
        cmd.objectType = ObjectType::Device;
        cmd.objectId = object->GetWireId();
        SerializeCommand(cmd);
        mObjectStores[ObjectType::Device].Free(object);
    }

    for (auto& objectList : mObjects) {
        ObjectType objectType = static_cast<ObjectType>(&objectList - mObjects.data());
        if (objectType == ObjectType::Device) {
            continue;
        }
        while (!objectList.empty()) {
            ObjectBase* object = objectList.head()->value();

            DestroyObjectCmd cmd;
            cmd.objectType = objectType;
            cmd.objectId = object->GetWireId();
            SerializeCommand(cmd);
            mObjectStores[objectType].Free(object);
        }
    }
}

ReservedTexture Client::ReserveTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor) {
    Texture* texture = Make<Texture>(descriptor);

    ReservedTexture result;
    result.texture = ToAPI(texture);
    result.id = texture->GetWireId();
    result.generation = texture->GetWireGeneration();
    result.deviceId = FromAPI(device)->GetWireId();
    result.deviceGeneration = FromAPI(device)->GetWireGeneration();
    return result;
}

ReservedSwapChain Client::ReserveSwapChain(WGPUDevice device) {
    SwapChain* swapChain = Make<SwapChain>();

    ReservedSwapChain result;
    result.swapchain = ToAPI(swapChain);
    result.id = swapChain->GetWireId();
    result.generation = swapChain->GetWireGeneration();
    result.deviceId = FromAPI(device)->GetWireId();
    result.deviceGeneration = FromAPI(device)->GetWireGeneration();
    return result;
}

ReservedDevice Client::ReserveDevice() {
    Device* device = Make<Device>();

    ReservedDevice result;
    result.device = ToAPI(device);
    result.id = device->GetWireId();
    result.generation = device->GetWireGeneration();
    return result;
}

ReservedInstance Client::ReserveInstance() {
    Instance* instance = Make<Instance>();

    ReservedInstance result;
    result.instance = ToAPI(instance);
    result.id = instance->GetWireId();
    result.generation = instance->GetWireGeneration();
    return result;
}

void Client::ReclaimTextureReservation(const ReservedTexture& reservation) {
    Free(FromAPI(reservation.texture));
}

void Client::ReclaimSwapChainReservation(const ReservedSwapChain& reservation) {
    Free(FromAPI(reservation.swapchain));
}

void Client::ReclaimDeviceReservation(const ReservedDevice& reservation) {
    Free(FromAPI(reservation.device));
}

void Client::ReclaimInstanceReservation(const ReservedInstance& reservation) {
    Free(FromAPI(reservation.instance));
}

void Client::Disconnect() {
    mDisconnected = true;
    mSerializer = ChunkedCommandSerializer(NoopCommandSerializer::GetInstance());

    auto& deviceList = mObjects[ObjectType::Device];
    {
        for (LinkNode<ObjectBase>* device = deviceList.head(); device != deviceList.end();
             device = device->next()) {
            static_cast<Device*>(device->value())
                ->HandleDeviceLost(WGPUDeviceLostReason_Undefined, "GPU connection lost");
        }
    }
    for (auto& objectList : mObjects) {
        for (LinkNode<ObjectBase>* object = objectList.head(); object != objectList.end();
             object = object->next()) {
            object->value()->CancelCallbacksForDisconnect();
        }
    }
}

bool Client::IsDisconnected() const {
    return mDisconnected;
}

void Client::Free(ObjectBase* obj, ObjectType type) {
    mObjectStores[type].Free(obj);
}

}  // namespace dawn::wire::client
