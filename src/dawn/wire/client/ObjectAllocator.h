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

#ifndef SRC_DAWN_WIRE_CLIENT_OBJECTALLOCATOR_H_
#define SRC_DAWN_WIRE_CLIENT_OBJECTALLOCATOR_H_

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Compiler.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

template <typename T>
class ObjectAllocator {
  public:
    ObjectAllocator() {
        // ID 0 is nullptr
        mObjects.emplace_back(nullptr);
    }

    template <typename Client>
    T* New(Client* client) {
        ObjectHandle handle = GetFreeHandle();
        ObjectBaseParams params = {client, handle};
        auto object = std::make_unique<T>(params);
        client->TrackObject(object.get());

        if (handle.id >= mObjects.size()) {
            ASSERT(handle.id == mObjects.size());
            mObjects.emplace_back(std::move(object));
        } else {
            // The generation should never overflow. We don't recycle ObjectIds that would
            // overflow their next generation.
            ASSERT(handle.generation != 0);
            ASSERT(mObjects[handle.id] == nullptr);
            mObjects[handle.id] = std::move(object);
        }

        return mObjects[handle.id].get();
    }
    void Free(T* obj) {
        ASSERT(obj->IsInList());
        // The wire reuses ID for objects to keep them in a packed array starting from 0.
        // To avoid issues with asynchronous server->client communication referring to an ID that's
        // already reused, each handle also has a generation that's increment by one on each reuse.
        // Avoid overflows by only reusing the ID if the increment of the generation won't overflow.
        ObjectHandle currentHandle = obj->GetWireHandle();
        if (DAWN_LIKELY(currentHandle.generation != std::numeric_limits<ObjectGeneration>::max())) {
            mFreeHandles.push_back({currentHandle.id, currentHandle.generation + 1});
        }
        mObjects[currentHandle.id] = nullptr;
    }

    T* GetObject(uint32_t id) {
        if (id >= mObjects.size()) {
            return nullptr;
        }
        return mObjects[id].get();
    }

  private:
    ObjectHandle GetFreeHandle() {
        if (mFreeHandles.empty()) {
            return {mCurrentId++, 0};
        }
        ObjectHandle handle = mFreeHandles.back();
        mFreeHandles.pop_back();
        return handle;
    }

    // 0 is an ID reserved to represent nullptr
    uint32_t mCurrentId = 1;
    std::vector<ObjectHandle> mFreeHandles;
    std::vector<std::unique_ptr<T>> mObjects;
};
}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_OBJECTALLOCATOR_H_
