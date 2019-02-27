// Copyright 2017 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include <gmock/gmock.h>

#include <memory>

using namespace testing;

class MockBufferMapReadCallback {
    public:
      MOCK_METHOD4(Call,
                   void(dawnBufferMapAsyncStatus status,
                        const uint32_t* ptr,
                        uint32_t dataLength,
                        dawnCallbackUserdata userdata));
};

static std::unique_ptr<MockBufferMapReadCallback> mockBufferMapReadCallback;
static void ToMockBufferMapReadCallback(dawnBufferMapAsyncStatus status,
                                        const void* ptr,
                                        uint32_t dataLength,
                                        dawnCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, reinterpret_cast<const uint32_t*>(ptr), dataLength,
                                    userdata);
}

class MockBufferMapWriteCallback {
    public:
      MOCK_METHOD4(Call,
                   void(dawnBufferMapAsyncStatus status,
                        uint32_t* ptr,
                        uint32_t dataLength,
                        dawnCallbackUserdata userdata));
};

static std::unique_ptr<MockBufferMapWriteCallback> mockBufferMapWriteCallback;
static void ToMockBufferMapWriteCallback(dawnBufferMapAsyncStatus status,
                                         void* ptr,
                                         uint32_t dataLength,
                                         dawnCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapWriteCallback->Call(status, reinterpret_cast<uint32_t*>(ptr), dataLength,
                                     userdata);
}

class BufferValidationTest : public ValidationTest {
    protected:
        dawn::Buffer CreateMapReadBuffer(uint32_t size) {
            dawn::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = dawn::BufferUsageBit::MapRead;

            return device.CreateBuffer(&descriptor);
        }
        dawn::Buffer CreateMapWriteBuffer(uint32_t size) {
            dawn::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = dawn::BufferUsageBit::MapWrite;

            return device.CreateBuffer(&descriptor);
        }
        dawn::Buffer CreateSetSubDataBuffer(uint32_t size) {
            dawn::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = dawn::BufferUsageBit::TransferDst;

            return device.CreateBuffer(&descriptor);
        }

        dawn::Queue queue;

    private:
        void SetUp() override {
            ValidationTest::SetUp();

            mockBufferMapReadCallback = std::make_unique<MockBufferMapReadCallback>();
            mockBufferMapWriteCallback = std::make_unique<MockBufferMapWriteCallback>();
            queue = device.CreateQueue();
        }

        void TearDown() override {
            // Delete mocks so that expectations are checked
            mockBufferMapReadCallback = nullptr;
            mockBufferMapWriteCallback = nullptr;

            ValidationTest::TearDown();
        }
};

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationSuccess) {
    // Success
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = dawn::BufferUsageBit::Uniform;

        device.CreateBuffer(&descriptor);
    }
}

// Test restriction on usages allowed with MapRead and MapWrite
TEST_F(BufferValidationTest, CreationMapUsageRestrictions) {
    // MapRead with TransferDst is ok
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;

        device.CreateBuffer(&descriptor);
    }

    // MapRead with something else is an error
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }

    // MapWrite with TransferSrc is ok
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc;

        device.CreateBuffer(&descriptor);
    }

    // MapWrite with something else is an error
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test the success case for mapping buffer for reading
TEST_F(BufferValidationTest, MapReadSuccess) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40598;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test the success case for mapping buffer for writing
TEST_F(BufferValidationTest, MapWriteSuccess) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40598;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test map reading a buffer with wrong current usage
TEST_F(BufferValidationTest, MapReadWrongUsage) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsageBit::TransferDst;

    dawn::Buffer buf = device.CreateBuffer(&descriptor);

    dawn::CallbackUserdata userdata = 40600;
    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, 0u, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, userdata));
}

// Test map writing a buffer with wrong current usage
TEST_F(BufferValidationTest, MapWriteWrongUsage) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc;

    dawn::Buffer buf = device.CreateBuffer(&descriptor);

    dawn::CallbackUserdata userdata = 40600;
    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, 0u, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata));
}

// Test map reading a buffer that is already mapped
TEST_F(BufferValidationTest, MapReadAlreadyMapped) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata1 = 40601;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata1);
    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata1))
        .Times(1);

    dawn::CallbackUserdata userdata2 = 40602;
    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, 0u, userdata2))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, userdata2));

    queue.Submit(0, nullptr);
}

// Test map writing a buffer that is already mapped
TEST_F(BufferValidationTest, MapWriteAlreadyMapped) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata1 = 40601;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata1);
    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata1))
        .Times(1);

    dawn::CallbackUserdata userdata2 = 40602;
    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, 0u, userdata2))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata2));

    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// Test unmapping before having the result gives UNKNOWN - for reading
