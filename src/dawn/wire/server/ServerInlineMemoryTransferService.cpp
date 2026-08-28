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

#include "src/dawn/wire/server/ServerInlineMemoryTransferService.h"

#include <cstring>
#include <memory>
#include <utility>

#include "dawn/wire/WireServer.h"
#include "src/dawn/wire/InlineSharedMemoryManager.h"
#include "src/dawn/wire/server/Server.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

namespace dawn::wire::server {

class InlineMemoryTransferService : public MemoryTransferService {
  public:
    class MemoryHandleImpl : public MemoryHandle {
      public:
        MemoryHandleImpl() {}
        ~MemoryHandleImpl() override = default;

        size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const override {
            return size;
        }

        void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                 size_t offset,
                                 size_t size,
                                 std::span<const std::byte> data) const override {
            DAWN_ASSERT(serializeData.size() == GetSerializeDataUpdateSize(offset, size));
            DAWN_ASSERT(data.size() == size);
            DAWN_ASSERT(serializeData.size() >= data.size());
            std::ranges::copy(data, serializeData.begin());
        }

        bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                   size_t offset,
                                   size_t size,
                                   std::span<std::byte> target) override {
            DAWN_ASSERT(target.size() == size);
            if (size > deserializeData.size()) {
                return false;
            }
            std::ranges::copy(deserializeData.subspan(0, size), target.begin());
            return true;
        }
    };

    // TODO(386255678): support importing shared memory as shared buffer memory.
    class MemoryHandleWithSharedMemoryImpl : public MemoryHandle {
      public:
        explicit MemoryHandleWithSharedMemoryImpl(Ref<SharedMemory> sharedMemory)
            : mSharedMemory(std::move(sharedMemory)) {}

        size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const override {
            // The data is transferred out-of-band through the shared memory, so nothing needs to
            // be serialized onto the wire.
            return 0;
        }

        void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                 size_t offset,
                                 size_t size,
                                 std::span<const std::byte> data) const override {
            DAWN_ASSERT(serializeData.size() == GetSerializeDataUpdateSize(offset, size));

            std::span<std::byte> mapped = mSharedMemory->GetMappedSpan();
            DAWN_ASSERT(data.size() == size);
            DAWN_ASSERT(offset <= mapped.size());
            DAWN_ASSERT(size <= mapped.size() - offset);
            std::ranges::copy(data, mapped.subspan(offset, size).begin());
        }

        bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                   size_t offset,
                                   size_t size,
                                   std::span<std::byte> target) override {
            std::span<std::byte> mapped = mSharedMemory->GetMappedSpan();
            if (size > target.size() || offset > mapped.size() || size > mapped.size() - offset) {
                return false;
            }
            std::ranges::copy(mapped.subspan(offset, size), target.begin());
            return true;
        }

      private:
        Ref<SharedMemory> mSharedMemory;
    };

    InlineMemoryTransferService() = default;
    explicit InlineMemoryTransferService(
        std::shared_ptr<InlineSharedMemoryManager> sharedMemoryManager)
        : mSharedMemoryManager(std::move(sharedMemoryManager)) {}
    ~InlineMemoryTransferService() override = default;

    std::unique_ptr<MemoryHandle> DeserializeMemoryHandle(
        std::span<const std::byte> creationData) override {
        if (creationData.size() != sizeof(SharedMemoryHandle)) {
            return nullptr;
        }

        SharedMemoryHandle handle{};
        ByteSpanFromRef(handle).CopyFrom(creationData);

        if (handle.id == kInvalidSharedMemoryID || mSharedMemoryManager == nullptr) {
            return std::make_unique<MemoryHandleImpl>();
        }

        Ref<SharedMemory> sharedMemory = mSharedMemoryManager->AcquireFromWire(handle.id);
        if (sharedMemory == nullptr) {
            return std::make_unique<MemoryHandleImpl>();
        }

        return std::make_unique<MemoryHandleWithSharedMemoryImpl>(std::move(sharedMemory));
    }

  private:
    std::shared_ptr<InlineSharedMemoryManager> mSharedMemoryManager;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService(
    std::shared_ptr<InlineSharedMemoryManager> sharedMemoryManager) {
    return std::make_unique<InlineMemoryTransferService>(std::move(sharedMemoryManager));
}

}  // namespace dawn::wire::server
