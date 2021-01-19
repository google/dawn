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
#include "utils/TestUtils.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

class DepthStencilCopyTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Draw a square in the bottom left quarter of the screen.
        mVertexModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 6> = array<vec2<f32>, 6>(
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 0.0, -1.0),
                    vec2<f32>(-1.0,  0.0),
                    vec2<f32>(-1.0,  0.0),
                    vec2<f32>( 0.0, -1.0),
                    vec2<f32>( 0.0,  0.0));
                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");
    }

    wgpu::Texture CreateDepthStencilTexture(uint32_t width,
                                            uint32_t height,
                                            wgpu::TextureUsage usage,
                                            uint32_t mipLevelCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        return device.CreateTexture(&texDescriptor);
    }

    wgpu::Texture CreateDepthTexture(uint32_t width,
                                     uint32_t height,
                                     wgpu::TextureUsage usage,
                                     uint32_t mipLevelCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = wgpu::TextureFormat::Depth32Float;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        return device.CreateTexture(&texDescriptor);
    }

    void PopulatePipelineDescriptorWriteDepth(utils::ComboRenderPipelineDescriptor* desc,
                                              wgpu::TextureFormat format,
                                              float regionDepth) {
        desc->vertexStage.module = mVertexModule;

        std::string fsSource = R"(
        [[builtin(frag_depth)]] var<out> FragDepth : f32;
        [[stage(fragment)]] fn main() -> void {
            FragDepth = )" + std::to_string(regionDepth) +
                               ";\n}";

        desc->cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, fsSource.c_str());
        desc->cDepthStencilState.format = format;
        desc->cDepthStencilState.depthWriteEnabled = true;
        desc->depthStencilState = &desc->cDepthStencilState;
        desc->colorStateCount = 0;
    }

    // Initialize the depth/stencil values for the texture using a render pass.
    // The texture will be cleared to the "clear" value, and then bottom left corner will
    // be written with the "region" value.
    void InitializeDepthTextureRegion(wgpu::Texture texture,
                                      float clearDepth,
                                      float regionDepth,
                                      uint32_t mipLevel = 0) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.baseMipLevel = mipLevel;
        viewDesc.mipLevelCount = 1;

        utils::ComboRenderPassDescriptor renderPassDesc({}, texture.CreateView(&viewDesc));
        renderPassDesc.cDepthStencilAttachmentInfo.clearDepth = clearDepth;

        utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
        PopulatePipelineDescriptorWriteDepth(&renderPipelineDesc, wgpu::TextureFormat::Depth32Float,
                                             regionDepth);

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    // Initialize the depth/stencil values for the texture using a render pass.
    // The texture will be cleared to the "clear" values, and then bottom left corner will
    // be written with the "region" values.
    void InitializeDepthStencilTextureRegion(wgpu::Texture texture,
                                             float clearDepth,
                                             float regionDepth,
                                             uint8_t clearStencil,
                                             uint8_t regionStencil,
                                             uint32_t mipLevel = 0) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.baseMipLevel = mipLevel;
        viewDesc.mipLevelCount = 1;

        utils::ComboRenderPassDescriptor renderPassDesc({}, texture.CreateView(&viewDesc));
        renderPassDesc.cDepthStencilAttachmentInfo.clearDepth = clearDepth;
        renderPassDesc.cDepthStencilAttachmentInfo.clearStencil = clearStencil;

        utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
        PopulatePipelineDescriptorWriteDepth(&renderPipelineDesc,
                                             wgpu::TextureFormat::Depth24PlusStencil8, regionDepth);

        renderPipelineDesc.cDepthStencilState.stencilFront.passOp = wgpu::StencilOperation::Replace;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

        // Draw the quad (two triangles)
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(pipeline);
        pass.SetStencilReference(regionStencil);
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::Texture CreateInitializeDepthStencilTextureAndCopyT2T(float clearDepth,
                                                                float regionDepth,
                                                                uint8_t clearStencil,
                                                                uint8_t regionStencil,
                                                                uint32_t width,
                                                                uint32_t height,
                                                                wgpu::TextureUsage usage,
                                                                uint32_t mipLevel = 0) {
        wgpu::Texture src = CreateDepthStencilTexture(
            width, height, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
            mipLevel + 1);

        wgpu::Texture dst = CreateDepthStencilTexture(
            width, height, usage | wgpu::TextureUsage::CopyDst, mipLevel + 1);

        InitializeDepthStencilTextureRegion(src, clearDepth, regionDepth, clearStencil,
                                            regionStencil, mipLevel);

        // Perform a T2T copy of all aspects
        {
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            wgpu::TextureCopyView srcView = utils::CreateTextureCopyView(src, mipLevel, {0, 0, 0});
            wgpu::TextureCopyView dstView = utils::CreateTextureCopyView(dst, mipLevel, {0, 0, 0});
            wgpu::Extent3D copySize = {width >> mipLevel, height >> mipLevel, 1};
            commandEncoder.CopyTextureToTexture(&srcView, &dstView, &copySize);

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);
        }

        return dst;
    }

    // Check depth by uploading expected data to a sampled texture, writing it out as a depth
    // attachment, and then using the "equals" depth test to check the contents are the same.
    void ExpectDepthData(wgpu::Texture depthTexture,
                         wgpu::TextureFormat depthFormat,
                         uint32_t width,
                         uint32_t height,
                         uint32_t mipLevel,
                         std::vector<float> expected) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Make the color attachment that we'll use to read back.
        wgpu::TextureDescriptor colorTexDesc = {};
        colorTexDesc.size = {width, height, 1};
        colorTexDesc.format = wgpu::TextureFormat::R32Uint;
        colorTexDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture colorTexture = device.CreateTexture(&colorTexDesc);

        // Make a sampleable texture to store the depth data. We'll sample this in the
        // shader to output depth.
        wgpu::TextureDescriptor depthDataDesc = {};
        depthDataDesc.size = {width, height, 1};
        depthDataDesc.format = wgpu::TextureFormat::R32Float;
        depthDataDesc.usage = wgpu::TextureUsage::Sampled | wgpu::TextureUsage::CopyDst;
        wgpu::Texture depthDataTexture = device.CreateTexture(&depthDataDesc);

        // Upload the depth data.
        uint32_t bytesPerRow = utils::GetMinimumBytesPerRow(wgpu::TextureFormat::R32Float, width);
        wgpu::BufferDescriptor uploadBufferDesc = {};
        uploadBufferDesc.size = utils::RequiredBytesInCopy(bytesPerRow, height, depthDataDesc.size,
                                                           wgpu::TextureFormat::R32Float);
        uploadBufferDesc.usage = wgpu::BufferUsage::CopySrc;
        uploadBufferDesc.mappedAtCreation = true;

        // TODO(enga): Use WriteTexture when implemented on OpenGL.
        wgpu::Buffer uploadBuffer = device.CreateBuffer(&uploadBufferDesc);
        uint8_t* dst = static_cast<uint8_t*>(uploadBuffer.GetMappedRange());
        float* src = expected.data();
        for (uint32_t y = 0; y < height; ++y) {
            memcpy(dst, src, width * sizeof(float));
            dst += bytesPerRow;
            src += width;
        }
        uploadBuffer.Unmap();

        wgpu::BufferCopyView bufferCopy =
            utils::CreateBufferCopyView(uploadBuffer, 0, bytesPerRow, height);
        wgpu::TextureCopyView textureCopy =
            utils::CreateTextureCopyView(depthDataTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        commandEncoder.CopyBufferToTexture(&bufferCopy, &textureCopy, &depthDataDesc.size);

        // Pipeline for a full screen quad.
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        pipelineDescriptor.vertexStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;

            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 3.0, -1.0),
                    vec2<f32>(-1.0,  3.0));
                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        // Sample the input texture and write out depth. |result| will only be set to 1 if we
        // pass the depth test.
        pipelineDescriptor.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[group(0), binding(0)]] var<uniform_constant> texture0 : texture_2d<f32>;
            [[builtin(frag_coord)]] var<in> FragCoord : vec4<f32>;

            [[location(0)]] var<out> result : u32;
            [[builtin(frag_depth)]] var<out> FragDepth : f32;

            [[stage(fragment)]] fn main() -> void {
                result = 1u;
                FragDepth = textureLoad(texture0, vec2<i32>(FragCoord.xy), 0)[0];
            })");

        // Pass the depth test only if the depth is equal.
        pipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDescriptor.depthStencilState = &pipelineDescriptor.cDepthStencilState;
        pipelineDescriptor.cDepthStencilState.format = depthFormat;
        pipelineDescriptor.cDepthStencilState.depthCompare = wgpu::CompareFunction::Equal;
        pipelineDescriptor.cColorStates[0].format = colorTexDesc.format;

        // TODO(jiawei.shao@intel.com): The Intel Mesa Vulkan driver can't set gl_FragDepth unless
        // depthWriteEnabled == true. This either needs to be fixed in the driver or restricted by
        // the WebGPU API.
        pipelineDescriptor.cDepthStencilState.depthWriteEnabled = true;

        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.baseMipLevel = mipLevel;
        viewDesc.mipLevelCount = 1;

        utils::ComboRenderPassDescriptor passDescriptor({colorTexture.CreateView()},
                                                        depthTexture.CreateView(&viewDesc));
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        // Bind the depth data texture.
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, depthDataTexture.CreateView()}});

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint32_t> colorData(width * height, 1u);
        EXPECT_TEXTURE_EQ(colorData.data(), colorTexture, 0, 0, width, height, 0, 0);
    }

    wgpu::ShaderModule mVertexModule;
};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromDepthAspect) {
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture depthTexture = CreateDepthTexture(
        kWidth, kHeight, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    InitializeDepthTextureRegion(depthTexture, 0.f, 0.3f);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    std::vector<float> expectedData = {
        0.0, 0.0, 0.0, 0.0,  //
        0.0, 0.0, 0.0, 0.0,  //
        0.3, 0.3, 0.0, 0.0,  //
        0.3, 0.3, 0.0, 0.0,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::DepthOnly);
}

// Test copying the stencil-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromStencilAspect) {
    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture depthStencilTexture = CreateDepthStencilTexture(
        kWidth, kHeight, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    InitializeDepthStencilTextureRegion(depthStencilTexture, 0.f, 0.3f, 0u, 1u);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    std::vector<uint8_t> expectedData = {
        0u, 0u, 0u, 0u,  //
        0u, 0u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), depthStencilTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying the non-zero mip, stencil-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromNonZeroMipStencilAspect) {
    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    wgpu::Texture depthStencilTexture = CreateDepthStencilTexture(
        9, 9, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, 2);

    InitializeDepthStencilTextureRegion(depthStencilTexture, 0.f, 0.3f, 0u, 1u, 1u);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    std::vector<uint8_t> expectedData = {
        0u, 0u, 0u, 0u,  //
        0u, 0u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
        1u, 1u, 0u, 0u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), depthStencilTexture, 0, 0, 4, 4, 1, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying the non-zero mip, depth-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromNonZeroMipDepthAspect) {
    wgpu::Texture depthTexture = CreateDepthTexture(
        9, 9, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, 2);

    InitializeDepthTextureRegion(depthTexture, 0.f, 0.4f, 1);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    std::vector<float> expectedData = {
        0.0, 0.0, 0.0, 0.0,  //
        0.0, 0.0, 0.0, 0.0,  //
        0.4, 0.4, 0.0, 0.0,  //
        0.4, 0.4, 0.0, 0.0,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, 0, 0, 4, 4, 1, 0,
                      wgpu::TextureAspect::DepthOnly);
}

// Test copying both aspects in a T2T copy, then copying only stencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencil) {
    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    // Maybe has to do with the RenderAttachment usage. Notably, a later test
    // T2TBothAspectsThenCopyNonRenderableStencil does not use RenderAttachment and works correctly.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableStencil) {
    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::CopySrc);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable, non-zero mip stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableNonZeroMipStencil) {
    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    // Maybe has to do with the non-zero mip. Notably, a previous test
    // T2TBothAspectsThenCopyNonRenderableStencil works correctly.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 9, 9, wgpu::TextureUsage::CopySrc, 1);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, 0, 0, 4, 4, 1, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying both aspects in a T2T copy, then copying only depth.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepth) {
    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectDepthData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth, kHeight, 0,
                    {
                        0.1, 0.1, 0.1, 0.1,  //
                        0.1, 0.1, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                    });
}

