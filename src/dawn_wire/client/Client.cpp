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

#include "dawn_wire/client/Client.h"

#include "common/Compiler.h"
#include "dawn_wire/client/Device.h"

namespace dawn_wire { namespace client {

    namespace {

        class NoopCommandSerializer final : public CommandSerializer {
          public:
            static NoopCommandSerializer* GetInstance() {
                static NoopCommandSerializer gNoopCommandSerializer;
                return &gNoopCommandSerializer;
            }

            ~NoopCommandSerializer() = default;

            size_t GetMaximumAllocationSize() const final {
                return 0;
            }
            void* GetCmdSpace(size_t size) final {
                return nullptr;
            }
            bool Flush() final {
                return false;
            }
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
        for (auto& objectList : mObjects) {
            ObjectType objectType = static_cast<ObjectType>(&objectList - mObjects.data());
            if (objectType == ObjectType::Device) {
                continue;
            }
            while (!objectList.empty()) {
                ObjectBase* object = objectList.head()->value();

                DestroyObjectCmd cmd;
                cmd.objectType = objectType;
                cmd.objectId = object->id;
                SerializeCommand(cmd);
                FreeObject(objectType, object);
            }
        }

        while (!mObjects[ObjectType::Device].empty()) {
            ObjectBase* object = mObjects[ObjectType::Device].head()->value();

            DestroyObjectCmd cmd;
            cmd.objectType = ObjectType::Device;
            cmd.objectId = object->id;
            SerializeCommand(cmd);
            FreeObject(ObjectType::Device, object);
        }
    }

    WGPUDevice Client::GetDevice() {
        // This function is deprecated. The concept of a "default" device on the wire
        // will be removed in favor of ReserveDevice/InjectDevice.
        if (mDevice == nullptr) {
            ReservedDevice reservation = ReserveDevice();
            mDevice = FromAPI(reservation.device);
            ASSERT(reservation.id == 1);
            ASSERT(reservation.generation == 0);
        }
        return reinterpret_cast<WGPUDeviceImpl*>(mDevice);
    }

    ReservedTexture Client::ReserveTexture(WGPUDevice device) {
        auto* allocation = TextureAllocator().New(this);

        ReservedTexture result;
        result.texture = ToAPI(allocation->object.get());
        result.id = allocation->object->id;
        result.generation = allocation->generation;
        result.deviceId = FromAPI(device)->id;
        result.deviceGeneration = DeviceAllocator().GetGeneration(FromAPI(device)->id);
        return result;
    }

    ReservedDevice Client::ReserveDevice() {
        auto* allocation = DeviceAllocator().New(this);

        ReservedDevice result;
        result.device = ToAPI(allocation->object.get());
        result.id = allocation->object->id;
        result.generation = allocation->generation;
        return result;
    }

    void Client::ReclaimTextureReservation(const ReservedTexture& reservation) {
        TextureAllocator().Free(FromAPI(reservation.texture));
    }

    void Client::ReclaimDeviceReservation(const ReservedDevice& reservation) {
        DeviceAllocator().Free(FromAPI(reservation.device));
    }

    void Client::Disconnect() {
        mDisconnected = true;
        mSerializer = ChunkedCommandSerializer(NoopCommandSerializer::GetInstance());

        auto& deviceList = mObjects[ObjectType::Device];
        {
            for (LinkNode<ObjectBase>* device = deviceList.head(); device != deviceList.end();
                 device = device->next()) {
                static_cast<Device*>(device->value())->HandleDeviceLost("GPU connection lost");
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

}}  // namespace dawn_wire::client
