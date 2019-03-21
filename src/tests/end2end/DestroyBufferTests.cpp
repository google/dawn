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
#include "utils/DawnHelpers.h"

constexpr uint32_t kRTSize = 4;

class DestroyBufferTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        dawn::VertexInputDescriptor input;
        input.inputSlot = 0;
        input.stride = 4 * sizeof(float);
        input.stepMode = dawn::InputStepMode::Vertex;

        dawn::VertexAttributeDescriptor attribute;
        attribute.shaderLocation = 0;
        attribute.inputSlot = 0;
        attribute.offset = 0;
        attribute.format = dawn::VertexFormat::Float4;

        dawn::InputState inputState =
            device.CreateInputStateBuilder().SetInput(&input).SetAttribute(&attribute).GetResult();

        dawn::ShaderModule vsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
              #version 450
              layout(location = 0) in vec4 pos;
              void main() {
                  gl_Position = pos;
              })");

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
              #version 450
              layout(location = 0) out vec4 fragColor;
              void main() {
                  fragColor = vec4(0.0, 1.0, 0.0, 1.0);
              })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.cVertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = dawn::PrimitiveTopology::TriangleStrip;
        descriptor.indexFormat = dawn::IndexFormat::Uint32;
        descriptor.inputState = inputState;
        descriptor.cColorStates[0]->format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, dawn::BufferUsageBit::Vertex,
            {// The bottom left triangle
             -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginRenderPass(&renderPass.renderPassInfo).EndPass();
        dawn::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    utils::BasicRenderPass renderPass;
    dawn::RenderPipeline pipeline;
    dawn::Buffer vertexBuffer;

    dawn::CommandBuffer CreateTriangleCommandBuffer() {
        uint32_t zeroOffset = 0;
        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset);
            pass.Draw(3, 1, 0, 0);
            pass.EndPass();
        }
        dawn::CommandBuffer commands = encoder.Finish();
        return commands;
    }
};

// Destroy before submit will result in error, and nothing drawn
TEST_P(DestroyBufferTest, DestroyBeforeSubmit) {
    RGBA8 notFilled(0, 0, 0, 0);

    dawn::CommandBuffer commands = CreateTriangleCommandBuffer();
    vertexBuffer.Destroy();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
}

// Destroy after submit will draw successfully
TEST_P(DestroyBufferTest, DestroyAfterSubmit) {
    RGBA8 filled(0, 255, 0, 255);

    dawn::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    vertexBuffer.Destroy();
}

// First submit succeeds, draws triangle, second submit fails
// after destroy is called on the buffer, pixel does not change
TEST_P(DestroyBufferTest, SubmitDestroySubmit) {
    RGBA8 filled(0, 255, 0, 255);

    dawn::CommandBuffer commands = CreateTriangleCommandBuffer();
    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);

    vertexBuffer.Destroy();

    // Submit fails because vertex buffer was destroyed
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

    // Pixel stays the same
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
}

DAWN_INSTANTIATE_TEST(DestroyBufferTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);