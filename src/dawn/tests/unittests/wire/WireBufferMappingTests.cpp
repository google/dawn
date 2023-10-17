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

#include <limits>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::Return;
using testing::StrictMock;

// Mock class to add expectations on the wire calling callbacks
class MockBufferMapCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUBufferMapAsyncStatus status, void* userdata));
};

std::unique_ptr<StrictMock<MockBufferMapCallback>> mockBufferMapCallback;
void ToMockBufferMapCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
    mockBufferMapCallback->Call(status, userdata);
}

class WireBufferMappingTests : public WireTest {
  public:
    WireBufferMappingTests() {}
    ~WireBufferMappingTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockBufferMapCallback = std::make_unique<StrictMock<MockBufferMapCallback>>();
        apiBuffer = api.GetNewBuffer();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockBufferMapCallback = nullptr;
    }

    void FlushClient() {
        WireTest::FlushClient();
        Mock::VerifyAndClearExpectations(&mockBufferMapCallback);
    }

    void FlushServer() {
        WireTest::FlushServer();
        Mock::VerifyAndClearExpectations(&mockBufferMapCallback);
    }

    void SetupBuffer(WGPUBufferUsageFlags usage) {
        WGPUBufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;
        descriptor.usage = usage;

        buffer = wgpuDeviceCreateBuffer(device, &descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
            .WillOnce(Return(apiBuffer))
            .RetiresOnSaturation();
        FlushClient();
    }

  protected:
    static constexpr uint64_t kBufferSize = sizeof(uint32_t);
    // A successfully created buffer
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
};

// Tests specific to mapping for reading
class WireBufferMappingReadTests : public WireBufferMappingTests {
  public:
    WireBufferMappingReadTests() {}
    ~WireBufferMappingReadTests() override = default;

    void SetUp() override {
        WireBufferMappingTests::SetUp();

        SetupBuffer(WGPUBufferUsage_MapRead);
    }
};

// Check mapping for reading a succesfully created buffer
TEST_F(WireBufferMappingReadTests, MappingForReadSuccessBuffer) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Check that things work correctly when a validation error happens when mapping the buffer for
// reading
TEST_F(WireBufferMappingReadTests, ErrorWhileMappingForRead) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize));
}

