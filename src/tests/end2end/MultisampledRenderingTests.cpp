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

#include "common/Assert.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class MultisampledRenderingTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        InitTexturesForTest();
    }

    void InitTexturesForTest() {
        mMultisampledColorView =
            CreateTextureForOutputAttachment(kColorFormat, kSampleCount).CreateView();
        mResolveTexture = CreateTextureForOutputAttachment(kColorFormat, 1);
        mResolveView = mResolveTexture.CreateView();

        mDepthStencilTexture = CreateTextureForOutputAttachment(kDepthStencilFormat, kSampleCount);
        mDepthStencilView = mDepthStencilTexture.CreateView();
    }

    wgpu::RenderPipeline CreateRenderPipelineWithOneOutputForTest(bool testDepth) {
        const char* kFsOneOutputWithDepth =
            R"(#version 450
            layout(location = 0) out vec4 fragColor;
            layout (std140, set = 0, binding = 0) uniform uBuffer {
                vec4 color;
                float depth;
            };
            void main() {
                fragColor = color;
                gl_FragDepth = depth;
            })";

        const char* kFsOneOutputWithoutDepth =
            R"(#version 450
            layout(location = 0) out vec4 fragColor;
            layout (std140, set = 0, binding = 0) uniform uBuffer {
                vec4 color;
            };
            void main() {
                fragColor = color;
            })";

        const char* fs = testDepth ? kFsOneOutputWithDepth : kFsOneOutputWithoutDepth;


        return CreateRenderPipelineForTest(fs, 1, testDepth);
    }

    wgpu::RenderPipeline CreateRenderPipelineWithTwoOutputsForTest() {
        const char* kFsTwoOutputs =
            R"(#version 450
            layout(location = 0) out vec4 fragColor1;
            layout(location = 1) out vec4 fragColor2;
            layout (std140, set = 0, binding = 0) uniform uBuffer {
                vec4 color1;
                vec4 color2;
            };
            void main() {
                fragColor1 = color1;
                fragColor2 = color2;
            })";

        return CreateRenderPipelineForTest(kFsTwoOutputs, 2, false);
    }

    wgpu::Texture CreateTextureForOutputAttachment(wgpu::TextureFormat format,
                                                   uint32_t sampleCount,
                                                   uint32_t mipLevelCount = 1,
                                                   uint32_t arrayLayerCount = 1) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kWidth << (mipLevelCount - 1);
        descriptor.size.height = kHeight << (mipLevelCount - 1);
        descriptor.size.depth = arrayLayerCount;
        descriptor.sampleCount = sampleCount;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        return device.CreateTexture(&descriptor);
    }

    void EncodeRenderPassForTest(wgpu::CommandEncoder commandEncoder,
                                 const wgpu::RenderPassDescriptor& renderPass,
                                 const wgpu::RenderPipeline& pipeline,
                                 const float* uniformData,
                                 uint32_t uniformDataSize) {
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, uniformData, uniformDataSize, wgpu::BufferUsage::Uniform);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, uniformBuffer, 0, uniformDataSize}});

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.Draw(3);
        renderPassEncoder.EndPass();
    }

    utils::ComboRenderPassDescriptor CreateComboRenderPassDescriptorForTest(
        std::initializer_list<wgpu::TextureView> colorViews,
        std::initializer_list<wgpu::TextureView> resolveTargetViews,
        wgpu::LoadOp colorLoadOp,
        wgpu::LoadOp depthStencilLoadOp,
        bool hasDepthStencilAttachment) {
        ASSERT(colorViews.size() == resolveTargetViews.size());

        constexpr wgpu::Color kClearColor = {0.0f, 0.0f, 0.0f, 0.0f};
        constexpr float kClearDepth = 1.0f;

        utils::ComboRenderPassDescriptor renderPass(colorViews);
        uint32_t i = 0;
        for (const wgpu::TextureView& resolveTargetView : resolveTargetViews) {
            renderPass.cColorAttachments[i].loadOp = colorLoadOp;
            renderPass.cColorAttachments[i].clearColor = kClearColor;
            renderPass.cColorAttachments[i].resolveTarget = resolveTargetView;
            ++i;
        }

        renderPass.cDepthStencilAttachmentInfo.clearDepth = kClearDepth;
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = depthStencilLoadOp;

        if (hasDepthStencilAttachment) {
            renderPass.cDepthStencilAttachmentInfo.attachment = mDepthStencilView;
            renderPass.depthStencilAttachment = &renderPass.cDepthStencilAttachmentInfo;
        }

        return renderPass;
    }

    void VerifyResolveTarget(const wgpu::Color& inputColor,
                             wgpu::Texture resolveTexture,
                             uint32_t mipmapLevel = 0,
                             uint32_t arrayLayer = 0) {
        constexpr float kMSAACoverage = 0.5f;

        // In this test we only check the pixel in the middle of the texture.
        constexpr uint32_t kMiddleX = (kWidth - 1) / 2;
        constexpr uint32_t kMiddleY = (kHeight - 1) / 2;

        RGBA8 expectedColor;
        expectedColor.r = static_cast<uint8_t>(0xFF * inputColor.r * kMSAACoverage);
        expectedColor.g = static_cast<uint8_t>(0xFF * inputColor.g * kMSAACoverage);
        expectedColor.b = static_cast<uint8_t>(0xFF * inputColor.b * kMSAACoverage);
        expectedColor.a = static_cast<uint8_t>(0xFF * inputColor.a * kMSAACoverage);

        EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, resolveTexture, kMiddleX, kMiddleY, 1, 1,
                                mipmapLevel, arrayLayer);
    }

    constexpr static uint32_t kWidth = 3;
    constexpr static uint32_t kHeight = 3;
    constexpr static uint32_t kSampleCount = 4;
    constexpr static wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr static wgpu::TextureFormat kDepthStencilFormat =
        wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::TextureView mMultisampledColorView;
    wgpu::Texture mResolveTexture;
    wgpu::TextureView mResolveView;
    wgpu::Texture mDepthStencilTexture;
    wgpu::TextureView mDepthStencilView;

  private:
    wgpu::RenderPipeline CreateRenderPipelineForTest(const char* fs,
                                                     uint32_t numColorAttachments,
                                                     bool hasDepthStencilAttachment) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        // Draw a bottom-right triangle. In standard 4xMSAA pattern, for the pixels on diagonal,
        // only two of the samples will be touched.
        const char* vs =
            R"(#version 450
            const vec2 pos[3] = vec2[3](vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(1.f, -1.f));
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
            })";
        pipelineDescriptor.vertexStage.module =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, vs);

        pipelineDescriptor.cFragmentStage.module =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, fs);

        if (hasDepthStencilAttachment) {
            pipelineDescriptor.cDepthStencilState.format = kDepthStencilFormat;
            pipelineDescriptor.cDepthStencilState.depthWriteEnabled = true;
            pipelineDescriptor.cDepthStencilState.depthCompare = wgpu::CompareFunction::Less;
            pipelineDescriptor.depthStencilState = &pipelineDescriptor.cDepthStencilState;
        }

        pipelineDescriptor.sampleCount = kSampleCount;

        pipelineDescriptor.colorStateCount = numColorAttachments;
        for (uint32_t i = 0; i < numColorAttachments; ++i) {
            pipelineDescriptor.cColorStates[i].format = kColorFormat;
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        return pipeline;
    }
};

