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

#include <cstring>
#include <memory>
#include <utility>

#include "dawn/wire/WireClient.h"
#include "src/dawn/common/Alloc.h"
#include "src/dawn/wire/client/Client.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"

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

        bool DeserializeDataUpdate(std::span<const uint8_t> deserializeData,
                                   size_t offset) override {
            if (offset > mSize || deserializeData.size() > mSize - offset) {
                return false;
            }

            void* start = DAWN_UNSAFE_TODO(static_cast<uint8_t*>(mStagingData.get()) + offset);
            DAWN_UNSAFE_TODO(memcpy(start, deserializeData.data(), deserializeData.size()));
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
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(size <= mSize - offset);
            return size;
        }

        void SerializeDataUpdate(std::span<char> serializeData, size_t offset) override {
            DAWN_ASSERT(mStagingData != nullptr);
            DAWN_ASSERT(serializeData.data() != nullptr);
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(serializeData.size() <= mSize - offset);
            DAWN_UNSAFE_TODO(memcpy(serializeData.data(),
                                    static_cast<uint8_t*>(mStagingData.get()) + offset,
                                    serializeData.size()));
        }

      private:
        std::unique_ptr<uint8_t[]> mStagingData;
        size_t mSize;
    };

    class MemoryHandleImpl : public MemoryHandle {
      public:
        explicit MemoryHandleImpl(std::unique_ptr<std::byte[]> stagingData, size_t size)
            : mStagingData(std::move(stagingData)), mSize(size) {
            DAWN_ASSERT(mStagingData != nullptr);
        }

        ~MemoryHandleImpl() override = default;

        size_t GetSerializeCreateSize() const override { return 0; }
        void SerializeCreate(std::span<std::byte> serializeSpace) const override {
            DAWN_ASSERT(serializeSpace.size() == GetSerializeCreateSize());
        }

        std::span<std::byte> GetData() const override {
            // SAFETY: The creator of the object must make mSize be the size of the allocation of
            // mStagingData.
            return DAWN_UNSAFE_BUFFERS({mStagingData.get(), mSize});
        }

        size_t GetSerializeDataUpdateSize(size_t offset, size_t size) const override {
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(size <= mSize - offset);
            return size;
        }

        void SerializeDataUpdate(std::span<std::byte> serializeData,
                                 size_t offset,
                                 size_t size) const override {
            DAWN_ASSERT(serializeData.size() == GetSerializeDataUpdateSize(offset, size));
            DAWN_ASSERT(offset <= mSize);
            DAWN_ASSERT(size <= mSize - offset);

            // TODO(https://crbug.com/524406299): Use span::copy_from
            std::span<const std::byte> source = GetConstData().subspan(offset, size);
            DAWN_UNSAFE_TODO(memcpy(serializeData.data(), source.data(), size));
        }

        bool DeserializeDataUpdate(std::span<const std::byte> deserializeData,
                                   size_t offset,
                                   size_t size) override {
            if (offset > mSize || deserializeData.size() > mSize - offset) {
                return false;
            }

            // TODO(https://crbug.com/524406299): Use span::copy_from
            std::span<std::byte> target = GetData().subspan(offset, size);
            DAWN_UNSAFE_TODO(memcpy(target.data(), deserializeData.data(), size));
            return true;
        }

      private:
        // TODO(https://crbug.com/512465980): Use HeapArray instead.
        std::unique_ptr<std::byte[]> mStagingData;
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
            DAWN_UNSAFE_TODO(memset(stagingData.get(), 0, size));
            return new WriteHandleImpl(std::move(stagingData), size);
        }
        return nullptr;
    }

    std::unique_ptr<MemoryHandle> CreateMemoryHandle(size_t size) override {
        // TODO(https://crbug.com/512465980): Use HeapArray instead.
        auto stagingData = std::unique_ptr<std::byte[]>(AllocNoThrow<std::byte>(size));
        if (!stagingData) {
            return nullptr;
        }

        return std::make_unique<MemoryHandleImpl>(std::move(stagingData), size);
    }
};

std::unique_ptr<MemoryTransferService> CreateInlineMemoryTransferService() {
    return std::make_unique<InlineMemoryTransferService>();
}

}  // namespace dawn::wire::client
