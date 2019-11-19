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

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "tests/DawnTest.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class StorageToUniformSyncTests : public DawnTest {
  protected:
    void CreateBuffer() {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform;
        mBuffer = device.CreateBuffer(&bufferDesc);
    }

    std::tuple<wgpu::ComputePipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForCompute() {
        wgpu::ShaderModule csModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
        #version 450
        layout(std140, set = 0, binding = 0) buffer Data {
            float a;
        } data;
        void main() {
            data.a = 1.0;
        })");

        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                    });
        wgpu::PipelineLayout pipelineLayout0 = utils::MakeBasicPipelineLayout(device, &bgl);

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.layout = pipelineLayout0;
        cpDesc.computeStage.module = csModule;
        cpDesc.computeStage.entryPoint = "main";
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, bgl, {{0, mBuffer, 0, sizeof(float)}});
        return std::make_tuple(pipeline, bindGroup);
    }

    std::tuple<wgpu::RenderPipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForRender(
        wgpu::TextureFormat colorFormat) {
        wgpu::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.f, 0.f, 0.f, 1.f);
            gl_PointSize = 1.0;
        })");

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
        #version 450
        layout (set = 0, binding = 0) uniform Contents{
            float color;
        };
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(color, 0.f, 0.f, 1.f);
        })");

        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Fragment, wgpu::BindingType::UniformBuffer},
                    });
        wgpu::PipelineLayout pipelineLayout = utils::MakeBasicPipelineLayout(device, &bgl);

        utils::ComboRenderPipelineDescriptor rpDesc(device);
        rpDesc.layout = pipelineLayout;
        rpDesc.vertexStage.module = vsModule;
        rpDesc.cFragmentStage.module = fsModule;
        rpDesc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        rpDesc.cColorStates[0].format = colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, bgl, {{0, mBuffer, 0, sizeof(float)}});
        return std::make_tuple(pipeline, bindGroup);
    }

    wgpu::Buffer mBuffer;
};

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the same command buffer.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithSameCommandBuffer) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1, 1, 1);
    pass0.EndPass();

    // Read that data in render pass.
    wgpu::RenderPassEncoder pass1 = encoder0.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1, 1, 0, 0);
    pass1.EndPass();

    wgpu::CommandBuffer commands = encoder0.Finish();
    queue.Submit(1, &commands);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue in one shot.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentCommandBuffers) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1, 1, 1);
    pass0.EndPass();
    cb[0] = encoder0.Finish();

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1, 1, 0, 0);
    pass1.EndPass();

    cb[1] = encoder1.Finish();
    queue.Submit(2, cb);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue separately.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentQueueSubmits) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1, 1, 1);
    pass0.EndPass();
    cb[0] = encoder0.Finish();
    queue.Submit(1, &cb[0]);

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1, 1, 0, 0);
    pass1.EndPass();

    cb[1] = encoder1.Finish();
    queue.Submit(1, &cb[1]);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(StorageToUniformSyncTests,
                      D3D12Backend,
                      MetalBackend,
                      OpenGLBackend,
                      VulkanBackend);