// Test using one multisampled color attachment with resolve target can render correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DTexture) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr uint32_t kSize = sizeof(kGreen);

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, &kGreen.r, kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test multisampled rendering with depth test works correctly.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTest) {
    constexpr bool kTestDepth = true;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    {
        utils::ComboRenderPassDescriptor renderPass =
            CreateComboRenderPassDescriptorForTest({mMultisampledColorView}, {mResolveView},
                                                   wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, true);
        std::array<float, 5> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a, // Color
                                             0.2f};                                  // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // This red triangle should not be displayed because it is behind the green one that is drawn in
    // the last render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a, // color
                                             0.5f};                          // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The color of the pixel in the middle of mResolveTexture should be green if MSAA resolve runs
    // correctly with depth test.
    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test rendering into a multisampled color attachment and doing MSAA resolve in another render pass
// works correctly.
TEST_P(MultisampledRenderingTest, ResolveInAnotherRenderPass) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr uint32_t kSize = sizeof(kGreen);

    // In first render pass we draw a green triangle and do not set the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, &kGreen.r, kSize);
    }

    // In second render pass we ony do MSAA resolve with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.EndPass();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test doing MSAA resolve into multiple resolve targets works correctly.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargets) {
    wgpu::TextureView multisampledColorView2 =
        CreateTextureForOutputAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForOutputAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,          // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a}; // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, mResolveTexture);
    VerifyResolveTarget(kGreen, resolveTexture2);
}

