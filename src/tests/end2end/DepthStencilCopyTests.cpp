// Copyright 2020 The Dawn Authors
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

#include <array>
#include "common/Constants.h"
#include "common/Math.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

class DepthStencilCopyTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Draw a square in the bottom left quarter of the screen.
        mVertexModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
    #version 450
    void main() {
        const vec2 pos[6] = vec2[6](vec2(-1.f, -1.f), vec2(0.f, -1.f), vec2(-1.f,  0.f),
                                    vec2(-1.f,  0.f), vec2(0.f, -1.f), vec2( 0.f,  0.f));
        gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
    })");

        mFragmentModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
    #version 450
    void main() {
        gl_FragDepth = 0.3;
    })");
    }

    static constexpr float kWrittenDepthValue = 0.3;

    wgpu::ShaderModule mVertexModule;
    wgpu::ShaderModule mFragmentModule;
};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromDepthAspect) {
    // Create a depth texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    wgpu::TextureDescriptor texDescriptor = {};
    texDescriptor.size = {kWidth, kHeight, 1};
    texDescriptor.format = wgpu::TextureFormat::Depth32Float;
    texDescriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture depthTexture = device.CreateTexture(&texDescriptor);

    // Create a render pass which clears depth to 0
    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.clearDepth = 0.f;

    // Create a render pipeline to render a bottom-left quad with depth 0.3.
    utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
    renderPipelineDesc.vertexStage.module = mVertexModule;
    renderPipelineDesc.cFragmentStage.module = mFragmentModule;
    renderPipelineDesc.cDepthStencilState.format = texDescriptor.format;
    renderPipelineDesc.cDepthStencilState.depthWriteEnabled = true;
    renderPipelineDesc.depthStencilState = &renderPipelineDesc.cDepthStencilState;
    renderPipelineDesc.colorStateCount = 0;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

    // Draw the quad (two triangles)
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(6);
    pass.EndPass();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    // Only the bottom left quad has depth values
    std::vector<float> expected = {
        0.0, 0.0, 0.0, 0.0,  //
        0.0, 0.0, 0.0, 0.0,  //
        0.3, 0.3, 0.0, 0.0,  //
        0.3, 0.3, 0.0, 0.0,  //
    };

    // This expectation is the test as it performs the CopyTextureToBuffer.
    EXPECT_TEXTURE_EQ(expected.data(), depthTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::DepthOnly);
}

