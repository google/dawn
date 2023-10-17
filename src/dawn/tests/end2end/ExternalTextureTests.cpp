// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

wgpu::Texture Create2DTexture(wgpu::Device device,
                              uint32_t width,
                              uint32_t height,
                              wgpu::TextureFormat format,
                              wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = usage;
    return device.CreateTexture(&descriptor);
}

class ExternalTextureTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var positions = array(
                    vec4f(-1.0, 1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0),
                    vec4f(1.0, -1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0)
                );
                return positions[VertexIndex];
            })");

        fsSampleExternalTextureModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var s : sampler;
            @group(0) @binding(1) var t : texture_external;

            @fragment fn main(@builtin(position) FragCoord : vec4f)
                                     -> @location(0) vec4f {
                return textureSampleBaseClampToEdge(t, s, FragCoord.xy / vec2f(4.0, 4.0));
            })");
    }

    wgpu::ExternalTextureDescriptor CreateDefaultExternalTextureDescriptor() {
        wgpu::ExternalTextureDescriptor desc;
        desc.yuvToRgbConversionMatrix = yuvBT709ToRGBSRGB.yuvToRgbConversionMatrix.data();
        desc.gamutConversionMatrix = yuvBT709ToRGBSRGB.gamutConversionMatrix.data();
        desc.srcTransferFunctionParameters = yuvBT709ToRGBSRGB.srcTransferFunctionParameters.data();
        desc.dstTransferFunctionParameters = yuvBT709ToRGBSRGB.dstTransferFunctionParameters.data();

        return desc;
    }

    void RenderToSourceTexture(wgpu::ShaderModule fragmentShader, wgpu::Texture texture) {
        {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fragmentShader;
            descriptor.cTargets[0].format = texture.GetFormat();
            wgpu::RenderPipeline pipeline1 = device.CreateRenderPipeline(&descriptor);

            wgpu::Sampler sampler = device.CreateSampler();

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TextureView renderView = texture.CreateView();
            utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            {
                pass.SetPipeline(pipeline1);
                pass.Draw(6);
                pass.End();
            }
            wgpu::CommandBuffer copy = encoder.Finish();
            queue.Submit(1, &copy);
        }
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::Norm16TextureFormats})) {
            mIsNorm16TextureFormatsSupported = true;
            requiredFeatures.push_back(wgpu::FeatureName::Norm16TextureFormats);
        }
        return requiredFeatures;
    }

    bool IsNorm16TextureFormatsSupported() { return mIsNorm16TextureFormatsSupported; }

    static constexpr uint32_t kWidth = 4;
    static constexpr uint32_t kHeight = 4;
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
    static constexpr wgpu::TextureUsage kSampledUsage = wgpu::TextureUsage::TextureBinding;
    utils::ColorSpaceConversionInfo yuvBT709ToRGBSRGB =
        utils::GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsSampleExternalTextureModule;

    bool mIsNorm16TextureFormatsSupported = false;
};

TEST_P(ExternalTextureTests, CreateExternalTextureSuccess) {
    wgpu::Texture texture = Create2DTexture(device, kWidth, kHeight, kFormat, kSampledUsage);

    // Create a texture view for the external texture
    wgpu::TextureView view = texture.CreateView();

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = view;
    externalDesc.visibleOrigin = {0, 0};
    externalDesc.visibleSize = {kWidth, kHeight};

    // Import the external texture
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    ASSERT_NE(externalTexture.Get(), nullptr);
}

TEST_P(ExternalTextureTests, SampleExternalTexture) {
    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalView = sampledTexture.CreateView();

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({externalView}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsSampleExternalTextureModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = externalView;
    externalDesc.visibleOrigin = {0, 0};
    externalDesc.visibleSize = {kWidth, kHeight};

    // Import the external texture
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a sampler and bind group
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, externalTexture}});

    // Run the shader, which should sample from the external texture and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
}