// Test doing MSAA resolve on one multisampled texture twice works correctly.
TEST_P(MultisampledRenderingTest, ResolveOneMultisampledTextureTwice) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr uint32_t kSize = sizeof(kGreen);

    wgpu::Texture resolveTexture2 = CreateTextureForOutputAttachment(kColorFormat, 1);

    // In first render pass we draw a green triangle and specify mResolveView as the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, &kGreen.r, kSize);
    }

    // In second render pass we do MSAA resolve into resolveTexture2.
    {
        wgpu::TextureView resolveView2 = resolveTexture2.CreateView();
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {resolveView2}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.EndPass();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
    VerifyResolveTarget(kGreen, resolveTexture2);
}

// Test using a layer of a 2D texture as resolve target works correctly.
TEST_P(MultisampledRenderingTest, ResolveIntoOneMipmapLevelOf2DTexture) {
    constexpr uint32_t kBaseMipLevel = 2;

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.format = kColorFormat;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseMipLevel = kBaseMipLevel;

    wgpu::Texture resolveTexture =
        CreateTextureForOutputAttachment(kColorFormat, 1, kBaseMipLevel + 1, 1);
    wgpu::TextureView resolveView = resolveTexture.CreateView(&textureViewDescriptor);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr uint32_t kSize = sizeof(kGreen);
    constexpr bool kTestDepth = false;

    // Draw a green triangle and do MSAA resolve.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {resolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, &kGreen.r, kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, resolveTexture, kBaseMipLevel, 0);
}

// Test using a level or a layer of a 2D array texture as resolve target works correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DArrayTexture) {
    wgpu::TextureView multisampledColorView2 =
        CreateTextureForOutputAttachment(kColorFormat, kSampleCount).CreateView();

    wgpu::TextureViewDescriptor baseTextureViewDescriptor;
    baseTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    baseTextureViewDescriptor.format = kColorFormat;
    baseTextureViewDescriptor.arrayLayerCount = 1;
    baseTextureViewDescriptor.mipLevelCount = 1;

    // Create resolveTexture1 with only 1 mipmap level.
    constexpr uint32_t kBaseArrayLayer1 = 2;
    constexpr uint32_t kBaseMipLevel1 = 0;
    wgpu::Texture resolveTexture1 =
        CreateTextureForOutputAttachment(kColorFormat, 1, kBaseMipLevel1 + 1, kBaseArrayLayer1 + 1);
    wgpu::TextureViewDescriptor resolveViewDescriptor1 = baseTextureViewDescriptor;
    resolveViewDescriptor1.baseArrayLayer = kBaseArrayLayer1;
    resolveViewDescriptor1.baseMipLevel = kBaseMipLevel1;
    wgpu::TextureView resolveView1 = resolveTexture1.CreateView(&resolveViewDescriptor1);

    // Create resolveTexture2 with (kBaseMipLevel2 + 1) mipmap levels and resolve into its last
    // mipmap level.
    constexpr uint32_t kBaseArrayLayer2 = 5;
    constexpr uint32_t kBaseMipLevel2 = 3;
    wgpu::Texture resolveTexture2 =
        CreateTextureForOutputAttachment(kColorFormat, 1, kBaseMipLevel2 + 1, kBaseArrayLayer2 + 1);
    wgpu::TextureViewDescriptor resolveViewDescriptor2 = baseTextureViewDescriptor;
    resolveViewDescriptor2.baseArrayLayer = kBaseArrayLayer2;
    resolveViewDescriptor2.baseMipLevel = kBaseMipLevel2;
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView(&resolveViewDescriptor2);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a green triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {resolveView1, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,          // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a}; // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, resolveTexture1, kBaseMipLevel1, kBaseArrayLayer1);
    VerifyResolveTarget(kGreen, resolveTexture2, kBaseMipLevel2, kBaseArrayLayer2);
}

DAWN_INSTANTIATE_TEST(MultisampledRenderingTest,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend(),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));
