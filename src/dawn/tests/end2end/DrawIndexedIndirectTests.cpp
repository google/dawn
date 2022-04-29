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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 4;

class DrawIndexedIndirectTest : public DawnTest {
  protected:
    wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits& supported) override {
        // Force larger limits, that might reach into the upper 32 bits of the 64bit limit values,
        // to help detect integer arithmetic bugs like overflows and truncations.
        wgpu::RequiredLimits required = {};
        required.limits = supported.limits;
        return required;
    }

    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@location(0) pos : vec4<f32>) -> @builtin(position) vec4<f32> {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
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
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;

    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }

    wgpu::CommandBuffer EncodeDrawCommands(std::initializer_list<uint32_t> bufferList,
                                           wgpu::Buffer indexBuffer,
                                           uint64_t indexOffset,
                                           uint64_t indirectOffset) {
        wgpu::Buffer indirectBuffer = CreateIndirectBuffer(bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, indexOffset);
            pass.DrawIndexedIndirect(indirectBuffer, indirectOffset);
            pass.End();
        }

        return encoder.Finish();
    }

    void TestDraw(wgpu::CommandBuffer commands, RGBA8 bottomLeftExpected, RGBA8 topRightExpected) {
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3);
        EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1);
    }

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indexOffset,
              uint64_t indirectOffset,
              RGBA8 bottomLeftExpected,
              RGBA8 topRightExpected) {
        wgpu::Buffer indexBuffer =
            CreateIndexBuffer({0, 1, 2, 0, 3, 1,
                               // The indices below are added to test negatve baseVertex
                               0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        TestDraw(EncodeDrawCommands(bufferList, indexBuffer, indexOffset, indirectOffset),
                 bottomLeftExpected, topRightExpected);
    }
};

// The most basic DrawIndexed triangle draw.
TEST_P(DrawIndexedIndirectTest, Uint32) {
    // TODO(crbug.com/dawn/789): Test is failing after a roll on SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with no indices.
    Test({0, 0, 0, 0, 0}, 0, 0, notFilled, notFilled);

    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test({3, 1, 0, 0, 0}, 0, 0, filled, notFilled);

    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test({3, 1, 3, 0, 0}, 0, 0, notFilled, filled);

    // Test a draw with all 6 indices (both triangles).
    Test({6, 1, 0, 0, 0}, 0, 0, filled, filled);
}

// Test the parameter 'baseVertex' of DrawIndexed() works.
TEST_P(DrawIndexedIndirectTest, BaseVertex) {
    // TODO(crbug.com/dawn/161): add workaround for OpenGL index buffer offset (could be compute
    // shader that adds it to the draw calls)
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    // TODO(crbug.com/dawn/966): Fails on Metal Intel, likely because @builtin(vertex_index)
    // doesn't take into account BaseVertex, which breaks programmable vertex pulling.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test a draw with only the first 3 indices of the second quad (top right triangle)
    Test({3, 1, 0, 4, 0}, 0, 0, notFilled, filled);

    // Test a draw with only the last 3 indices of the second quad (bottom left triangle)
    Test({3, 1, 3, 4, 0}, 0, 0, filled, notFilled);

    const int negFour = -4;
    uint32_t unsignedNegFour;
    std::memcpy(&unsignedNegFour, &negFour, sizeof(int));

    // Test negative baseVertex
    // Test a draw with only the first 3 indices of the first quad (bottom left triangle)
    Test({3, 1, 0, unsignedNegFour, 0}, 6 * sizeof(uint32_t), 0, filled, notFilled);

    // Test a draw with only the last 3 indices of the first quad (top right triangle)
    Test({3, 1, 3, unsignedNegFour, 0}, 6 * sizeof(uint32_t), 0, notFilled, filled);
}

TEST_P(DrawIndexedIndirectTest, IndirectOffset) {
    // TODO(crbug.com/dawn/789): Test is failing after a roll on SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/966): Fails on Metal Intel, likely because @builtin(vertex_index)
    // doesn't take into account BaseVertex, which breaks programmable vertex pulling.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Test an offset draw call, with indirect buffer containing 2 calls:
    // 1) first 3 indices of the second quad (top right triangle)
    // 2) last 3 indices of the second quad

    // Test #1 (no offset)
    Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 0, notFilled, filled);

    // Offset to draw #2
    Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 5 * sizeof(uint32_t), filled, notFilled);
}