TEST_P(ExternalTextureTests, SampleMultiplanarExternalTexture) {
    wgpu::Texture sampledTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sampledTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalViewPlane0 = sampledTexturePlane0.CreateView();
    wgpu::TextureView externalViewPlane1 = sampledTexturePlane1.CreateView();

    struct ConversionExpectation {
        double y;
        double u;
        double v;
        utils::RGBA8 rgba;
    };

    // Conversion expectations for BT.709 YUV source and sRGB destination.
    std::array<ConversionExpectation, 7> expectations = {
        {{0.0, .5, .5, utils::RGBA8::kBlack},
         {0.2126, 0.4172, 1.0, utils::RGBA8::kRed},
         {0.7152, 0.1402, 0.0175, utils::RGBA8::kGreen},
         {0.0722, 1.0, 0.4937, utils::RGBA8::kBlue},
         {0.6382, 0.3232, 0.6644, {246, 169, 90, 255}},
         {0.5423, 0.5323, 0.4222, {120, 162, 169, 255}},
         {0.2345, 0.4383, 0.6342, {126, 53, 33, 255}}}};

    for (const ConversionExpectation& expectation : expectations) {
        // Initialize the texture planes with YUV data
        {
            utils::ComboRenderPassDescriptor renderPass({externalViewPlane0, externalViewPlane1},
                                                        nullptr);
            renderPass.cColorAttachments[0].clearValue = {expectation.y, 0.0f, 0.0f, 0.0f};
            renderPass.cColorAttachments[1].clearValue = {expectation.u, expectation.v, 0.0f, 0.0f};
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        // descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture views
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = externalViewPlane0;
        externalDesc.plane1 = externalViewPlane1;
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expectation.rgba, renderTexture, 0, 0);
    }
}

TEST_P(ExternalTextureTests, SampleMultiplanarExternalTextureNorm16) {
    DAWN_TEST_UNSUPPORTED_IF(!IsNorm16TextureFormatsSupported());

    wgpu::Texture sampledTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R16Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sampledTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG16Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalViewPlane0 = sampledTexturePlane0.CreateView();
    wgpu::TextureView externalViewPlane1 = sampledTexturePlane1.CreateView();

    struct ConversionExpectation {
        double y;
        double u;
        double v;
        utils::RGBA8 rgba;
    };

    // Conversion expectations for BT.709 YUV source and sRGB destination.
    std::array<ConversionExpectation, 7> expectations = {
        {{0.0, .5, .5, utils::RGBA8::kBlack},
         {0.2126, 0.4172, 1.0, utils::RGBA8::kRed},
         {0.7152, 0.1402, 0.0175, utils::RGBA8::kGreen},
         {0.0722, 1.0, 0.4937, utils::RGBA8::kBlue},
         {0.6382, 0.3232, 0.6644, {246, 169, 90, 255}},
         {0.5423, 0.5323, 0.4222, {120, 162, 169, 255}},
         {0.2345, 0.4383, 0.6342, {125, 53, 32, 255}}}};

    for (const ConversionExpectation& expectation : expectations) {
        // Initialize the texture planes with YUV data
        {
            utils::ComboRenderPassDescriptor renderPass({externalViewPlane0, externalViewPlane1},
                                                        nullptr);
            renderPass.cColorAttachments[0].clearValue = {expectation.y, 0.0f, 0.0f, 0.0f};
            renderPass.cColorAttachments[1].clearValue = {expectation.u, expectation.v, 0.0f, 0.0f};
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture views
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = externalViewPlane0;
        externalDesc.plane1 = externalViewPlane1;
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expectation.rgba, renderTexture, 0, 0);
    }
}

// Test draws a green square in the upper left quadrant, a black square in the upper right, a red
// square in the lower left and a blue square in the lower right. The image is then sampled as an
// external texture and rotated 0, 90, 180, and 270 degrees with and without the y-axis flipped.
TEST_P(ExternalTextureTests, RotateAndOrFlipSinglePlane) {
    const wgpu::ShaderModule sourceTextureFsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {
            if(FragCoord.y < 2.0 && FragCoord.x < 2.0) {
               return vec4f(0.0, 1.0, 0.0, 1.0);
            }

            if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
               return vec4f(0.0, 0.0, 1.0, 1.0);
            }

            if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
               return vec4f(0.0, 0.0, 0.0, 1.0);
            }

            return vec4f(1.0, 0.0, 0.0, 1.0);
        })");

    wgpu::Texture sourceTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    RenderToSourceTexture(sourceTextureFsModule, sourceTexture);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Control case to verify flipY and rotation defaults
    {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexture.CreateView();
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTexture, 3, 3);
    }

    struct RotationExpectation {
        wgpu::ExternalTextureRotation rotation;
        bool flipY;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<RotationExpectation, 8> expectations = {
        {{wgpu::ExternalTextureRotation::Rotate0Degrees, false, utils::RGBA8::kGreen,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, false, utils::RGBA8::kBlack,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, false, utils::RGBA8::kBlue,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kGreen},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, false, utils::RGBA8::kRed,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate0Degrees, true, utils::RGBA8::kRed,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, true, utils::RGBA8::kGreen,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, true, utils::RGBA8::kBlack,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, true, utils::RGBA8::kBlue,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kGreen}}};

    for (const RotationExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexture.CreateView();
        externalDesc.rotation = exp.rotation;
        externalDesc.flipY = exp.flipY;
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 2, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 1, 2);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 2, 2);
    }
}