// Test copying both aspects in a T2T copy, then copying only depth at a nonzero mip.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonZeroMipDepth) {
    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 8, 8, wgpu::TextureUsage::RenderAttachment, 1);

    // Check the depth
    ExpectDepthData(texture, wgpu::TextureFormat::Depth24PlusStencil8, 4, 4, 1,
                    {
                        0.1, 0.1, 0.1, 0.1,  //
                        0.1, 0.1, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                    });
}

// Test copying both aspects in a T2T copy, then copying stencil, then copying depth
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencilThenDepth) {
    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);

    // Check the depth
    ExpectDepthData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth, kHeight, 0,
                    {
                        0.1, 0.1, 0.1, 0.1,  //
                        0.1, 0.1, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                    });
}

// Test copying both aspects in a T2T copy, then copying depth, then copying stencil
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepthThenStencil) {
    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    // It seems like the depth readback copy mutates the stencil because the previous
    // test T2TBothAspectsThenCopyStencil passes.
    // T2TBothAspectsThenCopyStencilThenDepth which checks stencil first also passes.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/634): Diagnose and fix ANGLE failure.
    DAWN_SKIP_TEST_IF(IsANGLE());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectDepthData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth, kHeight, 0,
                    {
                        0.1, 0.1, 0.1, 0.1,  //
                        0.1, 0.1, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                        0.3, 0.3, 0.1, 0.1,  //
                    });

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying to the stencil-aspect of a buffer
TEST_P(DepthStencilCopyTests, ToStencilAspect) {
    // Copies to a single aspect are unsupported on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    // TODO(enga): Figure out why this fails on MacOS Intel Iris.
    // It passes on AMD Radeon Pro and Intel HD Graphics 630.
    DAWN_SKIP_TEST_IF(IsMetal() && IsIntel());

    // TODO(enga): Figure out why this fails on Windows Intel Vulkan.
    // Reading back the depth does not work.
    DAWN_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture depthStencilTexture =
        CreateDepthStencilTexture(kWidth, kHeight,
                                  wgpu::TextureUsage::RenderAttachment |
                                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Clear depth to 0.7, so we can check that the stencil copy doesn't mutate the depth.
        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.clearDepth = 0.7;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

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

    // Upload the stencil data.
    wgpu::TextureDataLayout stencilDataLayout = {};
    stencilDataLayout.bytesPerRow = kWidth * sizeof(uint8_t);

    wgpu::TextureCopyView stencilDataCopyView = utils::CreateTextureCopyView(
        depthStencilTexture, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);

    wgpu::Extent3D writeSize = {kWidth, kHeight, 1};
    queue.WriteTexture(&stencilDataCopyView, stencilData.data(),
                       stencilData.size() * sizeof(uint8_t), &stencilDataLayout, &writeSize);

    // Decrement the stencil value in a render pass to ensure the data is visible to the pipeline.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        // Create a render pipline which decrements the stencil value for passing fragments.
        // A quad is drawn in the bottom left.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
        renderPipelineDesc.vertexStage.module = mVertexModule;
        renderPipelineDesc.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[stage(fragment)]] fn main() -> void {
            })");
        renderPipelineDesc.cDepthStencilState.format = wgpu::TextureFormat::Depth24PlusStencil8;
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

    // Copy back the stencil data and check it is correct.
    EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, 0, 0, kWidth, kHeight, 0, 0,
                      wgpu::TextureAspect::StencilOnly);

    ExpectDepthData(depthStencilTexture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth, kHeight,
                    0,
                    {
                        0.7, 0.7, 0.7, 0.7,  //
                        0.7, 0.7, 0.7, 0.7,  //
                        0.7, 0.7, 0.7, 0.7,  //
                        0.7, 0.7, 0.7, 0.7,  //
                    });
}

DAWN_INSTANTIATE_TEST(DepthStencilCopyTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
