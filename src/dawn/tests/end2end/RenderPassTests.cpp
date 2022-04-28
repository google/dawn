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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 16;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

class RenderPassTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Shaders to draw a bottom-left triangle in blue.
        mVSModule = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 3>(
                    vec2<f32>(-1.0,  1.0),
                    vec2<f32>( 1.0, -1.0),
                    vec2<f32>(-1.0, -1.0));

                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 1.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = mVSModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    wgpu::Texture CreateDefault2DTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kRTSize;
        descriptor.size.height = kRTSize;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = kFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        return device.CreateTexture(&descriptor);
    }

    wgpu::ShaderModule mVSModule;
    wgpu::RenderPipeline pipeline;
};

// Test using two different render passes in one commandBuffer works correctly.
TEST_P(RenderPassTest, TwoRenderPassesInOneCommandBuffer) {
    if (IsOpenGL() || IsMetal()) {
        // crbug.com/950768
        // This test is consistently failing on OpenGL and flaky on Metal.
        return;
    }

    wgpu::Texture renderTarget1 = CreateDefault2DTexture();
    wgpu::Texture renderTarget2 = CreateDefault2DTexture();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        // In the first render pass we clear renderTarget1 to red and draw a blue triangle in the
        // bottom left of renderTarget1.
        utils::ComboRenderPassDescriptor renderPass({renderTarget1.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    {
        // In the second render pass we clear renderTarget2 to green and draw a blue triangle in the
        // bottom left of renderTarget2.
        utils::ComboRenderPassDescriptor renderPass({renderTarget2.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kBlue, renderTarget1, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderTarget1, kRTSize - 1, 1);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kBlue, renderTarget2, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderTarget2, kRTSize - 1, 1);
}

// Verify that the content in the color attachment will not be changed if there is no corresponding
// fragment shader outputs in the render pipeline, the load operation is LoadOp::Load and the store
// operation is StoreOp::Store.
TEST_P(RenderPassTest, NoCorrespondingFragmentShaderOutputs) {
    wgpu::Texture renderTarget = CreateDefault2DTexture();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderTargetView});
    renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

    {
        // First we draw a blue triangle in the bottom left of renderTarget.
        pass.SetPipeline(pipeline);
        pass.Draw(3);
    }

    {
        // Next we use a pipeline whose fragment shader has no outputs.
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() {
            })");
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = mVSModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        wgpu::RenderPipeline pipelineWithNoFragmentOutput =
            device.CreateRenderPipeline(&descriptor);

        pass.SetPipeline(pipelineWithNoFragmentOutput);
        pass.Draw(3);
    }

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kBlue, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderTarget, kRTSize - 1, 1);
}

class RenderPassTest_RegressionDawn1071 : public RenderPassTest {};

DAWN_INSTANTIATE_TEST(RenderPassTest,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Test that clearing the lower mips of an R8Unorm texture works. This is a regression test for
// dawn:1071 where Intel Metal devices fail to do that correctly, requiring a workaround.
TEST_P(RenderPassTest_RegressionDawn1071, ClearLowestMipOfR8Unorm) {
    const uint32_t kLastMipLevel = 2;

    // Create the texture and buffer used for readback.
    wgpu::TextureDescriptor texDesc;
    texDesc.format = wgpu::TextureFormat::R8Unorm;
    texDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    texDesc.size = {32, 32};
    texDesc.mipLevelCount = kLastMipLevel + 1;
    wgpu::Texture tex = device.CreateTexture(&texDesc);

    wgpu::BufferDescriptor bufDesc;
    bufDesc.size = 4;
    bufDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buf = device.CreateBuffer(&bufDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Clear the texture with a render pass.
    {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.baseMipLevel = kLastMipLevel;

        utils::ComboRenderPassDescriptor renderPass({tex.CreateView(&viewDesc)});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
    }

    // Copy the texture in the buffer.
    {
        wgpu::Extent3D copySize = {1, 1};
        wgpu::ImageCopyTexture src = utils::CreateImageCopyTexture(tex, kLastMipLevel);
        wgpu::ImageCopyBuffer dst = utils::CreateImageCopyBuffer(buf);

        encoder.CopyTextureToBuffer(&src, &dst, &copySize);
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The content of the texture should be reflected in the buffer (prior to the workaround it
    // would be 0s).
    EXPECT_BUFFER_U8_EQ(255, buf, 0);
}

DAWN_INSTANTIATE_TEST(RenderPassTest_RegressionDawn1071,
                      D3D12Backend(),
                      MetalBackend(),
                      MetalBackend({"metal_render_r8_rg8_unorm_small_mip_to_temp_texture"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
