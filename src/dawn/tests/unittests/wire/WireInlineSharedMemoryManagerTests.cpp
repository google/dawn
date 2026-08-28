// Copyright 2026 The Dawn & Tint Authors
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

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "dawn/wire/WireClient.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/wire/InlineSharedMemoryManager.h"
#include "src/dawn/wire/client/ClientInlineMemoryTransferService.h"
#include "src/dawn/wire/server/ServerInlineMemoryTransferService.h"
#include "src/utils/platform.h"

namespace dawn::wire {
namespace {

class InlineSharedMemoryManagerTest : public testing::Test {
  protected:
    void SetUp() override {
#if !DAWN_PLATFORM_IS(WINDOWS)
        GTEST_SKIP() << "InlineSharedMemoryManager is only implemented on Windows";
#endif
        mManager = CreateInlineSharedMemoryManager();
    }

    static constexpr std::array<std::byte, 8> kExpectedData = {
        std::byte(0), std::byte(1), std::byte(2), std::byte(3),
        std::byte(4), std::byte(5), std::byte(6), std::byte(7)};
    static constexpr size_t kSize = kExpectedData.size();

    std::shared_ptr<InlineSharedMemoryManager> mManager;
};

TEST_F(InlineSharedMemoryManagerTest, CreateSharedMemory_ReturnsNonNull) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(1024);
    EXPECT_NE(nullptr, memory.Get());
}

TEST_F(InlineSharedMemoryManagerTest, CreateSharedMemory_MultipleBuffersAreDistinct) {
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(256);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(256);
    EXPECT_NE(nullptr, memory1.Get());
    EXPECT_NE(nullptr, memory2.Get());
    EXPECT_NE(memory1.Get(), memory2.Get());
}

TEST_F(InlineSharedMemoryManagerTest, GetMappedSpan_ReturnsNonEmptySpanWithCorrectSize) {
    constexpr size_t kSize = 1024;
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(kSize);
    std::span<std::byte> span = memory->GetMappedSpan();
    EXPECT_FALSE(span.empty());
    EXPECT_EQ(kSize, span.size());
    EXPECT_NE(nullptr, span.data());
}

TEST_F(InlineSharedMemoryManagerTest, GetHandle_ReturnsNonNullHandle) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    EXPECT_TRUE(memory->GetSystemHandle().IsValid());
}

TEST_F(InlineSharedMemoryManagerTest, PutOnWire_ReturnsUniqueIds) {
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(256);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(256);
    SharedMemoryID id1 = mManager->PutOnWireAndGetID(memory1.Get());
    SharedMemoryID id2 = mManager->PutOnWireAndGetID(memory2.Get());
    EXPECT_NE(id1, id2);
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_ReturnsSameMemory) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    EXPECT_EQ(memory.Get(), acquired.Get());
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_ReturnsNullForUnknownId) {
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(SharedMemoryID(0u)).Get());
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(SharedMemoryID(99999u)).Get());
}

TEST_F(InlineSharedMemoryManagerTest, AcquireFromWire_RemovesFromWire) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    EXPECT_NE(nullptr, mManager->AcquireFromWire(id).Get());

    // A second acquire finds nothing since the reference was already transferred off the wire.
    EXPECT_EQ(nullptr, mManager->AcquireFromWire(id).Get());
}

TEST_F(InlineSharedMemoryManagerTest, PutOnWire_KeepsMemoryAliveAfterLocalRefDropped) {
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(256);
    SharedMemory* rawSharedMemoryPtr = memory.Get();
    SharedMemoryID id = mManager->PutOnWireAndGetID(rawSharedMemoryPtr);

    // Drop the local reference; the wire still holds one so the memory stays alive.
    memory = nullptr;

    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    EXPECT_EQ(rawSharedMemoryPtr, acquired.Get());
    EXPECT_FALSE(acquired->GetMappedSpan().empty());
}

