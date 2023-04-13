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

#include <algorithm>
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
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f( 0.0, -1.0),
                    vec2f(-1.0,  0.0),
                    vec2f(-1.0,  0.0),
                    vec2f( 0.0, -1.0),
                    vec2f( 0.0,  0.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mTextureFormat) {
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
                                             uint32_t mipLevel = 0,
                                             uint32_t arrayLayer = 0) {
        wgpu::TextureFormat format = GetParam().mTextureFormat;
        // Create the render pass used for the initialization.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
        renderPipelineDesc.vertex.module = mVertexModule;
        renderPipelineDesc.cFragment.targetCount = 0;

        wgpu::DepthStencilState* depthStencil = renderPipelineDesc.EnableDepthStencil(format);

        if (utils::IsStencilOnlyFormat(format)) {
            depthStencil->depthCompare = wgpu::CompareFunction::Always;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                @fragment fn main() {}
            )");
        } else {
            depthStencil->depthWriteEnabled = true;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, std::string(R"(
                @fragment fn main() -> @builtin(frag_depth) f32 {
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
        viewDesc.baseArrayLayer = arrayLayer;
        viewDesc.arrayLayerCount = 1;

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

    uint32_t BufferSizeForTextureCopy(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm,
                                      wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        uint32_t bytesPerPixel = 0;
        switch (format) {
            case wgpu::TextureFormat::Depth24PlusStencil8: {
                ASSERT(aspect == wgpu::TextureAspect::StencilOnly);
                bytesPerPixel = 1;
                break;
            }
            case wgpu::TextureFormat::Depth32FloatStencil8: {
                switch (aspect) {
                    case wgpu::TextureAspect::DepthOnly:
                        bytesPerPixel = 4;
                        break;
                    case wgpu::TextureAspect::StencilOnly:
                        bytesPerPixel = 1;
                        break;
                    default:
                        UNREACHABLE();
                        break;
                }
                break;
            }
            default:
                bytesPerPixel = utils::GetTexelBlockSizeInBytes(format);
                break;
        }

        uint32_t bytesPerRow = Align(width * bytesPerPixel, kTextureBytesPerRowAlignment);
        return (bytesPerRow * (height - 1) + width * bytesPerPixel) * depth;
    }

    wgpu::ShaderModule mVertexModule;

  private:
    bool mIsFormatSupported = false;
};

// Test copying both aspects in a T2T copy, then copying only stencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencil) {
    // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type combination.
    DAWN_SUPPRESS_TEST_IF(IsANGLE());

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
    // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type combination.
    DAWN_SUPPRESS_TEST_IF(IsANGLE());

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
    // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type combination.
    DAWN_SUPPRESS_TEST_IF(IsANGLE());

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

class DepthCopyTests : public DepthStencilCopyTests {
  public:
    void DoCopyFromDepthTest(uint32_t bufferCopyOffset,
                             float initDepth,
                             uint32_t textureWidth,
                             uint32_t textureHeight,
                             uint32_t testLevel) {
        // TODO(crbug.com/dawn/1237): Depth16Unorm test failed on OpenGL and OpenGLES which says
        // Invalid format and type combination in glReadPixels
        DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm &&
                                 (IsOpenGL() || IsOpenGLES()));

        // TODO(crbug.com/dawn/1291): These tests are failing on GLES (both native and ANGLE)
        // when using Tint/GLSL.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

        uint32_t mipLevelCount = testLevel + 1;
        wgpu::Texture texture = CreateTexture(
            textureWidth, textureHeight,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, mipLevelCount);

        InitializeDepthStencilTextureRegion(texture, 0.f, initDepth, 0, 0, testLevel);

        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        wgpu::Extent3D copySize = {copyWidth, copyHeight, 1};

        constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size =
            bufferCopyOffset + BufferSizeForTextureCopy(copyWidth, copyHeight, 1,
                                                        GetParam().mTextureFormat,
                                                        wgpu::TextureAspect::DepthOnly);
        wgpu::Buffer destinationBuffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
            texture, testLevel, {0, 0, 0}, wgpu::TextureAspect::DepthOnly);
        wgpu::ImageCopyBuffer imageCopyBuffer = utils::CreateImageCopyBuffer(
            destinationBuffer, bufferCopyOffset, kBytesPerRow, copyHeight);
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
            uint16_t expected = FloatToUnorm<uint16_t>(initDepth);
            std::vector<uint16_t> expectedData = {
                0,        0,        0, 0,  //
                0,        0,        0, 0,  //
                expected, expected, 0, 0,  //
                expected, expected, 0, 0,  //
            };

            for (uint32_t y = 0; y < copyHeight; ++y) {
                EXPECT_BUFFER_U16_RANGE_EQ(expectedData.data() + copyWidth * y, destinationBuffer,
                                           bufferCopyOffset + y * kBytesPerRow, copyWidth);
            }

        } else {
            std::vector<float> expectedData = {
                0.0,       0.0,       0.0, 0.0,  //
                0.0,       0.0,       0.0, 0.0,  //
                initDepth, initDepth, 0.0, 0.0,  //
                initDepth, initDepth, 0.0, 0.0,  //
            };

            for (uint32_t y = 0; y < copyHeight; ++y) {
                EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data() + copyWidth * y, destinationBuffer,
                                             bufferCopyOffset + y * kBytesPerRow, copyWidth);
            }
        }
    }
};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromDepthAspect) {
    constexpr float kInitDepth = 0.2f;
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kTestLevel = 0;
    DoCopyFromDepthTest(kBufferCopyOffset, kInitDepth, kWidth, kHeight, kTestLevel);
}

