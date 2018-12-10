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

#include <array>

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class VertexBufferValidationTest : public ValidationTest {
    protected:
        void SetUp() override {
            ValidationTest::SetUp();

            renderpass = CreateSimpleRenderPass();

            fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })");
        }

        template <unsigned int N>
        std::array<dawn::Buffer, N> MakeVertexBuffers() {
            std::array<dawn::Buffer, N> buffers;
            for (auto& buffer : buffers) {
                dawn::BufferDescriptor descriptor;
                descriptor.size = 256;
                descriptor.usage = dawn::BufferUsageBit::Vertex;

                buffer = device.CreateBuffer(&descriptor);
            }
            return buffers;
        }

        dawn::ShaderModule MakeVertexShader(unsigned int numInputs) {
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

            return utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vs.str().c_str());
        }

        dawn::InputState MakeInputState(unsigned int numInputs) {
            auto builder = device.CreateInputStateBuilder();
            for (unsigned int i = 0; i < numInputs; ++i) {
                builder.SetAttribute(i, i, dawn::VertexFormat::FloatR32G32B32, 0);
                builder.SetInput(i, 0, dawn::InputStepMode::Vertex);
            }
            return builder.GetResult();
        }

        dawn::RenderPipeline MakeRenderPipeline(const dawn::ShaderModule& vsModule, const dawn::InputState& inputState) {

            utils::ComboRenderPipelineDescriptor descriptor(device);
            descriptor.cVertexStage.module = vsModule;
            descriptor.cFragmentStage.module = fsModule;
            descriptor.inputState = inputState;

            return device.CreateRenderPipeline(&descriptor);
        }

        dawn::RenderPassDescriptor renderpass;
        dawn::ShaderModule fsModule;
};

TEST_F(VertexBufferValidationTest, VertexInputsInheritedBetweenPipelines) {
    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto inputState2 = MakeInputState(2);
    auto inputState1 = MakeInputState(1);

    auto pipeline2 = MakeRenderPipeline(vsModule2, inputState2);
    auto pipeline1 = MakeRenderPipeline(vsModule1, inputState1);

    auto vertexBuffers = MakeVertexBuffers<2>();
    uint32_t offsets[] = { 0, 0 };

    // Check failure when vertex buffer is not set
    dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    builder.GetResult();

    // Check success when vertex buffer is inherited from previous pipeline
    builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline2);
        pass.SetVertexBuffers(0, 2, vertexBuffers.data(), offsets);
        pass.Draw(3, 1, 0, 0);
        pass.SetRenderPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    builder.GetResult();
}

TEST_F(VertexBufferValidationTest, VertexInputsNotInheritedBetweenRendePasses) {
    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto inputState2 = MakeInputState(2);
    auto inputState1 = MakeInputState(1);

    auto pipeline2 = MakeRenderPipeline(vsModule2, inputState2);
    auto pipeline1 = MakeRenderPipeline(vsModule1, inputState1);

    auto vertexBuffers = MakeVertexBuffers<2>();
    uint32_t offsets[] = { 0, 0 };

    // Check success when vertex buffer is set for each render pass
    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline2);
        pass.SetVertexBuffers(0, 2, vertexBuffers.data(), offsets);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline1);
        pass.SetVertexBuffers(0, 1, vertexBuffers.data(), offsets);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    builder.GetResult();

    // Check failure because vertex buffer is not inherited in second subpass
    builder = AssertWillBeError(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline2);
        pass.SetVertexBuffers(0, 2, vertexBuffers.data(), offsets);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.SetRenderPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    builder.GetResult();
}
