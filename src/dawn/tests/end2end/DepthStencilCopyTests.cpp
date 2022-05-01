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

#include <array>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(DepthStencilCopyTestParams, TextureFormat);

constexpr std::array<wgpu::TextureFormat, 3> kValidDepthCopyTextureFormats = {
    wgpu::TextureFormat::Depth16Unorm,
    wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth32FloatStencil8,
};

constexpr std::array<wgpu::TextureFormat, 1> kValidDepthCopyFromBufferFormats = {
    wgpu::TextureFormat::Depth16Unorm,
};
}  // namespace

class DepthStencilCopyTests : public DawnTestWithParams<DepthStencilCopyTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<DepthStencilCopyTestParams>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);

        // Draw a square in the bottom left quarter of the screen.
        mVertexModule = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
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

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth24UnormStencil8:
                if (SupportsFeatures({wgpu::FeatureName::Depth24UnormStencil8})) {
                    mIsFormatSupported = true;
                    return {wgpu::FeatureName::Depth24UnormStencil8};
                }
                return {};
            case wgpu::TextureFormat::Depth32FloatStencil8:
                if (SupportsFeatures({wgpu::FeatureName::Depth32FloatStencil8})) {
                    mIsFormatSupported = true;
                    return {wgpu::FeatureName::Depth32FloatStencil8};
                }
                return {};
            default:
                mIsFormatSupported = true;
                return {};
        }
    }

    bool IsValidDepthCopyTextureFormat() {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth16Unorm:
            case wgpu::TextureFormat::Depth32Float:
            case wgpu::TextureFormat::Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    wgpu::Texture CreateTexture(uint32_t width,
                                uint32_t height,
                                wgpu::TextureUsage usage,
                                uint32_t mipLevelCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = GetParam().mTextureFormat;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        return device.CreateTexture(&texDescriptor);
    }

    wgpu::Texture CreateDepthStencilTexture(uint32_t width,
                                            uint32_t height,
                                            wgpu::TextureUsage usage,
                                            uint32_t mipLevelCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = GetParam().mTextureFormat;
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
        texDescriptor.format = GetParam().mTextureFormat;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        return device.CreateTexture(&texDescriptor);
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
        wgpu::TextureFormat format = GetParam().mTextureFormat;
        // Create the render pass used for the initialization.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
        renderPipelineDesc.vertex.module = mVertexModule;
        renderPipelineDesc.cFragment.targetCount = 0;

        wgpu::DepthStencilState* depthStencil = renderPipelineDesc.EnableDepthStencil(format);

        if (utils::IsStencilOnlyFormat(format)) {
            depthStencil->depthCompare = wgpu::CompareFunction::Always;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                @stage(fragment) fn main() {}
            )");
        } else {
            depthStencil->depthWriteEnabled = true;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, std::string(R"(
                @stage(fragment) fn main() -> @builtin(frag_depth) f32 {
                    return )" + std::to_string(regionDepth) + R"(;
                })")
                                                                                        .c_str());
        }
        if (!utils::IsDepthOnlyFormat(format)) {
            depthStencil->stencilFront.passOp = wgpu::StencilOperation::Replace;
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

        // Build the render pass used for initialization.
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.baseMipLevel = mipLevel;
        viewDesc.mipLevelCount = 1;

        utils::ComboRenderPassDescriptor renderPassDesc({}, texture.CreateView(&viewDesc));
        renderPassDesc.UnsetDepthStencilLoadStoreOpsForFormat(format);
        renderPassDesc.cDepthStencilAttachmentInfo.depthClearValue = clearDepth;
        renderPassDesc.cDepthStencilAttachmentInfo.stencilClearValue = clearStencil;

        // Draw the quad (two triangles)
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(pipeline);
        pass.SetStencilReference(regionStencil);
        pass.Draw(6);
        pass.End();

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

    uint32_t BufferSizeForTextureCopy(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm) {
        uint32_t bytesPerPixel = utils::GetTexelBlockSizeInBytes(format);
        uint32_t bytesPerRow = Align(width * bytesPerPixel, kTextureBytesPerRowAlignment);
        return (bytesPerRow * (height - 1) + width * bytesPerPixel) * depth;
    }

    wgpu::ShaderModule mVertexModule;

  private:
    bool mIsFormatSupported = false;
};

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
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying only depth at a nonzero mip.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonZeroMipDepth) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 8, 8, wgpu::TextureUsage::RenderAttachment, 1);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, 4, 4, 0, 1,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying stencil, then copying depth
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencilThenDepth) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

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
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying depth, then copying stencil
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepthThenStencil) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

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
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
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

