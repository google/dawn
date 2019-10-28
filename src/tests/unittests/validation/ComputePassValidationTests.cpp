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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/WGPUHelpers.h"

class ComputePassValidationTest : public ValidationTest {};

// Test that it is invalid to use a buffer with both read and write usages in a compute pass.
TEST_F(ComputePassValidationTest, ReadWriteUsage) {
    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform;
    bufferDesc.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::ShaderModule module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
        #version 450
        layout(std430, set = 0, binding = 0) buffer BufA { uint bufA; };
        layout(std140, set = 0, binding = 1) uniform BufB { uint bufB; };
        void main() {}
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                 {1, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer}});

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, buffer, 0, 4},
                                                         {1, buffer, 0, 4},
                                                     });

    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = layout;
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);

    pass.Dispatch(1, 1, 1);
    pass.Dispatch(1, 1, 1);

    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that it is valid to use a buffer with a single write usage multiple times in a compute pass.
TEST_F(ComputePassValidationTest, MultipleWrites) {
    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    bufferDesc.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::ShaderModule module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
        #version 450
        layout(std430, set = 0, binding = 0) buffer Buf { uint buf; };
        void main() {}
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer}});

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = layout;
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);

    pass.Dispatch(1, 1, 1);
    pass.Dispatch(1, 1, 1);

    pass.EndPass();
    encoder.Finish();
}

// Test that it is valid to use a buffer with a single write usage multiple times in a compute pass,
// even if the buffer is referenced in separate bind groups.
TEST_F(ComputePassValidationTest, MultipleWritesSeparateBindGroups) {
    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    bufferDesc.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::ShaderModule module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
        #version 450
        #define kNumValues 100
        layout(std430, set = 0, binding = 0) buffer Buf { uint buf; };
        void main() {}
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer}});

    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});
    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = layout;
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    pass.SetBindGroup(0, bindGroupA);
    pass.Dispatch(1, 1, 1);

    pass.SetBindGroup(0, bindGroupB);
    pass.Dispatch(1, 1, 1);

    pass.EndPass();
    encoder.Finish();
}
