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

#include "src/dawn/wire/client/ClientInlineMemoryTransferService.h"

#include <memory>
#include <utility>

#include "dawn/wire/WireClient.h"
#include "src/dawn/wire/InlineSharedMemoryManager.h"
#include "src/dawn/wire/client/Client.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/heap_array.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"

namespace dawn::wire::client {

class InlineMemoryTransferService : public MemoryTransferService {
    class MemoryHandleImpl : public MemoryHandle {
      public:
        explicit MemoryHandleImpl(HeapArray<std::byte> stagingData)
            : mStagingData(std::move(stagingData)) {
            DAWN_ASSERT(mStagingData);
        }

        ~MemoryHandleImpl() override = default;

        size_t GetSerializeCreateSize() const override { return sizeof(SharedMemoryHandle); }
        void SerializeCreate(std::span<volatile std::byte> serializeSpace) const override {
            DAWN_ASSERT(serializeSpace.size() == GetSerializeCreateSize());

            //  When we serialize a handle backed by staging data, the ID will always be
            // `kInvalidSharedMemoryID`.
            SharedMemoryHandle handle{kInvalidSharedMemoryID};
            Span<volatile std::byte>(serializeSpace).CopyFrom(ByteSpanFromRef(handle));
        }

        std::span<std::byte> GetData() const override { return mStagingData; }

        size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const override {
            DAWN_ASSERT(offset <= mStagingData.size());
            DAWN_ASSERT(size <= mStagingData.size() - offset);
            return size;
        }

        void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                 size_t offset,
                                 size_t size) const override {
            DAWN_ASSERT(serializeData.size() == GetSerializeDataUpdateSize(offset, size));
            DAWN_ASSERT(offset <= mStagingData.size());
            DAWN_ASSERT(size <= mStagingData.size() - offset);

            auto src = GetData().subspan(offset, serializeData.size());
            std::ranges::copy(src, serializeData.begin());
        }

        bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                   size_t offset,
                                   size_t size) override {
            if (offset > mStagingData.size() ||
                deserializeData.size() > mStagingData.size() - offset) {
                return false;
            }

            std::ranges::copy(deserializeData, GetData().begin() + sign_cast(offset));
            return true;
        }

      private:
        HeapArray<std::byte> mStagingData;
    };

    class MemoryHandleWithSharedMemoryImpl : public MemoryHandle {
      public:
        MemoryHandleWithSharedMemoryImpl(
            std::shared_ptr<InlineSharedMemoryManager> sharedMemoryManager,
            Ref<SharedMemory> sharedMemory)
            : mSharedMemoryManager(std::move(sharedMemoryManager)),
              mSharedMemory(std::move(sharedMemory)) {}

        ~MemoryHandleWithSharedMemoryImpl() override = default;

        size_t GetSerializeCreateSize() const override { return sizeof(SharedMemoryHandle); }

        void SerializeCreate(std::span<volatile std::byte> serializeSpace) const override {
            DAWN_ASSERT(serializeSpace.size() == GetSerializeCreateSize());

            // When we serialize a handle backend by shared memory, the ID will never be
            // `kInvalidSharedMemoryID`.
            SharedMemoryID id = mSharedMemoryManager->PutOnWireAndGetID(mSharedMemory.Get());
            SharedMemoryHandle handle{id};
            Span<volatile std::byte>(serializeSpace).CopyFrom(ByteSpanFromRef(handle));
        }

        std::span<std::byte> GetData() const override { return mSharedMemory->GetMappedSpan(); }

        size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const override { return 0; }

        void SerializeDataUpdate(std::span<volatile std::byte> serializeData,
                                 size_t offset,
                                 size_t size) const override {
            DAWN_ASSERT(serializeData.size() == GetSerializeDataUpdateSize(offset, size));
        }

        bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                   size_t offset,
                                   size_t size) override {
            DAWN_ASSERT(deserializeData.size() == 0u);
            return true;
        }

      private:
        std::shared_ptr<InlineSharedMemoryManager> mSharedMemoryManager;
        Ref<SharedMemory> mSharedMemory;
    };

  public:
    InlineMemoryTransferService() {}
    explicit InlineMemoryTransferService(
        std::shared_ptr<InlineSharedMemoryManager> sharedMemoryManager)
        : mSharedMemoryManager(std::move(sharedMemoryManager)) {}

    ~InlineMemoryTransferService() override = default;

    std::unique_ptr<MemoryHandle> CreateMemoryHandle(size_t size) override {
        auto stagingData = HeapArray<std::byte>(size, std::nothrow);
        if (!stagingData) {
            return nullptr;
        }

        return std::make_unique<MemoryHandleImpl>(std::move(stagingData));
    }

    std::unique_ptr<MemoryHandle> CreateMemoryHandle(size_t size,
                                                     MemoryHandleUse memoryHandleUse) override {
        if (CanUseSharedMemoryInWire(size, memoryHandleUse)) {
            Ref<SharedMemory> sharedMemory = mSharedMemoryManager->CreateSharedMemory(size);
            if (sharedMemory != nullptr) {
                return std::make_unique<MemoryHandleWithSharedMemoryImpl>(mSharedMemoryManager,
                                                                          std::move(sharedMemory));
            }
        }
        return CreateMemoryHandle(size);
    }

  private:
    bool CanUseSharedMemoryInWire(size_t size, MemoryHandleUse memoryHandleUse) const {
        if (mSharedMemoryManager == nullptr) {
            return false;
        }
        if (size == 0) {
            return false;
        }

        return memoryHandleUse == MemoryHandleUse::MappedBuffer;
    }

    std::shared_ptr<InlineSharedMemoryManager> mSharedMemoryManager;
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService(
    std::shared_ptr<InlineSharedMemoryManager> sharedMemoryManager) {
    return std::make_unique<InlineMemoryTransferService>(std::move(sharedMemoryManager));
}

}  // namespace dawn::wire::client
