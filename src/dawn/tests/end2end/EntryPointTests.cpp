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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class EntryPointTests : public DawnTest {};

// Test creating a render pipeline from two entryPoints in the same module.
TEST_P(EntryPointTests, FragAndVertexSameModule) {
    // TODO(crbug.com/dawn/658): Crashes on bots
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn vertex_main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        @stage(fragment) fn fragment_main() -> @location(0) vec4<f32> {
          return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Create a point pipeline from the module.
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.vertex.entryPoint = "vertex_main";
    desc.cFragment.module = module;
    desc.cFragment.entryPoint = "fragment_main";
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    // Render the point and check that it was rendered.
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Test creating two compute pipelines from the same module.
TEST_P(EntryPointTests, TwoComputeInModule) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Storage;
    binding.visibility = wgpu::ShaderStage::Compute;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&desc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Data {
            data : u32
        }
        @binding(0) @group(0) var<storage, read_write> data : Data;

        @stage(compute) @workgroup_size(1) fn write1() {
            data.data = 1u;
            return;
        }

        @stage(compute) @workgroup_size(1) fn write42() {
            data.data = 42u;
            return;
        }
    )");

    // Create both pipelines from the module.
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.compute.module = module;

    pipelineDesc.compute.entryPoint = "write1";
    wgpu::ComputePipeline write1 = device.CreateComputePipeline(&pipelineDesc);

    pipelineDesc.compute.entryPoint = "write42";
    wgpu::ComputePipeline write42 = device.CreateComputePipeline(&pipelineDesc);

    // Create the bindGroup.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup group = utils::MakeBindGroup(device, bindGroupLayout, {{0, buffer}});

    // Use the first pipeline and check it wrote 1.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(write1);
        pass.SetBindGroup(0, group);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(1, buffer, 0);
    }

    // Use the second pipeline and check it wrote 42.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(write42);
        pass.SetBindGroup(0, group);
        pass.DispatchWorkgroups(42);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(42, buffer, 0);
    }
}

DAWN_INSTANTIATE_TEST(EntryPointTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
