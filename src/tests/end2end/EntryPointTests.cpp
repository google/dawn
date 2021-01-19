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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class EntryPointTests : public DawnTest {};

// Test creating a render pipeline from two entryPoints in the same module.
TEST_P(EntryPointTests, FragAndVertexSameModule) {
    // TODO: Reenable once Tint is able to produce Vulkan 1.0 / 1.1 SPIR-V.
    DAWN_SKIP_TEST_IF(IsVulkan() || IsOpenGL() || IsOpenGLES());

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn vertex_main() -> void {
            Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            return;
        }

        [[location(0)]] var<out> outColor : vec4<f32>;

        [[stage(fragment)]] fn fragment_main() -> void {
          outColor = vec4<f32>(1.0, 0.0, 0.0, 1.0);
          return;
        }
    )");

    // Create a point pipeline from the module.
    utils::ComboRenderPipelineDescriptor desc(device);
    desc.vertexStage.module = module;
    desc.vertexStage.entryPoint = "vertex_main";
    desc.cFragmentStage.module = module;
    desc.cFragmentStage.entryPoint = "fragment_main";
    desc.cColorStates[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    // Render the point and check that it was rendered.
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.EndPass();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Test creating two compute pipelines from the same module.
TEST_P(EntryPointTests, TwoComputeInModule) {
    // TODO: Reenable once Tint is able to produce Vulkan 1.0 / 1.1 SPIR-V.
    DAWN_SKIP_TEST_IF(IsVulkan());

    // TODO: Reenable once Tint's HLSL writer supports multiple entryPoints on a single stage.
    // https://crbug.com/tint/297
    DAWN_SKIP_TEST_IF(IsD3D12() && HasToggleEnabled("use_tint_generator"));

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Data {
            [[offset(0)]] data : u32;
        };
        [[binding(0), group(0)]] var<storage_buffer> data : Data;

        [[stage(compute)]] fn write1() -> void {
            data.data = 1u;
            return;
        }

        [[stage(compute)]] fn write42() -> void {
            data.data = 42u;
            return;
        }
    )");

    // Create both pipelines from the module.
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.computeStage.module = module;

    pipelineDesc.computeStage.entryPoint = "write1";
    wgpu::ComputePipeline write1 = device.CreateComputePipeline(&pipelineDesc);

    pipelineDesc.computeStage.entryPoint = "write42";
    wgpu::ComputePipeline write42 = device.CreateComputePipeline(&pipelineDesc);

    // Create the bindGroup.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup group =
        utils::MakeBindGroup(device, write1.GetBindGroupLayout(0), {{0, buffer}});

    // Use the first pipeline and check it wrote 1.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(write1);
        pass.SetBindGroup(0, group);
        pass.Dispatch(1);
        pass.EndPass();
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
        pass.Dispatch(42);
        pass.EndPass();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(42, buffer, 0);
    }
}

DAWN_INSTANTIATE_TEST(EntryPointTests,
                      D3D12Backend(),
                      D3D12Backend({"use_tint_generator"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
