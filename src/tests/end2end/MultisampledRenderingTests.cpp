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
        mMultisampledColorTexture = CreateTextureForRenderAttachment(kColorFormat, kSampleCount);
        mMultisampledColorView = mMultisampledColorTexture.CreateView();
        mResolveTexture = CreateTextureForRenderAttachment(kColorFormat, 1);
        mResolveView = mResolveTexture.CreateView();

        mDepthStencilTexture = CreateTextureForRenderAttachment(kDepthStencilFormat, kSampleCount);
        mDepthStencilView = mDepthStencilTexture.CreateView();
    }

    wgpu::RenderPipeline CreateRenderPipelineWithOneOutputForTest(
        bool testDepth,
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false,
        bool flipTriangle = false) {
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

        return CreateRenderPipelineForTest(fs, 1, testDepth, sampleMask, alphaToCoverageEnabled,
                                           flipTriangle);
    }

    wgpu::RenderPipeline CreateRenderPipelineWithTwoOutputsForTest(
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false) {
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

        return CreateRenderPipelineForTest(kFsTwoOutputs, 2, false, sampleMask,
                                           alphaToCoverageEnabled);
    }

    wgpu::Texture CreateTextureForRenderAttachment(wgpu::TextureFormat format,
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
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
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

    void EncodeRenderPassForTest(wgpu::CommandEncoder commandEncoder,
                                 const wgpu::RenderPassDescriptor& renderPass,
                                 const wgpu::RenderPipeline& pipeline,
                                 const wgpu::Color& color) {
        const float uniformData[4] = {static_cast<float>(color.r), static_cast<float>(color.g),
                                      static_cast<float>(color.b), static_cast<float>(color.a)};
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, uniformData,
                                sizeof(float) * 4);
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
                             uint32_t arrayLayer = 0,
                             const float msaaCoverage = 0.5f) {
        // In this test we only check the pixel in the middle of the texture.
        constexpr uint32_t kMiddleX = (kWidth - 1) / 2;
        constexpr uint32_t kMiddleY = (kHeight - 1) / 2;

        RGBA8 expectedColor = ExpectedMSAAColor(inputColor, msaaCoverage);
        EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, resolveTexture, kMiddleX, kMiddleY, 1, 1,
                                mipmapLevel, arrayLayer);
    }

    constexpr static uint32_t kWidth = 3;
    constexpr static uint32_t kHeight = 3;
    constexpr static uint32_t kSampleCount = 4;
    constexpr static wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr static wgpu::TextureFormat kDepthStencilFormat =
        wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr static uint32_t kFirstSampleMaskBit = 0x00000001;
    constexpr static uint32_t kSecondSampleMaskBit = 0x00000002;
    constexpr static uint32_t kThirdSampleMaskBit = 0x00000004;
    constexpr static uint32_t kFourthSampleMaskBit = 0x00000008;

    wgpu::Texture mMultisampledColorTexture;
    wgpu::TextureView mMultisampledColorView;
    wgpu::Texture mResolveTexture;
    wgpu::TextureView mResolveView;
    wgpu::Texture mDepthStencilTexture;
    wgpu::TextureView mDepthStencilView;

    wgpu::RenderPipeline CreateRenderPipelineForTest(const char* fs,
                                                     uint32_t numColorAttachments,
                                                     bool hasDepthStencilAttachment,
                                                     uint32_t sampleMask = 0xFFFFFFFF,
                                                     bool alphaToCoverageEnabled = false,
                                                     bool flipTriangle = false) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        // Draw a bottom-right triangle. In standard 4xMSAA pattern, for the pixels on diagonal,
        // only two of the samples will be touched.
        const char* vs =
            R"(#version 450
            const vec2 pos[3] = vec2[3](vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(1.f, -1.f));
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
            })";

        // Draw a bottom-left triangle.
        const char* vsFlipped =
            R"(#version 450
            const vec2 pos[3] = vec2[3](vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(-1.f, -1.f));
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
            })";

        if (flipTriangle) {
            pipelineDescriptor.vertexStage.module =
                utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, vsFlipped);
        } else {
            pipelineDescriptor.vertexStage.module =
                utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, vs);
        }

        pipelineDescriptor.cFragmentStage.module =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, fs);

        if (hasDepthStencilAttachment) {
            pipelineDescriptor.cDepthStencilState.format = kDepthStencilFormat;
            pipelineDescriptor.cDepthStencilState.depthWriteEnabled = true;
            pipelineDescriptor.cDepthStencilState.depthCompare = wgpu::CompareFunction::Less;
            pipelineDescriptor.depthStencilState = &pipelineDescriptor.cDepthStencilState;
        }

        pipelineDescriptor.sampleCount = kSampleCount;
        pipelineDescriptor.sampleMask = sampleMask;
        pipelineDescriptor.alphaToCoverageEnabled = alphaToCoverageEnabled;

        pipelineDescriptor.colorStateCount = numColorAttachments;
        for (uint32_t i = 0; i < numColorAttachments; ++i) {
            pipelineDescriptor.cColorStates[i].format = kColorFormat;
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        return pipeline;
    }

    RGBA8 ExpectedMSAAColor(const wgpu::Color color, const double msaaCoverage) {
        RGBA8 result;
        result.r = static_cast<uint8_t>(std::min(255.0, 256 * color.r * msaaCoverage));
        result.g = static_cast<uint8_t>(std::min(255.0, 256 * color.g * msaaCoverage));
        result.b = static_cast<uint8_t>(std::min(255.0, 256 * color.b * msaaCoverage));
        result.a = static_cast<uint8_t>(std::min(255.0, 256 * color.a * msaaCoverage));
        return result;
    }
};

