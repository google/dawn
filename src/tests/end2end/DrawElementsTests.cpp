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

            renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

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
                .SetColorAttachmentFormat(0, renderPass.colorFormat)
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

        utils::BasicRenderPass renderPass;
        nxt::RenderPipeline pipeline;
        nxt::Buffer vertexBuffer;
        nxt::Buffer indexBuffer;

        void Test(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                  uint32_t firstInstance, RGBA8 bottomLeftExpected, RGBA8 topRightExpected) {
            uint32_t zeroOffset = 0;
            nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
                .BeginRenderPass(renderPass.renderPassInfo)
                    .SetRenderPipeline(pipeline)
                    .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
                    .SetIndexBuffer(indexBuffer, 0)
                    .DrawElements(indexCount, instanceCount, firstIndex, firstInstance)
                .EndRenderPass()
                .GetResult();

            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
            EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
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
