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

#include "tests/DawnTest.h"

#include <array>
#include <vector>

class NonzeroBufferCreationTests : public DawnTest {
  public:
    void MapReadAsyncAndWait(wgpu::Buffer buffer, uint64_t offset, uint64_t size) {
        bool done = false;
        buffer.MapAsync(
            wgpu::MapMode::Read, offset, size,
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
                *static_cast<bool*>(userdata) = true;
            },
            &done);

        while (!done) {
            WaitABit();
        }
    }
};

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when it
// is created with CopyDst usage.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithCopyDstUsage) {
    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expectedData.data()), buffer, 0,
                               kSize / sizeof(uint32_t));
}

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when it
// is created with MapWrite without CopyDst usage.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithMapWriteWithoutCopyDstUsage) {
    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expectedData.data()), buffer, 0,
                               kSize / sizeof(uint32_t));
}

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when
// it is created with mappedAtCreation == true.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithMappedAtCreation) {
    // When we use Dawn wire, the lazy initialization of the buffers with mappedAtCreation == true
    // are done in the Dawn wire and we don't plan to get it work with the toggle
    // "nonzero_clear_resources_on_creation_for_testing" (we will have more tests on it in the
    // BufferZeroInitTests.
    DAWN_SKIP_TEST_IF(UsesWire());

    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor defaultDescriptor;
    defaultDescriptor.size = kSize;
    defaultDescriptor.mappedAtCreation = true;

    const std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    const uint32_t* expectedDataPtr = reinterpret_cast<const uint32_t*>(expectedData.data());

    // Buffer with MapRead usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::MapRead;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        MapReadAsyncAndWait(buffer, 0, kSize);
        mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();
    }

    // Buffer with MapWrite usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        EXPECT_BUFFER_U32_RANGE_EQ(expectedDataPtr, buffer, 0, kSize / sizeof(uint32_t));
    }

    // Buffer with neither MapRead nor MapWrite usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        EXPECT_BUFFER_U32_RANGE_EQ(expectedDataPtr, buffer, 0, kSize / sizeof(uint32_t));
    }
}

DAWN_INSTANTIATE_TEST(NonzeroBufferCreationTests,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}));