TEST_F(BufferValidationTest, MapReadUnmapBeforeResult) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40603;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
        .Times(1);
    buf.Unmap();

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// Test unmapping before having the result gives UNKNOWN - for writing
TEST_F(BufferValidationTest, MapWriteUnmapBeforeResult) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40603;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
        .Times(1);
    buf.Unmap();

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// Test destroying the buffer before having the result gives UNKNOWN - for reading
// TODO(cwallez@chromium.org) currently this doesn't work because the buffer doesn't know
// when its external ref count reaches 0.
TEST_F(BufferValidationTest, DISABLED_MapReadDestroyBeforeResult) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);

        dawn::CallbackUserdata userdata = 40604;
        buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

        EXPECT_CALL(*mockBufferMapReadCallback,
                    Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
            .Times(1);
    }

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// Test destroying the buffer before having the result gives UNKNOWN - for writing
// TODO(cwallez@chromium.org) currently this doesn't work because the buffer doesn't know
// when its external ref count reaches 0.
TEST_F(BufferValidationTest, DISABLED_MapWriteDestroyBeforeResult) {
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);

        dawn::CallbackUserdata userdata = 40604;
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

        EXPECT_CALL(*mockBufferMapWriteCallback,
                    Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
            .Times(1);
    }

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// When a MapRead is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapReadUnmapBeforeResultThenMapAgain) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40605;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
        .Times(1);
    buf.Unmap();

    userdata ++;

    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .Times(1);
    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// When a MapWrite is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapWriteUnmapBeforeResultThenMapAgain) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40605;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0u, userdata))
        .Times(1);
    buf.Unmap();

    userdata ++;

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .Times(1);
    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapReadCallback) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40678;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapWriteCallback) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40678;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapReadCallback) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40679;
    buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .WillOnce(InvokeWithoutArgs([&]() { buf = dawn::Buffer(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapWriteCallback) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40679;
    buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), 4u, userdata))
        .WillOnce(InvokeWithoutArgs([&]() { buf = dawn::Buffer(); }));

    queue.Submit(0, nullptr);
}

// Test the success case for Buffer::SetSubData
TEST_F(BufferValidationTest, SetSubDataSuccess) {
    dawn::Buffer buf = CreateSetSubDataBuffer(4);

    uint32_t foo = 0x01020304;
    buf.SetSubData(0, sizeof(foo), reinterpret_cast<uint8_t*>(&foo));
}

// Test error case for SetSubData out of bounds
TEST_F(BufferValidationTest, SetSubDataOutOfBounds) {
    dawn::Buffer buf = CreateSetSubDataBuffer(1);

    uint8_t foo[2] = {0, 0};
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, 2, foo));
}

// Test error case for SetSubData out of bounds with an overflow
TEST_F(BufferValidationTest, SetSubDataOutOfBoundsOverflow) {
    dawn::Buffer buf = CreateSetSubDataBuffer(1000);

    uint8_t foo[2] = {0, 0};

    // An offset that when added to "2" would overflow to be zero and pass validation without
    // overflow checks.
    uint32_t offset = uint32_t(int32_t(0) - int32_t(2));

    ASSERT_DEVICE_ERROR(buf.SetSubData(offset, 2, foo));
}

// Test error case for SetSubData with the wrong usage
TEST_F(BufferValidationTest, SetSubDataWrongUsage) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsageBit::Vertex;

    dawn::Buffer buf = device.CreateBuffer(&descriptor);

    uint8_t foo = 0;
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(foo), &foo));
}

// Test SetSubData with unaligned size
TEST_F(BufferValidationTest, SetSubDataWithUnalignedSize) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;

    dawn::Buffer buf = device.CreateBuffer(&descriptor);

    uint8_t value = 123;
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(value), &value));
}

// Test SetSubData with unaligned offset
TEST_F(BufferValidationTest, SetSubDataWithUnalignedOffset) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4000;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;

    dawn::Buffer buf = device.CreateBuffer(&descriptor);

    uint32_t kOffset = 2999;
    uint32_t value = 0x01020304;
    ASSERT_DEVICE_ERROR(buf.SetSubData(kOffset, sizeof(value), reinterpret_cast<uint8_t*>(&value)));
}

// Test that it is valid to destroy an unmapped buffer
TEST_F(BufferValidationTest, DestroyUnmappedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
    }
}

// Test that it is valid to destroy a mapped buffer
TEST_F(BufferValidationTest, DestroyMappedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, 30303);
        buf.Destroy();
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, 30233);
        buf.Destroy();
    }
}