// Test using one multisampled color attachment with resolve target can render correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DTexture) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        std::array<float, 4> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a};
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test that a single-layer multisampled texture view can be created and resolved from.
TEST_P(MultisampledRenderingTest, ResolveFromSingleLayerArrayInto2DTexture) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        wgpu::TextureViewDescriptor desc = {};
        desc.dimension = wgpu::TextureViewDimension::e2DArray;
        desc.arrayLayerCount = 1;

        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorTexture.CreateView(&desc)}, {mResolveView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear, kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
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
        std::array<float, 5> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
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

        std::array<float, 5> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
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

    // In first render pass we draw a green triangle and do not set the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
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
    // TODO(dawn:462): Investigate backend validation failure.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsNvidia() && IsBackendValidationEnabled());

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
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

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
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

    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);

    // In first render pass we draw a green triangle and specify mResolveView as the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
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
    // TODO(dawn:462): Investigate backend validation failure.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kBaseMipLevel = 2;

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.format = kColorFormat;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseMipLevel = kBaseMipLevel;

    wgpu::Texture resolveTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel + 1, 1);
    wgpu::TextureView resolveView = resolveTexture.CreateView(&textureViewDescriptor);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a green triangle and do MSAA resolve.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {resolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, resolveTexture, kBaseMipLevel, 0);
}

// Test using a level or a layer of a 2D array texture as resolve target works correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DArrayTexture) {
    // TODO(dawn:462): Investigate backend validation failure.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();

    wgpu::TextureViewDescriptor baseTextureViewDescriptor;
    baseTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    baseTextureViewDescriptor.format = kColorFormat;
    baseTextureViewDescriptor.arrayLayerCount = 1;
    baseTextureViewDescriptor.mipLevelCount = 1;

    // Create resolveTexture1 with only 1 mipmap level.
    constexpr uint32_t kBaseArrayLayer1 = 2;
    constexpr uint32_t kBaseMipLevel1 = 0;
    wgpu::Texture resolveTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel1 + 1, kBaseArrayLayer1 + 1);
    wgpu::TextureViewDescriptor resolveViewDescriptor1 = baseTextureViewDescriptor;
    resolveViewDescriptor1.baseArrayLayer = kBaseArrayLayer1;
    resolveViewDescriptor1.baseMipLevel = kBaseMipLevel1;
    wgpu::TextureView resolveView1 = resolveTexture1.CreateView(&resolveViewDescriptor1);

    // Create resolveTexture2 with (kBaseMipLevel2 + 1) mipmap levels and resolve into its last
    // mipmap level.
    constexpr uint32_t kBaseArrayLayer2 = 5;
    constexpr uint32_t kBaseMipLevel2 = 3;
    wgpu::Texture resolveTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel2 + 1, kBaseArrayLayer2 + 1);
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

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, resolveTexture1, kBaseMipLevel1, kBaseArrayLayer1);
    VerifyResolveTarget(kGreen, resolveTexture2, kBaseMipLevel2, kBaseArrayLayer2);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with a non-default sample mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithSampleMask) {
    constexpr bool kTestDepth = false;
    // The second and third samples are included,
    // only the second one is covered by the triangle.
    constexpr uint32_t kSampleMask = kSecondSampleMaskBit | kThirdSampleMaskBit;
    constexpr float kMSAACoverage = 0.25f;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with the final sample mask empty.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithEmptyFinalSampleMask) {
    constexpr bool kTestDepth = false;
    // The third and fourth samples are included,
    // none of which is covered by the triangle.
    constexpr uint32_t kSampleMask = kThirdSampleMaskBit | kFourthSampleMaskBit;
    constexpr float kMSAACoverage = 0.00f;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test doing MSAA resolve into multiple resolve targets works correctly with a non-default sample
// mask.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithSampleMask) {
    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    // The first and fourth samples are included,
    // only the first one is covered by the triangle.
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kFourthSampleMaskBit;
    constexpr float kMSAACoverage = 0.25f;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, mResolveTexture, 0, 0, kMSAACoverage);
    VerifyResolveTarget(kGreen, resolveTexture2, 0, 0, kMSAACoverage);
}

// Test multisampled rendering with depth test works correctly with a non-default sample mask.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTestAndSampleMask) {
    constexpr bool kTestDepth = true;
    // The second sample is included in the first render pass and it's covered by the triangle.
    constexpr uint32_t kSampleMaskGreen = kSecondSampleMaskBit;
    // The first and second samples are included in the second render pass,
    // both are covered by the triangle.
    constexpr uint32_t kSampleMaskRed = kFirstSampleMaskBit | kSecondSampleMaskBit;
    constexpr float kMSAACoverage = 0.50f;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipelineGreen =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMaskGreen);
    wgpu::RenderPipeline pipelineRed =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMaskRed);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    // We will only write to the second sample.
    {
        utils::ComboRenderPassDescriptor renderPass =
            CreateComboRenderPassDescriptorForTest({mMultisampledColorView}, {mResolveView},
                                                   wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, true);
        std::array<float, 5> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineGreen, kUniformData.data(),
                                kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // We will only write to the first sample, since the second one is red with a smaller depth
    // value.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 5> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineRed, kUniformData.data(),
                                kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    constexpr wgpu::Color kHalfGreenHalfRed = {(kGreen.r + kRed.r) / 2.0, (kGreen.g + kRed.g) / 2.0,
                                               (kGreen.b + kRed.b) / 2.0,
                                               (kGreen.a + kRed.a) / 2.0};

    // The color of the pixel in the middle of mResolveTexture should be half green and half
    // red if MSAA resolve runs correctly with depth test.
    VerifyResolveTarget(kHalfGreenHalfRed, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with non-default sample mask and shader-output mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithSampleMaskAndShaderOutputMask) {
    // TODO(crbug.com/tint/372): Support sample mask builtin.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator"));

    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    // The second and third samples are included in the shader-output mask.
    // The first and third samples are included in the sample mask.
    // Since we're now looking at a fully covered pixel, the rasterization mask
    // includes all the samples.
    // Thus the final mask includes only the third sample.
    constexpr float kMSAACoverage = 0.25f;
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kThirdSampleMaskBit;
    const char* fs =
        R"(#version 450
        layout(location = 0) out vec4 fragColor;
        layout (std140, set = 0, binding = 0) uniform uBuffer {
            vec4 color;
        };
        void main() {
            fragColor = color;
            gl_SampleMask[0] = 6;
        })";

    wgpu::RenderPipeline pipeline = CreateRenderPipelineForTest(fs, 1, false, kSampleMask);
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    RGBA8 expectedColor = ExpectedMSAAColor(kGreen, kMSAACoverage);
    EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, mResolveTexture, 1, 0, 1, 1, 0, 0);
}

