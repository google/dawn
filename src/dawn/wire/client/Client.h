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

#ifndef SRC_DAWN_WIRE_CLIENT_CLIENT_H_
#define SRC_DAWN_WIRE_CLIENT_CLIENT_H_

#include <memory>

#include "dawn/common/LinkedList.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/webgpu.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireDeserializeAllocator.h"
#include "dawn/wire/client/ClientBase_autogen.h"

namespace dawn::wire::client {

class Device;
class MemoryTransferService;

class Client : public ClientBase {
  public:
    Client(CommandSerializer* serializer, MemoryTransferService* memoryTransferService);
    ~Client() override;

    // ChunkedCommandHandler implementation
    const volatile char* HandleCommandsImpl(const volatile char* commands, size_t size) override;

    MemoryTransferService* GetMemoryTransferService() const { return mMemoryTransferService; }

    ReservedTexture ReserveTexture(WGPUDevice device);
    ReservedSwapChain ReserveSwapChain(WGPUDevice device);
    ReservedDevice ReserveDevice();
    ReservedInstance ReserveInstance();

    void ReclaimTextureReservation(const ReservedTexture& reservation);
    void ReclaimSwapChainReservation(const ReservedSwapChain& reservation);
    void ReclaimDeviceReservation(const ReservedDevice& reservation);
    void ReclaimInstanceReservation(const ReservedInstance& reservation);

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

#include "dawn/wire/client/ClientPrototypes_autogen.inc"

    ChunkedCommandSerializer mSerializer;
    WireDeserializeAllocator mAllocator;
    MemoryTransferService* mMemoryTransferService = nullptr;
    std::unique_ptr<MemoryTransferService> mOwnedMemoryTransferService = nullptr;

    PerObjectType<LinkedList<ObjectBase>> mObjects;
    bool mDisconnected = false;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService();

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_CLIENT_H_
