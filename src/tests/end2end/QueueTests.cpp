// Copyright 2020 The Dawn Authors
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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

class QueueTests : public DawnTest {};

// Test that GetDefaultQueue always returns the same object.
TEST_P(QueueTests, GetDefaultQueueSameObject) {
    wgpu::Queue q1 = device.GetDefaultQueue();
    wgpu::Queue q2 = device.GetDefaultQueue();
    EXPECT_EQ(q1.Get(), q2.Get());
}

DAWN_INSTANTIATE_TEST(QueueTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class QueueWriteBufferTests : public DawnTest {};

// Test the simplest WriteBuffer setting one u32 at offset 0.
TEST_P(QueueWriteBufferTests, SmallDataAtZero) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test an empty WriteBuffer
TEST_P(QueueWriteBufferTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t initialValue = 0x42;
    queue.WriteBuffer(buffer, 0, &initialValue, sizeof(initialValue));

    queue.WriteBuffer(buffer, 0, nullptr, 0);

    // The content of the buffer isn't changed
    EXPECT_BUFFER_U32_EQ(initialValue, buffer, 0);
}

// Call WriteBuffer at offset 0 via a u32 twice. Test that data is updated accoordingly.
TEST_P(QueueWriteBufferTests, SetTwice) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);

    value = 0x05060708;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test that WriteBuffer offset works.
TEST_P(QueueWriteBufferTests, SmallDataAtOffset) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4000;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr uint64_t kOffset = 2000;
    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, kOffset, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, kOffset);
}

// Stress test for many calls to WriteBuffer
TEST_P(QueueWriteBufferTests, ManyWriteBuffer) {
    // Note: Increasing the size of the buffer will likely cause timeout issues.
    // In D3D12, timeout detection occurs when the GPU scheduler tries but cannot preempt the task
    // executing these commands in-flight. If this takes longer than ~2s, a device reset occurs and
    // fails the test. Since GPUs may or may not complete by then, this test must be disabled OR
    // modified to be well-below the timeout limit.

    // TODO (jiawei.shao@intel.com): find out why this test fails on Intel Vulkan Linux bots.
    DAWN_SKIP_TEST_IF(IsIntel() && IsVulkan() && IsLinux());
    // TODO(https://bugs.chromium.org/p/dawn/issues/detail?id=228): Re-enable
    // once the issue with Metal on 10.14.6 is fixed.
    DAWN_SKIP_TEST_IF(IsMacOS() && IsIntel() && IsMetal());

    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 500 * 500;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        queue.WriteBuffer(buffer, i * sizeof(uint32_t), &i, sizeof(i));
        expectedData.push_back(i);
    }

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using WriteBuffer for lots of data
TEST_P(QueueWriteBufferTests, LargeWriteBuffer) {
    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 1000 * 1000;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    queue.WriteBuffer(buffer, 0, expectedData.data(), kElements * sizeof(uint32_t));

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using WriteBuffer for super large data block
TEST_P(QueueWriteBufferTests, SuperLargeWriteBuffer) {
    constexpr uint64_t kSize = 12000 * 1000;
    constexpr uint64_t kElements = 3000 * 1000;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    queue.WriteBuffer(buffer, 0, expectedData.data(), kElements * sizeof(uint32_t));

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

DAWN_INSTANTIATE_TEST(QueueWriteBufferTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
