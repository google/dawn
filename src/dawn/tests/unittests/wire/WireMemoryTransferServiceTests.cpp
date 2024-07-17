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

#include <memory>
#include <utility>

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/client/ClientMemoryTransferService_mock.h"
#include "dawn/wire/server/ServerMemoryTransferService_mock.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Eq;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::Pointee;
using testing::Return;
using testing::StrictMock;
using testing::WithArg;

// Mock class to add expectations on the wire calling callbacks
class MockBufferMapCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUBufferMapAsyncStatus status, void* userdata));
};

std::unique_ptr<StrictMock<MockBufferMapCallback>> mockBufferMapCallback;
void ToMockBufferMapCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
    mockBufferMapCallback->Call(status, userdata);
}

// WireMemoryTransferServiceTests test the MemoryTransferService with buffer mapping.
// They test the basic success and error cases for buffer mapping, and they test
// mocked failures of each fallible MemoryTransferService method that an embedder
// could implement.
// The test harness defines multiple helpers for expecting operations on Read/Write handles
// and for mocking failures. The helpers are designed such that for a given run of a test,
// a Serialization expection has a corresponding Deserialization expectation for which the
// serialized data must match.
// There are tests which check for Success for every mapping operation which mock an entire
// mapping operation from map to unmap, and add all MemoryTransferService expectations. Tests
// which check for errors perform the same mapping operations but insert mocked failures for
// various mapping or MemoryTransferService operations.
class WireMemoryTransferServiceTests : public WireTest {
  public:
    WireMemoryTransferServiceTests() {}
    ~WireMemoryTransferServiceTests() override = default;

    client::MemoryTransferService* GetClientMemoryTransferService() override {
        return &clientMemoryTransferService;
    }

    server::MemoryTransferService* GetServerMemoryTransferService() override {
        return &serverMemoryTransferService;
    }

    void SetUp() override {
        WireTest::SetUp();

        mockBufferMapCallback = std::make_unique<StrictMock<MockBufferMapCallback>>();

        // TODO(enga): Make this thread-safe.
        mBufferContent++;
        mMappedBufferContent = 0;
        mUpdatedBufferContent++;
        mSerializeCreateInfo++;
        mReadHandleSerializeDataInfo++;
        mWriteHandleSerializeDataInfo++;
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockBufferMapCallback = nullptr;
    }

    void FlushClient(bool success = true) {
        WireTest::FlushClient(success);
        Mock::VerifyAndClearExpectations(&serverMemoryTransferService);
    }

    void FlushServer(bool success = true) {
        WireTest::FlushServer(success);

        Mock::VerifyAndClearExpectations(&mockBufferMapCallback);
        Mock::VerifyAndClearExpectations(&clientMemoryTransferService);
    }

  protected:
    using ClientReadHandle = client::MockMemoryTransferService::MockReadHandle;
    using ServerReadHandle = server::MockMemoryTransferService::MockReadHandle;
    using ClientWriteHandle = client::MockMemoryTransferService::MockWriteHandle;
    using ServerWriteHandle = server::MockMemoryTransferService::MockWriteHandle;

    std::pair<WGPUBuffer, WGPUBuffer> CreateBuffer(WGPUBufferUsage usage = WGPUBufferUsage_None) {
        WGPUBufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;
        descriptor.usage = usage;

        WGPUBuffer apiBuffer = api.GetNewBuffer();
        WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
            .WillOnce(Return(apiBuffer))
            .RetiresOnSaturation();

        return std::make_pair(apiBuffer, buffer);
    }