TEST_P(DrawIndexedIndirectTest, BasicValidation) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1});

    // Test a draw with an excessive indexCount. Should draw nothing.
    TestDraw(EncodeDrawCommands({7, 1, 0, 0, 0}, indexBuffer, 0, 0), notFilled, notFilled);

    // Test a draw with an excessive firstIndex. Should draw nothing.
    TestDraw(EncodeDrawCommands({3, 1, 7, 0, 0}, indexBuffer, 0, 0), notFilled, notFilled);

    // Test a valid draw. Should draw only the second triangle.
    TestDraw(EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0), notFilled, filled);
}

TEST_P(DrawIndexedIndirectTest, ValidateWithOffsets) {
    // TODO(crbug.com/dawn/161): The GL/GLES backend doesn't support indirect index buffer offsets
    // yet.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    // Test that validation properly accounts for index buffer offset.
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0}, indexBuffer, 6 * sizeof(uint32_t), 0), filled,
             notFilled);
    TestDraw(EncodeDrawCommands({4, 1, 0, 0, 0}, indexBuffer, 6 * sizeof(uint32_t), 0), notFilled,
             notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 4, 0, 0}, indexBuffer, 3 * sizeof(uint32_t), 0), notFilled,
             notFilled);

    // Test that validation properly accounts for indirect buffer offset.
    TestDraw(
        EncodeDrawCommands({3, 1, 0, 0, 0, 1000, 1, 0, 0, 0}, indexBuffer, 0, 4 * sizeof(uint32_t)),
        notFilled, notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 0, 0, 0, 1000, 1, 0, 0, 0}, indexBuffer, 0, 0), filled,
             notFilled);
}

TEST_P(DrawIndexedIndirectTest, ValidateMultiplePasses) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    // Test validation with multiple passes in a row. Namely this is exercising that scratch buffer
    // data for use with a previous pass's validation commands is not overwritten before it can be
    // used.
    TestDraw(EncodeDrawCommands({10, 1, 0, 0, 0}, indexBuffer, 0, 0), notFilled, notFilled);
    TestDraw(EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0), filled, filled);
    TestDraw(EncodeDrawCommands({4, 1, 6, 0, 0}, indexBuffer, 0, 0), notFilled, notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 6, 0, 0}, indexBuffer, 0, 0), filled, notFilled);
    TestDraw(EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0), notFilled, filled);
    TestDraw(EncodeDrawCommands({6, 1, 3, 0, 0}, indexBuffer, 0, 0), filled, filled);
    TestDraw(EncodeDrawCommands({6, 1, 6, 0, 0}, indexBuffer, 0, 0), notFilled, notFilled);
}

TEST_P(DrawIndexedIndirectTest, ValidateMultipleDraws) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    // Validate multiple draw calls using the same index and indirect buffers as input, but with
    // different indirect offsets.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::Buffer indirectBuffer =
            CreateIndirectBuffer({3, 1, 3, 0, 0, 10, 1, 0, 0, 0, 3, 1, 6, 0, 0});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(indirectBuffer, 0);
        pass.DrawIndexedIndirect(indirectBuffer, 20);
        pass.DrawIndexedIndirect(indirectBuffer, 40);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);

    // Validate multiple draw calls using the same indirect buffer but different index buffers as
    // input.
    encoder = device.CreateCommandEncoder();
    {
        wgpu::Buffer indirectBuffer =
            CreateIndirectBuffer({3, 1, 3, 0, 0, 10, 1, 0, 0, 0, 3, 1, 6, 0, 0});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(indirectBuffer, 0);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 3, 1, 0, 2, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(indirectBuffer, 20);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 2, 1}),
                            wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(indirectBuffer, 40);
        pass.End();
    }
    commands = encoder.Finish();

    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);

    // Validate multiple draw calls using the same index buffer but different indirect buffers as
    // input.
    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({3, 1, 3, 0, 0}), 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({10, 1, 0, 0, 0}), 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({3, 1, 6, 0, 0}), 0);
        pass.End();
    }
    commands = encoder.Finish();

    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);

    // Validate multiple draw calls across different index and indirect buffers.
    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({3, 1, 3, 0, 0}), 0);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 1, 2, 0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({10, 1, 0, 0, 0}), 0);
        pass.SetIndexBuffer(CreateIndexBuffer({0, 3, 1}), wgpu::IndexFormat::Uint32, 0);
        pass.DrawIndexedIndirect(CreateIndirectBuffer({3, 1, 3, 0, 0}), 0);
        pass.End();
    }
    commands = encoder.Finish();

    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

