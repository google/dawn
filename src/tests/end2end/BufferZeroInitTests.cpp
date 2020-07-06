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

#include "tests/DawnTest.h"

#include "utils/WGPUHelpers.h"

#define EXPECT_LAZY_CLEAR(N, statement)                                                       \
    do {                                                                                      \
        if (UsesWire()) {                                                                     \
            statement;                                                                        \
        } else {                                                                              \
            size_t lazyClearsBefore = dawn_native::GetLazyClearCountForTesting(device.Get()); \
            statement;                                                                        \
            size_t lazyClearsAfter = dawn_native::GetLazyClearCountForTesting(device.Get());  \
            EXPECT_EQ(N, lazyClearsAfter - lazyClearsBefore);                                 \
        }                                                                                     \
    } while (0)

class BufferZeroInitTest : public DawnTest {
  public:
    wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        return device.CreateBuffer(&descriptor);
    }
};

// Test that calling writeBuffer to overwrite the entire buffer doesn't need to lazily initialize
// the destination buffer.
TEST_P(BufferZeroInitTest, WriteBufferToEntireBuffer) {
    constexpr uint32_t kBufferSize = 8u;
    constexpr wgpu::BufferUsage kBufferUsage =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = CreateBuffer(kBufferSize, kBufferUsage);

    constexpr std::array<uint32_t, kBufferSize / sizeof(uint32_t)> kExpectedData = {
        {0x02020202u, 0x02020202u}};
    EXPECT_LAZY_CLEAR(0u, queue.WriteBuffer(buffer, 0, kExpectedData.data(), kBufferSize));

    EXPECT_BUFFER_U32_RANGE_EQ(kExpectedData.data(), buffer, 0, kBufferSize / sizeof(uint32_t));
}

// Test that calling writeBuffer to overwrite a part of buffer needs to lazily initialize the
// destination buffer.
TEST_P(BufferZeroInitTest, WriteBufferToSubBuffer) {
    constexpr uint32_t kBufferSize = 8u;
    constexpr wgpu::BufferUsage kBufferUsage =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

    constexpr uint32_t kCopyValue = 0x02020202u;

    // offset == 0
    {
        wgpu::Buffer buffer = CreateBuffer(kBufferSize, kBufferUsage);

        constexpr uint32_t kCopyOffset = 0u;
        EXPECT_LAZY_CLEAR(1u,
                          queue.WriteBuffer(buffer, kCopyOffset, &kCopyValue, sizeof(kCopyValue)));

        EXPECT_BUFFER_U32_EQ(kCopyValue, buffer, kCopyOffset);
        EXPECT_BUFFER_U32_EQ(0, buffer, kBufferSize - sizeof(kCopyValue));
    }

    // offset > 0
    {
        wgpu::Buffer buffer = CreateBuffer(kBufferSize, kBufferUsage);

        constexpr uint32_t kCopyOffset = 4u;
        EXPECT_LAZY_CLEAR(1u,
                          queue.WriteBuffer(buffer, kCopyOffset, &kCopyValue, sizeof(kCopyValue)));

        EXPECT_BUFFER_U32_EQ(0, buffer, 0);
        EXPECT_BUFFER_U32_EQ(kCopyValue, buffer, kCopyOffset);
    }
}

DAWN_INSTANTIATE_TEST(BufferZeroInitTest,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing",
                                    "lazy_clear_buffer_on_first_use"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing",
                                    "lazy_clear_buffer_on_first_use"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing",
                                     "lazy_clear_buffer_on_first_use"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing",
                                     "lazy_clear_buffer_on_first_use"}));
