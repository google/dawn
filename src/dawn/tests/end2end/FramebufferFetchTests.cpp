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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class FramebufferFetchTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::FramebufferFetch));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::FramebufferFetch})) {
            requiredFeatures.push_back(wgpu::FeatureName::FramebufferFetch);
        }
        return requiredFeatures;
    }
};

// A basic test of framebuffer fetch to help get folks started before a more complete set of tests
// is added.
TEST_P(FramebufferFetchTests, Basic) {
    // A shader using framebuffer fetch.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_framebuffer_fetch;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @fragment fn fs(@color(0) in : u32) -> @location(0) u32 {
            return in + 1;
        }
    )");

    // The pipeline using the shader, drawing points at 0, 0.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.vertex.entryPoint = "vs";
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.entryPoint = "fs";
    pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Our render target.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    tDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Draw 10 points with the pipeline on the render target.
    utils::ComboRenderPassDescriptor passDesc({texture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(10);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The 10 points should have successfully used framebuffer fetch to do increment ten times
    // without races.
    EXPECT_TEXTURE_EQ(uint32_t(10), texture, {0, 0});
}

// TODO(dawn:2195): Make a test plan.

DAWN_INSTANTIATE_TEST(FramebufferFetchTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn
