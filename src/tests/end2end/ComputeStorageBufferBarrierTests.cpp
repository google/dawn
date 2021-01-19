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

#include "utils/WGPUHelpers.h"

class ComputeStorageBufferBarrierTests : public DawnTest {
  protected:
    static constexpr uint32_t kNumValues = 100;
    static constexpr uint32_t kIterations = 100;
};

// Test that multiple dispatches to increment values in a storage buffer are synchronized.
TEST_P(ComputeStorageBufferBarrierTests, AddIncrement) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expected(kNumValues, 0x1234 * kIterations);

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buf {
            [[offset(0)]] data : [[stride(4)]] array<u32, 100>;
        };

        [[group(0), binding(0)]] var<storage_buffer> buf : [[access(read_write)]] Buf;

        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        [[stage(compute)]] fn main() -> void {
            buf.data[GlobalInvocationID.x] = buf.data[GlobalInvocationID.x] + 0x1234;
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer, 0, bufferSize}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    for (uint32_t i = 0; i < kIterations; ++i) {
        pass.Dispatch(kNumValues);
    }
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kNumValues);
}

// Test that multiple dispatches to increment values by ping-ponging between two storage buffers
// are synchronized.
TEST_P(ComputeStorageBufferBarrierTests, AddPingPong) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expectedA(kNumValues, 0x1234 * kIterations);
    std::vector<uint32_t> expectedB(kNumValues, 0x1234 * (kIterations - 1));

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));

    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/386):  Use the same struct.
        [[block]] struct Src {
            [[offset(0)]] data : [[stride(4)]] array<u32, 100>;
        };

        [[block]] struct Dst {
            [[offset(0)]] data : [[stride(4)]] array<u32, 100>;
        };

        [[group(0), binding(0)]] var<storage_buffer> src : [[access(read_write)]] Src;
        [[group(0), binding(1)]] var<storage_buffer> dst : [[access(read_write)]] Dst;
        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        [[stage(compute)]] fn main() -> void {
            dst.data[GlobalInvocationID.x] = src.data[GlobalInvocationID.x] + 0x1234;
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferA, 0, bufferSize},
                                                          {1, bufferB, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferB, 0, bufferSize},
                                                          {1, bufferA, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroups[2] = {bindGroupA, bindGroupB};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    for (uint32_t i = 0; i < kIterations / 2; ++i) {
        pass.SetBindGroup(0, bindGroups[0]);
        pass.Dispatch(kNumValues);
        pass.SetBindGroup(0, bindGroups[1]);
        pass.Dispatch(kNumValues);
    }
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedA.data(), bufferA, 0, kNumValues);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedB.data(), bufferB, 0, kNumValues);
}

// Test that multiple dispatches to increment values by ping-ponging between storage buffers and
// read-only storage buffers are synchronized in one compute pass.
TEST_P(ComputeStorageBufferBarrierTests, StorageAndReadonlyStoragePingPongInOnePass) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expectedA(kNumValues, 0x1234 * kIterations);
    std::vector<uint32_t> expectedB(kNumValues, 0x1234 * (kIterations - 1));

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));

    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/386):  Use the same struct.
        [[block]] struct Src {
            [[offset(0)]] data : [[stride(4)]] array<u32, 100>;
        };

        [[block]] struct Dst {
            [[offset(0)]] data : [[stride(4)]] array<u32, 100>;
        };

        [[group(0), binding(0)]] var<storage_buffer> src : [[access(read)]] Src;
        [[group(0), binding(1)]] var<storage_buffer> dst : [[access(read_write)]] Dst;

        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        [[stage(compute)]] fn main() -> void {
            dst.data[GlobalInvocationID.x] = src.data[GlobalInvocationID.x] + 0x1234;
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferA, 0, bufferSize},
                                                          {1, bufferB, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferB, 0, bufferSize},
                                                          {1, bufferA, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroups[2] = {bindGroupA, bindGroupB};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    for (uint32_t i = 0; i < kIterations / 2; ++i) {
        pass.SetBindGroup(0, bindGroups[0]);
        pass.Dispatch(kNumValues);
        pass.SetBindGroup(0, bindGroups[1]);
        pass.Dispatch(kNumValues);
    }
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedA.data(), bufferA, 0, kNumValues);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedB.data(), bufferB, 0, kNumValues);
}

// Test that Storage to Uniform buffer transitions work and synchronize correctly
// by ping-ponging between Storage/Uniform usage in sequential compute passes.
TEST_P(ComputeStorageBufferBarrierTests, UniformToStorageAddPingPong) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expectedA(kNumValues, 0x1234 * kIterations);
    std::vector<uint32_t> expectedB(kNumValues, 0x1234 * (kIterations - 1));

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));

    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, data.data(), bufferSize,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc);

    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, data.data(), bufferSize,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buf {
            [[offset(0)]] data : [[stride(16)]] array<vec4<u32>, 25>;
        };

        [[group(0), binding(0)]] var<uniform> src : Buf;
        [[group(0), binding(1)]] var<storage_buffer> dst : [[access(read_write)]] Buf;
        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        [[stage(compute)]] fn main() -> void {
            dst.data[GlobalInvocationID.x] = src.data[GlobalInvocationID.x] +
                vec4<u32>(0x1234, 0x1234, 0x1234, 0x1234);
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferA, 0, bufferSize},
                                                          {1, bufferB, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferB, 0, bufferSize},
                                                          {1, bufferA, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroups[2] = {bindGroupA, bindGroupB};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    for (uint32_t i = 0, b = 0; i < kIterations; ++i, b = 1 - b) {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroups[b]);
        pass.Dispatch(kNumValues / 4);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedA.data(), bufferA, 0, kNumValues);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedB.data(), bufferB, 0, kNumValues);
}

// Test that Storage to Uniform buffer transitions work and synchronize correctly
// by ping-ponging between Storage/Uniform usage in one compute pass.
TEST_P(ComputeStorageBufferBarrierTests, UniformToStorageAddPingPongInOnePass) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expectedA(kNumValues, 0x1234 * kIterations);
    std::vector<uint32_t> expectedB(kNumValues, 0x1234 * (kIterations - 1));

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));

    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, data.data(), bufferSize,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc);

    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, data.data(), bufferSize,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buf {
            [[offset(0)]] data : [[stride(16)]] array<vec4<u32>, 25>;
        };

        [[group(0), binding(0)]] var<uniform> src : Buf;
        [[group(0), binding(1)]] var<storage_buffer> dst : [[access(read_write)]] Buf;
        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        [[stage(compute)]] fn main() -> void {
            dst.data[GlobalInvocationID.x] = src.data[GlobalInvocationID.x] +
                vec4<u32>(0x1234, 0x1234, 0x1234, 0x1234);
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferA, 0, bufferSize},
                                                          {1, bufferB, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, bufferB, 0, bufferSize},
                                                          {1, bufferA, 0, bufferSize},
                                                      });

    wgpu::BindGroup bindGroups[2] = {bindGroupA, bindGroupB};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    for (uint32_t i = 0, b = 0; i < kIterations; ++i, b = 1 - b) {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroups[b]);
        pass.Dispatch(kNumValues / 4);
    }
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expectedA.data(), bufferA, 0, kNumValues);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedB.data(), bufferB, 0, kNumValues);
}

DAWN_INSTANTIATE_TEST(ComputeStorageBufferBarrierTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
