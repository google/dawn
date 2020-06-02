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
      MOCK_METHOD(void,
                  Call,
                  (WGPUBufferMapAsyncStatus status,
                   const uint32_t* ptr,
                   uint64_t dataLength,
                   void* userdata));
};

static std::unique_ptr<MockBufferMapReadCallback> mockBufferMapReadCallback;
static void ToMockBufferMapReadCallback(WGPUBufferMapAsyncStatus status,
                                        const void* ptr,
                                        uint64_t dataLength,
                                        void* userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, reinterpret_cast<const uint32_t*>(ptr), dataLength,
                                    userdata);
}

class MockBufferMapWriteCallback {
    public:
      MOCK_METHOD(
          void,
          Call,
          (WGPUBufferMapAsyncStatus status, uint32_t* ptr, uint64_t dataLength, void* userdata));
};

static std::unique_ptr<MockBufferMapWriteCallback> mockBufferMapWriteCallback;
static void ToMockBufferMapWriteCallback(WGPUBufferMapAsyncStatus status,
                                         void* ptr,
                                         uint64_t dataLength,
                                         void* userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapWriteCallback->Call(status, reinterpret_cast<uint32_t*>(ptr), dataLength,
                                     userdata);
}

class BufferValidationTest : public ValidationTest {
    protected:
      wgpu::Buffer CreateMapReadBuffer(uint64_t size) {
          wgpu::BufferDescriptor descriptor;
          descriptor.size = size;
          descriptor.usage = wgpu::BufferUsage::MapRead;

          return device.CreateBuffer(&descriptor);
      }
      wgpu::Buffer CreateMapWriteBuffer(uint64_t size) {
          wgpu::BufferDescriptor descriptor;
          descriptor.size = size;
          descriptor.usage = wgpu::BufferUsage::MapWrite;

          return device.CreateBuffer(&descriptor);
      }

      wgpu::CreateBufferMappedResult CreateBufferMapped(uint64_t size, wgpu::BufferUsage usage) {
          wgpu::BufferDescriptor descriptor;
          descriptor.size = size;
          descriptor.usage = usage;

          return device.CreateBufferMapped(&descriptor);
      }

      wgpu::Queue queue;

    private:
        void SetUp() override {
            ValidationTest::SetUp();

            mockBufferMapReadCallback = std::make_unique<MockBufferMapReadCallback>();
            mockBufferMapWriteCallback = std::make_unique<MockBufferMapWriteCallback>();
            queue = device.GetDefaultQueue();
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
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        device.CreateBuffer(&descriptor);
    }
}

// Test restriction on usages allowed with MapRead and MapWrite
TEST_F(BufferValidationTest, CreationMapUsageRestrictions) {
    // MapRead with CopyDst is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

        device.CreateBuffer(&descriptor);
    }

    // MapRead with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }

    // MapWrite with CopySrc is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

        device.CreateBuffer(&descriptor);
    }

    // MapWrite with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test the success case for mapping buffer for reading
TEST_F(BufferValidationTest, MapReadSuccess) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test the success case for mapping buffer for writing
TEST_F(BufferValidationTest, MapWriteSuccess) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .Times(1);
    queue.Submit(0, nullptr);

    buf.Unmap();
}

// Test the success case for CreateBufferMapped
TEST_F(BufferValidationTest, CreateBufferMappedSuccess) {
    wgpu::CreateBufferMappedResult result = CreateBufferMapped(4, wgpu::BufferUsage::MapWrite);
    ASSERT_NE(result.data, nullptr);
    ASSERT_EQ(result.dataLength, 4u);
    result.buffer.Unmap();
}

// Test the success case for non-mappable CreateBufferMapped
TEST_F(BufferValidationTest, NonMappableCreateBufferMappedSuccess) {
    wgpu::CreateBufferMappedResult result = CreateBufferMapped(4, wgpu::BufferUsage::CopySrc);
    ASSERT_NE(result.data, nullptr);
    ASSERT_EQ(result.dataLength, 4u);
    result.buffer.Unmap();
}

// Test map reading a buffer with wrong current usage
TEST_F(BufferValidationTest, MapReadWrongUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(WGPUBufferMapAsyncStatus_Error, nullptr, 0u, _))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr));
}

// Test map writing a buffer with wrong current usage
TEST_F(BufferValidationTest, MapWriteWrongUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc;

    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(WGPUBufferMapAsyncStatus_Error, nullptr, 0u, _))
        .Times(1);

    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr));
}

// Test map reading a buffer that is already mapped
TEST_F(BufferValidationTest, MapReadAlreadyMapped) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, this + 0);
    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, this + 0))
        .Times(1);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Error, nullptr, 0u, this + 1))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, this + 1));

    queue.Submit(0, nullptr);
}

// Test map writing a buffer that is already mapped
TEST_F(BufferValidationTest, MapWriteAlreadyMapped) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, this + 0);
    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, this + 0))
        .Times(1);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Error, nullptr, 0u, this + 1))
        .Times(1);
    ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, this + 1));

    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// Test unmapping before having the result gives UNKNOWN - for reading
TEST_F(BufferValidationTest, MapReadUnmapBeforeResult) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, _))
        .Times(1);
    buf.Unmap();

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// Test unmapping before having the result gives UNKNOWN - for writing
TEST_F(BufferValidationTest, MapWriteUnmapBeforeResult) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, _))
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
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

        EXPECT_CALL(*mockBufferMapReadCallback,
                    Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, _))
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
        wgpu::Buffer buf = CreateMapWriteBuffer(4);

        buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

        EXPECT_CALL(*mockBufferMapWriteCallback,
                    Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, _))
            .Times(1);
    }

    // Submitting the queue makes the null backend process map request, but the callback shouldn't
    // be called again
    queue.Submit(0, nullptr);
}