// Test copying the depth-only aspect into a buffer at a non-zero offset.
TEST_P(DepthCopyTests, FromDepthAspectToBufferAtNonZeroOffset) {
    constexpr float kInitDepth = 0.2f;
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kTestLevel = 0;
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {4u, 512u};
    for (uint32_t offset : kBufferCopyOffsets) {
        DoCopyFromDepthTest(offset, kInitDepth, kWidth, kHeight, kTestLevel);
    }
}

// Test copying the non-zero mip, depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromNonZeroMipDepthAspect) {
    constexpr float kInitDepth = 0.2f;
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    constexpr uint32_t kTestLevel = 1;
    DoCopyFromDepthTest(kBufferCopyOffset, kInitDepth, kWidth, kHeight, kTestLevel);
}

class DepthCopyFromBufferTests : public DepthStencilCopyTests {
  public:
    void DoTest(uint32_t bufferCopyOffset, bool hasRenderAttachmentUsage) {
        // TODO(crbug.com/dawn/1237): Depth16Unorm test failed on OpenGL and OpenGLES which says
        // Invalid format and type combination in glReadPixels
        DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm &&
                                 (IsOpenGL() || IsOpenGLES()));

        constexpr uint32_t kWidth = 8;
        constexpr uint32_t kHeight = 1;

        wgpu::TextureUsage textureUsage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        // On D3D12 backend the resource flag D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL is set only
        // when wgpu::TextureUsage::RenderAttachment is set on the creation of a depth stencil
        // texture.
        if (hasRenderAttachmentUsage) {
            textureUsage |= wgpu::TextureUsage::RenderAttachment;
        }
        wgpu::Texture destTexture = CreateTexture(kWidth, kHeight, textureUsage);

        wgpu::BufferDescriptor descriptor;
        descriptor.size = bufferCopyOffset +
                          BufferSizeForTextureCopy(kWidth, kHeight, 1, GetParam().mTextureFormat);
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        descriptor.mappedAtCreation = true;
        wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

        constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(srcBuffer, bufferCopyOffset, kBytesPerRow, kHeight);
        wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
            destTexture, 0, {0, 0, 0}, wgpu::TextureAspect::DepthOnly);
        wgpu::Extent3D extent = {kWidth, kHeight, 1};

        constexpr float kInitDepth = 0.2f;

        // This expectation is the test as it performs the CopyTextureToBuffer.
        if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
            uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
            std::vector<uint16_t> expectedData = {
                0, 0, expected, expected, 0, 0, expected, expected,
            };
            size_t expectedSize = expectedData.size() * sizeof(uint16_t);
            memcpy(srcBuffer.GetMappedRange(bufferCopyOffset, expectedSize), expectedData.data(),
                   expectedSize);
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

            memcpy(srcBuffer.GetMappedRange(bufferCopyOffset, expectedSize), expectedData.data(),
                   expectedSize);
            srcBuffer.Unmap();

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &extent);
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(expectedData.data(), destTexture, {0, 0}, {kWidth, kHeight}, 0,
                              wgpu::TextureAspect::DepthOnly);
        }
    }
};

// Test copying the depth-only aspect from a buffer.
TEST_P(DepthCopyFromBufferTests, BufferToDepthAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr bool kIsRenderable = false;
    DoTest(kBufferCopyOffset, kIsRenderable);
}

// Test copying the depth-only aspect from a buffer at a non-zero offset.
TEST_P(DepthCopyFromBufferTests, BufferToNonRenderableDepthAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    constexpr bool kIsRenderable = false;
    for (uint32_t offset : kBufferCopyOffsets) {
        DoTest(offset, kIsRenderable);
    }
}

// Test copying the depth-only aspect from a buffer at a non-zero offset.
TEST_P(DepthCopyFromBufferTests, BufferToRenderableDepthAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    constexpr bool kIsRenderable = true;
    for (uint32_t offset : kBufferCopyOffsets) {
        DoTest(offset, kIsRenderable);
    }
}

class StencilCopyTests : public DepthStencilCopyTests {
  public:
    void DoCopyFromStencilTest(uint32_t bufferCopyOffset,
                               uint32_t textureWidth,
                               uint32_t textureHeight,
                               uint32_t testLevel) {
        // TODO(crbug.com/dawn/1497): glReadPixels: GL error: HIGH: Invalid format and type
        // combination.
        DAWN_SUPPRESS_TEST_IF(IsANGLE());

        // TODO(crbug.com/dawn/667): Work around the fact that some platforms are unable to read
        // stencil.
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_depth_stencil_read"));

        uint32_t mipLevelCount = testLevel + 1;
        wgpu::Texture depthStencilTexture = CreateDepthStencilTexture(
            textureWidth, textureHeight,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, mipLevelCount);

        InitializeDepthStencilTextureRegion(depthStencilTexture, 0.f, 0.3f, 0u, 1u, testLevel);

        std::vector<uint8_t> expectedData = {
            0u, 0u, 0u, 0u,  //
            0u, 0u, 0u, 0u,  //
            1u, 1u, 0u, 0u,  //
            1u, 1u, 0u, 0u,  //
        };

        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        ASSERT_EQ(expectedData.size(), copyWidth * copyHeight);
        wgpu::Extent3D copySize = {copyWidth, copyHeight, 1};

        constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size =
            bufferCopyOffset + BufferSizeForTextureCopy(copyWidth, copyHeight, 1,
                                                        GetParam().mTextureFormat,
                                                        wgpu::TextureAspect::StencilOnly);
        wgpu::Buffer destinationBuffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
            depthStencilTexture, testLevel, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
        wgpu::ImageCopyBuffer imageCopyBuffer = utils::CreateImageCopyBuffer(
            destinationBuffer, bufferCopyOffset, kBytesPerRow, copyHeight);
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        for (uint32_t y = 0; y < copyHeight; ++y) {
            EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data() + copyWidth * y, destinationBuffer,
                                      bufferCopyOffset + y * kBytesPerRow, copyWidth);
        }
    }

    void DoCopyToStencilTest(uint32_t bufferCopyOffset) {
        // Copies to a single aspect are unsupported on OpenGL.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

        // TODO(crbug.com/dawn/1273): Fails on Win11 with D3D12 debug layer and full validation
        DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

        // Create a stencil texture
        constexpr uint32_t kWidth = 4;
        constexpr uint32_t kHeight = 4;
        const bool hasDepth = !utils::IsStencilOnlyFormat(GetParam().mTextureFormat);

        wgpu::Texture depthStencilTexture = CreateDepthStencilTexture(
            kWidth, kHeight,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                wgpu::TextureUsage::CopyDst);

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
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = bufferCopyOffset + BufferSizeForTextureCopy(
                                                     kWidth, kHeight, 1, GetParam().mTextureFormat,
                                                     wgpu::TextureAspect::StencilOnly);
            descriptor.usage = wgpu::BufferUsage::CopySrc;
            descriptor.mappedAtCreation = true;
            wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);
            uint8_t* mappedPtr = static_cast<uint8_t*>(srcBuffer.GetMappedRange(bufferCopyOffset));
            constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
            for (uint32_t y = 0; y < kHeight; ++y) {
                memcpy(mappedPtr + y * kBytesPerRow, stencilData.data() + y * kWidth, kWidth);
            }
            srcBuffer.Unmap();

            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            wgpu::ImageCopyBuffer imageCopyBuffer =
                utils::CreateImageCopyBuffer(srcBuffer, bufferCopyOffset, kBytesPerRow, kHeight);
            wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
                depthStencilTexture, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
            wgpu::Extent3D copySize = {kWidth, kHeight, 1};
            commandEncoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);
            wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
            queue.Submit(1, &commandBuffer);
        }

        // Decrement the stencil value in a render pass to ensure the data is visible to the
        // pipeline.
        {
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            // Create a render pipline which decrements the stencil value for passing fragments.
            // A quad is drawn in the bottom left.
            utils::ComboRenderPipelineDescriptor renderPipelineDesc;
            renderPipelineDesc.vertex.module = mVertexModule;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() {
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
        EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, {0, 0},
                          {kWidth, kHeight}, 0, wgpu::TextureAspect::StencilOnly);

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
};

