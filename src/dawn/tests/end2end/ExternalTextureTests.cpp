// Copyright 2021 The Dawn Authors
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
        static constexpr uint32_t kWidth = 4;
        static constexpr uint32_t kHeight = 4;
        static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
        static constexpr wgpu::TextureUsage kSampledUsage = wgpu::TextureUsage::TextureBinding;
    };
}  // anonymous namespace

TEST_P(ExternalTextureTests, CreateExternalTextureSuccess) {
    wgpu::Texture texture = Create2DTexture(device, kWidth, kHeight, kFormat, kSampledUsage);

    // Create a texture view for the external texture
    wgpu::TextureView view = texture.CreateView();

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = view;

    // Import the external texture
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    ASSERT_NE(externalTexture.Get(), nullptr);
}

TEST_P(ExternalTextureTests, SampleExternalTexture) {
    // TODO(crbug.com/dawn/1263): SPIR-V has an issue compiling the output from Tint's external
    // texture transform. Re-enable this test for OpenGL when the switch to Tint is complete.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    const wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
            var positions = array<vec4<f32>, 3>(
                vec4<f32>(-1.0, 1.0, 0.0, 1.0),
                vec4<f32>(-1.0, -1.0, 0.0, 1.0),
                vec4<f32>(1.0, 1.0, 0.0, 1.0)
            );
            return positions[VertexIndex];
        })");

    const wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var s : sampler;
        @group(0) @binding(1) var t : texture_external;

        @stage(fragment) fn main(@builtin(position) FragCoord : vec4<f32>)
                                 -> @location(0) vec4<f32> {
            return textureSampleLevel(t, s, FragCoord.xy / vec2<f32>(4.0, 4.0));
        })");

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
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = externalView;

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

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderTexture, 0, 0);
}

TEST_P(ExternalTextureTests, SampleMultiplanarExternalTexture) {
    // TODO(crbug.com/dawn/1263): SPIR-V has an issue compiling the output from Tint's external
    // texture transform. Re-enable this test for OpenGL when the switch to Tint is complete.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    const wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
            var positions = array<vec4<f32>, 3>(
                vec4<f32>(-1.0, 1.0, 0.0, 1.0),
                vec4<f32>(-1.0, -1.0, 0.0, 1.0),
                vec4<f32>(1.0, 1.0, 0.0, 1.0)
            );
            return positions[VertexIndex];
        })");

    const wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var s : sampler;
        @group(0) @binding(1) var t : texture_external;

        @stage(fragment) fn main(@builtin(position) FragCoord : vec4<f32>)
                                 -> @location(0) vec4<f32> {
            return textureSampleLevel(t, s, FragCoord.xy / vec2<f32>(4.0, 4.0));
        })");

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
        RGBA8 rgba;
    };

    // Conversion expectations for BT.709 YUV source and sRGB destination.
    std::array<ConversionExpectation, 7> expectations = {
        {{0.0, .5, .5, RGBA8::kBlack},
         {0.2126, 0.4172, 1.0, RGBA8::kRed},
         {0.7152, 0.1402, 0.0175, RGBA8::kGreen},
         {0.0722, 1.0, 0.4937, RGBA8::kBlue},
         {0.6382, 0.3232, 0.6644, {246, 169, 90, 255}},
         {0.5423, 0.5323, 0.4222, {120, 162, 169, 255}},
         {0.2345, 0.4383, 0.6342, {126, 53, 33, 255}}}};

    for (ConversionExpectation expectation : expectations) {
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
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture views
        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = externalViewPlane0;
        externalDesc.plane1 = externalViewPlane1;

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

DAWN_INSTANTIATE_TEST(ExternalTextureTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
