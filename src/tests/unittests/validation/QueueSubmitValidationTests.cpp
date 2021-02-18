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

        wgpu::Queue queue = device.GetQueue();

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

    // Test it is invalid to submit a command buffer twice
    TEST_F(QueueSubmitValidationTest, CommandBufferSubmittedTwice) {
        wgpu::CommandBuffer commandBuffer = device.CreateCommandEncoder().Finish();
        wgpu::Queue queue = device.GetQueue();

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

        wgpu::Queue queue = device.GetQueue();

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