// Test copying the stencil-only aspect into a buffer.
TEST_P(StencilCopyTests, FromStencilAspect) {
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, kTestLevel);
}

// Test copying the stencil-only aspect into a buffer at a non-zero offset
TEST_P(StencilCopyTests, FromStencilAspectAtNonZeroOffset) {
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kTestLevel = 0;
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {4u, 512u};
    for (uint32_t offset : kBufferCopyOffsets) {
        DoCopyFromStencilTest(offset, kWidth, kHeight, kTestLevel);
    }
}

// Test copying the non-zero mip, stencil-only aspect into a buffer.
TEST_P(StencilCopyTests, FromNonZeroMipStencilAspect) {
    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    constexpr uint32_t kTestLevel = 1;
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, kTestLevel);
}

// Test copying to the stencil-aspect of a texture
TEST_P(StencilCopyTests, ToStencilAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyToStencilTest(kBufferCopyOffset);
}

// Test copying to the stencil-aspect of a texture at non-zero offset
TEST_P(StencilCopyTests, ToStencilAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    for (uint32_t offset : kBufferCopyOffsets) {
        DoCopyToStencilTest(offset);
    }
}

// Test uploading to the non-zero mip, stencil-only aspect of a texture,
// and then checking the contents with a stencil test.
TEST_P(StencilCopyTests, CopyNonzeroMipThenReadWithStencilTest) {
    // Copies to a single aspect are unsupported on OpenGL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kMipLevel = 1;

    wgpu::Texture depthStencilTexture =
        CreateDepthStencilTexture(kWidth, kHeight,
                                  wgpu::TextureUsage::RenderAttachment |
                                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst,
                                  kMipLevel + 1);

    std::vector<uint8_t> stencilData = {
        7u, 7u,  //
        7u, 7u,  //
    };

    // Upload the stencil data.
    {
        wgpu::TextureDataLayout dataLayout = {};
        dataLayout.bytesPerRow = kWidth >> kMipLevel;

        wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
            depthStencilTexture, 1, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
        wgpu::Extent3D copySize = {kWidth >> kMipLevel, kHeight >> kMipLevel, 1};

        queue.WriteTexture(&imageCopyTexture, stencilData.data(), stencilData.size(), &dataLayout,
                           &copySize);
    }

    // Check the stencil contents.
    ExpectAttachmentStencilTestData(depthStencilTexture, GetParam().mTextureFormat,
                                    kWidth >> kMipLevel, kWidth >> kMipLevel, 0u, kMipLevel, 7u);
}