TEST_P(DrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitInOrder) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[7];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[1] = EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[3] = EncodeDrawCommands({3, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0);
    commands[5] = EncodeDrawCommands({6, 1, 3, 0, 0}, indexBuffer, 0, 0);
    commands[6] = EncodeDrawCommands({6, 1, 6, 0, 0}, indexBuffer, 0, 0);

    TestDraw(commands[0], notFilled, notFilled);
    TestDraw(commands[1], filled, filled);
    TestDraw(commands[2], notFilled, notFilled);
    TestDraw(commands[3], filled, notFilled);
    TestDraw(commands[4], notFilled, filled);
    TestDraw(commands[5], filled, filled);
    TestDraw(commands[6], notFilled, notFilled);
}

TEST_P(DrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitAtOnce) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1124): Fails on Intel+Vulkan+Windows for drivers
    // older than 27.20.100.8587, which bots are actively using.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsVulkan() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[5];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[1] = EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[3] = EncodeDrawCommands({3, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0);

    queue.Submit(5, commands);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

TEST_P(DrawIndexedIndirectTest, ValidateEncodeMultipleThenSubmitOutOfOrder) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[7];
    commands[0] = EncodeDrawCommands({10, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[1] = EncodeDrawCommands({6, 1, 0, 0, 0}, indexBuffer, 0, 0);
    commands[2] = EncodeDrawCommands({4, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[3] = EncodeDrawCommands({3, 1, 6, 0, 0}, indexBuffer, 0, 0);
    commands[4] = EncodeDrawCommands({3, 1, 3, 0, 0}, indexBuffer, 0, 0);
    commands[5] = EncodeDrawCommands({6, 1, 3, 0, 0}, indexBuffer, 0, 0);
    commands[6] = EncodeDrawCommands({6, 1, 6, 0, 0}, indexBuffer, 0, 0);

    TestDraw(commands[6], notFilled, notFilled);
    TestDraw(commands[5], filled, filled);
    TestDraw(commands[4], notFilled, filled);
    TestDraw(commands[3], filled, notFilled);
    TestDraw(commands[2], notFilled, notFilled);
    TestDraw(commands[1], filled, filled);
    TestDraw(commands[0], notFilled, notFilled);
}

TEST_P(DrawIndexedIndirectTest, ValidateWithBundlesInSamePass) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indirectBuffer =
        CreateIndirectBuffer({3, 1, 3, 0, 0, 10, 1, 0, 0, 0, 3, 1, 6, 0, 0});
    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    std::vector<wgpu::RenderBundle> bundles;
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatsCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&desc);
        bundleEncoder.SetPipeline(pipeline);
        bundleEncoder.SetVertexBuffer(0, vertexBuffer);
        bundleEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
        bundleEncoder.DrawIndexedIndirect(indirectBuffer, 20);
        bundles.push_back(bundleEncoder.Finish());
    }
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatsCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&desc);
        bundleEncoder.SetPipeline(pipeline);
        bundleEncoder.SetVertexBuffer(0, vertexBuffer);
        bundleEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
        bundleEncoder.DrawIndexedIndirect(indirectBuffer, 40);
        bundles.push_back(bundleEncoder.Finish());
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(bundles.size(), bundles.data());
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 3, 1);
}

