// Copyright 2017 The Dawn Authors
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

#include "utils/DawnHelpers.h"

#include <array>

constexpr static unsigned int kRTSize = 16;

class DrawQuad {
    public:
        DrawQuad() {}
        DrawQuad(dawn::Device* device, const char* vsSource, const char* fsSource)
            : device(device) {
                vsModule = utils::CreateShaderModule(*device, dawn::ShaderStage::Vertex, vsSource);
                fsModule = utils::CreateShaderModule(*device, dawn::ShaderStage::Fragment, fsSource);

                pipelineLayout = utils::MakeBasicPipelineLayout(*device, nullptr);
            }

        void Draw(dawn::RenderPassEncoder* pass) {
            auto renderPipeline = device->CreateRenderPipelineBuilder()
                .SetColorAttachmentFormat(0, dawn::TextureFormat::R8G8B8A8Unorm)
                .SetLayout(pipelineLayout)
                .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
                .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
                .GetResult();

            pass->SetRenderPipeline(renderPipeline);
            pass->DrawArrays(6, 1, 0, 0);
        }

    private:
        dawn::Device* device = nullptr;
        dawn::ShaderModule vsModule = {};
        dawn::ShaderModule fsModule = {};
        dawn::PipelineLayout pipelineLayout = {};
};

class RenderPassLoadOpTests : public DawnTest {
    protected:
        void SetUp() override {
            DawnTest::SetUp();

            dawn::TextureDescriptor descriptor;
            descriptor.dimension = dawn::TextureDimension::e2D;
            descriptor.size.width = kRTSize;
            descriptor.size.height = kRTSize;
            descriptor.size.depth = 1;
            descriptor.arrayLayer = 1;
            descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
            descriptor.levelCount = 1;
            descriptor.usage = dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::TransferSrc;
            renderTarget = device.CreateTexture(&descriptor);

            renderTargetView = renderTarget.CreateDefaultTextureView();

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
                layout(location = 0) out vec4 color;
                void main() {
                    color = vec4(0.f, 0.f, 1.f, 1.f);
                }
                )";
            blueQuad = DrawQuad(&device, vsSource, fsSource);
        }

        dawn::Texture renderTarget;
        dawn::TextureView renderTargetView;

        std::array<RGBA8, kRTSize * kRTSize> expectZero;
        std::array<RGBA8, kRTSize * kRTSize> expectGreen;
        std::array<RGBA8, kRTSize * kRTSize> expectBlue;

        DrawQuad blueQuad = {};
};

// Tests clearing, loading, and drawing into color attachments
TEST_P(RenderPassLoadOpTests, ColorClearThenLoadAndDraw) {

    // Part 1: clear once, check to make sure it's cleared

    auto renderPassClearZero = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, renderTargetView, dawn::LoadOp::Clear)
        .SetColorAttachmentClearColor(0, 0.0f, 0.0f, 0.0f, 0.0f)
        .GetResult();

    auto commandsClearZeroBuilder = device.CreateCommandBufferBuilder();
    auto clearZeroPass = commandsClearZeroBuilder.BeginRenderPass(renderPassClearZero);
    clearZeroPass.EndPass();
    auto commandsClearZero = commandsClearZeroBuilder.GetResult();

    auto renderPassClearGreen = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, renderTargetView, dawn::LoadOp::Clear)
        .SetColorAttachmentClearColor(0, 0.0f, 1.0f, 0.0f, 1.0f)
        .GetResult();

    auto commandsClearGreenBuilder = device.CreateCommandBufferBuilder();
    auto clearGreenPass = commandsClearGreenBuilder.BeginRenderPass(renderPassClearGreen);
    clearGreenPass.EndPass();
    auto commandsClearGreen = commandsClearGreenBuilder.GetResult();

    queue.Submit(1, &commandsClearZero);
    EXPECT_TEXTURE_RGBA8_EQ(expectZero.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0, 0);

    queue.Submit(1, &commandsClearGreen);
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0, 0);

    // Part 2: draw a blue quad into the right half of the render target, and check result

    auto renderPassLoad = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, renderTargetView, dawn::LoadOp::Load)
        .GetResult();

    dawn::CommandBuffer commandsLoad;
    {
        auto builder = device.CreateCommandBufferBuilder();
        auto pass = builder.BeginRenderPass(renderPassLoad);
        blueQuad.Draw(&pass);
        pass.EndPass();
        commandsLoad = builder.GetResult();
    }

    queue.Submit(1, &commandsLoad);
    // Left half should still be green
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize / 2, kRTSize, 0, 0);
    // Right half should now be blue
    EXPECT_TEXTURE_RGBA8_EQ(expectBlue.data(), renderTarget, kRTSize / 2, 0, kRTSize / 2, kRTSize, 0, 0);
}

DAWN_INSTANTIATE_TEST(RenderPassLoadOpTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
