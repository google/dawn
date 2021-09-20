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
#include "utils/WGPUHelpers.h"

class CommandEncoderTests : public DawnTest {};

// Tests WriteBuffer commands on CommandEncoder.
TEST_P(CommandEncoderTests, WriteBuffer) {
    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});
    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});
    wgpu::Buffer bufferC = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    const uint32_t kData1 = 1;
    encoder.WriteBuffer(bufferA, 0, reinterpret_cast<const uint8_t*>(&kData1), sizeof(kData1));
    encoder.CopyBufferToBuffer(bufferA, 0, bufferB, sizeof(uint32_t), 3 * sizeof(uint32_t));

    const uint32_t kData2 = 2;
    encoder.WriteBuffer(bufferB, 0, reinterpret_cast<const uint8_t*>(&kData2), sizeof(kData2));
    encoder.CopyBufferToBuffer(bufferB, 0, bufferC, sizeof(uint32_t), 3 * sizeof(uint32_t));

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0, bufferC, 0);
    EXPECT_BUFFER_U32_EQ(2, bufferC, sizeof(uint32_t));
    EXPECT_BUFFER_U32_EQ(1, bufferC, 2 * sizeof(uint32_t));
    EXPECT_BUFFER_U32_EQ(0, bufferC, 3 * sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(CommandEncoderTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
