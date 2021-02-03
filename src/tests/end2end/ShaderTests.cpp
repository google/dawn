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

#include "tests/DawnTest.h"
#include "utils/WGPUHelpers.h"

#include <vector>

class ShaderTests : public DawnTest {};

// Test that log2 is being properly calculated, base on crbug.com/1046622
TEST_P(ShaderTests, ComputeLog2) {
    uint32_t const kSteps = 19;
    std::vector<uint32_t> data(kSteps, 0);
    std::vector<uint32_t> expected{0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 32};
    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    std::string shader = R"(
[[block]] struct Buf {
    [[offset(0)]] data : [[stride(4)]] array<u32, 19>;
};

[[group(0), binding(0)]] var<storage_buffer> buf : [[access(read_write)]] Buf;

[[stage(compute)]] fn main() -> void {
    const factor : f32 = 1.0001;

    buf.data[0] = u32(log2(1.0 * factor));
    buf.data[1] = u32(log2(2.0 * factor));
    buf.data[2] = u32(log2(3.0 * factor));
    buf.data[3] = u32(log2(4.0 * factor));
    buf.data[4] = u32(log2(7.0 * factor));
    buf.data[5] = u32(log2(8.0 * factor));
    buf.data[6] = u32(log2(15.0 * factor));
    buf.data[7] = u32(log2(16.0 * factor));
    buf.data[8] = u32(log2(31.0 * factor));
    buf.data[9] = u32(log2(32.0 * factor));
    buf.data[10] = u32(log2(63.0 * factor));
    buf.data[11] = u32(log2(64.0 * factor));
    buf.data[12] = u32(log2(127.0 * factor));
    buf.data[13] = u32(log2(128.0 * factor));
    buf.data[14] = u32(log2(255.0 * factor));
    buf.data[15] = u32(log2(256.0 * factor));
    buf.data[16] = u32(log2(511.0 * factor));
    buf.data[17] = u32(log2(512.0 * factor));
    buf.data[18] = u32(log2(4294967295.0 * factor));
})";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModuleFromWGSL(device, shader.c_str());
    csDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1);
        pass.EndPass();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kSteps);
}

DAWN_INSTANTIATE_TEST(ShaderTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
