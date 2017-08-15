// Copyright 2017 The NXT Authors
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

#include <array>

constexpr static unsigned int kRTSize = 16;

class DrawQuad {
    public:
        DrawQuad() {}
        DrawQuad(nxt::Device* device, const char* vsSource, const char* fsSource)
            : device(device) {
                vsModule = utils::CreateShaderModule(*device, nxt::ShaderStage::Vertex, vsSource);
                fsModule = utils::CreateShaderModule(*device, nxt::ShaderStage::Fragment, fsSource);

                pipelineLayout = device->CreatePipelineLayoutBuilder()
                    .GetResult();
            }

        void Draw(const nxt::RenderPass& renderpass, nxt::CommandBufferBuilder* builder) {
            auto renderPipeline = device->CreateRenderPipelineBuilder()
                .SetSubpass(renderpass, 0)
                .SetLayout(pipelineLayout)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .GetResult();

            builder->SetRenderPipeline(renderPipeline);
            builder->DrawArrays(6, 1, 0, 0);
        }

    private:
        nxt::Device* device = nullptr;
        nxt::ShaderModule vsModule = {};
        nxt::ShaderModule fsModule = {};
        nxt::PipelineLayout pipelineLayout = {};
};

class RenderPassLoadOpTests : public NXTTest {
    protected:
        void SetUp() override {
            NXTTest::SetUp();

            renderTarget = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(kRTSize, kRTSize, 1)
                .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::TransferSrc)
                .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
                .GetResult();

            renderTargetView = renderTarget.CreateTextureViewBuilder().GetResult();

            RGBA8 zero(0, 0, 0, 0);
            std::fill(expectZero.begin(), expectZero.end(), zero);

            RGBA8 green(0, 255, 0, 255);
            std::fill(expectGreen.begin(), expectGreen.end(), green);

            RGBA8 blue(0, 0, 255, 255);
            std::fill(expectBlue.begin(), expectBlue.end(), blue);

            // draws a blue quad on the right half of the screen
            const char* vsSource = R"(
                #version 450
                void main() {
                    const vec2 pos[6] = vec2[6](
                        vec2(0, -1), vec2(1, -1), vec2(0, 1),
                        vec2(0,  1), vec2(1, -1), vec2(1, 1));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
                }
                )";
            const char* fsSource = R"(
                #version 450
                out vec4 color;
                void main() {
                    color = vec4(0.f, 0.f, 1.f, 1.f);
                }
                )";
            blueQuad = DrawQuad(&device, vsSource, fsSource);
        }

        nxt::Texture renderTarget;
        nxt::TextureView renderTargetView;

        std::array<RGBA8, kRTSize * kRTSize> expectZero;
        std::array<RGBA8, kRTSize * kRTSize> expectGreen;
        std::array<RGBA8, kRTSize * kRTSize> expectBlue;

        DrawQuad blueQuad = {};
};

// Tests clearing, loading, and drawing into color attachments
TEST_P(RenderPassLoadOpTests, ColorClearThenLoadAndDraw) {
    if (IsOpenGL()) {
        // TODO(kainino@chromium.org): currently fails on OpenGL backend
        return;
    }

    // Part 1: clear once, check to make sure it's cleared

    auto renderpass1 = device.CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .SetSubpassCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .AttachmentSetColorLoadOp(0, nxt::LoadOp::Clear)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
    auto framebuffer1 = device.CreateFramebufferBuilder()
        .SetRenderPass(renderpass1)
        .SetDimensions(kRTSize, kRTSize)
        .SetAttachment(0, renderTargetView)
        .GetResult();

    auto commands1 = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass1, framebuffer1)
        .BeginRenderSubpass()
            // Clear should occur implicitly
            // Store should occur implicitly
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    framebuffer1.AttachmentSetClearColor(0, 0.0f, 0.0f, 0.0f, 0.0f); // zero
    queue.Submit(1, &commands1);
    // Cleared to zero
    EXPECT_TEXTURE_RGBA8_EQ(expectZero.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0);

    framebuffer1.AttachmentSetClearColor(0, 0.0f, 1.0f, 0.0f, 1.0f); // green
    queue.Submit(1, &commands1);
    // Now cleared to green
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0);

    // Part 2: draw a blue quad into the right half of the render target, and check result

    auto renderpass2 = device.CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .SetSubpassCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .AttachmentSetColorLoadOp(0, nxt::LoadOp::Load)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
    auto framebuffer2 = device.CreateFramebufferBuilder()
        .SetRenderPass(renderpass2)
        .SetDimensions(kRTSize, kRTSize)
        .SetAttachment(0, renderTargetView)
        .GetResult();
    framebuffer2.AttachmentSetClearColor(0, 1.0f, 0.0f, 0.0f, 1.0f); // red

    nxt::CommandBuffer commands2;
    {
        auto builder = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderpass2, framebuffer2)
            .BeginRenderSubpass()
                // Clear should occur implicitly
            .Clone();
        blueQuad.Draw(renderpass2, &builder);
        commands2 = builder
                // Store should occur implicitly
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();
    }

    queue.Submit(1, &commands2);
    // Left half should still be green
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize / 2, kRTSize, 0);
    // Right half should now be blue
    EXPECT_TEXTURE_RGBA8_EQ(expectBlue.data(), renderTarget, kRTSize / 2, 0, kRTSize / 2, kRTSize, 0);
}

// Tests that an attachment is cleared only on the first subpass that uses it in a renderpass
TEST_P(RenderPassLoadOpTests, ClearsOnlyOnFirstUsePerRenderPass) {
    if (IsOpenGL()) {
        // TODO(kainino@chromium.org): currently fails on OpenGL backend
        return;
    }

    auto renderpass = device.CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .SetSubpassCount(2)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .AttachmentSetColorLoadOp(0, nxt::LoadOp::Clear)
        .SubpassSetColorAttachment(0, 0, 0)
        .SubpassSetColorAttachment(1, 0, 0)
        .GetResult();
    auto framebuffer = device.CreateFramebufferBuilder()
        .SetRenderPass(renderpass)
        .SetDimensions(kRTSize, kRTSize)
        .SetAttachment(0, renderTargetView)
        .GetResult();

    nxt::CommandBuffer commands;
    auto builder = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            // Clear should occur implicitly
        .Clone();
    blueQuad.Draw(renderpass, &builder);
    commands = builder
            // Store should occur implicitly
        .EndRenderSubpass()
        .BeginRenderSubpass()
            // Load (not clear!) should occur implicitly
            // Store should occur implicitly
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    framebuffer.AttachmentSetClearColor(0, 0.0f, 1.0f, 0.0f, 1.0f); // green
    queue.Submit(1, &commands);
    // Left half should still be green from the first clear
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize / 2, kRTSize, 0);
    // Right half should be blue, not cleared by the second subpass
    EXPECT_TEXTURE_RGBA8_EQ(expectBlue.data(), renderTarget, kRTSize / 2, 0, kRTSize / 2, kRTSize, 0);
}

NXT_INSTANTIATE_TEST(RenderPassLoadOpTests, D3D12Backend, MetalBackend, OpenGLBackend)
