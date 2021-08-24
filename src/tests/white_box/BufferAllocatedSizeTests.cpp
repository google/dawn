// Copyright 2021 The Dawn Authors
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

#include "tests/DawnTest.h"

#include "common/Math.h"
#include "dawn_native/DawnNative.h"

#include <algorithm>

class BufferAllocatedSizeTests : public DawnTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::BufferUsage usage, uint64_t size) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = usage;
        desc.size = size;
        return device.CreateBuffer(&desc);
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test expected allocated size for buffers with uniform usage
TEST_P(BufferAllocatedSizeTests, UniformUsage) {
    // Some backends have a minimum buffer size, so make sure
    // we allocate above that.
    constexpr uint32_t kMinBufferSize = 4u;

    uint32_t requiredBufferAlignment = 1u;
    if (IsD3D12()) {
        requiredBufferAlignment = 256u;
    } else if (IsMetal()) {
        requiredBufferAlignment = 16u;
    } else if (IsVulkan()) {
        requiredBufferAlignment = 4u;
    }

    // Test uniform usage
    {
        const uint32_t bufferSize = kMinBufferSize;
        wgpu::Buffer buffer = CreateBuffer(wgpu::BufferUsage::Uniform, bufferSize);
        EXPECT_EQ(dawn_native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }

    // Test uniform usage and with size just above requiredBufferAlignment allocates to the next
    // multiple of |requiredBufferAlignment|
    {
        const uint32_t bufferSize = std::max(1u + requiredBufferAlignment, kMinBufferSize);
        wgpu::Buffer buffer =
            CreateBuffer(wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage, bufferSize);
        EXPECT_EQ(dawn_native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }

    // Test uniform usage and another usage
    {
        const uint32_t bufferSize = kMinBufferSize;
        wgpu::Buffer buffer =
            CreateBuffer(wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage, bufferSize);
        EXPECT_EQ(dawn_native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }
}

DAWN_INSTANTIATE_TEST(BufferAllocatedSizeTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
