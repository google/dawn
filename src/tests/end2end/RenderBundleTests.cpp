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

#include "utils/ComboRenderBundleEncoderDescriptor.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 4;
const RGBA8 kColors[2] = {RGBA8::kGreen, RGBA8::kBlue};

// RenderBundleTest tests simple usage of RenderBundles to draw. The implementaiton
// of RenderBundle is shared significantly with render pass execution which is
// tested in all other rendering tests.
class RenderBundleTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<in> pos : vec4<f32>;
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[block]] struct Ubo {
                [[offset(0)]] color : vec4<f32>;
            };
            [[group(0), binding(0)]] var<uniform> fragmentUniformBuffer : Ubo;

            [[stage(fragment)]] fn main() -> void {
                fragColor = fragmentUniformBuffer.color;
            })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cVertexState.vertexBufferCount = 1;
        descriptor.cVertexState.cVertexBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cVertexState.cVertexBuffers[0].attributeCount = 1;
        descriptor.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
        descriptor.cColorStates[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        float colors0[] = {kColors[0].r / 255.f, kColors[0].g / 255.f, kColors[0].b / 255.f,
                           kColors[0].a / 255.f};
        float colors1[] = {kColors[1].r / 255.f, kColors[1].g / 255.f, kColors[1].b / 255.f,
                           kColors[1].a / 255.f};

        wgpu::Buffer buffer0 = utils::CreateBufferFromData(device, colors0, 4 * sizeof(float),
                                                           wgpu::BufferUsage::Uniform);
        wgpu::Buffer buffer1 = utils::CreateBufferFromData(device, colors1, 4 * sizeof(float),
                                                           wgpu::BufferUsage::Uniform);

        bindGroups[0] = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {{0, buffer0, 0, 4 * sizeof(float)}});
        bindGroups[1] = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {{0, buffer1, 0, 4 * sizeof(float)}});

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,

             // The top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::BindGroup bindGroups[2];
};

// Basic test of RenderBundle.
TEST_P(RenderBundleTest, Basic) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
    renderBundleEncoder.Draw(6);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 3, 1);
}

// Test execution of multiple render bundles
TEST_P(RenderBundleTest, MultipleBundles) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundle renderBundles[2];
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
        renderBundleEncoder.Draw(3);

        renderBundles[0] = renderBundleEncoder.Finish();
    }
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.SetBindGroup(0, bindGroups[1]);
        renderBundleEncoder.Draw(3, 1, 3);

        renderBundles[1] = renderBundleEncoder.Finish();
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(2, renderBundles);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

// Test execution of a bundle along with render pass commands.
TEST_P(RenderBundleTest, BundleAndRenderPassCommands) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
    renderBundleEncoder.Draw(3);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);

    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.SetBindGroup(0, bindGroups[1]);
    pass.Draw(3, 1, 3);

    pass.ExecuteBundles(1, &renderBundle);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

DAWN_INSTANTIATE_TEST(RenderBundleTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
