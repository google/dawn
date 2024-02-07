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

#ifndef SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
#define SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_

#include "dawn/webgpu.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/common/LinkedList.h"
#include "dawn/wire/ObjectHandle.h"
#include "dawn/wire/ObjectType_autogen.h"
#include "dawn/wire/client/EventManager.h"

namespace dawn::wire::client {

class Client;

struct ObjectBaseParams {
    raw_ptr<Client> client;
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
    virtual ObjectType GetObjectType() const = 0;

    const ObjectHandle& GetWireHandle() const;
    ObjectId GetWireId() const;
    ObjectGeneration GetWireGeneration() const;
    Client* GetClient() const;

    void Reference();
    void Release();

  protected:
    uint32_t GetRefcount() const { return mRefcount; }

  private:
    const raw_ptr<Client> mClient;
    const ObjectHandle mHandle;
    uint32_t mRefcount;
};

// Compositable functionality for objects on the client side that need to have access to the event
// manager.
class ObjectWithEventsBase : public ObjectBase {
  public:
    // Note that the ObjectHandle associated with an EventManager is the same handle associated to
    // the Instance that "owns" the EventManager.
    ObjectWithEventsBase(const ObjectBaseParams& params, const ObjectHandle& eventManager);

    const ObjectHandle& GetEventManagerHandle() const;
    EventManager& GetEventManager() const;

  private:
    // The EventManager is owned by the client and long-lived. When the client is destroyed all
    // objects are also freed.
    ObjectHandle mEventManagerHandle;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
