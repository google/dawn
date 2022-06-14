// Copyright 2022 The Dawn Authors
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

#include "dawn/wire/client/ObjectBase.h"

#include "dawn/common/Assert.h"

namespace dawn::wire::client {

ObjectBase::ObjectBase(const ObjectBaseParams& params)
    : mClient(params.client), mHandle(params.handle), mRefcount(1) {}

ObjectBase::~ObjectBase() {
    RemoveFromList();
}

const ObjectHandle& ObjectBase::GetWireHandle() const {
    return mHandle;
}

ObjectId ObjectBase::GetWireId() const {
    return mHandle.id;
}

ObjectGeneration ObjectBase::GetWireGeneration() const {
    return mHandle.generation;
}

Client* ObjectBase::GetClient() const {
    return mClient;
}

void ObjectBase::Reference() {
    mRefcount++;
}

bool ObjectBase::Release() {
    ASSERT(mRefcount != 0);
    mRefcount--;
    return mRefcount == 0;
}

}  // namespace dawn::wire::client
