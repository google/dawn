// Copyright 2018 The NXT Authors
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

#include "tests/NXTTest.h"

#include "utils/NXTHelpers.h"

class ScissorTest: public NXTTest {
  protected:
    nxt::RenderPipeline CreateQuadPipeline(const nxt::RenderPass& renderPass) {
        nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
            #version 450
            const vec2 pos[6] = vec2[6](
                vec2(-1.0f, -1.0f), vec2(-1.0f, 1.0f), vec2(1.0f, -1.0f),
                vec2(1.0f, 1.0f), vec2(-1.0f, 1.0f), vec2(1.0f, -1.0f)
            );
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 0.5, 1.0);
            })");

        nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
            #version 450
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
            })");

        nxt::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
            .SetSubpass(renderPass, 0)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .GetResult();

        return pipeline;
    }
};

// Test that by default the scissor test is disabled and the whole attachment can be drawn to.
TEST_P(ScissorTest, DefaultsToWholeRenderTarget) {
    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 100, 100);
    nxt::RenderPipeline pipeline = CreateQuadPipeline(fb.renderPass);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .DrawArrays(6, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 99);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 99);
}

// Test setting the scissor to something larger than the attachments.
TEST_P(ScissorTest, LargerThanAttachment) {
    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 100, 100);
    nxt::RenderPipeline pipeline = CreateQuadPipeline(fb.renderPass);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .SetScissorRect(0, 0, 200, 200)
        .DrawArrays(6, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 99);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 99);
}

// Test setting an empty scissor rect
TEST_P(ScissorTest, EmptyRect) {
    if (IsMetal()) {
        std::cout << "Test skipped on Metal" << std::endl;
        return;
    }

    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 2, 2);
    nxt::RenderPipeline pipeline = CreateQuadPipeline(fb.renderPass);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .SetScissorRect(0, 0, 0, 0)
        .DrawArrays(6, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 0, 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 1, 1);
}

// Test setting a partial scissor (not empty, not full attachment)
TEST_P(ScissorTest, PartialRect) {
    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 100, 100);
    nxt::RenderPipeline pipeline = CreateQuadPipeline(fb.renderPass);

    constexpr uint32_t kX = 3;
    constexpr uint32_t kY = 7;
    constexpr uint32_t kW = 5;
    constexpr uint32_t kH = 13;


    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .SetScissorRect(kX, kY, kW, kH)
        .DrawArrays(6, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    // Test the two opposite corners of the scissor box. With one pixel inside and on outside
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, kX - 1, kY - 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, kX, kY);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, kX + kW, kY + kH);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, kX + kW - 1, kY + kH - 1);
}

// Test that the scissor setting doesn't get inherited between renderpasses
// TODO(cwallez@chromium.org): do the same between subpasses?
TEST_P(ScissorTest, NoInheritanceBetweenRenderPass) {
    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 100, 100);
    nxt::RenderPipeline pipeline = CreateQuadPipeline(fb.renderPass);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        // RenderPass 1 set the scissor
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetScissorRect(0, 0, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        // RenderPass 2 draw a full quad, it shouldn't be scissored
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .DrawArrays(6, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 99);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 99, 99);
}

NXT_INSTANTIATE_TEST(ScissorTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