class DepthStencilCopyTests_RegressionDawn1083 : public DepthStencilCopyTests {};

// Regression test for crbug.com/dawn/1083. Checks that T2T copies with
// various mip/layer counts/offsets works.
TEST_P(DepthStencilCopyTests_RegressionDawn1083, Run) {
    // TODO(crbug.com/dawn/1648): Diagnose failure on NVIDIA GLES.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsNvidia());

    // TODO(crbug.com/dawn/1651): Failing on Mac AMD.
    DAWN_SUPPRESS_TEST_IF(IsAMD() && IsMetal() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth32FloatStencil8);

    uint32_t mipLevelCount = 3;
    uint32_t arrayLayerCount = 3;
    wgpu::TextureDescriptor texDesc = {};
    texDesc.size = {8, 8, arrayLayerCount};
    texDesc.format = GetParam().mTextureFormat;
    texDesc.mipLevelCount = mipLevelCount;

    for (uint32_t mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel) {
        uint32_t mipWidth = texDesc.size.width >> mipLevel;
        uint32_t mipHeight = texDesc.size.height >> mipLevel;
        for (uint32_t srcArrayLayer = 0; srcArrayLayer < arrayLayerCount; ++srcArrayLayer) {
            for (uint32_t dstArrayLayer = 0; dstArrayLayer < arrayLayerCount; ++dstArrayLayer) {
                // Test copying 1 layer, and all possible layers.
                for (uint32_t layerCount :
                     {1u, arrayLayerCount - std::max(srcArrayLayer, dstArrayLayer)}) {
                    texDesc.usage =
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
                    wgpu::Texture src = device.CreateTexture(&texDesc);

                    texDesc.usage =
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst;
                    wgpu::Texture dst = device.CreateTexture(&texDesc);

                    // Initialize texture data.
                    uint8_t stencilValue = 42;
                    float d1 = 0.1;
                    float d2 = 0.01;
                    for (uint32_t z = 0; z < layerCount; ++z) {
                        InitializeDepthStencilTextureRegion(src, d1, d2, stencilValue, stencilValue,
                                                            mipLevel, srcArrayLayer + z);
                    }

                    // Perform a T2T copy
                    {
                        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
                        wgpu::ImageCopyTexture srcView =
                            utils::CreateImageCopyTexture(src, mipLevel, {0, 0, srcArrayLayer});
                        wgpu::ImageCopyTexture dstView =
                            utils::CreateImageCopyTexture(dst, mipLevel, {0, 0, dstArrayLayer});
                        wgpu::Extent3D copySize = {mipWidth, mipHeight, layerCount};
                        commandEncoder.CopyTextureToTexture(&srcView, &dstView, &copySize);

                        wgpu::CommandBuffer commands = commandEncoder.Finish();
                        queue.Submit(1, &commands);
                    }

                    // Helper to compute expected depth data.
                    auto GetExpectedDepthData = [&](uint32_t mipLevel) -> std::vector<float> {
                        if (mipLevel == 0) {
                            return {
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                            };
                        } else if (mipLevel == 1) {
                            return {
                                d1, d1, d1, d1,  //
                                d1, d1, d1, d1,  //
                                d2, d2, d1, d1,  //
                                d2, d2, d1, d1,  //
                            };
                        } else if (mipLevel == 2) {
                            return {
                                d1, d1,  //
                                d2, d1,  //
                            };
                        }
                        UNREACHABLE();
                    };

                    // Check the depth
                    for (uint32_t z = 0; z < layerCount; ++z) {
                        ExpectAttachmentDepthTestData(dst, GetParam().mTextureFormat, mipWidth,
                                                      mipHeight, dstArrayLayer + z, mipLevel,
                                                      GetExpectedDepthData(mipLevel))
                            << "depth aspect"
                            << "\nmipLevelCount: " << mipLevelCount
                            << "\narrayLayerCount: " << arrayLayerCount
                            << "\nmipLevel: " << mipLevel
                            << "\nsrcArrayLayer: " << srcArrayLayer + z
                            << "\ndstArrayLayer: " << dstArrayLayer + z;

                        // Check the stencil
                        if (!utils::IsDepthOnlyFormat(GetParam().mTextureFormat)) {
                            ExpectAttachmentStencilTestData(dst, GetParam().mTextureFormat,
                                                            mipWidth, mipHeight, dstArrayLayer + z,
                                                            mipLevel, stencilValue)
                                << "stencil aspect"
                                << "\nmipLevelCount: " << mipLevelCount
                                << "\narrayLayerCount: " << arrayLayerCount
                                << "\nmipLevel: " << mipLevel
                                << "\nsrcArrayLayer: " << srcArrayLayer + z
                                << "\ndstArrayLayer: " << dstArrayLayer + z;
                        }
                    }
                }
            }
        }
    }
}