// Test that destroying a buffer implicitly unmaps it
TEST_F(BufferValidationTest, DestroyMappedBufferCausesImplicitUnmap) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        dawn::CallbackUserdata userdata = 40598;
        buf.MapReadAsync(ToMockBufferMapReadCallback, userdata);
        // Buffer is destroyed. Callback should be called with UNKNOWN status
        EXPECT_CALL(*mockBufferMapReadCallback,
                    Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0, userdata))
            .Times(1);
        buf.Destroy();
        queue.Submit(0, nullptr);
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        dawn::CallbackUserdata userdata = 23980;
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, userdata);
        // Buffer is destroyed. Callback should be called with UNKNOWN status
        EXPECT_CALL(*mockBufferMapWriteCallback,
                    Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, 0, userdata))
            .Times(1);
        buf.Destroy();
        queue.Submit(0, nullptr);
    }
}

// Test that it is valid to Destroy a destroyed buffer
TEST_F(BufferValidationTest, DestroyDestroyedBuffer) {
    dawn::Buffer buf = CreateSetSubDataBuffer(4);
    buf.Destroy();
    buf.Destroy();
}

// Test that it is invalid to Unmap a destroyed buffer
TEST_F(BufferValidationTest, UnmapDestroyedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
}

// Test that it is invalid to map a destroyed buffer
TEST_F(BufferValidationTest, MapDestroyedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, 11303));
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, 56303));
    }
}

// Test that it is invalid to call SetSubData on a destroyed buffer
TEST_F(BufferValidationTest, SetSubDataDestroyedBuffer) {
    dawn::Buffer buf = CreateSetSubDataBuffer(4);
    buf.Destroy();
    uint8_t foo = 0;
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(foo), &foo));
}

// Test that is is invalid to Map a mapped buffer
TEST_F(BufferValidationTest, MapMappedbuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, 43309);
        ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, 34309));
        queue.Submit(0, nullptr);
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, 20301);
        ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, 40303));
        queue.Submit(0, nullptr);
    }
}

// Test that it is invalid to call SetSubData on a mapped buffer
TEST_F(BufferValidationTest, SetSubDataMappedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, 42899);
        uint8_t foo = 0;
        ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(foo), &foo));
        queue.Submit(0, nullptr);
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, 40329);
        uint8_t foo = 0;
        ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(foo), &foo));
        queue.Submit(0, nullptr);
    }
}

// Test that it is valid to submit a buffer in a queue with a map usage if it is unmapped
TEST_F(BufferValidationTest, SubmitBufferWithMapUsage) {
    dawn::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::MapWrite;

    dawn::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::MapRead;

    dawn::Buffer bufA = device.CreateBuffer(&descriptorA);
    dawn::Buffer bufB = device.CreateBuffer(&descriptorB);

    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that it is invalid to submit a mapped buffer in a queue
TEST_F(BufferValidationTest, SubmitMappedBuffer) {
    dawn::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::MapWrite;

    dawn::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::MapRead;
    {
        dawn::Buffer bufA = device.CreateBuffer(&descriptorA);
        dawn::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufA.MapWriteAsync(ToMockBufferMapWriteCallback, 40329);

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        dawn::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
    {
        dawn::Buffer bufA = device.CreateBuffer(&descriptorA);
        dawn::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufB.MapReadAsync(ToMockBufferMapReadCallback, 11329);

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        dawn::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
}

// Test that it is invalid to submit a destroyed buffer in a queue
TEST_F(BufferValidationTest, SubmitDestroyedBuffer) {
    dawn::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = dawn::BufferUsageBit::TransferSrc;

    dawn::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = dawn::BufferUsageBit::TransferDst;

    dawn::Buffer bufA = device.CreateBuffer(&descriptorA);
    dawn::Buffer bufB = device.CreateBuffer(&descriptorB);

    bufA.Destroy();
    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    dawn::CommandBuffer commands = encoder.Finish();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that a map usage is required to call Unmap
TEST_F(BufferValidationTest, UnmapWithoutMapUsage) {
    dawn::Buffer buf = CreateSetSubDataBuffer(4);
    ASSERT_DEVICE_ERROR(buf.Unmap());
}

// Test that it is valid to call Unmap on a buffer that is not mapped
TEST_F(BufferValidationTest, UnmapUnmappedBuffer) {
    {
        dawn::Buffer buf = CreateMapReadBuffer(4);
        // Buffer starts unmapped. Unmap should succeed.
        buf.Unmap();
        buf.MapReadAsync(ToMockBufferMapReadCallback, 30603);
        buf.Unmap();
        // Unmapping twice should succeed
        buf.Unmap();
    }
    {
        dawn::Buffer buf = CreateMapWriteBuffer(4);
        // Buffer starts unmapped. Unmap should succeed.
        buf.Unmap();
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, 23890);
        // Unmapping twice should succeed
        buf.Unmap();
        buf.Unmap();
    }
}