    std::pair<WGPUBuffer, WGPUBuffer> CreateBufferMapped(
        WGPUBufferUsage usage = WGPUBufferUsage_None) {
        WGPUBufferDescriptor descriptor = {};
        descriptor.size = sizeof(mBufferContent);
        descriptor.mappedAtCreation = true;
        descriptor.usage = usage;

        WGPUBuffer apiBuffer = api.GetNewBuffer();

        WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, sizeof(mBufferContent)))
            .WillOnce(Return(&mMappedBufferContent));

        return std::make_pair(apiBuffer, buffer);
    }

    ClientReadHandle* ExpectReadHandleCreation() {
        // Create the handle first so we can use it in later expectations.
        ClientReadHandle* handle = clientMemoryTransferService.NewReadHandle();

        EXPECT_CALL(clientMemoryTransferService, OnCreateReadHandle(sizeof(mBufferContent)))
            .WillOnce(InvokeWithoutArgs([=] { return handle; }));

        return handle;
    }

    void MockReadHandleCreationFailure() {
        EXPECT_CALL(clientMemoryTransferService, OnCreateReadHandle(sizeof(mBufferContent)))
            .WillOnce(InvokeWithoutArgs([=] { return nullptr; }));
    }

    void ExpectReadHandleSerialization(ClientReadHandle* handle) {
        EXPECT_CALL(clientMemoryTransferService, OnReadHandleSerializeCreateSize(handle))
            .WillOnce(InvokeWithoutArgs([&] { return sizeof(mSerializeCreateInfo); }));
        EXPECT_CALL(clientMemoryTransferService, OnReadHandleSerializeCreate(handle, _))
            .WillOnce(WithArg<1>([&](void* serializePointer) {
                memcpy(serializePointer, &mSerializeCreateInfo, sizeof(mSerializeCreateInfo));
                return sizeof(mSerializeCreateInfo);
            }));
    }

    ServerReadHandle* ExpectServerReadHandleDeserialize() {
        // Create the handle first so we can use it in later expectations.
        ServerReadHandle* handle = serverMemoryTransferService.NewReadHandle();

        EXPECT_CALL(serverMemoryTransferService,
                    OnDeserializeReadHandle(Pointee(Eq(mSerializeCreateInfo)),
                                            sizeof(mSerializeCreateInfo), _))
            .WillOnce(WithArg<2>([=](server::MemoryTransferService::ReadHandle** readHandle) {
                *readHandle = handle;
                return true;
            }));

        return handle;
    }

    void MockServerReadHandleDeserializeFailure() {
        EXPECT_CALL(serverMemoryTransferService,
                    OnDeserializeReadHandle(Pointee(Eq(mSerializeCreateInfo)),
                                            sizeof(mSerializeCreateInfo), _))
            .WillOnce(InvokeWithoutArgs([&] { return false; }));
    }

    void ExpectServerReadHandleSerializeDataUpdate(ServerReadHandle* handle) {
        EXPECT_CALL(serverMemoryTransferService,
                    OnReadHandleSizeOfSerializeDataUpdate(handle, _, _))
            .WillOnce(InvokeWithoutArgs([&] { return sizeof(mReadHandleSerializeDataInfo); }));
        EXPECT_CALL(serverMemoryTransferService,
                    OnReadHandleSerializeDataUpdate(handle, _, _, _, _))
            .WillOnce(WithArg<4>([&](void* serializePointer) {
                memcpy(serializePointer, &mReadHandleSerializeDataInfo,
                       sizeof(mReadHandleSerializeDataInfo));
                return sizeof(mReadHandleSerializeDataInfo);
            }));
    }

    void ExpectClientReadHandleDeserializeDataUpdate(ClientReadHandle* handle,
                                                     uint32_t* mappedData) {
        EXPECT_CALL(
            clientMemoryTransferService,
            OnReadHandleDeserializeDataUpdate(handle, Pointee(Eq(mReadHandleSerializeDataInfo)),
                                              sizeof(mReadHandleSerializeDataInfo), _, _))
            .WillOnce(Return(true));
    }

    void MockClientReadHandleDeserializeDataUpdateFailure(ClientReadHandle* handle) {
        EXPECT_CALL(
            clientMemoryTransferService,
            OnReadHandleDeserializeDataUpdate(handle, Pointee(Eq(mReadHandleSerializeDataInfo)),
                                              sizeof(mReadHandleSerializeDataInfo), _, _))
            .WillOnce(Return(false));
    }

    ClientWriteHandle* ExpectWriteHandleCreation(bool mappedAtCreation) {
        // Create the handle first so we can use it in later expectations.
        ClientWriteHandle* handle = clientMemoryTransferService.NewWriteHandle();

        EXPECT_CALL(clientMemoryTransferService, OnCreateWriteHandle(sizeof(mBufferContent)))
            .WillOnce(InvokeWithoutArgs([=] { return handle; }));
        if (mappedAtCreation) {
            EXPECT_CALL(clientMemoryTransferService, OnWriteHandleGetData(handle))
                .WillOnce(Return(&mBufferContent));
        }

        return handle;
    }

    void MockWriteHandleCreationFailure() {
        EXPECT_CALL(clientMemoryTransferService, OnCreateWriteHandle(sizeof(mBufferContent)))
            .WillOnce(InvokeWithoutArgs([=] { return nullptr; }));
    }

    void ExpectWriteHandleSerialization(ClientWriteHandle* handle) {
        EXPECT_CALL(clientMemoryTransferService, OnWriteHandleSerializeCreateSize(handle))
            .WillOnce(InvokeWithoutArgs([&] { return sizeof(mSerializeCreateInfo); }));
        EXPECT_CALL(clientMemoryTransferService, OnWriteHandleSerializeCreate(handle, _))
            .WillOnce(WithArg<1>([&](void* serializePointer) {
                memcpy(serializePointer, &mSerializeCreateInfo, sizeof(mSerializeCreateInfo));
                return sizeof(mSerializeCreateInfo);
            }));
    }

    ServerWriteHandle* ExpectServerWriteHandleDeserialization() {
        // Create the handle first so it can be used in later expectations.
        ServerWriteHandle* handle = serverMemoryTransferService.NewWriteHandle();

        EXPECT_CALL(serverMemoryTransferService,
                    OnDeserializeWriteHandle(Pointee(Eq(mSerializeCreateInfo)),
                                             sizeof(mSerializeCreateInfo), _))
            .WillOnce(WithArg<2>([=](server::MemoryTransferService::WriteHandle** writeHandle) {
                *writeHandle = handle;
                return true;
            }));

        return handle;
    }

    void MockServerWriteHandleDeserializeFailure() {
        EXPECT_CALL(serverMemoryTransferService,
                    OnDeserializeWriteHandle(Pointee(Eq(mSerializeCreateInfo)),
                                             sizeof(mSerializeCreateInfo), _))
            .WillOnce(Return(false));
    }

    void ExpectClientWriteHandleSerializeDataUpdate(ClientWriteHandle* handle) {
        EXPECT_CALL(clientMemoryTransferService,
                    OnWriteHandleSizeOfSerializeDataUpdate(handle, _, _))
            .WillOnce(InvokeWithoutArgs([&] { return sizeof(mWriteHandleSerializeDataInfo); }));
        EXPECT_CALL(clientMemoryTransferService, OnWriteHandleSerializeDataUpdate(handle, _, _, _))
            .WillOnce(WithArg<1>([&](void* serializePointer) {
                memcpy(serializePointer, &mWriteHandleSerializeDataInfo,
                       sizeof(mWriteHandleSerializeDataInfo));
                return sizeof(mWriteHandleSerializeDataInfo);
            }));
    }

    void ExpectServerWriteHandleDeserializeDataUpdate(ServerWriteHandle* handle,
                                                      uint32_t expectedData) {
        EXPECT_CALL(
            serverMemoryTransferService,
            OnWriteHandleDeserializeDataUpdate(handle, Pointee(Eq(mWriteHandleSerializeDataInfo)),
                                               sizeof(mWriteHandleSerializeDataInfo), _, _))
            .WillOnce(Return(true));
    }

    void MockServerWriteHandleDeserializeDataUpdateFailure(ServerWriteHandle* handle) {
        EXPECT_CALL(
            serverMemoryTransferService,
            OnWriteHandleDeserializeDataUpdate(handle, Pointee(Eq(mWriteHandleSerializeDataInfo)),
                                               sizeof(mWriteHandleSerializeDataInfo), _, _))
            .WillOnce(Return(false));
    }

    // Arbitrary values used within tests to check if serialized data is correctly passed
    // between the client and server. The static data changes between runs of the tests and
    // test expectations will check that serialized values are passed to the respective
    // deserialization function.
    static uint32_t mSerializeCreateInfo;
    static uint32_t mReadHandleSerializeDataInfo;
    static uint32_t mWriteHandleSerializeDataInfo;

    // Represents the buffer contents for the test.
    static uint32_t mBufferContent;

    static constexpr size_t kBufferSize = sizeof(mBufferContent);

    // The client's zero-initialized buffer for writing.
    uint32_t mMappedBufferContent = 0;

    // |mMappedBufferContent| should be set equal to |mUpdatedBufferContent| when the client
    // performs a write. Test expectations should check that |mBufferContent ==
    // mUpdatedBufferContent| after all writes are flushed.
    static uint32_t mUpdatedBufferContent;

    StrictMock<wire::server::MockMemoryTransferService> serverMemoryTransferService;
    StrictMock<wire::client::MockMemoryTransferService> clientMemoryTransferService;
};

