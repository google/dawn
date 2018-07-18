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

using namespace testing;

class MockBufferMapReadCallback {
    public:
        MOCK_METHOD3(Call, void(dawnBufferMapAsyncStatus status, const uint32_t* ptr, dawnCallbackUserdata userdata));
};

static MockBufferMapReadCallback* mockBufferMapReadCallback = nullptr;
static void ToMockBufferMapReadCallback(dawnBufferMapAsyncStatus status, const void* ptr, dawnCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, reinterpret_cast<const uint32_t*>(ptr), userdata);
}

class MockBufferMapWriteCallback {
    public:
        MOCK_METHOD3(Call, void(dawnBufferMapAsyncStatus status, uint32_t* ptr, dawnCallbackUserdata userdata));
};

static MockBufferMapWriteCallback* mockBufferMapWriteCallback = nullptr;
static void ToMockBufferMapWriteCallback(dawnBufferMapAsyncStatus status, void* ptr, dawnCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapWriteCallback->Call(status, reinterpret_cast<uint32_t*>(ptr), userdata);
}

class BufferValidationTest : public ValidationTest {
    protected:
        dawn::Buffer CreateMapReadBuffer(uint32_t size) {
            return device.CreateBufferBuilder()
                .SetSize(size)
                .SetAllowedUsage(dawn::BufferUsageBit::MapRead)
                .GetResult();
        }
        dawn::Buffer CreateMapWriteBuffer(uint32_t size) {
            return device.CreateBufferBuilder()
                .SetSize(size)
                .SetAllowedUsage(dawn::BufferUsageBit::MapWrite)
                .GetResult();
        }
        dawn::Buffer CreateSetSubDataBuffer(uint32_t size) {
            return device.CreateBufferBuilder()
                .SetSize(size)
                .SetAllowedUsage(dawn::BufferUsageBit::TransferDst)
                .GetResult();
        }

        dawn::Queue queue;

    private:
        void SetUp() override {
            ValidationTest::SetUp();

            mockBufferMapReadCallback = new MockBufferMapReadCallback;
            mockBufferMapWriteCallback = new MockBufferMapWriteCallback;
            queue = device.CreateQueue();
        }

        void TearDown() override {
            delete mockBufferMapReadCallback;
            delete mockBufferMapWriteCallback;

            ValidationTest::TearDown();
        }
};

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationSuccess) {
    // Success
    {
        dawn::Buffer buf = AssertWillBeSuccess(device.CreateBufferBuilder())
            .SetSize(4)
            .SetAllowedUsage(dawn::BufferUsageBit::Uniform)
            .GetResult();
    }
}

// Test failure when specifying properties multiple times
TEST_F(BufferValidationTest, CreationDuplicates) {
    // When size is specified multiple times
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder())
            .SetSize(4)
            .SetSize(3)
            .SetAllowedUsage(dawn::BufferUsageBit::Uniform)
            .GetResult();
    }

    // When allowed usage is specified multiple times
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder())
            .SetSize(4)
            .SetAllowedUsage(dawn::BufferUsageBit::Vertex)
            .SetAllowedUsage(dawn::BufferUsageBit::Uniform)
            .GetResult();
    }
}

// Test failure when required properties are missing
TEST_F(BufferValidationTest, CreationMissingProperties) {
    // When allowed usage is missing
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder())
            .SetSize(4)
            .GetResult();
    }

    // When size is missing
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder())
            .SetAllowedUsage(dawn::BufferUsageBit::Vertex)
            .GetResult();
    }
}

// Test restriction on usages allowed with MapRead and MapWrite
TEST_F(BufferValidationTest, CreationMapUsageRestrictions) {
    // MapRead with TransferDst is ok
    {
        dawn::Buffer buf = AssertWillBeSuccess(device.CreateBufferBuilder(), "1")
            .SetAllowedUsage(dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst)
            .SetSize(4)
            .GetResult();
    }

    // MapRead with something else is an error
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder(), "2")
            .SetAllowedUsage(dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::Uniform)
            .SetSize(4)
            .GetResult();
    }

    // MapWrite with TransferSrc is ok
    {
        dawn::Buffer buf = AssertWillBeSuccess(device.CreateBufferBuilder(), "3")
            .SetAllowedUsage(dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc)
            .SetSize(4)
            .GetResult();
    }

    // MapWrite with something else is an error
    {
        dawn::Buffer buf = AssertWillBeError(device.CreateBufferBuilder(), "4")
            .SetAllowedUsage(dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::Uniform)
            .SetSize(4)
            .GetResult();
    }
}