// Test copying the stencil-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromStencilAspect) {
    // TODO(enga): Figure out why this fails on Linux Vulkan Intel
    DAWN_SKIP_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    wgpu::TextureDescriptor texDescriptor = {};
    texDescriptor.size = {kWidth, kHeight, 1};
    texDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    texDescriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture depthStencilTexture = device.CreateTexture(&texDescriptor);

    // Create a render pass which clears the stencil to 0 on load.
    utils::ComboRenderPassDescriptor renderPassDesc({}, depthStencilTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.clearStencil = 0;

    // Create a render pipline which increments the stencil value for passing fragments.
    // A quad is drawn in the bottom left.
    utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
    renderPipelineDesc.vertexStage.module = mVertexModule;
    renderPipelineDesc.cFragmentStage.module = mFragmentModule;
    renderPipelineDesc.cDepthStencilState.format = texDescriptor.format;
    renderPipelineDesc.cDepthStencilState.stencilFront.passOp =
        wgpu::StencilOperation::IncrementClamp;
    renderPipelineDesc.depthStencilState = &renderPipelineDesc.cDepthStencilState;
    renderPipelineDesc.colorStateCount = 0;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

    // Draw the quad (two triangles)
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(6);
    pass.EndPass();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    // Only the bottom left quad has stencil values
    std::vector<uint8_t> expected = {
        0u, 0u, 0u, 0u,  //
        0u, 0u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
    };

    // This expectation is the test as it performs the CopyTextureToBuffer.
    EXPECT_TEXTURE_EQ(expected.data(), depthStencilTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying to the stencil-aspect of a buffer
TEST_P(DepthStencilCopyTests, ToStencilAspect) {
    // TODO(enga): Figure out why this fails on Vulkan Intel
    // Results are shifted by 1 byte on Windows, and crash/hang on Linux.
    DAWN_SKIP_TEST_IF(IsVulkan() && IsIntel());

    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    wgpu::TextureDescriptor texDescriptor = {};
    texDescriptor.size = {kWidth, kHeight, 1};
    texDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    texDescriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst;
    wgpu::Texture depthStencilTexture = device.CreateTexture(&texDescriptor);

    // Bytes per row for the stencil data we will upload.
    // TODO(enga): Use WriteTexture when implemented everywhere.
    uint32_t bytesPerRow = Align(kWidth * sizeof(uint8_t), kTextureBytesPerRowAlignment);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = wgpu::BufferUsage::CopySrc;
    bufferDesc.size = kHeight * bytesPerRow;
    bufferDesc.mappedAtCreation = true;

    std::vector<uint8_t> stencilData = {
        1u,  2u,  3u,  4u,   //
        5u,  6u,  7u,  8u,   //
        9u,  10u, 11u, 12u,  //
        13u, 14u, 15u, 16u,  //
    };

    // After copying stencil data in, we will decrement stencil values in the bottom left
    // of the screen. This is the expected result.
    std::vector<uint8_t> expectedStencilData = {
        1u,  2u,  3u,  4u,   //
        5u,  6u,  7u,  8u,   //
        8u,  9u,  11u, 12u,  //
        12u, 13u, 15u, 16u,  //
    };

    // Copy the stencil data into the buffer.
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    uint8_t* mappedData = static_cast<uint8_t*>(buffer.GetMappedRange());
    for (uint32_t r = 0; r < kHeight; ++r) {
        memcpy(mappedData + r * bytesPerRow, &stencilData[r * kWidth], kWidth);
    }
    buffer.Unmap();

    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Clear depth to 0.7, so we can check that the stencil copy doesn't mutate the depth.
        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearDepth = 0.7;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();

        // Copy from the buffer into the stencil aspect of the texture.
        wgpu::BufferCopyView bufferCopy = utils::CreateBufferCopyView(buffer, 0, bytesPerRow, 0);
        wgpu::TextureCopyView textureCopy = utils::CreateTextureCopyView(
            depthStencilTexture, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);

        commandEncoder.CopyBufferToTexture(&bufferCopy, &textureCopy, &texDescriptor.size);

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        // Create a render pipline which decrements the stencil value for passing fragments.
        // A quad is drawn in the bottom left.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
        renderPipelineDesc.vertexStage.module = mVertexModule;
        renderPipelineDesc.cFragmentStage.module = mFragmentModule;
        renderPipelineDesc.cDepthStencilState.format = texDescriptor.format;
        renderPipelineDesc.cDepthStencilState.stencilFront.passOp =
            wgpu::StencilOperation::DecrementClamp;
        renderPipelineDesc.depthStencilState = &renderPipelineDesc.cDepthStencilState;
        renderPipelineDesc.colorStateCount = 0;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

        // Create a render pass which loads the stencil. We want to load the values we
        // copied in. Also load the canary depth values so they're not lost.
        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;

        // Draw the quad in the bottom left (two triangles).
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    // Copy back the stencil data and check it is the same.
    EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);

    // Check that the depth buffer isn't changed.
    // We do this by running executing a draw call that only passes the depth test if
    // the depth is equal to the current depth buffer.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Make the color attachment that we'll use to read back.
        wgpu::TextureDescriptor colorTexDesc = {};
        colorTexDesc.size = {kWidth, kHeight, 1};
        colorTexDesc.format = wgpu::TextureFormat::R32Uint;
        colorTexDesc.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture colorTexture = device.CreateTexture(&colorTexDesc);

        // Pipeline for a full screen quad.
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        pipelineDescriptor.vertexStage.module =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
    #version 450
    void main() {
        const vec2 pos[3] = vec2[3](vec2(-1.f, -1.f), vec2(3.f, -1.f), vec2(-1.f, 3.f));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
        gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
    })");

        // Write out 0.7 for depth. This is the same canary value we wrote previously.
        pipelineDescriptor.cFragmentStage.module =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
    #version 450

    layout(location = 0) out uint result;
    void main() {
        result = 1u;
        gl_FragDepth = 0.7;
    })");

        // Pass the depth test only if the depth is equal.
        pipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDescriptor.depthStencilState = &pipelineDescriptor.cDepthStencilState;
        pipelineDescriptor.cDepthStencilState.format = texDescriptor.format;
        pipelineDescriptor.cDepthStencilState.depthCompare = wgpu::CompareFunction::Equal;
        pipelineDescriptor.cColorStates[0].format = colorTexDesc.format;

        utils::ComboRenderPassDescriptor passDescriptor({colorTexture.CreateView()},
                                                        depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint32_t> colorData(16, 1u);
        EXPECT_TEXTURE_EQ(colorData.data(), colorTexture, 0, 0, kWidth, kHeight, 0, 0);
    }
}

DAWN_INSTANTIATE_TEST(DepthStencilCopyTests, D3D12Backend(), MetalBackend(), VulkanBackend());
