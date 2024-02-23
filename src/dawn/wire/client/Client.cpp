// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    result.handle = texture->GetWireHandle();
    result.deviceHandle = FromAPI(device)->GetWireHandle();
    return result;
}

ReservedSwapChain Client::ReserveSwapChain(WGPUDevice device,
                                           const WGPUSwapChainDescriptor* descriptor) {
    SwapChain* swapChain = Make<SwapChain>(nullptr, descriptor);

    ReservedSwapChain result;
    result.swapchain = ToAPI(swapChain);
    result.handle = swapChain->GetWireHandle();
    result.deviceHandle = FromAPI(device)->GetWireHandle();
    return result;
}

ReservedInstance Client::ReserveInstance(const WGPUInstanceDescriptor* descriptor) {
    Instance* instance = Make<Instance>();

    if (instance->Initialize(descriptor) != WireResult::Success) {
        Free(instance);
        return {nullptr, {0, 0}};
    }

    // Reserve an EventManager for the given instance and make the association in the map.
    mEventManagers.emplace(ObjectHandle(instance->GetWireId(), instance->GetWireGeneration()),
                           std::make_unique<EventManager>());

    ReservedInstance result;
    result.instance = ToAPI(instance);
    result.handle = instance->GetWireHandle();
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

EventManager& Client::GetEventManager(const ObjectHandle& instance) {
    auto it = mEventManagers.find(instance);
    DAWN_ASSERT(it != mEventManagers.end());
    return *it->second;
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

    // Transition all event managers to ClientDropped state.
    for (auto& [_, eventManager] : mEventManagers) {
        eventManager->TransitionTo(EventManager::State::ClientDropped);
    }
}

bool Client::IsDisconnected() const {
    return mDisconnected;
}

void Client::Free(ObjectBase* obj, ObjectType type) {
    mObjectStores[type].Free(obj);
}

}  // namespace dawn::wire::client