uint32_t WireMemoryTransferServiceTests::mBufferContent = 1337;
uint32_t WireMemoryTransferServiceTests::mUpdatedBufferContent = 2349;
uint32_t WireMemoryTransferServiceTests::mSerializeCreateInfo = 4242;
uint32_t WireMemoryTransferServiceTests::mReadHandleSerializeDataInfo = 1394;
uint32_t WireMemoryTransferServiceTests::mWriteHandleSerializeDataInfo = 1235;

// Test successful mapping for reading.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadSuccess) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on creation.
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);

    // The server should deserialize the read handle from the client and then serialize
    // an initialization message.
    ServerReadHandle* serverHandle = ExpectServerReadHandleDeserialize();

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // The handle serialize data update on mapAsync cmd
    ExpectServerReadHandleSerializeDataUpdate(serverHandle);

    // Mock a successful callback
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleGetData(clientHandle))
        .WillOnce(Return(&mBufferContent));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mBufferContent));

    FlushClient();

    // The client receives a successful callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    // The client should receive the handle data update message from the server.
    ExpectClientReadHandleDeserializeDataUpdate(clientHandle, &mBufferContent);

    FlushServer();

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test ReadHandle destroy behavior
TEST_F(WireMemoryTransferServiceTests, BufferMapReadDestroy) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on creation.
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);

    // The server should deserialize the read handle from the client and then serialize
    // an initialization message.
    ServerReadHandle* serverHandle = ExpectServerReadHandleDeserialize();

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
    wgpuBufferDestroy(buffer);
    EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);

    FlushClient();

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test unsuccessful mapping for reading.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadError) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on creation.
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    // The server should deserialize the ReadHandle from the client.
    ServerReadHandle* serverHandle = ExpectServerReadHandleDeserialize();

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);
    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Mock a failed callback.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    // The client receives an error callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    wgpuBufferUnmap(buffer);

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test ReadHandle creation failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadHandleCreationFailure) {
    // Mock a ReadHandle creation failure
    MockReadHandleCreationFailure();

    WGPUBufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.usage = WGPUBufferUsage_MapRead;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);
    wgpuBufferRelease(buffer);
}