TEST_F(InlineSharedMemoryManagerTest, DataRoundtrip_WriteAndReadBack) {
    constexpr size_t kSize = 256;
    Ref<SharedMemory> memory = mManager->CreateSharedMemory(kSize);

    std::span<std::byte> span = memory->GetMappedSpan();
    ASSERT_EQ(kSize, span.size());

    // Write a known pattern.
    for (size_t i = 0; i < kSize; ++i) {
        span[i] = static_cast<std::byte>(i);
    }

    // Put the memory on the wire and retrieve it back.
    SharedMemoryID id = mManager->PutOnWireAndGetID(memory.Get());
    Ref<SharedMemory> acquired = mManager->AcquireFromWire(id);
    ASSERT_NE(nullptr, acquired.Get());

    // Read back through the acquired reference and compare the data with the expected values.
    std::span<std::byte> readSpan = acquired->GetMappedSpan();
    ASSERT_EQ(kSize, readSpan.size());
    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(static_cast<std::byte>(i), readSpan[i]) << " at index " << i;
    }
}

TEST_F(InlineSharedMemoryManagerTest, MultipleBuffers_DataIsIsolated) {
    constexpr size_t kSize = 128;
    Ref<SharedMemory> memory1 = mManager->CreateSharedMemory(kSize);
    Ref<SharedMemory> memory2 = mManager->CreateSharedMemory(kSize);

    std::span<std::byte> span1 = memory1->GetMappedSpan();
    std::span<std::byte> span2 = memory2->GetMappedSpan();

    constexpr std::byte kData1 = std::byte{0xAA};
    constexpr std::byte kData2 = std::byte{0xBB};
    std::fill(span1.begin(), span1.end(), kData1);
    std::fill(span2.begin(), span2.end(), kData2);

    for (std::byte b : memory1->GetMappedSpan()) {
        EXPECT_EQ(kData1, b);
    }
    for (std::byte b : memory2->GetMappedSpan()) {
        EXPECT_EQ(kData2, b);
    }
}

// Tests for the client-side `InlineMemoryTransferService` created with a non-null
// `InlineSharedMemoryManager`.
class ClientInlineMemoryTransferServiceTest : public InlineSharedMemoryManagerTest {
  protected:
    void SetUp() override {
        InlineSharedMemoryManagerTest::SetUp();
        mService = client::CreateInlineMemoryTransferService(mManager);
        ASSERT_NE(nullptr, mService);
    }

    using MemoryHandle = client::MemoryTransferService::MemoryHandle;
    using MemoryHandleUse = client::MemoryTransferService::MemoryHandleUse;

    // Reads back a `SharedMemoryHandle` serialized by `MemoryHandle::SerializeCreate`.
    static SharedMemoryHandle ReadSerializedHandle(Span<const std::byte> serialized) {
        EXPECT_EQ(sizeof(SharedMemoryHandle), serialized.size());
        return ReinterpretSpan<const SharedMemoryHandle>(serialized)[0];
    }

    void TestSerializedAsStagingHandle(MemoryHandleUse stagingMemoryHandleUse) {
        DAWN_ASSERT(stagingMemoryHandleUse == MemoryHandleUse::BulkData ||
                    stagingMemoryHandleUse == MemoryHandleUse::MappedAtCreationData);

        std::unique_ptr<MemoryHandle> handle =
            mService->CreateMemoryHandle(kSize, stagingMemoryHandleUse);

        ASSERT_EQ(sizeof(SharedMemoryHandle), handle->GetSerializeCreateSize());
        std::vector<std::byte> serialized(handle->GetSerializeCreateSize());
        handle->SerializeCreate(
            std::span<volatile std::byte>(serialized.data(), serialized.size()));

        // Serialize the handle and read back the serialized `SharedMemoryHandle`. `wireHandle.id`
        // won't be `kInvalidSharedMemoryID` for a shared-memory-backed handle.
        SharedMemoryHandle wireHandle = ReadSerializedHandle(serialized);
        EXPECT_EQ(kInvalidSharedMemoryID, wireHandle.id);
    }

    std::unique_ptr<client::MemoryTransferService> mService;
};

