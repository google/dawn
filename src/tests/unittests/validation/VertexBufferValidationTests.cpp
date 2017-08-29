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

#include <array>

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/NXTHelpers.h"

class VertexBufferValidationTest : public ValidationTest {
    protected:
        void SetUp() override {
            ValidationTest::SetUp();

            fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })");
        }

        void MakeRenderPassAndFrameBuffer(uint32_t subpassCount) {
            auto colorBuffer = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(640, 480, 1)
                .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
                .GetResult();
            colorBuffer.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);
            auto colorView = colorBuffer.CreateTextureViewBuilder()
                .GetResult();

            auto renderpassBuilder = device.CreateRenderPassBuilder();
                renderpassBuilder.SetAttachmentCount(1)
                .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
                .SetSubpassCount(subpassCount);

            for (uint32_t i = 0; i < subpassCount; ++i) {
                renderpassBuilder.SubpassSetColorAttachment(i, 0, 0);
            }

            renderpass = renderpassBuilder.GetResult();

            framebuffer = device.CreateFramebufferBuilder()
                .SetRenderPass(renderpass)
                .SetDimensions(640, 480)
                .SetAttachment(0, colorView)
                .GetResult();
        }

        template <unsigned int N>
        std::array<nxt::Buffer, N> MakeVertexBuffers() {
            std::array<nxt::Buffer, N> buffers;
            for (auto& buffer : buffers) {
                buffer = device.CreateBufferBuilder()
                    .SetSize(256)
                    .SetAllowedUsage(nxt::BufferUsageBit::Vertex)
                    .GetResult();
                buffer.FreezeUsage(nxt::BufferUsageBit::Vertex);
            }
            return buffers;
        }

        nxt::ShaderModule MakeVertexShader(unsigned int numInputs) {
            std::ostringstream vs;
            vs << "#version 450\n";
            for (unsigned int i = 0; i < numInputs; ++i) {
                vs << "layout(location = " << i << ") in vec3 a_position" << i << ";\n";
            }
            vs << "void main() {\n";

            vs << "gl_Position = vec4(";
            for (unsigned int i = 0; i < numInputs; ++i) {
                vs << "a_position" << i;
                if (i != numInputs - 1) {
                    vs << " + ";
                }
            }
            vs << ", 1.0);";

            vs << "}\n";

            return utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, vs.str().c_str());
        }

        nxt::InputState MakeInputState(unsigned int numInputs) {
            auto builder = device.CreateInputStateBuilder();
            for (unsigned int i = 0; i < numInputs; ++i) {
                builder.SetAttribute(i, i, nxt::VertexFormat::FloatR32G32B32, 0);
                builder.SetInput(i, 0, nxt::InputStepMode::Vertex);
            }
            return builder.GetResult();
        }

        nxt::RenderPipeline MakeRenderPipeline(uint32_t subpass, const nxt::ShaderModule& vsModule, const nxt::InputState& inputState) {
            return device.CreateRenderPipelineBuilder()
                .SetSubpass(renderpass, subpass)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetInputState(inputState)
                .GetResult();
        }

        nxt::RenderPass renderpass;
        nxt::Framebuffer framebuffer;
        nxt::ShaderModule fsModule;
};

TEST_F(VertexBufferValidationTest, VertexInputsInheritedBetweenPipelines) {
    MakeRenderPassAndFrameBuffer(1);

    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto inputState2 = MakeInputState(2);
    auto inputState1 = MakeInputState(1);

    auto pipeline2 = MakeRenderPipeline(0, vsModule2, inputState2);
    auto pipeline1 = MakeRenderPipeline(0, vsModule1, inputState1);

    auto vertexBuffers = MakeVertexBuffers<2>();
    uint32_t offsets[] = { 0, 0 };

    // Check failure when vertex buffer is not set
    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline1)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    // Check success when vertex buffer is inherited from previous pipeline
    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline2)
        .SetVertexBuffers(0, 2, vertexBuffers.data(), offsets)
        .DrawArrays(3, 1, 0, 0)
        .SetRenderPipeline(pipeline1)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}

TEST_F(VertexBufferValidationTest, VertexInputsNotInheritedBetweenSubpasses) {
    MakeRenderPassAndFrameBuffer(2);

    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto inputState2 = MakeInputState(2);
    auto inputState1 = MakeInputState(1);

    auto pipeline2 = MakeRenderPipeline(0, vsModule2, inputState2);
    auto pipeline1 = MakeRenderPipeline(1, vsModule1, inputState1);

    auto vertexBuffers = MakeVertexBuffers<2>();
    uint32_t offsets[] = { 0, 0 };

    // Check success when vertex buffer is set for each subpass
    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline2)
        .SetVertexBuffers(0, 2, vertexBuffers.data(), offsets)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline1)
        .SetVertexBuffers(0, 1, vertexBuffers.data(), offsets)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    // Check failure because vertex buffer is not inherited in second subpass
    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline2)
        .SetVertexBuffers(0, 2, vertexBuffers.data(), offsets)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline1)
        .DrawArrays(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}