DAWN_INSTANTIATE_TEST_P(
    DepthStencilCopyTests,
    {D3D12Backend(), MetalBackend(),
     MetalBackend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     MetalBackend({"use_blit_for_buffer_to_depth_texture_copy",
                   "use_blit_for_buffer_to_stencil_texture_copy"}),
     OpenGLBackend(), OpenGLESBackend(),
     // Test with the vulkan_use_s8 toggle forced on and off.
     VulkanBackend({"vulkan_use_s8"}, {}), VulkanBackend({}, {"vulkan_use_s8"})},
    std::vector<wgpu::TextureFormat>(utils::kDepthAndStencilFormats.begin(),
                                     utils::kDepthAndStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(
    DepthCopyTests,
    {D3D12Backend(),
     D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                   "copy_with_non_zero_buffer_offset"}),
     MetalBackend(),
     MetalBackend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::TextureFormat>(kValidDepthCopyTextureFormats.begin(),
                                     kValidDepthCopyTextureFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthCopyFromBufferTests,
                        {D3D12Backend(),
                         D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                                       "copy_with_non_zero_buffer_offset"}),
                         MetalBackend(),
                         MetalBackend({"use_blit_for_buffer_to_depth_texture_copy"}),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyFromBufferFormats.begin(),
                                                         kValidDepthCopyFromBufferFormats.end()));

DAWN_INSTANTIATE_TEST_P(
    StencilCopyTests,
    {D3D12Backend(),
     D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                   "copy_with_non_zero_buffer_offset"}),
     MetalBackend(), MetalBackend({"metal_use_combined_depth_stencil_format_for_stencil8"}),
     MetalBackend(
         {"metal_use_both_depth_and_stencil_attachments_for_combined_depth_stencil_formats"}),
     MetalBackend({"use_blit_for_buffer_to_stencil_texture_copy"}), OpenGLBackend(),
     OpenGLESBackend(),
     // Test with the vulkan_use_s8 toggle forced on and off.
     VulkanBackend({"vulkan_use_s8"}, {}), VulkanBackend({}, {"vulkan_use_s8"})},
    std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(), utils::kStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(
    DepthStencilCopyTests_RegressionDawn1083,
    {D3D12Backend(), MetalBackend(),
     MetalBackend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::TextureFormat>{wgpu::TextureFormat::Depth16Unorm,
                                     wgpu::TextureFormat::Depth32FloatStencil8,
                                     wgpu::TextureFormat::Depth24Plus});