// Check that the map read callback is called with UNKNOWN when the buffer is destroyed before
// the request is finished
TEST_F(WireBufferMappingReadTests, DestroyBeforeReadRequestEnd) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Return success
    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    // Destroy before the client gets the success, so the callback is called with
    // DestroyedBeforeCallback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
        .Times(1);
    wgpuBufferRelease(buffer);
    EXPECT_CALL(api, BufferRelease(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check the map read callback is called with "UnmappedBeforeCallback" when the map request
// would have worked, but Unmap was called
TEST_F(WireBufferMappingReadTests, UnmapCalledTooEarlyForRead) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    // The callback should get called immediately with UnmappedBeforeCallback status
    // even if the request succeeds on the server side
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
        .Times(1);

    // Oh no! We are calling Unmap too early! The callback should get fired immediately
    // before we get an answer from the server.
    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check that even if Unmap() was called early client-side, we correctly surface server-side
// validation errors.
TEST_F(WireBufferMappingReadTests, UnmapCalledTooEarlyForReadButServerSideError) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    // The callback should get called immediately with UnmappedBeforeCallback status,
    // not server-side error, even if the request fails on the server side
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
        .Times(1);

    // Oh no! We are calling Unmap too early! The callback should get fired immediately
    // before we get an answer from the server that the mapAsync call was an error.
    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check the map read callback is called with "DestroyedBeforeCallback" when the map request
// would have worked, but Destroy was called
TEST_F(WireBufferMappingReadTests, DestroyCalledTooEarlyForRead) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    // The callback should get called immediately with DestroyedBeforeCallback status
    // even if the request succeeds on the server side
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
        .Times(1);

    // Oh no! We are calling Destroy too early! The callback should get fired immediately
    // before we get an answer from the server.
    wgpuBufferDestroy(buffer);
    EXPECT_CALL(api, BufferDestroy(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check that even if Destroy() was called early client-side, we correctly surface server-side
// validation errors.
TEST_F(WireBufferMappingReadTests, DestroyCalledTooEarlyForReadButServerSideError) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    // The callback should be called with the server-side error and not the
    // DestroyedBeforCallback..
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
        .Times(1);

    // Oh no! We are calling Destroy too early! The callback should get fired immediately
    // before we get an answer from the server that the mapAsync call was an error.
    wgpuBufferDestroy(buffer);
    EXPECT_CALL(api, BufferDestroy(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check that an error map read while a buffer is already mapped won't changed the result of get
// mapped range
TEST_F(WireBufferMappingReadTests, MappingForReadingErrorWhileAlreadyMappedUnchangeMapData) {
    // Successful map
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingReadTests, UnmapInsideMapReadCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&] { wgpuBufferUnmap(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the
// callback
TEST_F(WireBufferMappingReadTests, DestroyInsideMapReadCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&] { wgpuBufferRelease(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer));

    FlushClient();
}

// Tests specific to mapping for writing
class WireBufferMappingWriteTests : public WireBufferMappingTests {
  public:
    WireBufferMappingWriteTests() {}
    ~WireBufferMappingWriteTests() override = default;

    void SetUp() override {
        WireBufferMappingTests::SetUp();

        SetupBuffer(WGPUBufferUsage_MapWrite);
    }
};

// Check mapping for writing a succesfully created buffer
TEST_F(WireBufferMappingWriteTests, MappingForWriteSuccessBuffer) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&serverBufferContent));

    FlushClient();

    // The map write callback always gets a buffer full of zeroes.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    uint32_t* lastMapWritePointer =
        static_cast<uint32_t*>(wgpuBufferGetMappedRange(buffer, 0, kBufferSize));
    ASSERT_EQ(0u, *lastMapWritePointer);

    // Write something to the mapped pointer
    *lastMapWritePointer = updatedContent;

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    // After the buffer is unmapped, the content of the buffer is updated on the server
    ASSERT_EQ(serverBufferContent, updatedContent);
}

// Check that things work correctly when a validation error happens when mapping the buffer for
// writing
TEST_F(WireBufferMappingWriteTests, ErrorWhileMappingForWrite) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetMappedRange(buffer, 0, kBufferSize));
}

// Check that the map write callback is called with "DestroyedBeforeCallback" when the buffer is
// destroyed before the request is finished
TEST_F(WireBufferMappingWriteTests, DestroyBeforeWriteRequestEnd) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Return success
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    // Destroy before the client gets the success, so the callback is called with
    // DestroyedBeforeCallback.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
        .Times(1);
    wgpuBufferRelease(buffer);
    EXPECT_CALL(api, BufferRelease(apiBuffer));

    FlushClient();
    FlushServer();
}

// Check the map write callback is called with "UnmappedBeforeCallback" when the map request
// would have worked, but Unmap was called
TEST_F(WireBufferMappingWriteTests, UnmapCalledTooEarlyForWrite) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
        .Times(1);
    wgpuBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map write while a buffer is already mapped
TEST_F(WireBufferMappingWriteTests, MappingForWritingErrorWhileAlreadyMapped) {
    // Successful map
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    EXPECT_NE(nullptr,
              static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingWriteTests, UnmapInsideMapWriteCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&] { wgpuBufferUnmap(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in
// the callback
// TODO(dawn:1621): Suppressed because the mapping handling still touches the buffer after it is
// destroyed triggering an ASAN error.
TEST_F(WireBufferMappingWriteTests, DISABLED_DestroyInsideMapWriteCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&] { wgpuBufferRelease(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer));

    FlushClient();
}

// Test successful buffer creation with mappedAtCreation=true
TEST_F(WireBufferMappingTests, MappedAtCreationSuccess) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that releasing a buffer mapped at creation does not call Unmap
TEST_F(WireBufferMappingTests, MappedAtCreationReleaseBeforeUnmap) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferRelease(buffer);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);

    FlushClient();
}

// Test that it is valid to map a buffer after it is mapped at creation and unmapped
TEST_F(WireBufferMappingTests, MappedAtCreationThenMapSuccess) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.usage = WGPUMapMode_Write;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&apiBufferData));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();
}

// Test that it is invalid to map a buffer after mappedAtCreation but before Unmap
TEST_F(WireBufferMappingTests, MappedAtCreationThenMapFailure) {
    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.mappedAtCreation = true;

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    uint32_t apiBufferData = 1234;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &descriptor);

    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    FlushClient();

    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _)).Times(1);

    FlushServer();

    EXPECT_NE(nullptr,
              static_cast<const uint32_t*>(wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize)));

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Check that trying to create a buffer of size MAX_SIZE_T won't get OOM error at the client side.
TEST_F(WireBufferMappingTests, MaxSizeMappableBufferOOMDirectly) {
    size_t kOOMSize = std::numeric_limits<size_t>::max();
    WGPUBuffer apiBuffer = api.GetNewBuffer();

    // Check for CreateBufferMapped.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_CopySrc;
        descriptor.size = kOOMSize;
        descriptor.mappedAtCreation = true;

        wgpuDeviceCreateBuffer(device, &descriptor);
        FlushClient();
    }

    // Check for MapRead usage.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_MapRead;
        descriptor.size = kOOMSize;

        wgpuDeviceCreateBuffer(device, &descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        FlushClient();
    }

    // Check for MapWrite usage.
    {
        WGPUBufferDescriptor descriptor = {};
        descriptor.usage = WGPUBufferUsage_MapWrite;
        descriptor.size = kOOMSize;

        wgpuDeviceCreateBuffer(device, &descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        FlushClient();
    }
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireBufferMappingTests, MapThenDisconnect) {
    SetupBuffer(WGPUMapMode_Write);
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, this);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize)).Times(1);

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DeviceLost, this)).Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireBufferMappingTests, MapAfterDisconnect) {
    SetupBuffer(WGPUMapMode_Read);

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DeviceLost, this)).Times(1);
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, this);
}

