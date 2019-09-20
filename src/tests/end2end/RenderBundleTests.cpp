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
#include "utils/DawnHelpers.h"

constexpr uint32_t kRTSize = 4;
constexpr RGBA8 kColors[2] = {RGBA8(0, 255, 0, 255), RGBA8(0, 0, 255, 255)};

// RenderBundleTest tests simple usage of RenderBundles to draw. The implementaiton
// of RenderBundle is shared significantly with render pass execution which is
// tested in all other rendering tests.
class RenderBundleTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        dawn::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
                #version 450
                layout(location = 0) in vec4 pos;
                void main() {
                    gl_Position = pos;
                })");

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                layout (set = 0, binding = 0) uniform fragmentUniformBuffer {
                    vec4 color;
                };
                void main() {
                    fragColor = color;
                })");

        dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStage::Fragment, dawn::BindingType::UniformBuffer}});

        float colors0[] = {kColors[0].r / 255.f, kColors[0].g / 255.f, kColors[0].b / 255.f,
                           kColors[0].a / 255.f};
        float colors1[] = {kColors[1].r / 255.f, kColors[1].g / 255.f, kColors[1].b / 255.f,
                           kColors[1].a / 255.f};

        dawn::Buffer buffer0 = utils::CreateBufferFromData(device, colors0, 4 * sizeof(float),
                                                           dawn::BufferUsage::Uniform);
        dawn::Buffer buffer1 = utils::CreateBufferFromData(device, colors1, 4 * sizeof(float),
                                                           dawn::BufferUsage::Uniform);

        bindGroups[0] = utils::MakeBindGroup(device, bgl, {{0, buffer0, 0, 4 * sizeof(float)}});
        bindGroups[1] = utils::MakeBindGroup(device, bgl, {{0, buffer1, 0, 4 * sizeof(float)}});

        dawn::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &bgl;

        dawn::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.layout = pipelineLayout;
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = dawn::PrimitiveTopology::TriangleStrip;
        descriptor.cVertexInput.bufferCount = 1;
        descriptor.cVertexInput.cBuffers[0].stride = 4 * sizeof(float);
        descriptor.cVertexInput.cBuffers[0].attributeCount = 1;
        descriptor.cVertexInput.cAttributes[0].format = dawn::VertexFormat::Float4;
        descriptor.cColorStates[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, dawn::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,

             // The top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f});
    }

    utils::BasicRenderPass renderPass;
    dawn::RenderPipeline pipeline;
    dawn::Buffer vertexBuffer;
    dawn::BindGroup bindGroups[2];
};

// Basic test of RenderBundle.
TEST_P(RenderBundleTest, Basic) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    dawn::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    uint64_t zeroOffset = 0;
    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0], 0, nullptr);
    renderBundleEncoder.Draw(6, 1, 0, 0);

    dawn::RenderBundle renderBundle = renderBundleEncoder.Finish();

    dawn::CommandEncoder encoder = device.CreateCommandEncoder();

    dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);
    pass.EndPass();

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 3, 1);
}

// Test execution of multiple render bundles
TEST_P(RenderBundleTest, MultipleBundles) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    dawn::RenderBundle renderBundles[2];
    uint64_t zeroOffset = 0;
    {
        dawn::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
        renderBundleEncoder.SetBindGroup(0, bindGroups[0], 0, nullptr);
        renderBundleEncoder.Draw(3, 1, 0, 0);

        renderBundles[0] = renderBundleEncoder.Finish();
    }
    {
        dawn::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
        renderBundleEncoder.SetBindGroup(0, bindGroups[1], 0, nullptr);
        renderBundleEncoder.Draw(3, 1, 3, 0);

        renderBundles[1] = renderBundleEncoder.Finish();
    }

    dawn::CommandEncoder encoder = device.CreateCommandEncoder();

    dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(2, renderBundles);
    pass.EndPass();

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

// Test execution of a bundle along with render pass commands.
TEST_P(RenderBundleTest, BundleAndRenderPassCommands) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    dawn::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    uint64_t zeroOffset = 0;
    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0], 0, nullptr);
    renderBundleEncoder.Draw(3, 1, 0, 0);

    dawn::RenderBundle renderBundle = renderBundleEncoder.Finish();

    dawn::CommandEncoder encoder = device.CreateCommandEncoder();

    dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);

    pass.SetPipeline(pipeline);
    pass.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
    pass.SetBindGroup(0, bindGroups[1], 0, nullptr);
    pass.Draw(3, 1, 3, 0);

    pass.ExecuteBundles(1, &renderBundle);
    pass.EndPass();

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

DAWN_INSTANTIATE_TEST(RenderBundleTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