// A handle backed by shared memory serializes a reference to the same underlying memory.
TEST_F(ClientInlineMemoryTransferServiceTest, SharedMemoryHandle_SerializeCreate) {
    std::unique_ptr<MemoryHandle> handle =
        mService->CreateMemoryHandle(kSize, MemoryHandleUse::MappedBuffer);

    ASSERT_EQ(sizeof(SharedMemoryHandle), handle->GetSerializeCreateSize());

    // Record the handle's mapped data before serializing.
    std::span<std::byte> handleData = handle->GetData();
    ASSERT_EQ(kSize, handleData.size());

    std::vector<std::byte> serialized(handle->GetSerializeCreateSize());
    handle->SerializeCreate(std::span<volatile std::byte>(serialized.data(), serialized.size()));

    // Deserializing should yield the exact same memory: same pointer and size.
    SharedMemoryHandle wireHandle = ReadSerializedHandle(serialized);
    Ref<SharedMemory> acquired = mManager->AcquireFromWire(SharedMemoryID(wireHandle.id));
    ASSERT_NE(nullptr, acquired.Get());
    EXPECT_EQ(handleData.data(), acquired->GetMappedSpan().data());
    EXPECT_EQ(handleData.size(), acquired->GetMappedSpan().size());
}

// A shared-memory-backed handle exposes the shared memory directly, so writes through GetData are
// visible in the acquired memory and data updates serialize nothing.
TEST_F(ClientInlineMemoryTransferServiceTest, SharedMemoryHandle_DataIsSharedDirectly) {
    std::unique_ptr<MemoryHandle> handle =
        mService->CreateMemoryHandle(kSize, MemoryHandleUse::MappedBuffer);

    std::span<std::byte> data = handle->GetData();
    ASSERT_EQ(kSize, data.size());

    // With shared memory we don't need to serialize anything.
    EXPECT_EQ(0u, handle->GetSerializeDataUpdateSize(0u, kSize));

    // Write a known pattern through the handle.
    Span<std::byte>(data).CopyFrom(kExpectedData);

    // Acquire the same shared memory off the wire and verify the pattern is visible.
    std::vector<std::byte> serialized(handle->GetSerializeCreateSize());
    handle->SerializeCreate(std::span<volatile std::byte>(serialized.data(), serialized.size()));
    SharedMemoryHandle wireHandle = ReadSerializedHandle(serialized);

    Ref<SharedMemory> acquired = mManager->AcquireFromWire(SharedMemoryID(wireHandle.id));
    ASSERT_NE(nullptr, acquired.Get());
    std::span<std::byte> acquiredData = acquired->GetMappedSpan();
    ASSERT_EQ(kSize, acquiredData.size());
    EXPECT_THAT(acquiredData, testing::ElementsAreArray(kExpectedData));
}

// With `MemoryHandleUse::BulkData` the service falls back to a staging-buffer-backed handle whose
// SerializeCreate writes an empty (zeroed) SharedMemoryHandle.
TEST_F(ClientInlineMemoryTransferServiceTest, StagingHandle_SerializeCreate_BulkData) {
    TestSerializedAsStagingHandle(MemoryHandleUse::BulkData);
}

// With `MemoryHandleUse::MappedAtCreationData` the service falls back to a staging-buffer-backed
// handle whose SerializeCreate writes an empty (zeroed) SharedMemoryHandle.
TEST_F(ClientInlineMemoryTransferServiceTest, StagingHandle_SerializeCreate_MappedAtCreationData) {
    TestSerializedAsStagingHandle(MemoryHandleUse::MappedAtCreationData);
}

// A staging-backed handle serializes its data updates into the provided array so the exact bytes
// written through GetData can be recovered from the serialized array.
TEST_F(ClientInlineMemoryTransferServiceTest, StagingHandle_SerializeDataUpdate) {
    std::unique_ptr<MemoryHandle> handle =
        mService->CreateMemoryHandle(kSize, MemoryHandleUse::BulkData);

    std::span<std::byte> data = handle->GetData();
    ASSERT_EQ(kSize, data.size());

    // Write a known pattern through the handle.
    Span<std::byte>(data).CopyFrom(kExpectedData);

    // Serialize the full range into an array and verify the array holds the written pattern.
    ASSERT_EQ(kSize, handle->GetSerializeDataUpdateSize(0u, kSize));
    std::vector<std::byte> serialized(handle->GetSerializeDataUpdateSize(0u, kSize));
    handle->SerializeDataUpdate(std::span<volatile std::byte>(serialized.data(), serialized.size()),
                                0u, kSize);

    EXPECT_THAT(serialized, testing::ElementsAreArray(kExpectedData));
}