// Test that mapping again while pending map immediately cause an error
TEST_F(WireBufferMappingTests, PendingMapImmediateError) {
    SetupBuffer(WGPUMapMode_Read);

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, nullptr, this);

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this))
        .Times(1);
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, this);
}

// Test that GetMapState() returns map state as expected
TEST_F(WireBufferMappingTests, GetMapState) {
    SetupBuffer(WGPUMapMode_Read);

    // Server-side success case
    {
        uint32_t bufferContent = 31337;
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
            }));
        EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
            .WillOnce(Return(&bufferContent));
        EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
        wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback,
                           nullptr);

        // map state should become pending immediately after map async call
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushClient();

        // map state should be pending until receiving a response from server
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushServer();

        // mapping succeeded
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Mapped);
    }

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    FlushClient();

    // Server-side error case
    {
        EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_ValidationError);
            }));
        EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _))
            .Times(1);

        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
        wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback,
                           nullptr);

        // map state should become pending immediately after map async call
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushClient();

        // map state should be pending until receiving a response from server
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Pending);
        FlushServer();

        // mapping failed
        ASSERT_EQ(wgpuBufferGetMapState(buffer), WGPUBufferMapState_Unmapped);
    }
}

#if defined(DAWN_ENABLE_ASSERTS)
static void ToMockBufferMapCallbackWithAssertErrorRequest(WGPUBufferMapAsyncStatus status,
                                                          void* userdata) {
    WGPUBuffer* buffer = reinterpret_cast<WGPUBuffer*>(userdata);

    mockBufferMapCallback->Call(status, buffer);
    ASSERT_DEATH_IF_SUPPORTED(
        {
            // This map async should cause assertion error because of
            // refcount == 0.
            wgpuBufferMapAsync(*buffer, WGPUMapMode_Read, 0, sizeof(uint32_t),
                               ToMockBufferMapCallback, nullptr);
        },
        "");
}

// Test that request inside user callbacks after object destruction is called
TEST_F(WireBufferMappingTests, MapInsideCallbackAfterDestruction) {
    SetupBuffer(WGPUMapMode_Read);

    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize,
                       ToMockBufferMapCallbackWithAssertErrorRequest, &buffer);

    // By releasing the buffer the refcount reaches zero and pending map async
    // should fail with destroyed before callback status.
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
        .Times(1);
    wgpuBufferRelease(buffer);
}
#endif  // defined(DAWN_ENABLE_ASSERTS)

// Hack to pass in test context into user callback
struct TestData {
    WireBufferMappingTests* pTest;
    WGPUBuffer* pTestBuffer;
    size_t numRequests;
};

static void ToMockBufferMapCallbackWithNewRequests(WGPUBufferMapAsyncStatus status,
                                                   void* userdata) {
    TestData* testData = reinterpret_cast<TestData*>(userdata);
    // Mimic the user callback is sending new requests
    ASSERT_NE(testData, nullptr);
    ASSERT_NE(testData->pTest, nullptr);
    ASSERT_NE(testData->pTestBuffer, nullptr);

    mockBufferMapCallback->Call(status, testData->pTest);

    // Send the requests a number of times
    for (size_t i = 0; i < testData->numRequests; i++) {
        wgpuBufferMapAsync(*(testData->pTestBuffer), WGPUMapMode_Write, 0, sizeof(uint32_t),
                           ToMockBufferMapCallback, testData->pTest);
    }
}

// Test that requests inside user callbacks before disconnect are called
TEST_F(WireBufferMappingTests, MapInsideCallbackBeforeDisconnect) {
    SetupBuffer(WGPUMapMode_Write);
    TestData testData = {this, &buffer, 10};
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize,
                       ToMockBufferMapCallbackWithNewRequests, &testData);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize)).Times(1);

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_DeviceLost, this))
        .Times(testData.numRequests + 1);
    GetWireClient()->Disconnect();
}

// Test that requests inside user callbacks before object destruction are called
TEST_F(WireBufferMappingWriteTests, MapInsideCallbackBeforeDestruction) {
    SetupBuffer(WGPUMapMode_Write);
    TestData testData = {this, &buffer, 10};
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize,
                       ToMockBufferMapCallbackWithNewRequests, &testData);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _, _))
        .WillOnce(InvokeWithoutArgs(
            [&] { api.CallBufferMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success); }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize)).Times(1);

    FlushClient();

    // The first map async call should succeed
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, this)).Times(1);

    // The second or later map async calls in the map async callback
    // should immediately fail because of pending map
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this))
        .Times(testData.numRequests - 1);

    // The first map async call in the map async callback should fail
    // with destroyed before callback status due to buffer release below
    EXPECT_CALL(*mockBufferMapCallback,
                Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, this))
        .Times(1);

    FlushServer();

    wgpuBufferRelease(buffer);
}

}  // anonymous namespace
}  // namespace dawn::wire
