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

#ifndef SRC_DAWN_WIRE_SERVER_OBJECTSTORAGE_H_
#define SRC_DAWN_WIRE_SERVER_OBJECTSTORAGE_H_

#include <algorithm>
#include <map>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire::server {

// Whether this object has been allocated, or reserved for async object creation.
// Used by the KnownObjects queries
enum class AllocationState : uint32_t {
    Free,
    Reserved,
    Allocated,
};

template <typename T>
struct ObjectDataBase {
    // The backend-provided handle and generation to this object.
    T handle;
    uint32_t generation = 0;

    AllocationState state;
};

// Stores what the backend knows about the type.
template <typename T>
struct ObjectData : public ObjectDataBase<T> {};

enum class BufferMapWriteState { Unmapped, Mapped, MapError };

template <>
struct ObjectData<WGPUBuffer> : public ObjectDataBase<WGPUBuffer> {
    // TODO(enga): Use a tagged pointer to save space.
    std::unique_ptr<MemoryTransferService::ReadHandle> readHandle;
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle;
    BufferMapWriteState mapWriteState = BufferMapWriteState::Unmapped;
    WGPUBufferUsageFlags usage = WGPUBufferUsage_None;
    // Indicate if writeHandle needs to be destroyed on unmap
    bool mappedAtCreation = false;
};

struct DeviceInfo {
    Server* server;
    ObjectHandle self;
};

template <>
struct ObjectData<WGPUDevice> : public ObjectDataBase<WGPUDevice> {
    // Store |info| as a separate allocation so that its address does not move.
    // The pointer to |info| is used as the userdata to device callback.
    std::unique_ptr<DeviceInfo> info = std::make_unique<DeviceInfo>();
};

// Keeps track of the mapping between client IDs and backend objects.
template <typename T>
class KnownObjectsBase {
  public:
    using Data = ObjectData<T>;

    KnownObjectsBase() {
        // Reserve ID 0 so that it can be used to represent nullptr for optional object values
        // in the wire format. However don't tag it as allocated so that it is an error to ask
        // KnownObjects for ID 0.
        Data reservation;
        reservation.handle = nullptr;
        reservation.state = AllocationState::Free;
        mKnown.push_back(std::move(reservation));
    }

    // Get a backend objects for a given client ID.
    // Returns nullptr if the ID hasn't previously been allocated.
    const Data* Get(uint32_t id) const {
        if (id >= mKnown.size()) {
            return nullptr;
        }

        const Data* data = &mKnown[id];
        if (data->state != AllocationState::Allocated) {
            return nullptr;
        }
        return data;
    }
    Data* Get(uint32_t id) {
        if (id >= mKnown.size()) {
            return nullptr;
        }

        Data* data = &mKnown[id];
        if (data->state != AllocationState::Allocated) {
            return nullptr;
        }
        return data;
    }

    Data* FillReservation(uint32_t id, T handle) {
        ASSERT(id < mKnown.size());
        Data* data = &mKnown[id];
        ASSERT(data->state == AllocationState::Reserved);
        data->handle = handle;
        data->state = AllocationState::Allocated;
        return data;
    }

    // Allocates the data for a given ID and returns it.
    // Returns nullptr if the ID is already allocated, or too far ahead, or if ID is 0 (ID 0 is
    // reserved for nullptr). Invalidates all the Data*
    Data* Allocate(ObjectHandle handle, AllocationState state = AllocationState::Allocated) {
        if (handle.id == 0 || handle.id > mKnown.size()) {
            return nullptr;
        }

        Data data;
        data.state = state;
        data.handle = nullptr;

        if (handle.id >= mKnown.size()) {
            mKnown.push_back(std::move(data));
            return &mKnown.back();
        }

        if (mKnown[handle.id].state != AllocationState::Free) {
            return nullptr;
        }

        // The generation should be strictly increasing.
        if (handle.generation <= mKnown[handle.id].generation) {
            return nullptr;
        }
        // update the generation in the slot
        data.generation = handle.generation;

        mKnown[handle.id] = std::move(data);
        return &mKnown[handle.id];
    }

    // Marks an ID as deallocated
    void Free(uint32_t id) {
        ASSERT(id < mKnown.size());
        mKnown[id].state = AllocationState::Free;
    }

    std::vector<T> AcquireAllHandles() {
        std::vector<T> objects;
        for (Data& data : mKnown) {
            if (data.state == AllocationState::Allocated && data.handle != nullptr) {
                objects.push_back(data.handle);
                data.state = AllocationState::Free;
                data.handle = nullptr;
            }
        }

        return objects;
    }

    std::vector<T> GetAllHandles() const {
        std::vector<T> objects;
        for (const Data& data : mKnown) {
            if (data.state == AllocationState::Allocated && data.handle != nullptr) {
                objects.push_back(data.handle);
            }
        }

        return objects;
    }

  protected:
    std::vector<Data> mKnown;
};

template <typename T>
class KnownObjects : public KnownObjectsBase<T> {
  public:
    KnownObjects() = default;
};

template <>
class KnownObjects<WGPUDevice> : public KnownObjectsBase<WGPUDevice> {
  public:
    KnownObjects() = default;

    Data* Allocate(ObjectHandle handle, AllocationState state = AllocationState::Allocated) {
        Data* data = KnownObjectsBase<WGPUDevice>::Allocate(handle, state);
        AddToKnownSet(data);
        return data;
    }

    Data* FillReservation(uint32_t id, WGPUDevice handle) {
        Data* data = KnownObjectsBase<WGPUDevice>::FillReservation(id, handle);
        AddToKnownSet(data);
        return data;
    }

    void Free(uint32_t id) {
        mKnownSet.erase(mKnown[id].handle);
        KnownObjectsBase<WGPUDevice>::Free(id);
    }

    bool IsKnown(WGPUDevice device) const { return mKnownSet.count(device) != 0; }

  private:
    void AddToKnownSet(Data* data) {
        if (data != nullptr && data->state == AllocationState::Allocated &&
            data->handle != nullptr) {
            mKnownSet.insert(data->handle);
        }
    }
    std::unordered_set<WGPUDevice> mKnownSet;
};

// ObjectIds are lost in deserialization. Store the ids of deserialized
// objects here so they can be used in command handlers. This is useful
// for creating ReturnWireCmds which contain client ids
template <typename T>
class ObjectIdLookupTable {
  public:
    void Store(T key, ObjectId id) { mTable[key] = id; }

    // Return the cached ObjectId, or 0 (null handle)
    ObjectId Get(T key) const {
        const auto it = mTable.find(key);
        if (it != mTable.end()) {
            return it->second;
        }
        return 0;
    }

    void Remove(T key) {
        auto it = mTable.find(key);
        if (it != mTable.end()) {
            mTable.erase(it);
        }
    }

  private:
    std::map<T, ObjectId> mTable;
};

}  // namespace dawn::wire::server

#endif  // SRC_DAWN_WIRE_SERVER_OBJECTSTORAGE_H_