// Tests for the server-side `InlineMemoryTransferService` created with a non-null
// `InlineSharedMemoryManager`.
class ServerInlineMemoryTransferServiceTest : public InlineSharedMemoryManagerTest {
  protected:
    void SetUp() override {
        InlineSharedMemoryManagerTest::SetUp();

        mClientService = client::CreateInlineMemoryTransferService(mManager);
        mServerService = server::CreateInlineMemoryTransferService(mManager);
        ASSERT_NE(nullptr, mClientService);
        ASSERT_NE(nullptr, mServerService);
    }

    using ClientMemoryHandle = client::MemoryTransferService::MemoryHandle;
    using ClientMemoryHandleUse = client::MemoryTransferService::MemoryHandleUse;
    using ServerMemoryHandle = server::MemoryTransferService::MemoryHandle;

    struct HandlePair {
        std::unique_ptr<ClientMemoryHandle> clientHandle;
        std::unique_ptr<ServerMemoryHandle> serverHandle;
    };

    // Creates a shared-memory-backed client handle, serializes its `SharedMemoryHandle` into a byte
    // vector, and deserializes the `SharedMemoryHandle` into a server handle through
    // `DeserializeMemoryHandle`.
    HandlePair CreateSharedMemoryHandlePair() {
        HandlePair pair;
        pair.clientHandle =
            mClientService->CreateMemoryHandle(kSize, ClientMemoryHandleUse::MappedBuffer);
        EXPECT_NE(nullptr, pair.clientHandle);

        std::vector<std::byte> createData(pair.clientHandle->GetSerializeCreateSize());
        pair.clientHandle->SerializeCreate(
            std::span<volatile std::byte>(createData.data(), createData.size()));

        pair.serverHandle = mServerService->DeserializeMemoryHandle(
            std::span<const std::byte>(createData.data(), createData.size()));
        EXPECT_NE(nullptr, pair.serverHandle);
        return pair;
    }

    std::unique_ptr<client::MemoryTransferService> mClientService;
    std::unique_ptr<server::MemoryTransferService> mServerService;
};

// Test that deserializing a data update copies the shared memory into the target buffer.
TEST_F(ServerInlineMemoryTransferServiceTest, DeserializeDataUpdate_CopiesSharedMemoryIntoTarget) {
    HandlePair pair = CreateSharedMemoryHandlePair();

    std::span<std::byte> clientData = pair.clientHandle->GetData();
    Span<std::byte>(clientData).CopyFrom(kExpectedData);

    EXPECT_EQ(0u, pair.serverHandle->GetSerializeDataUpdateSize(0u, kSize));

    std::vector<std::byte> target(kSize);
    EXPECT_TRUE(pair.serverHandle->DeserializeDataUpdate(
        std::span<const std::byte>(), 0u, kSize,
        std::span<std::byte>(target.data(), target.size())));
    EXPECT_THAT(target, testing::ElementsAreArray(kExpectedData));
}

// Test that serializing a data update copies the data into the shared memory.
TEST_F(ServerInlineMemoryTransferServiceTest, SerializeDataUpdate_CopiesDataIntoSharedMemory) {
    HandlePair pair = CreateSharedMemoryHandlePair();

    std::vector<std::byte> data(kSize);
    Span<std::byte>(data).CopyFrom(kExpectedData);
    pair.serverHandle->SerializeDataUpdate(std::span<volatile std::byte>(), 0u, kSize, data);

    std::span<std::byte> clientData = pair.clientHandle->GetData();
    EXPECT_THAT(clientData, testing::ElementsAreArray(kExpectedData));
}

}  // namespace
}  // namespace dawn::wire
