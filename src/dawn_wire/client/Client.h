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

#ifndef DAWNWIRE_CLIENT_CLIENT_H_
#define DAWNWIRE_CLIENT_CLIENT_H_

#include <dawn/webgpu.h>
#include <dawn_wire/Wire.h>

#include "common/LinkedList.h"
#include "dawn_wire/ChunkedCommandSerializer.h"
#include "dawn_wire/WireClient.h"
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/WireDeserializeAllocator.h"
#include "dawn_wire/client/ClientBase_autogen.h"

namespace dawn_wire { namespace client {

    class Device;
    class MemoryTransferService;

    class Client : public ClientBase {
      public:
        Client(CommandSerializer* serializer, MemoryTransferService* memoryTransferService);
        ~Client() override;

        // ChunkedCommandHandler implementation
        const volatile char* HandleCommandsImpl(const volatile char* commands,
                                                size_t size) override;

        WGPUDevice GetDevice();

        MemoryTransferService* GetMemoryTransferService() const {
            return mMemoryTransferService;
        }

        ReservedTexture ReserveTexture(WGPUDevice device);
        ReservedDevice ReserveDevice();

        template <typename Cmd>
        void SerializeCommand(const Cmd& cmd) {
            mSerializer.SerializeCommand(cmd, *this);
        }

        template <typename Cmd, typename ExtraSizeSerializeFn>
        void SerializeCommand(const Cmd& cmd,
                              size_t extraSize,
                              ExtraSizeSerializeFn&& SerializeExtraSize) {
            mSerializer.SerializeCommand(cmd, *this, extraSize, SerializeExtraSize);
        }

        void Disconnect();
        bool IsDisconnected() const;

        template <typename T>
        void TrackObject(T* object) {
            mObjects[ObjectTypeToTypeEnum<T>::value].Append(object);
        }

      private:
        void DestroyAllObjects();

#include "dawn_wire/client/ClientPrototypes_autogen.inc"

        Device* mDevice = nullptr;
        ChunkedCommandSerializer mSerializer;
        WireDeserializeAllocator mAllocator;
        MemoryTransferService* mMemoryTransferService = nullptr;
        std::unique_ptr<MemoryTransferService> mOwnedMemoryTransferService = nullptr;

        PerObjectType<LinkedList<ObjectBase>> mObjects;
        bool mDisconnected = false;
    };

    std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService();

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_CLIENT_H_
