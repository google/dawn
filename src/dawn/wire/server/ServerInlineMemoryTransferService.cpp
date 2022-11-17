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

#include "dawn/common/Assert.h"
#include "dawn/wire/WireServer.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

class InlineMemoryTransferService : public MemoryTransferService {
  public:
    class ReadHandleImpl : public ReadHandle {
      public:
        ReadHandleImpl() {}
        ~ReadHandleImpl() override = default;

        size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override { return size; }

        void SerializeDataUpdate(const void* data,
                                 size_t offset,
                                 size_t size,
                                 void* serializePointer) override {
            if (size > 0) {
                ASSERT(data != nullptr);
                ASSERT(serializePointer != nullptr);
                memcpy(serializePointer, data, size);
            }
        }
    };

    class WriteHandleImpl : public WriteHandle {
      public:
        WriteHandleImpl() {}
        ~WriteHandleImpl() override = default;

        bool DeserializeDataUpdate(const void* deserializePointer,
                                   size_t deserializeSize,
                                   size_t offset,
                                   size_t size) override {
            if (deserializeSize != size || mTargetData == nullptr ||
                deserializePointer == nullptr) {
                return false;
            }
            if (offset > mDataLength || size > mDataLength - offset) {
                return false;
            }
            memcpy(static_cast<uint8_t*>(mTargetData) + offset, deserializePointer, size);
            return true;
        }
    };

    InlineMemoryTransferService() {}
    ~InlineMemoryTransferService() override = default;

    bool DeserializeReadHandle(const void* deserializePointer,
                               size_t deserializeSize,
                               ReadHandle** readHandle) override {
        ASSERT(readHandle != nullptr);
        *readHandle = new ReadHandleImpl();
        return true;
    }

    bool DeserializeWriteHandle(const void* deserializePointer,
                                size_t deserializeSize,
                                WriteHandle** writeHandle) override {
        ASSERT(writeHandle != nullptr);
        *writeHandle = new WriteHandleImpl();
        return true;
    }
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

}  // namespace dawn::wire::server