// When a MapRead is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapReadUnmapBeforeResultThenMapAgain) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, this + 0);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, this + 0))
        .Times(1);
    buf.Unmap();

    buf.MapReadAsync(ToMockBufferMapReadCallback, this + 1);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, this + 1))
        .Times(1);
    queue.Submit(0, nullptr);
}
// TODO(cwallez@chromium.org) Test a MapWrite and already MapRead and vice-versa

// When a MapWrite is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapWriteUnmapBeforeResultThenMapAgain) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, this + 0);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0u, this + 0))
        .Times(1);
    buf.Unmap();

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, this + 1);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, this + 1))
        .Times(1);
    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapReadCallback) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, UnmapInsideMapWriteCallback) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapReadCallback) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

    EXPECT_CALL(*mockBufferMapReadCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .WillOnce(InvokeWithoutArgs([&]() { buf = wgpu::Buffer(); }));

    queue.Submit(0, nullptr);
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(BufferValidationTest, DestroyInsideMapWriteCallback) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

    EXPECT_CALL(*mockBufferMapWriteCallback,
                Call(WGPUBufferMapAsyncStatus_Success, Ne(nullptr), 4u, _))
        .WillOnce(InvokeWithoutArgs([&]() { buf = wgpu::Buffer(); }));

    queue.Submit(0, nullptr);
}

// Test that it is valid to destroy an unmapped buffer
TEST_F(BufferValidationTest, DestroyUnmappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
    }
}

// Test that it is valid to destroy a mapped buffer
TEST_F(BufferValidationTest, DestroyMappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);
        buf.Destroy();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);
        buf.Destroy();
    }
}

// Test that destroying a buffer implicitly unmaps it
TEST_F(BufferValidationTest, DestroyMappedBufferCausesImplicitUnmap) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, this + 0);
        // Buffer is destroyed. Callback should be called with UNKNOWN status
        EXPECT_CALL(*mockBufferMapReadCallback,
                    Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0, this + 0))
            .Times(1);
        buf.Destroy();
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, this + 1);
        // Buffer is destroyed. Callback should be called with UNKNOWN status
        EXPECT_CALL(*mockBufferMapWriteCallback,
                    Call(WGPUBufferMapAsyncStatus_Unknown, nullptr, 0, this + 1))
            .Times(1);
        buf.Destroy();
        queue.Submit(0, nullptr);
    }
}

// Test that it is valid to Destroy a destroyed buffer
TEST_F(BufferValidationTest, DestroyDestroyedBuffer) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);
    buf.Destroy();
    buf.Destroy();
}

// Test that it is invalid to Unmap a destroyed buffer
TEST_F(BufferValidationTest, UnmapDestroyedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
}

// Test that it is invalid to map a destroyed buffer
TEST_F(BufferValidationTest, MapDestroyedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr));
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr));
    }
}

// Test that is is invalid to Map a mapped buffer
TEST_F(BufferValidationTest, MapMappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);
        ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr));
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);
        ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr));
        queue.Submit(0, nullptr);
    }
}

// Test that is is invalid to Map a CreateBufferMapped buffer
TEST_F(BufferValidationTest, MapCreateBufferMappedBuffer) {
    {
        wgpu::Buffer buf = CreateBufferMapped(4, wgpu::BufferUsage::MapRead).buffer;
        ASSERT_DEVICE_ERROR(buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr));
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer buf = CreateBufferMapped(4, wgpu::BufferUsage::MapWrite).buffer;
        ASSERT_DEVICE_ERROR(buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr));
        queue.Submit(0, nullptr);
    }
}

// Test that it is valid to submit a buffer in a queue with a map usage if it is unmapped
TEST_F(BufferValidationTest, SubmitBufferWithMapUsage) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that it is invalid to submit a mapped buffer in a queue
TEST_F(BufferValidationTest, SubmitMappedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufA.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufB.MapReadAsync(ToMockBufferMapReadCallback, nullptr);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer bufA = device.CreateBufferMapped(&descriptorA).buffer;
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBufferMapped(&descriptorB).buffer;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        queue.Submit(0, nullptr);
    }
}

// Test that it is invalid to submit a destroyed buffer in a queue
TEST_F(BufferValidationTest, SubmitDestroyedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    bufA.Destroy();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that a map usage is required to call Unmap
TEST_F(BufferValidationTest, UnmapWithoutMapUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    ASSERT_DEVICE_ERROR(buf.Unmap());
}

// Test that it is valid to call Unmap on a buffer that is not mapped
TEST_F(BufferValidationTest, UnmapUnmappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        // Buffer starts unmapped. Unmap should succeed.
        buf.Unmap();
        buf.MapReadAsync(ToMockBufferMapReadCallback, nullptr);
        buf.Unmap();
        // Unmapping twice should succeed
        buf.Unmap();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        // Buffer starts unmapped. Unmap should succeed.
        buf.Unmap();
        buf.MapWriteAsync(ToMockBufferMapWriteCallback, nullptr);
        // Unmapping twice should succeed
        buf.Unmap();
        buf.Unmap();
    }
}
