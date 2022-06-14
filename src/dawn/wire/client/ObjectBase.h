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

#ifndef SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
#define SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_

#include "dawn/webgpu.h"

#include "dawn/common/LinkedList.h"
#include "dawn/wire/ObjectHandle.h"

namespace dawn::wire::client {

class Client;

struct ObjectBaseParams {
    Client* client;
    ObjectHandle handle;
};

// All objects on the client side have:
//  - A pointer to the Client to get where to serialize commands
//  - The external reference count, starting at 1.
//  - An ID that is used to refer to this object when talking with the server side
//  - A next/prev pointer. They are part of a linked list of objects of the same type.
class ObjectBase : public LinkNode<ObjectBase> {
  public:
    explicit ObjectBase(const ObjectBaseParams& params);
    virtual ~ObjectBase();

    virtual void CancelCallbacksForDisconnect() {}

    const ObjectHandle& GetWireHandle() const;
    ObjectId GetWireId() const;
    ObjectGeneration GetWireGeneration() const;
    Client* GetClient() const;

    void Reference();
    // Returns true if it was the last reference, indicating that the caller must destroy the
    // object.
    [[nodiscard]] bool Release();

  private:
    Client* const mClient;
    const ObjectHandle mHandle;
    uint32_t mRefcount;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
