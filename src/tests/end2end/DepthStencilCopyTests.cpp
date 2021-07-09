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
#include "utils/TextureUtils.h"
#include "utils/WGPUHelpers.h"

class DepthStencilCopyTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Draw a square in the bottom left quarter of the screen.
        mVertexModule = utils::CreateShaderModule(device, R"(
            [[stage(vertex)]]
            fn main([[builtin(vertex_index)]] VertexIndex : u32) -> [[builtin(position)]] vec4<f32> {
                var pos = array<vec2<f32>, 6>(
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 0.0, -1.0),
                    vec2<f32>(-1.0,  0.0),
                    vec2<f32>(-1.0,  0.0),
                    vec2<f32>( 0.0, -1.0),
                    vec2<f32>( 0.0,  0.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
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
        desc->vertex.module = mVertexModule;

        std::string fsSource = R"(
        [[stage(fragment)]] fn main() -> [[builtin(frag_depth)]] f32 {
            return )" + std::to_string(regionDepth) +
                               ";\n}";

        desc->cFragment.module = utils::CreateShaderModule(device, fsSource.c_str());
        wgpu::DepthStencilState* depthStencil = desc->EnableDepthStencil(format);
        depthStencil->depthWriteEnabled = true;
        desc->cFragment.targetCount = 0;
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

        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
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

        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
        PopulatePipelineDescriptorWriteDepth(&renderPipelineDesc,
                                             wgpu::TextureFormat::Depth24PlusStencil8, regionDepth);
        renderPipelineDesc.cDepthStencil.stencilFront.passOp = wgpu::StencilOperation::Replace;

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
            wgpu::ImageCopyTexture srcView =
                utils::CreateImageCopyTexture(src, mipLevel, {0, 0, 0});
            wgpu::ImageCopyTexture dstView =
                utils::CreateImageCopyTexture(dst, mipLevel, {0, 0, 0});
            wgpu::Extent3D copySize = {width >> mipLevel, height >> mipLevel, 1};
            commandEncoder.CopyTextureToTexture(&srcView, &dstView, &copySize);

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);
        }

        return dst;
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
    EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::DepthOnly);
}

// Test copying the stencil-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromStencilAspect) {
    // TODO(crbug.com/dawn/667): Work around the fact that some platforms are unable to read
    // stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

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
    EXPECT_TEXTURE_EQ(expectedData.data(), depthStencilTexture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying the non-zero mip, stencil-only aspect into a buffer.
TEST_P(DepthStencilCopyTests, FromNonZeroMipStencilAspect) {
    // TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/667): Work around some platforms' inability to read back stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

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
    EXPECT_TEXTURE_EQ(expectedData.data(), depthStencilTexture, {0, 0}, {4, 4}, 1,
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
    EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, {0, 0}, {4, 4}, 1,
                      wgpu::TextureAspect::DepthOnly);
}

// Test copying both aspects in a T2T copy, then copying only stencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencil) {
    // TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    // Maybe has to do with the RenderAttachment usage. Notably, a later test
    // T2TBothAspectsThenCopyNonRenderableStencil does not use RenderAttachment and works correctly.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/667): Work around some platforms' inability to read back stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

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
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableStencil) {
    // TODO(crbug.com/dawn/667): Work around some platforms' inability to read back stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

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
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable, non-zero mip stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableNonZeroMipStencil) {
    /// TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    // Maybe has to do with the non-zero mip. Notably, a previous test
    // T2TBothAspectsThenCopyNonRenderableStencil works correctly.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/667): Work around some platforms' inability to read back stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 9, 9, wgpu::TextureUsage::CopySrc, 1);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {4, 4}, 1,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying both aspects in a T2T copy, then copying only depth.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepth) {
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth,
                                  kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying only depth at a nonzero mip.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonZeroMipDepth) {
    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 8, 8, wgpu::TextureUsage::RenderAttachment, 1);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, wgpu::TextureFormat::Depth24PlusStencil8, 4, 4, 0, 1,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying stencil, then copying depth
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencilThenDepth) {
    // TODO(crbug.com/dawn/667): Work around some platforms' inability to read back stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

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
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth,
                                  kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying depth, then copying stencil
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepthThenStencil) {
    // TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    // It seems like the depth readback copy mutates the stencil because the previous
    // test T2TBothAspectsThenCopyStencil passes.
    // T2TBothAspectsThenCopyStencilThenDepth which checks stencil first also passes.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/667): Work around the fact that some platforms are unable to read
    // stencil.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, wgpu::TextureFormat::Depth24PlusStencil8, kWidth,
                                  kHeight, 0, 0,
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
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying to the stencil-aspect of a buffer
TEST_P(DepthStencilCopyTests, ToStencilAspect) {
    // Copies to a single aspect are unsupported on OpenGL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    // TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

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

    wgpu::ImageCopyTexture stencilDataCopyTexture = utils::CreateImageCopyTexture(
        depthStencilTexture, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);

    wgpu::Extent3D writeSize = {kWidth, kHeight, 1};
    queue.WriteTexture(&stencilDataCopyTexture, stencilData.data(),
                       stencilData.size() * sizeof(uint8_t), &stencilDataLayout, &writeSize);

    // Decrement the stencil value in a render pass to ensure the data is visible to the pipeline.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        // Create a render pipline which decrements the stencil value for passing fragments.
        // A quad is drawn in the bottom left.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
        renderPipelineDesc.vertex.module = mVertexModule;
        renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            [[stage(fragment)]] fn main() {
            })");
        wgpu::DepthStencilState* depthStencil =
            renderPipelineDesc.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->stencilFront.passOp = wgpu::StencilOperation::DecrementClamp;
        renderPipelineDesc.cFragment.targetCount = 0;

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
    EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);

    ExpectAttachmentDepthTestData(depthStencilTexture, wgpu::TextureFormat::Depth24PlusStencil8,
                                  kWidth, kHeight, 0, 0,
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
