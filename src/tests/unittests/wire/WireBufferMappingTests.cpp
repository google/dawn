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

#include "tests/unittests/wire/WireTest.h"

using namespace testing;
using namespace dawn_wire;

namespace {

    // Mock class to add expectations on the wire calling callbacks
    class MockBufferMapCallback {
      public:
        MOCK_METHOD(void,
                    Call,
                    (WGPUBufferMapAsyncStatus status,
                     void* userdata));
    };

    std::unique_ptr<StrictMock<MockBufferMapCallback>> mockBufferMapCallback;
    void ToMockBufferMapCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
        mockBufferMapCallback->Call(status, userdata);
    }

}  // anonymous namespace

class WireBufferMappingTests : public WireTest {
  public:
    WireBufferMappingTests() {
    }
    ~WireBufferMappingTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockBufferMapCallback = std::make_unique<StrictMock<MockBufferMapCallback>>();

        WGPUBufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;

        apiBuffer = api.GetNewBuffer();
        buffer = wgpuDeviceCreateBuffer(device, &descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
            .WillOnce(Return(apiBuffer))
            .RetiresOnSaturation();
        FlushClient();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockBufferMapCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockBufferMapCallback);
    }

  protected:
    static constexpr uint64_t kBufferSize = sizeof(uint32_t);
    // A successfully created buffer
    WGPUBuffer buffer;
    WGPUBuffer apiBuffer;
};

// Tests specific to mapping for reading

// Check mapping for reading a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForReadSuccessBuffer) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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
TEST_F(WireBufferMappingTests, ErrorWhileMappingForRead) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Error);
    }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize));
}

// Check that the map read callback is called with UNKNOWN when the buffer is destroyed before the
// request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeReadRequestEnd) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Return success
    uint32_t bufferContent = 0;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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

// Check the map read callback is called with "UnmappedBeforeCallback" when the map request would
// have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForRead) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
        .Times(1);
    wgpuBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForReadingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Error);
    }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize));
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapReadCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&]() { wgpuBufferUnmap(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the
// callback
TEST_F(WireBufferMappingTests, DestroyInsideMapReadCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Read, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&]() { wgpuBufferRelease(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer));

    FlushClient();
}

// Tests specific to mapping for writing

// Check mapping for writing a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForWriteSuccessBuffer) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;

    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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
TEST_F(WireBufferMappingTests, ErrorWhileMappingForWrite) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Error);
    }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetMappedRange(buffer, 0, kBufferSize));
}

// Check that the map write callback is called with "DestroyedBeforeCallback" when the buffer is
// destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeWriteRequestEnd) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    // Return success
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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

// Check the map read callback is called with "UnmappedBeforeCallback" when the map request would
// have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForWrite) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForWritingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Error);
    }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetMappedRange(buffer, 0, kBufferSize));
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapWriteCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&]() { wgpuBufferUnmap(buffer); }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the
// callback
TEST_F(WireBufferMappingTests, DestroyInsideMapWriteCallback) {
    wgpuBufferMapAsync(buffer, WGPUMapMode_Write, 0, kBufferSize, ToMockBufferMapCallback, nullptr);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
        .WillOnce(InvokeWithoutArgs([&]() { wgpuBufferRelease(buffer); }));

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

    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Success);
    }));
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

    EXPECT_CALL(api, OnBufferMapAsyncCallback(apiBuffer, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallMapAsyncCallback(apiBuffer, WGPUBufferMapAsyncStatus_Error);
    }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

    FlushServer();

    EXPECT_EQ(nullptr, wgpuBufferGetConstMappedRange(buffer, 0, kBufferSize));

    wgpuBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);

    FlushClient();
}
