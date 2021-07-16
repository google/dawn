// Copyright 2018 The Dawn Authors
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
#include "utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 4;

class DrawIndexedTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            [[stage(vertex)]]
            fn main([[location(0)]] pos : vec4<f32>) -> [[builtin(position)]] vec4<f32> {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// First quad: the first 3 vertices represent the bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             0.0f, 1.0f,

             // Second quad: the first 3 vertices represent the top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
             0.0f, 1.0f});
        indexBuffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Index,
            {0, 1, 2, 0, 3, 1,
             // The indices below are added to test negatve baseVertex
             0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        zeroSizedIndexBuffer =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::Buffer indexBuffer;
    wgpu::Buffer zeroSizedIndexBuffer;

    void Test(uint32_t indexCount,
              uint32_t instanceCount,
              uint32_t firstIndex,
              int32_t baseVertex,
              uint32_t firstInstance,
              uint64_t bufferOffset,
              RGBA8 bottomLeftExpected,
              RGBA8 topRightExpected) {
        // Regular draw with a reasonable index buffer
        TestImplementation(indexCount, instanceCount, firstIndex, baseVertex, firstInstance,
                           bufferOffset, indexBuffer, bottomLeftExpected, topRightExpected);
    }

    void TestZeroSizedIndexBufferDraw(uint32_t indexCount,
                                      uint32_t firstIndex,
                                      RGBA8 bottomLeftExpected,
                                      RGBA8 topRightExpected) {
        TestImplementation(indexCount, 1, firstIndex, 0, 0, 0, zeroSizedIndexBuffer,
                           bottomLeftExpected, topRightExpected);
    }

    void TestImplementation(uint32_t indexCount,
                            uint32_t instanceCount,
                            uint32_t firstIndex,
                            int32_t baseVertex,
                            uint32_t firstInstance,
                            uint64_t bufferOffset,
                            const wgpu::Buffer& curIndexBuffer,
                            RGBA8 bottomLeftExpected,
                            RGBA8 topRightExpected) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(curIndexBuffer, wgpu::IndexFormat::Uint32, bufferOffset);
            pass.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }
};

// The most basic DrawIndexed triangle draw.
TEST_P(DrawIndexedTest, Uint32) {
    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with no indices.
    Test(0, 0, 0, 0, 0, 0, notFilled, notFilled);
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test(3, 1, 0, 0, 0, 0, filled, notFilled);
    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test(3, 1, 3, 0, 0, 0, notFilled, filled);
    // Test a draw with all 6 indices (both triangles).
    Test(6, 1, 0, 0, 0, 0, filled, filled);
}

// Test the parameter 'baseVertex' of DrawIndexed() works.
TEST_P(DrawIndexedTest, BaseVertex) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_base_vertex"));
    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with only the first 3 indices of the second quad (top right triangle)
    Test(3, 1, 0, 4, 0, 0, notFilled, filled);
    // Test a draw with only the last 3 indices of the second quad (bottom left triangle)
    Test(3, 1, 3, 4, 0, 0, filled, notFilled);

    // Test negative baseVertex
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test(3, 1, 0, -4, 0, 6 * sizeof(uint32_t), filled, notFilled);
    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test(3, 1, 3, -4, 0, 6 * sizeof(uint32_t), notFilled, filled);
}

DAWN_INSTANTIATE_TEST(DrawIndexedTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
