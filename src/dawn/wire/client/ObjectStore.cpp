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

#include "dawn/wire/client/ObjectStore.h"

#include <limits>
#include <utility>

namespace dawn::wire::client {

ObjectStore::ObjectStore() {
    // ID 0 is nullptr
    mObjects.emplace_back(nullptr);
    mCurrentId = 1;
}

ObjectHandle ObjectStore::ReserveHandle() {
    if (mFreeHandles.empty()) {
        return {mCurrentId++, 0};
    }
    ObjectHandle handle = mFreeHandles.back();
    mFreeHandles.pop_back();
    return handle;
}

void ObjectStore::Insert(std::unique_ptr<ObjectBase> obj) {
    ObjectId id = obj->GetWireId();

    if (id >= mObjects.size()) {
        ASSERT(id == mObjects.size());
        mObjects.emplace_back(std::move(obj));
    } else {
        // The generation should never overflow. We don't recycle ObjectIds that would
        // overflow their next generation.
        ASSERT(obj->GetWireGeneration() != 0);
        ASSERT(mObjects[id] == nullptr);
        mObjects[id] = std::move(obj);
    }
}

void ObjectStore::Free(ObjectBase* obj) {
    ASSERT(obj->IsInList());
    // The wire reuses ID for objects to keep them in a packed array starting from 0.
    // To avoid issues with asynchronous server->client communication referring to an ID that's
    // already reused, each handle also has a generation that's increment by one on each reuse.
    // Avoid overflows by only reusing the ID if the increment of the generation won't overflow.
    const ObjectHandle& currentHandle = obj->GetWireHandle();
    if (DAWN_LIKELY(currentHandle.generation != std::numeric_limits<ObjectGeneration>::max())) {
        mFreeHandles.push_back({currentHandle.id, currentHandle.generation + 1});
    }
    mObjects[currentHandle.id] = nullptr;
}

ObjectBase* ObjectStore::Get(ObjectId id) const {
    if (id >= mObjects.size()) {
        return nullptr;
    }
    return mObjects[id].get();
}

}  // namespace dawn::wire::client
