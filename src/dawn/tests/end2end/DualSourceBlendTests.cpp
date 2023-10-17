// Copyright 2023 The Dawn & Tint Authors
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

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static unsigned int kRTSize = 1;

class DualSourceBlendTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::DualSourceBlending));

        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
        pipelineLayout = utils::MakePipelineLayout(device, {bindGroupLayout});

        vsModule = utils::CreateShaderModule(device, R"(
                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                    var pos = array(
                        vec2f(-1.0, -1.0),
                        vec2f(3.0, -1.0),
                        vec2f(-1.0, 3.0));
                    return vec4f(pos[VertexIndex], 0.0, 1.0);
                }
            )");

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::DualSourceBlending})) {
            requiredFeatures.push_back(wgpu::FeatureName::DualSourceBlending);
        }
        return requiredFeatures;
    }

    struct TestParams {
        wgpu::BlendFactor srcBlendFactor;
        wgpu::BlendFactor dstBlendFactor;
        utils::RGBA8 baseColor;
        utils::RGBA8 testColorIndex0;
        utils::RGBA8 testColorIndex1;
    };

    void RunTest(TestParams params, const utils::RGBA8& expectation) {
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                enable chromium_internal_dual_source_blending;

                struct TestData {
                    color : vec4f,
                    blend : vec4f
                }

                @group(0) @binding(0) var<uniform> testData : TestData;

                struct FragOut {
                  @location(0) @index(0) color : vec4<f32>,
                  @location(0) @index(1) blend : vec4<f32>,
                }

                @fragment fn main() -> FragOut {
                  var output : FragOut;
                  output.color = testData.color;
                  output.blend = testData.blend;
                  return output;
                }
            )");

        wgpu::BlendComponent blendComponent;
        blendComponent.operation = wgpu::BlendOperation::Add;
        blendComponent.srcFactor = params.srcBlendFactor;
        blendComponent.dstFactor = params.dstBlendFactor;

        wgpu::BlendState blend;
        blend.color = blendComponent;
        blend.alpha = blendComponent;

        wgpu::ColorTargetState colorTargetState;
        colorTargetState.blend = &blend;

        utils::ComboRenderPipelineDescriptor baseDescriptor;
        baseDescriptor.layout = pipelineLayout;
        baseDescriptor.vertex.module = vsModule;
        baseDescriptor.cFragment.module = fsModule;
        baseDescriptor.cTargets[0].format = renderPass.colorFormat;

        basePipeline = device.CreateRenderPipeline(&baseDescriptor);

        utils::ComboRenderPipelineDescriptor testDescriptor;
        testDescriptor.layout = pipelineLayout;
        testDescriptor.vertex.module = vsModule;
        testDescriptor.cFragment.module = fsModule;
        testDescriptor.cTargets[0] = colorTargetState;
        testDescriptor.cTargets[0].format = renderPass.colorFormat;

        testPipeline = device.CreateRenderPipeline(&testDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            // First use the base pipeline to draw a triangle with no blending
            pass.SetPipeline(basePipeline);
            wgpu::BindGroup baseColors = MakeBindGroupForColors(
                std::array<utils::RGBA8, 2>({{params.baseColor, params.baseColor}}));
            pass.SetBindGroup(0, baseColors);
            pass.Draw(3);

            // Then use the test pipeline to draw the test triangle with blending
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(
                0, MakeBindGroupForColors({params.testColorIndex0, params.testColorIndex1}));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectationMinusOne = utils::RGBA8(expectation.r - 1, expectation.g - 1,
                                                        expectation.b - 1, expectation.a - 1);
        EXPECT_PIXEL_RGBA8_BETWEEN(expectation, expectationMinusOne, renderPass.color, kRTSize / 2,
                                   kRTSize / 2);
    }

    // Create a bind group to set the colors as a uniform buffer
    wgpu::BindGroup MakeBindGroupForColors(std::array<utils::RGBA8, 2> colors) {
        std::array<float, 16> data;
        for (unsigned int i = 0; i < 2; ++i) {
            data[4 * i + 0] = static_cast<float>(colors[i].r) / 255.f;
            data[4 * i + 1] = static_cast<float>(colors[i].g) / 255.f;
            data[4 * i + 2] = static_cast<float>(colors[i].b) / 255.f;
            data[4 * i + 3] = static_cast<float>(colors[i].a) / 255.f;
        }

        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, &data, sizeof(data), wgpu::BufferUsage::Uniform);
        return utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                    {{0, buffer, 0, sizeof(data)}});
    }

    wgpu::PipelineLayout pipelineLayout;
    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline basePipeline;
    wgpu::RenderPipeline testPipeline;
    wgpu::ShaderModule vsModule;
};

// Test that Src and Src1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorSrc1) {
    // Test source blend factor with source index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::Src;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params, utils::RGBA8(39, 88, 157, 245));

    // Test source blend factor with source index 1
    params.srcBlendFactor = wgpu::BlendFactor::Src1;
    RunTest(params, utils::RGBA8(13, 38, 75, 125));

    // Test destination blend factor with source index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::Src;
    RunTest(params, utils::RGBA8(39, 88, 157, 245));

    // Test destination blend factor with source index 1
    params.dstBlendFactor = wgpu::BlendFactor::Src1;
    RunTest(params, utils::RGBA8(13, 38, 75, 125));
}

// Test that SrcAlpha and SrcAlpha1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorSrc1Alpha) {
    // Test source blend factor with source alpha index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::SrcAlpha;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params, utils::RGBA8(98, 147, 196, 245));

    // Test source blend factor with source alpha index 1
    params.srcBlendFactor = wgpu::BlendFactor::Src1Alpha;
    RunTest(params, utils::RGBA8(50, 75, 100, 125));

    // Test destination blend factor with source alpha index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::SrcAlpha;
    RunTest(params, utils::RGBA8(98, 147, 196, 245));

    // Test destination blend factor with source alpha index 1
    params.dstBlendFactor = wgpu::BlendFactor::Src1Alpha;
    RunTest(params, utils::RGBA8(50, 75, 100, 125));
}

// Test that OneMinusSrc and OneMinusSrc1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorOneMinusSrc1) {
    // Test source blend factor with one minus source index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params, utils::RGBA8(61, 62, 43, 5));

    // Test source blend factor with one minus source index 1
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc1;
    RunTest(params, utils::RGBA8(87, 112, 125, 125));

    // Test destination blend factor with one minus source index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc;
    RunTest(params, utils::RGBA8(61, 62, 43, 5));

    // Test destination blend factor with one minus source index 1
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc1;
    RunTest(params, utils::RGBA8(87, 112, 125, 125));
}

// Test that OneMinusSrcAlpha and OneMinusSrc1Alpha BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorOneMinusSrc1Alpha) {
    // Test source blend factor with one minus source alpha index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 96);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 160);
    RunTest(params, utils::RGBA8(62, 94, 125, 60));

    // Test source blend factor with one minus source alpha index 1
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc1Alpha;
    RunTest(params, utils::RGBA8(37, 56, 75, 36));

    // Test destination blend factor with one minus source alpha index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    RunTest(params, utils::RGBA8(62, 94, 125, 156));

    // Test destination blend factor with one minus source alpha index 1
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc1Alpha;
    RunTest(params, utils::RGBA8(37, 56, 75, 93));
}

DAWN_INSTANTIATE_TEST(DualSourceBlendTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