// Test MapRead DeserializeReadHandle failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadDeserializeReadHandleFailure) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on mapping for reading..
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);

    // Mock a Deserialization failure.
    MockServerReadHandleDeserializeFailure();

    FlushClient(false);

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test read handle DeserializeDataUpdate failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadDeserializeDataUpdateFailure) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on mapping for reading.
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    // The server should deserialize the read handle from the client and then serialize
    // an initialization message.
    ServerReadHandle* serverHandle = ExpectServerReadHandleDeserialize();

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);
    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // The handle serialize data update on mapAsync cmd
    ExpectServerReadHandleSerializeDataUpdate(serverHandle);

    // Mock a successful callback
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mBufferContent));

    FlushClient();

    // The client should receive the handle data update message from the server.
    // Mock a deserialization failure.
    MockClientReadHandleDeserializeDataUpdateFailure(clientHandle);

    // Failed deserialization is a fatal failure and the client synchronously receives a
    // DEVICE_LOST callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Unknown, _)).Times(1);

    FlushServer(false);

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test mapping for reading destroying the buffer before unmapping on the client side.
TEST_F(WireMemoryTransferServiceTests, BufferMapReadDestroyBeforeUnmap) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a ReadHandle on mapping for reading..
    ClientReadHandle* clientHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientHandle);

    // The server should deserialize the read handle from the client and then serialize
    // an initialization message.
    ServerReadHandle* serverHandle = ExpectServerReadHandleDeserialize();

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapRead);
    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // The handle serialize data update on mapAsync cmd
    ExpectServerReadHandleSerializeDataUpdate(serverHandle);

    // Mock a successful callback
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleGetData(clientHandle))
        .WillOnce(Return(&mBufferContent));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mBufferContent));

    FlushClient();

    // The client receives a successful callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    // The client should receive the handle data update message from the server.
    ExpectClientReadHandleDeserializeDataUpdate(clientHandle, &mBufferContent);

    FlushServer();

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientHandle)).Times(1);
        wgpuBufferDestroy(buffer);

        EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverHandle)).Times(1);
        EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
        FlushClient();

        // The handle is already destroyed so unmap only results in a server unmap call.
        wgpuBufferUnmap(buffer);

        EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
        FlushClient();
    }

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test successful mapping for writing.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteSuccess) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Mock a successful callback.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleGetData(clientHandle))
        .WillOnce(Return(&mBufferContent));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mMappedBufferContent));

    FlushClient();

    // The client receives a successful callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // The client writes to the handle contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // The client will then serialize data update and destroy the handle on Unmap()
    ExpectClientWriteHandleSerializeDataUpdate(clientHandle);

    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message.
    ExpectServerWriteHandleDeserializeDataUpdate(serverHandle, mUpdatedBufferContent);

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test WriteHandle destroy behavior
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteDestroy) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    wgpuBufferDestroy(buffer);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);

    FlushClient();

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test unsuccessful MapWrite.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteError) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a WriteHandle on buffer creation with MapWrite
    // usage.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Mock an error callback.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    // The client receives an error callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    wgpuBufferUnmap(buffer);

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test WriteHandle creation failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteHandleCreationFailure) {
    // Mock a WriteHandle creation failure
    MockWriteHandleCreationFailure();

    WGPUBufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.usage = WGPUBufferUsage_MapWrite;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);
    wgpuBufferRelease(buffer);
}

