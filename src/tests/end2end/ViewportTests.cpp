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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class ViewportTest : public DawnTest {
  private:
    void SetUp() override {
        DawnTest::SetUp();

        mQuadVS = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            const pos : array<vec2<f32>, 6> = array<vec2<f32>, 6>(
                vec2<f32>(-1.0,  1.0),
                vec2<f32>(-1.0, -1.0),
                vec2<f32>( 1.0,  1.0),
                vec2<f32>( 1.0,  1.0),
                vec2<f32>(-1.0, -1.0),
                vec2<f32>( 1.0, -1.0));

            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        mQuadFS = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
            })");
    }

  protected:
    wgpu::ShaderModule mQuadVS;
    wgpu::ShaderModule mQuadFS;

    static constexpr uint32_t kWidth = 5;
    static constexpr uint32_t kHeight = 6;

    // Viewport parameters are float, but use uint32_t because implementations of Vulkan are allowed
    // to just discard the fractional part.
    void TestViewportQuad(uint32_t x,
                          uint32_t y,
                          uint32_t width,
                          uint32_t height,
                          bool doViewportCall = true) {
        // Create a pipeline that will draw a white quad.
        utils::ComboRenderPipelineDescriptor pipelineDesc(device);
        pipelineDesc.vertexStage.module = mQuadVS;
        pipelineDesc.cFragmentStage.module = mQuadFS;
        pipelineDesc.cColorStates[0].format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        // Render the quad with the viewport call.
        utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kWidth, kHeight);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetPipeline(pipeline);
        if (doViewportCall) {
            pass.SetViewport(x, y, width, height, 0.0, 1.0);
        }
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check that only the texels that are in the veiwport were drawn.
        for (uint32_t checkX = 0; checkX < kWidth; checkX++) {
            for (uint32_t checkY = 0; checkY < kHeight; checkY++) {
                if (checkX >= x && checkX < x + width && checkY >= y && checkY < y + height) {
                    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kWhite, rp.color, checkX, checkY);
                } else {
                    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, rp.color, checkX, checkY);
                }
            }
        }
    }

    void TestViewportDepth(float minDepth, float maxDepth, bool doViewportCall = true) {
        // Create a pipeline drawing 3 points at depth 1.0, 0.5 and 0.0.
        utils::ComboRenderPipelineDescriptor pipelineDesc(device);
        pipelineDesc.vertexStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            const points : array<vec3<f32>, 3> = array<vec3<f32>, 3>(
                vec3<f32>(-0.9, 0.0, 1.0),
                vec3<f32>( 0.0, 0.0, 0.5),
                vec3<f32>( 0.9, 0.0, 0.0));

            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(points[VertexIndex], 1.0);
            })");
        pipelineDesc.cFragmentStage.module = mQuadFS;
        pipelineDesc.colorStateCount = 0;
        pipelineDesc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        pipelineDesc.depthStencilState = &pipelineDesc.cDepthStencilState;
        pipelineDesc.cDepthStencilState.depthWriteEnabled = true;
        pipelineDesc.cDepthStencilState.format = wgpu::TextureFormat::Depth32Float;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        // Create the texture that will store the post-viewport-transform depth.
        wgpu::TextureDescriptor depthDesc;
        depthDesc.size = {3, 1, 1};
        depthDesc.format = wgpu::TextureFormat::Depth32Float;
        depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

        // Render the three points with the viewport call.
        utils::ComboRenderPassDescriptor rpDesc({}, depthTexture.CreateView());
        rpDesc.cDepthStencilAttachmentInfo.clearDepth = 0.0f;
        rpDesc.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.SetPipeline(pipeline);
        if (doViewportCall) {
            pass.SetViewport(0, 0, 3, 1, minDepth, maxDepth);
        }
        pass.Draw(3);
        pass.EndPass();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check that the viewport transform was computed correctly for the depth.
        std::vector<float> expected = {
            maxDepth,
            (maxDepth + minDepth) / 2,
            minDepth,
        };
        EXPECT_TEXTURE_EQ(expected.data(), depthTexture, 0, 0, 3, 1, 0, 0);
    }
};

// Test that by default the full viewport is used.
TEST_P(ViewportTest, DefaultViewportRect) {
    TestViewportQuad(0, 0, kWidth, kHeight, false);
}

// Test various viewport values in the X direction.
TEST_P(ViewportTest, VaryingInX) {
    TestViewportQuad(0, 0, kWidth - 1, kHeight);
    TestViewportQuad(1, 0, kWidth - 1, kHeight);
    TestViewportQuad(2, 0, 1, kHeight);
}

// Test various viewport values in the Y direction.
TEST_P(ViewportTest, VaryingInY) {
    TestViewportQuad(0, 0, kWidth, kHeight - 1);
    TestViewportQuad(0, 1, kWidth, kHeight - 1);
    TestViewportQuad(0, 2, kWidth, 1);
}

// Test various viewport values in both X and Y
TEST_P(ViewportTest, SubBoxes) {
    TestViewportQuad(1, 1, kWidth - 2, kHeight - 2);
    TestViewportQuad(2, 2, 2, 2);
    TestViewportQuad(2, 3, 2, 1);
}

// Test that by default the [0, 1] depth range is used.
TEST_P(ViewportTest, DefaultViewportDepth) {
    TestViewportDepth(0.0, 1.0, false);
}

// Test various viewport depth ranges
TEST_P(ViewportTest, ViewportDepth) {
    TestViewportDepth(0.0, 0.5);
    TestViewportDepth(0.5, 1.0);
}

// Test that a draw with an empty viewport doesn't draw anything.
TEST_P(ViewportTest, EmptyViewport) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
    pipelineDescriptor.cColorStates[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDescriptor.vertexStage.module = mQuadVS;
    pipelineDescriptor.cFragmentStage.module = mQuadFS;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto DoEmptyViewportTest = [&](uint32_t width, uint32_t height) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 0, 0);
    };

    // Test with a 0x0, 0xN and nx0 viewport.
    DoEmptyViewportTest(0, 0);
    DoEmptyViewportTest(0, 1);
    DoEmptyViewportTest(1, 0);
}

DAWN_INSTANTIATE_TEST(ViewportTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