// Test doing MSAA resolve into multiple resolve targets works correctly with a non-default
// shader-output mask.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithShaderOutputMask) {
    // TODO(crbug.com/tint/372): Support sample mask builtin.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    // The second and third samples are included in the shader-output mask,
    // only the first one is covered by the triangle.
    constexpr float kMSAACoverage = 0.25f;
    const char* fs =
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
            gl_SampleMask[0] = 6;
        })";

    wgpu::RenderPipeline pipeline = CreateRenderPipelineForTest(fs, 2, false);
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, mResolveTexture, 0, 0, kMSAACoverage);
    VerifyResolveTarget(kGreen, resolveTexture2, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverage) {
    constexpr bool kTestDepth = false;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr bool kAlphaToCoverageEnabled = true;

    // Setting alpha <= 0 must result in alpha-to-coverage mask being empty.
    // Setting alpha = 0.5f should result in alpha-to-coverage mask including half the samples,
    // but this is not guaranteed by the spec. The Metal spec seems to guarantee that this is
    // indeed the case.
    // Setting alpha >= 1 must result in alpha-to-coverage mask being full.
    for (float alpha : {-1.0f, 0.0f, 0.5f, 1.0f, 2.0f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        // For alpha = {0, 0.5, 1} we expect msaaCoverage to correspond to the value of alpha.
        float msaaCoverage = alpha;
        if (alpha < 0.0f) {
            msaaCoverage = 0.0f;
        }
        if (alpha > 1.0f) {
            msaaCoverage = 1.0f;
        }

        RGBA8 expectedColor = ExpectedMSAAColor(kGreen, msaaCoverage);
        EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, mResolveTexture, 1, 0, 1, 1, 0, 0);
    }
}

// Test doing MSAA resolve into multiple resolve targets works correctly with
// alphaToCoverage. The alphaToCoverage mask is computed based on the alpha
// component of the first color output attachment.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithAlphaToCoverage) {
    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr float kMSAACoverage = 0.50f;
    constexpr bool kAlphaToCoverageEnabled = true;

    // The alpha-to-coverage mask should not depend on the alpha component of the
    // second color output attachment.
    // We test alpha = 0.51f and 0.99f instead of 0.50f and 1.00f because there are some rounding
    // differences on QuadroP400 devices in that case.
    for (float alpha : {0.0f, 0.51f, 0.99f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline =
            CreateRenderPipelineWithTwoOutputsForTest(kSampleMask, kAlphaToCoverageEnabled);

        constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.51f};
        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha};
        constexpr bool kTestDepth = false;

        // Draw a red triangle to the first color attachment, and a blue triangle to the second
        // color attachment, and do MSAA resolve on two render targets in one render pass.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
                wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

            std::array<float, 8> kUniformData = {
                static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
                static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
                static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
                static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
            constexpr uint32_t kSize = sizeof(kUniformData);
            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(),
                                    kSize);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Alpha to coverage affects both the color outputs, but the mask is computed
        // using only the first one.
        RGBA8 expectedRed = ExpectedMSAAColor(kRed, kMSAACoverage);
        RGBA8 expectedGreen = ExpectedMSAAColor(kGreen, kMSAACoverage);
        EXPECT_TEXTURE_RGBA8_EQ(&expectedRed, mResolveTexture, 1, 0, 1, 1, 0, 0);
        EXPECT_TEXTURE_RGBA8_EQ(&expectedGreen, resolveTexture2, 1, 0, 1, 1, 0, 0);
    }
}