// Test draws a green square in the upper left quadrant, a black square in the upper right, a red
// square in the lower left and a blue square in the lower right. The image is then sampled as an
// external texture and rotated 0, 90, 180, and 270 degrees with and without the y-axis flipped.
TEST_P(ExternalTextureTests, RotateAndOrFlipMultiplanar) {
    const wgpu::ShaderModule sourceTexturePlane0FsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {

            if(FragCoord.y < 2.0 && FragCoord.x < 2.0) {
               return vec4f(0.7152, 0.0, 0.0, 0.0);
            }

            if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
               return vec4f(0.0722, 0.0, 1.0, 1.0);
            }

            if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
               return vec4f(0.0, 0.0, 0.0, 0.0);
            }

            return vec4f(0.2126, 0.0, 0.0, 0.0);
        })");

    const wgpu::ShaderModule sourceTexturePlane1FsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {

            if(FragCoord.x < 2.0 && FragCoord.y < 2.0) {
               return vec4f(0.1402, 0.0175, 0.0, 0.0);
            }

            if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
               return vec4f(1.0, 0.4937, 0.0, 0.0);
            }

            if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
               return vec4f(0.5, 0.5, 0.0, 0.0);
            }

            return vec4f(0.4172, 1.0, 0.0, 0.0);
        })");

    wgpu::Texture sourceTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sourceTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    RenderToSourceTexture(sourceTexturePlane0FsModule, sourceTexturePlane0);
    RenderToSourceTexture(sourceTexturePlane1FsModule, sourceTexturePlane1);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Control case to verify flipY and rotation defaults
    {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexturePlane0.CreateView();
        externalDesc.plane1 = sourceTexturePlane1.CreateView();
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTexture, 3, 3);
    }

    struct RotationExpectation {
        wgpu::ExternalTextureRotation rotation;
        bool flipY;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<RotationExpectation, 8> expectations = {
        {{wgpu::ExternalTextureRotation::Rotate0Degrees, false, utils::RGBA8::kGreen,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, false, utils::RGBA8::kBlack,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, false, utils::RGBA8::kBlue,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kGreen},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, false, utils::RGBA8::kRed,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate0Degrees, true, utils::RGBA8::kRed,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, true, utils::RGBA8::kGreen,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, true, utils::RGBA8::kBlack,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, true, utils::RGBA8::kBlue,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kGreen}}};

    for (const RotationExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexturePlane0.CreateView();
        externalDesc.plane1 = sourceTexturePlane1.CreateView();
        externalDesc.rotation = exp.rotation;
        externalDesc.flipY = exp.flipY;
        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 2, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 1, 2);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 2, 2);
    }
}

// This test draws a 2x2 multi-colored square surrounded by a 1px black border. We test the external
// texture crop functionality by cropping to specific ranges inside the texture.
TEST_P(ExternalTextureTests, CropSinglePlane) {
    const wgpu::ShaderModule sourceTextureFsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {
            if(FragCoord.x >= 1.0 && FragCoord.x < 3.0 && FragCoord.y >= 1.0 && FragCoord.y < 3.0) {
                if(FragCoord.y < 2.0 && FragCoord.x < 2.0) {
                   return vec4f(0.0, 1.0, 0.0, 1.0);
                }

                if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
                   return vec4f(1.0, 1.0, 1.0, 1.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x < 2.0) {
                   return vec4f(1.0, 0.0, 0.0, 1.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
                   return vec4f(0.0, 0.0, 1.0, 1.0);
                }
            }

            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::Texture sourceTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    RenderToSourceTexture(sourceTextureFsModule, sourceTexture);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    struct CropExpectation {
        wgpu::Origin2D visibleOrigin;
        wgpu::Extent2D visibleSize;
        wgpu::ExternalTextureRotation rotation;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<CropExpectation, 9> expectations = {{
        {{0, 0},
         {kWidth, kHeight},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen},
        {{kWidth / 2, kHeight / 4},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kWhite,
         utils::RGBA8::kWhite,
         utils::RGBA8::kWhite,
         utils::RGBA8::kWhite},
        {{kWidth / 4, kHeight / 2},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed},
        {{kWidth / 2, kHeight / 2},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kGreen,
         utils::RGBA8::kWhite,
         utils::RGBA8::kRed,
         utils::RGBA8::kBlue},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate90Degrees,
         utils::RGBA8::kWhite,
         utils::RGBA8::kBlue,
         utils::RGBA8::kGreen,
         utils::RGBA8::kRed},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate180Degrees,
         utils::RGBA8::kBlue,
         utils::RGBA8::kRed,
         utils::RGBA8::kWhite,
         utils::RGBA8::kGreen},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate270Degrees,
         utils::RGBA8::kRed,
         utils::RGBA8::kGreen,
         utils::RGBA8::kBlue,
         utils::RGBA8::kWhite},
    }};

    for (const CropExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexture.CreateView();
        externalDesc.rotation = exp.rotation;
        externalDesc.visibleOrigin = exp.visibleOrigin;
        externalDesc.visibleSize = exp.visibleSize;

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
    }
}