TEST_P(DrawIndexedIndirectTest, ValidateWithBundlesInDifferentPasses) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indirectBuffer =
        CreateIndirectBuffer({3, 1, 3, 0, 0, 10, 1, 0, 0, 0, 3, 1, 6, 0, 0});
    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1, 0, 1, 2});

    wgpu::CommandBuffer commands[2];
    {
        wgpu::RenderBundle bundle;
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatsCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&desc);
        bundleEncoder.SetPipeline(pipeline);
        bundleEncoder.SetVertexBuffer(0, vertexBuffer);
        bundleEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
        bundleEncoder.DrawIndexedIndirect(indirectBuffer, 20);
        bundle = bundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        renderPass.renderPassInfo.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &bundle);
        pass.End();

        commands[0] = encoder.Finish();
    }

    {
        wgpu::RenderBundle bundle;
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatsCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&desc);
        bundleEncoder.SetPipeline(pipeline);
        bundleEncoder.SetVertexBuffer(0, vertexBuffer);
        bundleEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
        bundleEncoder.DrawIndexedIndirect(indirectBuffer, 40);
        bundle = bundleEncoder.Finish();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        renderPass.renderPassInfo.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &bundle);
        pass.End();

        commands[1] = encoder.Finish();
    }

    queue.Submit(1, &commands[1]);
    queue.Submit(1, &commands[0]);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, 3, 1);
}

TEST_P(DrawIndexedIndirectTest, ValidateReusedBundleWithChangingParams) {
    // TODO(crbug.com/dawn/789): Test is failing under SwANGLE on Windows.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1124): Fails on Intel+Vulkan+Windows for drivers
    // older than 27.20.100.8587, which bots are actively using.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsVulkan() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // It doesn't make sense to test invalid inputs when validation is disabled.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    RGBA8 filled(0, 255, 0, 255);
    // RGBA8 notFilled(0, 0, 0, 0);

    wgpu::Buffer indirectBuffer = CreateIndirectBuffer({0, 0, 0, 0, 0});
    wgpu::Buffer indexBuffer = CreateIndexBuffer({0, 1, 2, 0, 3, 1});

    // Encode a single bundle that always uses indirectBuffer offset 0 for its params.
    wgpu::RenderBundle bundle;
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatsCount = 1;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&desc);
    bundleEncoder.SetPipeline(pipeline);
    bundleEncoder.SetVertexBuffer(0, vertexBuffer);
    bundleEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
    bundleEncoder.DrawIndexedIndirect(indirectBuffer, 0);
    bundle = bundleEncoder.Finish();

    wgpu::ShaderModule paramWriterModule = utils::CreateShaderModule(device,
                                                                     R"(
            struct Input { firstIndex: u32 }
            struct Params {
                indexCount: u32,
                instanceCount: u32,
                firstIndex: u32,
            }
            @group(0) @binding(0) var<uniform> input: Input;
            @group(0) @binding(1) var<storage, write> params: Params;
            @stage(compute) @workgroup_size(1) fn main() {
                params.indexCount = 3u;
                params.instanceCount = 1u;
                params.firstIndex = input.firstIndex;
            }
        )");

    wgpu::ComputePipelineDescriptor computeDesc;
    computeDesc.compute.module = paramWriterModule;
    computeDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline computePipeline = device.CreateComputePipeline(&computeDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    auto encodeComputePassToUpdateFirstIndex = [&](uint32_t newFirstIndex) {
        wgpu::Buffer input = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform, {newFirstIndex});
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, computePipeline.GetBindGroupLayout(0),
            {{0, input, 0, sizeof(uint32_t)}, {1, indirectBuffer, 0, 5 * sizeof(uint32_t)}});
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(computePipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
    };

    auto encodeRenderPassToExecuteBundle = [&](wgpu::LoadOp colorLoadOp) {
        renderPass.renderPassInfo.cColorAttachments[0].loadOp = colorLoadOp;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &bundle);
        pass.End();
    };

    encodeComputePassToUpdateFirstIndex(0);
    encodeRenderPassToExecuteBundle(wgpu::LoadOp::Clear);
    encodeComputePassToUpdateFirstIndex(3);
    encodeRenderPassToExecuteBundle(wgpu::LoadOp::Load);
    encodeComputePassToUpdateFirstIndex(6);
    encodeRenderPassToExecuteBundle(wgpu::LoadOp::Load);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 3, 1);
}

DAWN_INSTANTIATE_TEST(DrawIndexedIndirectTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
