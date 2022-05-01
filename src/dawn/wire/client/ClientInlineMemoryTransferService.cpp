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

#include <cstring>
#include <memory>
#include <utility>

#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

class InlineMemoryTransferService : public MemoryTransferService {
    class ReadHandleImpl : public ReadHandle {
      public:
        explicit ReadHandleImpl(std::unique_ptr<uint8_t[]> stagingData, size_t size)
            : mStagingData(std::move(stagingData)), mSize(size) {}

        ~ReadHandleImpl() override = default;

        size_t SerializeCreateSize() override { return 0; }

        void SerializeCreate(void*) override {}

        const void* GetData() override { return mStagingData.get(); }

        bool DeserializeDataUpdate(const void* deserializePointer,
                                   size_t deserializeSize,
                                   size_t offset,
                                   size_t size) override {
            if (deserializeSize != size || deserializePointer == nullptr) {
                return false;
            }

            if (offset > mSize || size > mSize - offset) {
                return false;
            }

            void* start = static_cast<uint8_t*>(mStagingData.get()) + offset;
            memcpy(start, deserializePointer, size);
            return true;
        }

      private:
        std::unique_ptr<uint8_t[]> mStagingData;
        size_t mSize;
    };

    class WriteHandleImpl : public WriteHandle {
      public:
        explicit WriteHandleImpl(std::unique_ptr<uint8_t[]> stagingData, size_t size)
            : mStagingData(std::move(stagingData)), mSize(size) {}

        ~WriteHandleImpl() override = default;

        size_t SerializeCreateSize() override { return 0; }

        void SerializeCreate(void*) override {}

        void* GetData() override { return mStagingData.get(); }

        size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override {
            ASSERT(offset <= mSize);
            ASSERT(size <= mSize - offset);
            return size;
        }

        void SerializeDataUpdate(void* serializePointer, size_t offset, size_t size) override {
            ASSERT(mStagingData != nullptr);
            ASSERT(serializePointer != nullptr);
            ASSERT(offset <= mSize);
            ASSERT(size <= mSize - offset);
            memcpy(serializePointer, static_cast<uint8_t*>(mStagingData.get()) + offset, size);
        }

      private:
        std::unique_ptr<uint8_t[]> mStagingData;
        size_t mSize;
    };

  public:
    InlineMemoryTransferService() {}
    ~InlineMemoryTransferService() override = default;

    ReadHandle* CreateReadHandle(size_t size) override {
        auto stagingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
        if (stagingData) {
            return new ReadHandleImpl(std::move(stagingData), size);
        }
        return nullptr;
    }

    WriteHandle* CreateWriteHandle(size_t size) override {
        auto stagingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
        if (stagingData) {
            memset(stagingData.get(), 0, size);
            return new WriteHandleImpl(std::move(stagingData), size);
        }
        return nullptr;
    }
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

}  // namespace dawn::wire::client
