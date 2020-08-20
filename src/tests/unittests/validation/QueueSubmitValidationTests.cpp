// Copyright 2018 The Dawn Authors
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

#include "utils/WGPUHelpers.h"

namespace {

    class QueueSubmitValidationTest : public ValidationTest {};

    // Test submitting with a mapped buffer is disallowed
    TEST_F(QueueSubmitValidationTest, SubmitWithMappedBuffer) {
        // Create a map-write buffer.
        const uint64_t kBufferSize = 4;
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        descriptor.size = kBufferSize;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        // Create a fake copy destination buffer
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer targetBuffer = device.CreateBuffer(&descriptor);

        // Create a command buffer that reads from the mappable buffer.
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        wgpu::Queue queue = device.GetDefaultQueue();

        // Submitting when the buffer has never been mapped should succeed
        queue.Submit(1, &commands);

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Map the buffer, submitting when the buffer is mapped should fail
        buffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, nullptr, nullptr);

        // Try submitting before the callback is fired.
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        WaitForAllOperations(device);

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Try submitting after the callback is fired.
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Unmap the buffer, queue submit should succeed
        buffer.Unmap();
        queue.Submit(1, &commands);
    }

    class QueueWriteBufferValidationTest : public ValidationTest {
      private:
        void SetUp() override {
            ValidationTest::SetUp();
            queue = device.GetDefaultQueue();
        }

      protected:
        wgpu::Buffer CreateBuffer(uint64_t size) {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = wgpu::BufferUsage::CopyDst;
            return device.CreateBuffer(&descriptor);
        }

        wgpu::Queue queue;
    };

    // Test the success case for WriteBuffer
    TEST_F(QueueWriteBufferValidationTest, Success) {
        wgpu::Buffer buf = CreateBuffer(4);

        uint32_t foo = 0x01020304;
        queue.WriteBuffer(buf, 0, &foo, sizeof(foo));
    }

    // Test error case for WriteBuffer out of bounds
    TEST_F(QueueWriteBufferValidationTest, OutOfBounds) {
        wgpu::Buffer buf = CreateBuffer(4);

        uint32_t foo[2] = {0, 0};
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, foo, 8));
    }

    // Test error case for WriteBuffer out of bounds with an overflow
    TEST_F(QueueWriteBufferValidationTest, OutOfBoundsOverflow) {
        wgpu::Buffer buf = CreateBuffer(1024);

        uint32_t foo[2] = {0, 0};

        // An offset that when added to "4" would overflow to be zero and pass validation without
        // overflow checks.
        uint64_t offset = uint64_t(int64_t(0) - int64_t(4));

        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, offset, foo, 4));
    }

    // Test error case for WriteBuffer with the wrong usage
    TEST_F(QueueWriteBufferValidationTest, WrongUsage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::Vertex;
        wgpu::Buffer buf = device.CreateBuffer(&descriptor);

        uint32_t foo = 0;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &foo, sizeof(foo)));
    }

    // Test WriteBuffer with unaligned size
    TEST_F(QueueWriteBufferValidationTest, UnalignedSize) {
        wgpu::Buffer buf = CreateBuffer(4);

        uint16_t value = 123;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
    }

    // Test WriteBuffer with unaligned offset
    TEST_F(QueueWriteBufferValidationTest, UnalignedOffset) {
        wgpu::Buffer buf = CreateBuffer(8);

        uint32_t value = 0x01020304;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 2, &value, sizeof(value)));
    }

    // Test WriteBuffer with destroyed buffer
    TEST_F(QueueWriteBufferValidationTest, DestroyedBuffer) {
        wgpu::Buffer buf = CreateBuffer(4);
        buf.Destroy();

        uint32_t value = 0;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
    }

    // Test WriteBuffer with mapped buffer
    TEST_F(QueueWriteBufferValidationTest, MappedBuffer) {
        // mappedAtCreation
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 4;
            descriptor.usage = wgpu::BufferUsage::CopyDst;
            descriptor.mappedAtCreation = true;
            wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

            uint32_t value = 0;
            ASSERT_DEVICE_ERROR(queue.WriteBuffer(buffer, 0, &value, sizeof(value)));
        }

        // MapAsync
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 4;
            descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
            wgpu::Buffer buf = device.CreateBuffer(&descriptor);

            buf.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);
            uint32_t value = 0;
            ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
        }
    }

    // Test it is invalid to submit a command buffer twice
    TEST_F(QueueSubmitValidationTest, CommandBufferSubmittedTwice) {
        wgpu::CommandBuffer commandBuffer = device.CreateCommandEncoder().Finish();
        wgpu::Queue queue = device.GetDefaultQueue();

        // Should succeed
        queue.Submit(1, &commandBuffer);

        // Should fail because command buffer was already submitted
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commandBuffer));
    }

    // Test resubmitting failed command buffers
    TEST_F(QueueSubmitValidationTest, CommandBufferSubmittedFailed) {
        // Create a map-write buffer
        const uint64_t kBufferSize = 4;
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        descriptor.size = kBufferSize;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        // Create a destination buffer for the b2b copy
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        descriptor.size = kBufferSize;
        wgpu::Buffer targetBuffer = device.CreateBuffer(&descriptor);

        // Create a command buffer that reads from the mappable buffer
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        wgpu::Queue queue = device.GetDefaultQueue();

        // Map the source buffer to force a failure
        buffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, nullptr, nullptr);

        // Submitting a command buffer with a mapped buffer should fail
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        // Unmap buffer to fix the failure
        buffer.Unmap();

        // Resubmitting any command buffer, even if the problem was fixed, should fail
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }

}  // anonymous namespace
