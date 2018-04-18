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

constexpr uint32_t kRTSize = 4;

class DrawElementsTest : public NXTTest {
    protected:
        void SetUp() override {
            NXTTest::SetUp();

            renderpass = device.CreateRenderPassBuilder()
                .SetAttachmentCount(1)
                .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
                .AttachmentSetColorLoadOp(0, nxt::LoadOp::Clear)
                .SetSubpassCount(1)
                .SubpassSetColorAttachment(0, 0, 0)
                .GetResult();

            renderTarget = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(kRTSize, kRTSize, 1)
                .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::TransferSrc)
                .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
                .GetResult();

            renderTargetView = renderTarget.CreateTextureViewBuilder().GetResult();

            framebuffer = device.CreateFramebufferBuilder()
                .SetRenderPass(renderpass)
                .SetAttachment(0, renderTargetView)
                .SetDimensions(kRTSize, kRTSize)
                .GetResult();

            nxt::InputState inputState = device.CreateInputStateBuilder()
                .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
                .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
                .GetResult();

            nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
                #version 450
                layout(location = 0) in vec4 pos;
                void main() {
                    gl_Position = pos;
                })"
            );

            nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })"
            );

            pipeline = device.CreateRenderPipelineBuilder()
                .SetSubpass(renderpass, 0)
                .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleStrip)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetIndexFormat(nxt::IndexFormat::Uint32)
                .SetInputState(inputState)
                .GetResult();

            vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
                -1.0f, -1.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, 0.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f
            });
            indexBuffer = utils::CreateFrozenBufferFromData<uint32_t>(device, nxt::BufferUsageBit::Index, {
                0, 1, 2, 0, 3, 1
            });
        }

        nxt::RenderPass renderpass;
        nxt::Texture renderTarget;
        nxt::TextureView renderTargetView;
        nxt::Framebuffer framebuffer;
        nxt::RenderPipeline pipeline;
        nxt::Buffer vertexBuffer;
        nxt::Buffer indexBuffer;

        void Test(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                  uint32_t firstInstance, RGBA8 bottomLeftExpected, RGBA8 topRightExpected) {
            uint32_t zeroOffset = 0;
            nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
                .BeginRenderPass(renderpass, framebuffer)
                .BeginRenderSubpass()
                    .SetRenderPipeline(pipeline)
                    .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
                    .SetIndexBuffer(indexBuffer, 0)
                    .DrawElements(indexCount, instanceCount, firstIndex, firstInstance)
                .EndRenderSubpass()
                .EndRenderPass()
                .GetResult();

            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderTarget, 1, 3);
            EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderTarget, 3, 1);
        }
};

// The most basic DrawElements triangle draw.
TEST_P(DrawElementsTest, Uint32) {

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with no indices.
    Test(0, 0, 0, 0, notFilled, notFilled);
    // Test a draw with only the first 3 indices (bottom left triangle)
    Test(3, 1, 0, 0, filled, notFilled);
    // Test a draw with only the last 3 indices (top right triangle)
    Test(3, 1, 3, 0, notFilled, filled);
    // Test a draw with all 6 indices (both triangles).
    Test(6, 1, 0, 0, filled, filled);
}

NXT_INSTANTIATE_TEST(DrawElementsTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
