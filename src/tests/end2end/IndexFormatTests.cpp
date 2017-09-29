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

#include "common/Assert.h"
#include "utils/NXTHelpers.h"

constexpr uint32_t kRTSize = 400;

class IndexFormatTest : public NXTTest {
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
        }

        nxt::RenderPass renderpass;
        nxt::Texture renderTarget;
        nxt::TextureView renderTargetView;
        nxt::Framebuffer framebuffer;

        nxt::RenderPipeline MakeTestPipeline(nxt::IndexFormat format) {
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

            return device.CreateRenderPipelineBuilder()
                .SetSubpass(renderpass, 0)
                .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleStrip)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetIndexFormat(format)
                .SetInputState(inputState)
                .GetResult();
        }
};

// Test that the Uint32 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint32) {
    nxt::RenderPipeline pipeline = MakeTestPipeline(nxt::IndexFormat::Uint32);

    nxt::Buffer vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
        -1.0f,  1.0f, 0.0f, 1.0f, // Note Vertices[0] = Vertices[1]
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    });
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    nxt::Buffer indexBuffer = utils::CreateFrozenBufferFromData<uint32_t>(device, nxt::BufferUsageBit::Index, {
        1, 2, 3
    });

    uint32_t zeroOffset = 0;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 100, 100);
}

// Test that the Uint16 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint16) {
    nxt::RenderPipeline pipeline = MakeTestPipeline(nxt::IndexFormat::Uint16);

    nxt::Buffer vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    });
    // If this is interpreted as uint32, it will have index 1 and 2 be both 0 and render nothing
    nxt::Buffer indexBuffer = utils::CreateFrozenBufferFromData<uint16_t>(device, nxt::BufferUsageBit::Index, {
        1, 2, 0, 0, 0, 0
    });

    uint32_t zeroOffset = 0;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 100, 100);
}

// Test for primitive restart use vertices like in the drawing and draw the following
// indices: 0 1 2 PRIM_RESTART 3 4 2. Then A and B should be written but not C.
//             0
//             |\
//             |B \
//             2---1
//            /| C
//          / A|
//         4---3

// Test use of primitive restart with an Uint32 index format
TEST_P(IndexFormatTest, Uint32PrimitiveRestart) {
    nxt::RenderPipeline pipeline = MakeTestPipeline(nxt::IndexFormat::Uint32);

    nxt::Buffer vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
         0.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  0.0f, 0.0f, 1.0f,
         0.0f,  0.0f, 0.0f, 1.0f,
         0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
    });
    nxt::Buffer indexBuffer = utils::CreateFrozenBufferFromData<uint32_t>(device, nxt::BufferUsageBit::Index, {
        0, 1, 2, 0xFFFFFFFFu, 3, 4, 2,
    });

    uint32_t zeroOffset = 0;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(7, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 190, 210); // A
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 210, 190); // B
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), renderTarget, 210, 210); // C
}

// Test use of primitive restart with an Uint16 index format
TEST_P(IndexFormatTest, Uint16PrimitiveRestart) {
    nxt::RenderPipeline pipeline = MakeTestPipeline(nxt::IndexFormat::Uint16);

    nxt::Buffer vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
         0.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  0.0f, 0.0f, 1.0f,
         0.0f,  0.0f, 0.0f, 1.0f,
         0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
    });
    nxt::Buffer indexBuffer = utils::CreateFrozenBufferFromData<uint16_t>(device, nxt::BufferUsageBit::Index, {
        0, 1, 2, 0xFFFFu, 3, 4, 2,
    });

    uint32_t zeroOffset = 0;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(7, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 190, 210); // A
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 210, 190); // B
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), renderTarget, 210, 210); // C
}

// Test that the index format used is the format of the last set pipeline. This is to
// prevent a case in D3D12 where the index format would be captured from the last
// pipeline on SetIndexBuffer.
TEST_P(IndexFormatTest, ChangePipelineAfterSetIndexBuffer) {
    nxt::RenderPipeline pipeline32 = MakeTestPipeline(nxt::IndexFormat::Uint32);
    nxt::RenderPipeline pipeline16 = MakeTestPipeline(nxt::IndexFormat::Uint16);

    nxt::Buffer vertexBuffer = utils::CreateFrozenBufferFromData<float>(device, nxt::BufferUsageBit::Vertex, {
        -1.0f,  1.0f, 0.0f, 1.0f, // Note Vertices[0] = Vertices[1]
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    });
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    nxt::Buffer indexBuffer = utils::CreateFrozenBufferFromData<uint32_t>(device, nxt::BufferUsageBit::Index, {
        1, 2, 3
    });

    uint32_t zeroOffset = 0;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline16)
            .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
            .SetIndexBuffer(indexBuffer, 0)
            .SetRenderPipeline(pipeline32)
            .DrawElements(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, 100, 100);
}

NXT_INSTANTIATE_TEST(IndexFormatTest, MetalBackend)