class DepthCopyTests : public DepthStencilCopyTests {};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromDepthAspect) {
    // TODO(crbug.com/dawn/1237): Depth16Unorm test failed on OpenGL and OpenGLES which says
    // Invalid format and type combination in glReadPixels
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm &&
                             (IsOpenGL() || IsOpenGLES()));

    // TODO(crbug.com/dawn/1291): These tests are failing on NVidia GLES
    // when using Tint/GLSL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES() && IsNvidia());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateTexture(
        kWidth, kHeight, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    constexpr float kInitDepth = 0.2f;
    InitializeDepthStencilTextureRegion(texture, 0.f, kInitDepth, 0, 0);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
        uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
        std::vector<uint16_t> expectedData = {
            0,        0,        0, 0,  //
            0,        0,        0, 0,  //
            expected, expected, 0, 0,  //
            expected, expected, 0, 0,  //
        };
        EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                          wgpu::TextureAspect::DepthOnly);
    } else {
        std::vector<float> expectedData = {
            0.0,        0.0,        0.0, 0.0,  //
            0.0,        0.0,        0.0, 0.0,  //
            kInitDepth, kInitDepth, 0.0, 0.0,  //
            kInitDepth, kInitDepth, 0.0, 0.0,  //
        };
        EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                          wgpu::TextureAspect::DepthOnly);
    }
}

// Test copying the non-zero mip, depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromNonZeroMipDepthAspect) {
    // TODO(crbug.com/dawn/1237): Depth16Unorm test failed on OpenGL and OpenGLES which says
    // Invalid format and type combination in glReadPixels
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm &&
                             (IsOpenGL() || IsOpenGLES()));

    // TODO(crbug.com/dawn/1291): These tests are failing on NVidia GLES
    // when using Tint/GLSL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES() && IsNvidia());

    wgpu::Texture depthTexture = CreateDepthTexture(
        9, 9, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, 2);

    constexpr float kInitDepth = 0.4f;
    InitializeDepthStencilTextureRegion(depthTexture, 0.f, kInitDepth, 0, 0, /*mipLevel*/ 1);

    // This expectation is the test as it performs the CopyTextureToBuffer.
    if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
        uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
        std::vector<uint16_t> expectedData = {
            0,        0,        0, 0,  //
            0,        0,        0, 0,  //
            expected, expected, 0, 0,  //
            expected, expected, 0, 0,  //
        };
        EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, {0, 0}, {4, 4}, 1,
                          wgpu::TextureAspect::DepthOnly);
    } else {
        std::vector<float> expectedData = {
            0.0,        0.0,        0.0, 0.0,  //
            0.0,        0.0,        0.0, 0.0,  //
            kInitDepth, kInitDepth, 0.0, 0.0,  //
            kInitDepth, kInitDepth, 0.0, 0.0,  //
        };
        EXPECT_TEXTURE_EQ(expectedData.data(), depthTexture, {0, 0}, {4, 4}, 1,
                          wgpu::TextureAspect::DepthOnly);
    }
}

class DepthCopyFromBufferTests : public DepthStencilCopyTests {};

// Test copying the depth-only aspect from a buffer.
TEST_P(DepthCopyFromBufferTests, BufferToDepthAspect) {
    // TODO(crbug.com/dawn/1237): Depth16Unorm test failed on OpenGL and OpenGLES which says
    // Invalid format and type combination in glReadPixels
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm &&
                             (IsOpenGL() || IsOpenGLES()));

    constexpr uint32_t kWidth = 8;
    constexpr uint32_t kHeight = 1;

    wgpu::Texture destTexture =
        CreateTexture(kWidth, kHeight, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = BufferSizeForTextureCopy(kWidth, kHeight, 1, GetParam().mTextureFormat);
    descriptor.usage = wgpu::BufferUsage::CopySrc;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

    wgpu::ImageCopyBuffer imageCopyBuffer =
        utils::CreateImageCopyBuffer(srcBuffer, 0, 256, kHeight);
    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(destTexture, 0, {0, 0, 0}, wgpu::TextureAspect::DepthOnly);
    wgpu::Extent3D extent = {kWidth, kHeight, 1};

    constexpr float kInitDepth = 0.2f;

    // This expectation is the test as it performs the CopyTextureToBuffer.
    if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
        uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
        std::vector<uint16_t> expectedData = {
            0, 0, expected, expected, 0, 0, expected, expected,
        };
        size_t expectedSize = expectedData.size() * sizeof(uint16_t);

        memcpy(srcBuffer.GetMappedRange(0, expectedSize), expectedData.data(), expectedSize);
        srcBuffer.Unmap();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &extent);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_EQ(expectedData.data(), destTexture, {0, 0}, {kWidth, kHeight}, 0,
                          wgpu::TextureAspect::DepthOnly);
    } else {
        std::vector<float> expectedData = {
            0.0, 0.0, kInitDepth, kInitDepth, 0.0, 0.0, kInitDepth, kInitDepth,
        };
        size_t expectedSize = expectedData.size() * sizeof(float);

        memcpy(srcBuffer.GetMappedRange(0, expectedSize), expectedData.data(), expectedSize);
        srcBuffer.Unmap();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &extent);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_EQ(expectedData.data(), destTexture, {0, 0}, {kWidth, kHeight}, 0,
                          wgpu::TextureAspect::DepthOnly);
    }
}