// Test multisampled rendering with depth test works correctly with alphaToCoverage.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTestAndAlphaToCoverage) {
    // This test fails because Swiftshader is off-by-one with its ((a+b)/2 + (c+d)/2)/2 fast resolve
    // algorithm.
    DAWN_SKIP_TEST_IF(IsSwiftshader());

    constexpr bool kTestDepth = true;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipelineGreen =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask, true);
    wgpu::RenderPipeline pipelineRed =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask, false);

    // We test alpha = 0.51f and 0.81f instead of 0.50f and 0.80f because there are some
    // rounding differences on QuadroP400 devices in that case.
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.51f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.81f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    // We will only write to half the samples since the alphaToCoverage mode
    // is enabled for that render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        std::array<float, 5> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineGreen, kUniformData.data(),
                                kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // We will write to all the samples since the alphaToCoverageMode is diabled for
    // that render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 5> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineRed, kUniformData.data(),
                                kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    constexpr wgpu::Color kHalfGreenHalfRed = {(kGreen.r + kRed.r) / 2.0, (kGreen.g + kRed.g) / 2.0,
                                               (kGreen.b + kRed.b) / 2.0,
                                               (kGreen.a + kRed.a) / 2.0};
    RGBA8 expectedColor = ExpectedMSAAColor(kHalfGreenHalfRed, 1.0f);

    EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, mResolveTexture, 1, 0, 1, 1, 0, 0);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled and a sample mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverageAndSampleMask) {
    // This test fails because Swiftshader is off-by-one with its ((a+b)/2 + (c+d)/2)/2 fast resolve
    // algorithm.
    DAWN_SKIP_TEST_IF(IsSwiftshader());

    // TODO(dawn:491): This doesn't work on Metal, because we're using both the shader-output
    // mask (emulting the sampleMask from RenderPipeline) and alpha-to-coverage at the same
    // time. See the issue: https://github.com/gpuweb/gpuweb/issues/959.
    DAWN_SKIP_TEST_IF(IsMetal());

    constexpr bool kTestDepth = false;
    constexpr float kMSAACoverage = 0.50f;
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kThirdSampleMaskBit;
    constexpr bool kAlphaToCoverageEnabled = true;

    // For those values of alpha we expect the proportion of samples to be covered
    // to correspond to the value of alpha.
    // We're assuming in the case of alpha = 0.50f that the implementation
    // dependendent algorithm will choose exactly one of the first and third samples.
    for (float alpha : {0.0f, 0.50f, 1.00f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha - 0.01f};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        RGBA8 expectedColor = ExpectedMSAAColor(kGreen, kMSAACoverage * alpha);
        EXPECT_TEXTURE_RGBA8_EQ(&expectedColor, mResolveTexture, 1, 0, 1, 1, 0, 0);
    }
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled and a rasterization mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverageAndRasterizationMask) {
    // This test fails because Swiftshader is off-by-one with its ((a+b)/2 + (c+d)/2)/2 fast resolve
    // algorithm.
    DAWN_SKIP_TEST_IF(IsSwiftshader());

    constexpr bool kTestDepth = false;
    constexpr float kMSAACoverage = 0.50f;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr bool kAlphaToCoverageEnabled = true;
    constexpr bool kFlipTriangle = true;

    // For those values of alpha we expect the proportion of samples to be covered
    // to correspond to the value of alpha.
    // We're assuming in the case of alpha = 0.50f that the implementation
    // dependendent algorithm will choose exactly one of the samples covered by the
    // triangle.
    for (float alpha : {0.0f, 0.50f, 1.00f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled, kFlipTriangle);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha - 0.01f};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage * alpha);
    }
}

DAWN_INSTANTIATE_TEST(MultisampledRenderingTest,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));