// Test MapWrite DeserializeWriteHandle failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteDeserializeWriteHandleFailure) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    // The client should create and serialize a WriteHandle on buffer creation with MapWrite
    // usage.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // Mock a deserialization failure.
    MockServerWriteHandleDeserializeFailure();

    FlushClient(false);

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test MapWrite DeserializeDataUpdate failure.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteDeserializeDataUpdateFailure) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Mock a successful callback.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleGetData(clientHandle))
        .WillOnce(Return(&mBufferContent));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mMappedBufferContent));

    FlushClient();

    // The client receives a success callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // The client writes to the handle contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // The client will then serialize data update
    ExpectClientWriteHandleSerializeDataUpdate(clientHandle);

    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message. Mock a deserialization failure.
    MockServerWriteHandleDeserializeDataUpdateFailure(serverHandle);

    FlushClient(false);

    // The handle is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test MapWrite destroying the buffer before unmapping on the client side.
TEST_F(WireMemoryTransferServiceTests, BufferMapWriteDestroyBeforeUnmap) {
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;

    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(false);
    ExpectWriteHandleSerialization(clientHandle);

    std::tie(apiBuffer, buffer) = CreateBuffer(WGPUBufferUsage_MapWrite);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Mock a successful callback.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleGetData(clientHandle))
        .WillOnce(Return(&mBufferContent));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mMappedBufferContent));

    FlushClient();

    // The client receives a successful callback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // The client writes to the handle contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        // The handle is destroyed once the buffer is destroyed.
        EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);

        wgpuBufferDestroy(buffer);

        EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
        EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
        FlushClient();

        // The handle is already destroyed so unmap only results in a server unmap call.
        wgpuBufferUnmap(buffer);

        EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
        FlushClient();
    }

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test successful buffer creation with mappedAtCreation = true.
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationSuccess) {
    // The client should create and serialize a WriteHandle on createBufferMapped.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(true);
    ExpectWriteHandleSerialization(clientHandle);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
    std::tie(apiBuffer, buffer) = CreateBufferMapped();
    FlushClient();

    // Update the mapped contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // When the client Unmaps the buffer, it will serialize data update writes to the handle and
    // destroy it.
    ExpectClientWriteHandleSerializeDataUpdate(clientHandle);
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);

    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message.
    ExpectServerWriteHandleDeserializeDataUpdate(serverHandle, mUpdatedBufferContent);

    // After the handle is updated it can be destroyed.
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test buffer creation with mappedAtCreation WriteHandle creation failure.
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationWriteHandleCreationFailure) {
    // Mock a WriteHandle creation failure
    MockWriteHandleCreationFailure();

    WGPUBufferDescriptor descriptor = {};
    descriptor.size = sizeof(mBufferContent);
    descriptor.mappedAtCreation = true;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);
    EXPECT_EQ(nullptr, buffer);
}