// This test draws a 2x2 multi-colored square surrounded by a 1px black border. We test the external
// texture crop functionality by cropping to specific ranges inside the texture.
TEST_P(ExternalTextureTests, CropMultiplanar) {
    const wgpu::ShaderModule sourceTexturePlane0FsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {
            if(FragCoord.x >= 1.0 && FragCoord.x < 3.0 && FragCoord.y >= 1.0 && FragCoord.y < 3.0) {
                if(FragCoord.y < 2.0 && FragCoord.x < 2.0) {
                   return vec4f(0.7152, 0.0, 0.0, 0.0);
                }

                if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
                   return vec4f(1.0, 0.0, 0.0, 0.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x < 2.0) {
                   return vec4f(0.2126, 0.0, 0.0, 0.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
                   return vec4f(0.0722, 0.0, 1.0, 1.0);
                }
            }

            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");

    const wgpu::ShaderModule sourceTexturePlane1FsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> @location(0) vec4f {
            if(FragCoord.x >= 1.0 && FragCoord.x < 3.0 && FragCoord.y >= 1.0 && FragCoord.y < 3.0) {
                if(FragCoord.y < 2.0 && FragCoord.x < 2.0) {
                    return vec4f(0.1402, 0.0175, 0.0, 0.0);
                }

                if(FragCoord.y < 2.0 && FragCoord.x >= 2.0) {
                    return vec4f(0.5, 0.5, 0.0, 0.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x < 2.0) {
                    return vec4f(0.4172, 1.0, 0.0, 0.0);
                }

                if(FragCoord.y >= 2.0 && FragCoord.x >= 2.0) {
                    return vec4f(1.0, 0.4937, 0.0, 0.0);
                }
            }

            return vec4f(0.5, 0.5, 0.0, 0.0);
        })");

    wgpu::Texture sourceTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sourceTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    RenderToSourceTexture(sourceTexturePlane0FsModule, sourceTexturePlane0);
    RenderToSourceTexture(sourceTexturePlane1FsModule, sourceTexturePlane1);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    struct CropExpectation {
        wgpu::Origin2D visibleOrigin;
        wgpu::Extent2D visibleSize;
        wgpu::ExternalTextureRotation rotation;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<CropExpectation, 9> expectations = {
        {{{0, 0},
          {kWidth, kHeight},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen},
         {{kWidth / 2, kHeight / 4},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kWhite,
          utils::RGBA8::kWhite,
          utils::RGBA8::kWhite,
          utils::RGBA8::kWhite},
         {{kWidth / 4, kHeight / 2},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed},
         {{kWidth / 2, kHeight / 2},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kGreen,
          utils::RGBA8::kWhite,
          utils::RGBA8::kRed,
          utils::RGBA8::kBlue},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate90Degrees,
          utils::RGBA8::kWhite,
          utils::RGBA8::kBlue,
          utils::RGBA8::kGreen,
          utils::RGBA8::kRed},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate180Degrees,
          utils::RGBA8::kBlue,
          utils::RGBA8::kRed,
          utils::RGBA8::kWhite,
          utils::RGBA8::kGreen},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate270Degrees,
          utils::RGBA8::kRed,
          utils::RGBA8::kGreen,
          utils::RGBA8::kBlue,
          utils::RGBA8::kWhite}}};

    for (const CropExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = sourceTexturePlane0.CreateView();
        externalDesc.plane1 = sourceTexturePlane1.CreateView();
        externalDesc.rotation = exp.rotation;
        externalDesc.visibleOrigin = exp.visibleOrigin;
        externalDesc.visibleSize = exp.visibleSize;

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
    }
}

// Test that sampling an external texture with non-one alpha preserves the alpha channel.
TEST_P(ExternalTextureTests, SampleExternalTextureAlpha) {
    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalView = sampledTexture.CreateView();

    utils::RGBA8 kColor = {255, 255, 255, 128};

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({externalView}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {kColor.r / 255.0f, kColor.g / 255.0f,
                                                      kColor.b / 255.0f, kColor.a / 255.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsSampleExternalTextureModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = externalView;
    externalDesc.visibleOrigin = {0, 0};
    externalDesc.visibleSize = {kWidth, kHeight};

    // Import the external texture
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a sampler and bind group
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, externalTexture}});

    // Run the shader, which should sample from the external texture and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColor, renderTexture, 0, 0);
}

DAWN_INSTANTIATE_TEST(ExternalTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
