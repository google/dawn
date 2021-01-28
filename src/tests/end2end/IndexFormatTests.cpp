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

#include "common/Assert.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 400;

class IndexFormatTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    }

    utils::BasicRenderPass renderPass;

    wgpu::RenderPipeline MakeTestPipeline(wgpu::IndexFormat format,
        wgpu::PrimitiveTopology primitiveTopology = wgpu::PrimitiveTopology::TriangleStrip) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<in> pos : vec4<f32>;
            [[builtin(vertex_index)]] var<in> idx : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                // 0xFFFFFFFE is a designated invalid index used by some tests.
                if (idx == 0xFFFFFFFEu) {
                    Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                } else {
                    Position = pos;
                }
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = primitiveTopology;
        descriptor.cVertexState.indexFormat = format;
        descriptor.cVertexState.vertexBufferCount = 1;
        descriptor.cVertexState.cVertexBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cVertexState.cVertexBuffers[0].attributeCount = 1;
        descriptor.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
        descriptor.cColorStates[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that the Uint32 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint32) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f,  // Note Vertices[0] = Vertices[1]
         -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {1, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that the Uint16 index format is correctly interpreted
TEST_P(IndexFormatTest, Uint16) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as uint32, it will have index 1 and 2 be both 0 and render nothing
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index, {1, 2, 0, 0, 0, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that the index format used is the format of the last set pipeline. This is to
// prevent a case in D3D12 where the index format would be captured from the last
// pipeline on SetIndexBuffer.
TEST_P(IndexFormatTest, ChangePipelineAfterSetIndexBuffer) {
    wgpu::RenderPipeline pipeline32 = MakeTestPipeline(wgpu::IndexFormat::Uint32);
    wgpu::RenderPipeline pipeline16 = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f,  // Note Vertices[0] = Vertices[1]
         -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    // If this is interpreted as Uint16, then it would be 0, 1, 0, ... and would draw nothing.
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {1, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline16);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline32);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 100, 300);
}

// Test that setting the index buffer before the pipeline works, this is important
// for backends where the index format is passed inside the call to SetIndexBuffer
// because it needs to be done lazily (to query the format from the last pipeline).
TEST_P(IndexFormatTest, SetIndexBufferBeforeSetPipeline) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);
}

// Test that index buffers of multiple formats can be used with a pipeline that
// doesn't use strip primitive topology.
TEST_P(IndexFormatTest, SetIndexBufferDifferentFormats) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Undefined, wgpu::PrimitiveTopology::TriangleList);

    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData<float>(
        device, wgpu::BufferUsage::Vertex,
        {-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f});
    wgpu::Buffer indexBuffer32 =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
    wgpu::Buffer indexBuffer16 =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index, {0, 1, 2, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer32, wgpu::IndexFormat::Uint32);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);

    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(indexBuffer16, wgpu::IndexFormat::Uint16);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.DrawIndexed(3);
        pass.EndPass();
    }

    commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderPass.color, 100, 300);
}

// Tests for primitive restart use vertices like in the drawing and draw the following
// indices: 0 1 2 PRIM_RESTART 3 4 5. Then A and B should be written but not C.
//      |--------------|
//      |      0---1   |
//      |       \ B|   |
//      |         \|   |
//      |  3   C   2   |
//      |  |\          |
//      |  |A \        |
//      |  4---5       |
//      |--------------|

class TriangleStripPrimitiveRestartTests : public IndexFormatTest {
  protected:
    wgpu::Buffer mVertexBuffer;

    void SetUp() override {
        IndexFormatTest::SetUp();
        mVertexBuffer = utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                                           {
                                                               0.0f,  1.0f,  0.0f, 1.0f,  // 0
                                                               1.0f,  1.0f,  0.0f, 1.0f,  // 1
                                                               1.0f,  0.0f,  0.0f, 1.0f,  // 2
                                                               -1.0f, 0.0f,  0.0f, 1.0f,  // 3
                                                               -1.0f, -1.0f, 0.0f, 1.0f,  // 4
                                                               0.0f,  -1.0f, 0.0f, 1.0f,  // 5
                                                           });
    }
};

// Test use of primitive restart with an Uint32 index format
TEST_P(TriangleStripPrimitiveRestartTests, Uint32PrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  0xFFFFFFFFu,
                                                  3,
                                                  4,
                                                  5,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(7);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 50, 350);  // A
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 350, 50);  // B
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 198, 200);  // C
}

// Same as the above test, but uses an OOB index to emulate primitive restart being disabled,
// causing point C to be written to.
TEST_P(TriangleStripPrimitiveRestartTests, Uint32WithoutPrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint32);
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  // Not a valid index.
                                                  0xFFFFFFFEu,
                                                  3,
                                                  4,
                                                  5,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(7);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 50, 350);   // A
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 350, 50);   // B
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 198, 200);  // C
}

// Test use of primitive restart with an Uint16 index format
TEST_P(TriangleStripPrimitiveRestartTests, Uint16PrimitiveRestart) {
    wgpu::RenderPipeline pipeline = MakeTestPipeline(wgpu::IndexFormat::Uint16);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index,
                                              {
                                                  0,
                                                  1,
                                                  2,
                                                  0xFFFFu,
                                                  3,
                                                  4,
                                                  5,
                                                  // This value is for padding.
                                                  0xFFFFu,
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(7);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 50, 350);  // A
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 350, 50);  // B
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 198, 200);  // C
}

// Tests for primitive restart use vertices like in the drawing and draw the following
// indices: 0 1 PRIM_RESTART 2 3. Then 1 and 2 should be written but not A.
//      |--------------|
//      |      3      0|
//      |      |      ||
//      |      |      ||
//      |      2  A   1|
//      |              |
//      |              |
//      |              |
//      |--------------|

class LineStripPrimitiveRestartTests : public IndexFormatTest {
  protected:
  protected:
    wgpu::Buffer mVertexBuffer;

    void SetUp() override {
        IndexFormatTest::SetUp();
        mVertexBuffer = utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                                           {
                                                               1.0f, 1.0f, 0.0f, 1.0f,  // 0
                                                               1.0f, 0.0f, 0.0f, 1.0f,  // 1
                                                               0.0f, 0.0f, 0.0f, 1.0f,  // 2
                                                               0.0f, 1.0f, 0.0f, 1.0f   // 3
                                                           });
    }
};

TEST_P(LineStripPrimitiveRestartTests, Uint32PrimitiveRestart) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint32, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer = utils::CreateBufferFromData<uint32_t>(
        device, wgpu::BufferUsage::Index, {0, 1, 0xFFFFFFFFu, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(5);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 300, 199);   // A
}

// Same as the above test, but uses an OOB index to emulate primitive restart being disabled,
// causing point A to be written to.
TEST_P(LineStripPrimitiveRestartTests, Uint32WithoutPrimitiveRestart) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint32, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index,
                                              {0, 1,  // Not a valid index
                                               0xFFFFFFFEu, 2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(5);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 300, 199);  // A
}

TEST_P(LineStripPrimitiveRestartTests, Uint16PrimitiveRestart) {
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(wgpu::IndexFormat::Uint16, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint16_t>(device, wgpu::BufferUsage::Index,
                                              {0, 1, 0xFFFFu, 2, 3,  // This value is for padding.
                                               0xFFFFu});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, mVertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        pass.DrawIndexed(5);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 399, 199);  // 1
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 199, 199);  // 2
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 300, 199);   // A
}

DAWN_INSTANTIATE_TEST(IndexFormatTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(TriangleStripPrimitiveRestartTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
DAWN_INSTANTIATE_TEST(LineStripPrimitiveRestartTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