class StencilCopyTests : public DepthStencilCopyTests {};

// Test copying the stencil-only aspect into a buffer.
TEST_P(StencilCopyTests, FromStencilAspect) {
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
TEST_P(StencilCopyTests, FromNonZeroMipStencilAspect) {
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

// Test copying to the stencil-aspect of a buffer
TEST_P(StencilCopyTests, ToStencilAspect) {
    // Copies to a single aspect are unsupported on OpenGL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    // TODO(crbug.com/dawn/704): Readback after clear via stencil copy does not work
    // on some Intel drivers.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(crbug.com/dawn/1273): Fails on Win11 with D3D12 debug layer and full validation
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    const bool hasDepth = !utils::IsStencilOnlyFormat(GetParam().mTextureFormat);

    wgpu::Texture depthStencilTexture =
        CreateDepthStencilTexture(kWidth, kHeight,
                                  wgpu::TextureUsage::RenderAttachment |
                                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

    if (hasDepth) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Clear depth to 0.7, so we can check that the stencil copy doesn't mutate the depth.
        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mTextureFormat);
        passDescriptor.cDepthStencilAttachmentInfo.depthClearValue = 0.7;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.End();

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
            @stage(fragment) fn main() {
            })");
        renderPipelineDesc.cFragment.targetCount = 0;
        wgpu::DepthStencilState* depthStencil =
            renderPipelineDesc.EnableDepthStencil(GetParam().mTextureFormat);
        depthStencil->stencilFront.passOp = wgpu::StencilOperation::DecrementClamp;
        if (!hasDepth) {
            depthStencil->depthWriteEnabled = false;
            depthStencil->depthCompare = wgpu::CompareFunction::Always;
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

        // Create a render pass which loads the stencil. We want to load the values we
        // copied in. Also load the canary depth values so they're not lost.
        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mTextureFormat);

        // Draw the quad in the bottom left (two triangles).
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.End();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    // Copy back the stencil data and check it is correct.
    EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);

    if (hasDepth) {
        ExpectAttachmentDepthTestData(depthStencilTexture, GetParam().mTextureFormat, kWidth,
                                      kHeight, 0, 0,
                                      {
                                          0.7, 0.7, 0.7, 0.7,  //
                                          0.7, 0.7, 0.7, 0.7,  //
                                          0.7, 0.7, 0.7, 0.7,  //
                                          0.7, 0.7, 0.7, 0.7,  //
                                      });
    }
}

DAWN_INSTANTIATE_TEST_P(DepthStencilCopyTests,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         // Test with the vulkan_use_s8 toggle forced on and off.
                         VulkanBackend({"vulkan_use_s8"}, {}),
                         VulkanBackend({}, {"vulkan_use_s8"})},
                        std::vector<wgpu::TextureFormat>(utils::kDepthAndStencilFormats.begin(),
                                                         utils::kDepthAndStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthCopyTests,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyTextureFormats.begin(),
                                                         kValidDepthCopyTextureFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthCopyFromBufferTests,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyFromBufferFormats.begin(),
                                                         kValidDepthCopyFromBufferFormats.end()));

DAWN_INSTANTIATE_TEST_P(StencilCopyTests,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         // Test with the vulkan_use_s8 toggle forced on and off.
                         VulkanBackend({"vulkan_use_s8"}, {}),
                         VulkanBackend({}, {"vulkan_use_s8"})},
                        std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(),
                                                         utils::kStencilFormats.end()));