// Test the success case for mapping buffer for reading
TEST_F(BufferValidationTest, MapReadSuccess) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40598;
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test the success case for mapping buffer for writing
TEST_F(BufferValidationTest, MapWriteSuccess) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40598;
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test map reading out of range causes an error
TEST_F(BufferValidationTest, MapReadOutOfRange) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40599;
    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapReadAsync(0, 5, ToMockBufferMapReadCallback, userdata));
}

// Test map writing out of range causes an error
TEST_F(BufferValidationTest, MapWriteOutOfRange) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40599;
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(0, 5, ToMockBufferMapWriteCallback, userdata));
}

// Test map reading a buffer with wrong current usage
TEST_F(BufferValidationTest, MapReadWrongUsage) {
    dawn::Buffer buf = device.CreateBufferBuilder()
        .SetSize(4)
        .SetAllowedUsage(dawn::BufferUsageBit::TransferDst)
        .GetResult();

    dawn::CallbackUserdata userdata = 40600;
    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata));
}

// Test map writing a buffer with wrong current usage
TEST_F(BufferValidationTest, MapWriteWrongUsage) {
    dawn::Buffer buf = device.CreateBufferBuilder()
        .SetSize(4)
        .SetAllowedUsage(dawn::BufferUsageBit::TransferSrc)
        .GetResult();

    dawn::CallbackUserdata userdata = 40600;
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata));
}

// Test map reading a buffer that is already mapped
TEST_F(BufferValidationTest, MapReadAlreadyMapped) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata1 = 40601;
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata1);
    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata1))
        .Times(1);

    dawn::CallbackUserdata userdata2 = 40602;
    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata2))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata2));

    queue.Submit(0, nullptr);
}

// Test map writing a buffer that is already mapped
TEST_F(BufferValidationTest, MapWriteAlreadyMapped) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata1 = 40601;
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata1);
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata1))
        .Times(1);

    dawn::CallbackUserdata userdata2 = 40602;
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata2))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata2));

    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// Test unmapping before having the result gives UNKNOWN - for reading
TEST_F(BufferValidationTest, MapReadUnmapBeforeResult) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40603;
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
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
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
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
        buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

        EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
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
        buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

        EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
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
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    buf.Unmap();

    userdata ++;

    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .Times(1);
    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// When a MapWrite is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapWriteUnmapBeforeResultThenMapAgain) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40605;
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    buf.Unmap();

    userdata ++;

    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .Times(1);
    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapReadCallback) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40678;
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            buf.Unmap();
        }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapWriteCallback) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40678;
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            buf.Unmap();
        }));

    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapReadCallback) {
    dawn::Buffer buf = CreateMapReadBuffer(4);

    dawn::CallbackUserdata userdata = 40679;
    buf.MapReadAsync(0, 4, ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            buf = dawn::Buffer();
        }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapWriteCallback) {
    dawn::Buffer buf = CreateMapWriteBuffer(4);

    dawn::CallbackUserdata userdata = 40679;
    buf.MapWriteAsync(0, 4, ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Ne(nullptr), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            buf = dawn::Buffer();
        }));

    queue.Submit(0, nullptr);
}

// Test the success case for Buffer::SetSubData
TEST_F(BufferValidationTest, SetSubDataSuccess) {
    dawn::Buffer buf = CreateSetSubDataBuffer(1);

    uint8_t foo = 0;
    buf.SetSubData(0, sizeof(foo), &foo);
}

// Test error case for SetSubData out of bounds
TEST_F(BufferValidationTest, SetSubDataOutOfBounds) {
    dawn::Buffer buf = CreateSetSubDataBuffer(1);

    uint8_t foo = 0;
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, 2, &foo));
}

// Test error case for SetSubData with the wrong usage
TEST_F(BufferValidationTest, SetSubDataWrongUsage) {
    dawn::Buffer buf = device.CreateBufferBuilder()
        .SetSize(1)
        .SetAllowedUsage(dawn::BufferUsageBit::Vertex)
        .GetResult();

    uint8_t foo = 0;
    ASSERT_DEVICE_ERROR(buf.SetSubData(0, sizeof(foo), &foo));
}