// Test buffer creation with mappedAtCreation DeserializeWriteHandle failure.
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationDeserializeWriteHandleFailure) {
    // The client should create and serialize a WriteHandle on createBufferMapped.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(true);
    ExpectWriteHandleSerialization(clientHandle);

    // The server should then deserialize the WriteHandle from the client.
    MockServerWriteHandleDeserializeFailure();

    WGPUBufferDescriptor descriptor = {};
    descriptor.size = sizeof(mBufferContent);
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(cDevice, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    // Now bufferGetMappedRange won't be called if deserialize writeHandle fails

    FlushClient(false);

    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test buffer creation with mappedAtCreation = true DeserializeDataUpdate failure.
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationDeserializeDataUpdateFailure) {
    // The client should create and serialize a WriteHandle on createBufferMapped.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(true);
    ExpectWriteHandleSerialization(clientHandle);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
    std::tie(apiBuffer, buffer) = CreateBufferMapped();
    FlushClient();

    // Update the mapped contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // When the client Unmaps the buffer, it will serialize data update writes to the handle and
    // destroy it.
    ExpectClientWriteHandleSerializeDataUpdate(clientHandle);
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);

    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message. Mock a deserialization failure.
    MockServerWriteHandleDeserializeDataUpdateFailure(serverHandle);

    FlushClient(false);

    // Failed BufferUpdateMappedData cmd will early return so BufferUnmap is not processed.
    // The server side writeHandle is destructed at buffer destruction.
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test mappedAtCreation=true destroying the buffer before unmapping on the client side.
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationDestroyBeforeUnmap) {
    // The client should create and serialize a WriteHandle on createBufferMapped.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(true);
    ExpectWriteHandleSerialization(clientHandle);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
    std::tie(apiBuffer, buffer) = CreateBufferMapped();
    FlushClient();

    // Update the mapped contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
        wgpuBufferDestroy(buffer);

        EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
        EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
        FlushClient();

        // The handle is already destroyed so unmap only results in a server unmap call.
        wgpuBufferUnmap(buffer);

        EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
        FlushClient();
    }

    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test a buffer with mappedAtCreation and MapRead usage destroy WriteHandle on unmap and switch
// data pointer to ReadHandle
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationAndMapReadSuccess) {
    // The client should create and serialize a ReadHandle and a WriteHandle on
    // createBufferMapped.
    ClientReadHandle* clientReadHandle = ExpectReadHandleCreation();
    ExpectReadHandleSerialization(clientReadHandle);
    ClientWriteHandle* clientWriteHandle = ExpectWriteHandleCreation(true);
    ExpectWriteHandleSerialization(clientWriteHandle);

    // The server should then deserialize a ReadHandle and a WriteHandle from the client.
    ServerReadHandle* serverReadHandle = ExpectServerReadHandleDeserialize();
    ServerWriteHandle* serverWriteHandle = ExpectServerWriteHandleDeserialization();

    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
    std::tie(apiBuffer, buffer) = CreateBufferMapped(WGPUBufferUsage_MapRead);
    FlushClient();

    // Update the mapped contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // When the client Unmaps the buffer, it will serialize data update writes to the handle and
    // destroy it.
    ExpectClientWriteHandleSerializeDataUpdate(clientWriteHandle);
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientWriteHandle)).Times(1);
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleGetData(clientReadHandle))
        .WillOnce(Return(&mBufferContent));
    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message.
    ExpectServerWriteHandleDeserializeDataUpdate(serverWriteHandle, mUpdatedBufferContent);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverWriteHandle)).Times(1);
    FlushClient();

    // The ReadHandle will be destroyed on buffer destroy.
    EXPECT_CALL(clientMemoryTransferService, OnReadHandleDestroy(clientReadHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnReadHandleDestroy(serverReadHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

// Test WriteHandle preserves after unmap for a buffer with mappedAtCreation and MapWrite usage
TEST_F(WireMemoryTransferServiceTests, MappedAtCreationAndMapWriteSuccess) {
    // The client should create and serialize a WriteHandle on createBufferMapped.
    ClientWriteHandle* clientHandle = ExpectWriteHandleCreation(true);

    ExpectWriteHandleSerialization(clientHandle);

    // The server should then deserialize the WriteHandle from the client.
    ServerWriteHandle* serverHandle = ExpectServerWriteHandleDeserialization();

    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
    std::tie(apiBuffer, buffer) = CreateBufferMapped(WGPUBufferUsage_MapWrite);
    FlushClient();

    // Update the mapped contents.
    mMappedBufferContent = mUpdatedBufferContent;

    // When the client Unmaps the buffer, it will serialize data update writes to the handle.
    ExpectClientWriteHandleSerializeDataUpdate(clientHandle);

    wgpuBufferUnmap(buffer);

    // The server deserializes the data update message.
    ExpectServerWriteHandleDeserializeDataUpdate(serverHandle, mUpdatedBufferContent);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // The writeHandle is preserved after unmap and is destroyed once the buffer is destroyed.
    EXPECT_CALL(clientMemoryTransferService, OnWriteHandleDestroy(clientHandle)).Times(1);
    EXPECT_CALL(serverMemoryTransferService, OnWriteHandleDestroy(serverHandle)).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    wgpuBufferRelease(buffer);

    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
