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

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

// Create a 2D texture for sampling in the tests.
dawn::Texture Create2DTexture(dawn::Device device,
                              dawn::TextureFormat format,
                              uint32_t width,
                              uint32_t height,
                              uint32_t arrayLayerCount = 1,
                              uint32_t mipLevelCount = 1,
                              dawn::TextureUsageBit usage = dawn::TextureUsageBit::Sampled |
                                                            dawn::TextureUsageBit::CopyDst) {
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.format = format;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depth = 1;
    descriptor.arrayLayerCount = arrayLayerCount;
    descriptor.sampleCount = 1;
    descriptor.mipLevelCount = mipLevelCount;
    descriptor.usage = usage;
    return device.CreateTexture(&descriptor);
}

// The helper struct to configure the copies between buffers and textures.
struct CopyConfig {
    dawn::TextureFormat format;
    uint32_t textureWidthLevel0;
    uint32_t textureHeightLevel0;
    dawn::Extent3D copyExtent3D;
    dawn::Origin3D copyOrigin3D = {0, 0, 0};
    uint32_t arrayLayerCount = 1;
    uint32_t mipmapLevelCount = 1;
    uint32_t baseMipmapLevel = 0;
    uint32_t baseArrayLayer = 0;
    uint32_t bufferOffset = 0;
    uint32_t rowPitchAlignment = kTextureRowPitchAlignment;
    uint32_t imageHeight = 0;
};

class CompressedTextureBCFormatTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
                     {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture}});
    }

    // Copy the compressed texture data into the destination texture as is specified in copyConfig.
    void CopyDataIntoCompressedTexture(dawn::Texture bcCompressedTexture,
                                       const CopyConfig& copyConfig) {
        // Compute the upload buffer size with rowPitchAlignment and the copy region.
        uint32_t actualWidthAtLevel = copyConfig.textureWidthLevel0 >> copyConfig.baseMipmapLevel;
        uint32_t actualHeightAtLevel = copyConfig.textureHeightLevel0 >> copyConfig.baseMipmapLevel;
        uint32_t copyWidthInBlockAtLevel =
            (actualWidthAtLevel + kBCBlockWidthInTexels - 1) / kBCBlockWidthInTexels;
        uint32_t copyHeightInBlockAtLevel =
            (actualHeightAtLevel + kBCBlockHeightInTexels - 1) / kBCBlockHeightInTexels;
        uint32_t bufferRowPitchInBytes = 0;
        if (copyConfig.rowPitchAlignment != 0) {
            bufferRowPitchInBytes = copyConfig.rowPitchAlignment;
        } else {
            bufferRowPitchInBytes =
                copyWidthInBlockAtLevel * CompressedFormatBlockSizeInBytes(copyConfig.format);
        }
        uint32_t uploadBufferSize =
            copyConfig.bufferOffset + bufferRowPitchInBytes * copyHeightInBlockAtLevel;

        // Fill uploadData with the pre-prepared one-block compressed texture data.
        std::vector<uint8_t> uploadData(uploadBufferSize, 0);
        std::vector<uint8_t> oneBlockCompressedTextureData =
            GetOneBlockBCFormatTextureData(copyConfig.format);
        for (uint32_t h = 0; h < copyHeightInBlockAtLevel; ++h) {
            for (uint32_t w = 0; w < copyWidthInBlockAtLevel; ++w) {
                uint32_t uploadBufferOffset = copyConfig.bufferOffset + bufferRowPitchInBytes * h +
                                              oneBlockCompressedTextureData.size() * w;
                std::memcpy(&uploadData[uploadBufferOffset], oneBlockCompressedTextureData.data(),
                            oneBlockCompressedTextureData.size() * sizeof(uint8_t));
            }
        }

        // Copy texture data from a staging buffer to the destination texture.
        dawn::Buffer stagingBuffer = utils::CreateBufferFromData(
            device, uploadData.data(), uploadBufferSize, dawn::BufferUsageBit::CopySrc);
        dawn::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(stagingBuffer, copyConfig.bufferOffset,
                                        copyConfig.rowPitchAlignment, copyConfig.imageHeight);
        dawn::TextureCopyView textureCopyView =
            utils::CreateTextureCopyView(bcCompressedTexture, copyConfig.baseMipmapLevel,
                                         copyConfig.baseArrayLayer, copyConfig.copyOrigin3D);

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copyConfig.copyExtent3D);
        dawn::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);
    }

    // Create the bind group that includes a BC texture and a sampler.
    dawn::BindGroup CreateBindGroupForTest(dawn::Texture bcCompressedTexture,
                                           dawn::TextureFormat bcFormat,
                                           uint32_t baseArrayLayer = 0,
                                           uint32_t baseMipLevel = 0) {
        dawn::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
        samplerDesc.minFilter = dawn::FilterMode::Nearest;
        samplerDesc.magFilter = dawn::FilterMode::Nearest;
        dawn::Sampler sampler = device.CreateSampler(&samplerDesc);

        dawn::TextureViewDescriptor textureViewDescriptor;
        textureViewDescriptor.format = bcFormat;
        textureViewDescriptor.dimension = dawn::TextureViewDimension::e2D;
        textureViewDescriptor.baseMipLevel = baseMipLevel;
        textureViewDescriptor.baseArrayLayer = baseArrayLayer;
        textureViewDescriptor.arrayLayerCount = 1;
        textureViewDescriptor.mipLevelCount = 1;
        dawn::TextureView bcTextureView = bcCompressedTexture.CreateView(&textureViewDescriptor);

        return utils::MakeBindGroup(device, mBindGroupLayout, {{0, sampler}, {1, bcTextureView}});
    }

    // Create a render pipeline for sampling from a BC texture and rendering into the render target.
    dawn::RenderPipeline CreateRenderPipelineForTest() {
        dawn::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);

        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor(device);
        dawn::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::ShaderStage::Vertex, R"(
            #version 450
            layout(location=0) out vec2 texCoord;
            void main() {
                const vec2 pos[3] = vec2[3](
                    vec2(-3.0f, -1.0f),
                    vec2( 3.0f, -1.0f),
                    vec2( 0.0f,  2.0f)
                );
                gl_Position = vec4(pos[gl_VertexIndex], 0.0f, 1.0f);
                texCoord = gl_Position.xy / 2.0f + vec2(0.5f);
            })");
        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::ShaderStage::Fragment, R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2D texture0;
            layout(location = 0) in vec2 texCoord;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor = texture(sampler2D(texture0, sampler0), texCoord);
            })");
        renderPipelineDescriptor.cVertexStage.module = vsModule;
        renderPipelineDescriptor.cFragmentStage.module = fsModule;
        renderPipelineDescriptor.layout = pipelineLayout;
        renderPipelineDescriptor.cColorStates[0]->format =
            utils::BasicRenderPass::kDefaultColorFormat;
        return device.CreateRenderPipeline(&renderPipelineDescriptor);
    }

    // Run the given render pipeline and bind group and verify the pixels in the render target.
    void VerifyCompressedTexturePixelValues(dawn::RenderPipeline renderPipeline,
                                            dawn::BindGroup bindGroup,
                                            const dawn::Extent3D& renderTargetSize,
                                            const dawn::Origin3D& expectedOrigin,
                                            const dawn::Extent3D& expectedExtent,
                                            const std::vector<RGBA8>& expected) {
        ASSERT(expected.size() == renderTargetSize.width * renderTargetSize.height);
        utils::BasicRenderPass renderPass =
            utils::CreateBasicRenderPass(device, renderTargetSize.width, renderTargetSize.height);

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(renderPipeline);
            pass.SetBindGroup(0, bindGroup, 0, nullptr);
            pass.Draw(6, 1, 0, 0);
            pass.EndPass();
        }

        dawn::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_RGBA8_EQ(expected.data(), renderPass.color, expectedOrigin.x,
                                expectedOrigin.y, expectedExtent.width, expectedExtent.height, 0,
                                0);
    }

    // Run the tests that copies pre-prepared BC format data into a BC texture and verifies if we
    // can render correctly with the pixel values sampled from the BC texture.
    void TestCopyRegionIntoBCFormatTextures(const CopyConfig& config) {
        dawn::Texture bcTexture = Create2DTexture(device, config.format, config.textureWidthLevel0,
                                                  config.textureHeightLevel0,
                                                  config.arrayLayerCount, config.mipmapLevelCount);
        CopyDataIntoCompressedTexture(bcTexture, config);

        dawn::BindGroup bindGroup = CreateBindGroupForTest(
            bcTexture, config.format, config.baseArrayLayer, config.baseMipmapLevel);
        dawn::RenderPipeline renderPipeline = CreateRenderPipelineForTest();

        dawn::Extent3D virtualSizeAtLevel = GetVirtualSizeAtLevel(config);

        // The copy region may exceed the subresource size because of the required paddings for BC
        // blocks, so we should limit the size of the expectedData to make it match the real size
        // of the render target.
        dawn::Extent3D noPaddingExtent3D = config.copyExtent3D;
        if (config.copyOrigin3D.x + config.copyExtent3D.width > virtualSizeAtLevel.width) {
            noPaddingExtent3D.width = virtualSizeAtLevel.width - config.copyOrigin3D.x;
        }
        if (config.copyOrigin3D.y + config.copyExtent3D.height > virtualSizeAtLevel.height) {
            noPaddingExtent3D.height = virtualSizeAtLevel.height - config.copyOrigin3D.y;
        }

        std::vector<RGBA8> expectedData = GetExpectedData(config.format, virtualSizeAtLevel);
        VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, virtualSizeAtLevel,
                                           config.copyOrigin3D, noPaddingExtent3D, expectedData);
    }

    // Return the BC block size in bytes.
    static uint32_t CompressedFormatBlockSizeInBytes(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::BC1RGBAUnorm:
            case dawn::TextureFormat::BC1RGBAUnormSrgb:
            case dawn::TextureFormat::BC4RSnorm:
            case dawn::TextureFormat::BC4RUnorm:
                return 8;
            case dawn::TextureFormat::BC2RGBAUnorm:
            case dawn::TextureFormat::BC2RGBAUnormSrgb:
            case dawn::TextureFormat::BC3RGBAUnorm:
            case dawn::TextureFormat::BC3RGBAUnormSrgb:
            case dawn::TextureFormat::BC5RGSnorm:
            case dawn::TextureFormat::BC5RGUnorm:
            case dawn::TextureFormat::BC6HRGBSfloat:
            case dawn::TextureFormat::BC6HRGBUfloat:
            case dawn::TextureFormat::BC7RGBAUnorm:
            case dawn::TextureFormat::BC7RGBAUnormSrgb:
                return 16;
            default:
                UNREACHABLE();
                return 0;
        }
    }

    // Return the pre-prepared one-block BC texture data.
    static std::vector<uint8_t> GetOneBlockBCFormatTextureData(dawn::TextureFormat bcFormat) {
        switch (bcFormat) {
            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. We specify the same compressed data in both sRGB and non-sRGB
            // tests, but the rendering result should be different because for sRGB formats, the
            // red, green, and blue components are converted from an sRGB color space to a linear
            // color space as part of filtering.
            case dawn::TextureFormat::BC1RGBAUnorm:
            case dawn::TextureFormat::BC1RGBAUnormSrgb:
                return {0x0, 0xC0, 0x60, 0x6, 0x50, 0x50, 0x50, 0x50};
            case dawn::TextureFormat::BC7RGBAUnorm:
            case dawn::TextureFormat::BC7RGBAUnormSrgb:
                return {0x50, 0x18, 0xfc, 0xf, 0x0,  0x30, 0xe3, 0xe1,
                        0xe1, 0xe1, 0xc1, 0xf, 0xfc, 0xc0, 0xf,  0xfc};

            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. The pixels in the left side of the block all have an alpha
            // value equal to 0x88. We specify the same compressed data in both sRGB and non-sRGB
            // tests, but the rendering result should be different because for sRGB formats, the
            // red, green, and blue components are converted from an sRGB color space to a linear
            // color space as part of filtering, and any alpha component is left unchanged.
            case dawn::TextureFormat::BC2RGBAUnorm:
            case dawn::TextureFormat::BC2RGBAUnormSrgb:
                return {0x88, 0xFF, 0x88, 0xFF, 0x88, 0xFF, 0x88, 0xFF,
                        0x0,  0xC0, 0x60, 0x6,  0x50, 0x50, 0x50, 0x50};
            case dawn::TextureFormat::BC3RGBAUnorm:
            case dawn::TextureFormat::BC3RGBAUnormSrgb:
                return {0x88, 0xFF, 0x40, 0x2, 0x24, 0x40, 0x2,  0x24,
                        0x0,  0xC0, 0x60, 0x6, 0x50, 0x50, 0x50, 0x50};

            // The expected data represents 4x4 pixel images with the left side red and the
            // right side black.
            case dawn::TextureFormat::BC4RSnorm:
                return {0x7F, 0x0, 0x40, 0x2, 0x24, 0x40, 0x2, 0x24};
            case dawn::TextureFormat::BC4RUnorm:
                return {0xFF, 0x0, 0x40, 0x2, 0x24, 0x40, 0x2, 0x24};

            // The expected data represents 4x4 pixel images with the left side red and the right
            // side green and was encoded with DirectXTex from Microsoft.
            case dawn::TextureFormat::BC5RGSnorm:
                return {0x7f, 0x81, 0x40, 0x2,  0x24, 0x40, 0x2,  0x24,
                        0x7f, 0x81, 0x9,  0x90, 0x0,  0x9,  0x90, 0x0};
            case dawn::TextureFormat::BC5RGUnorm:
                return {0xff, 0x0, 0x40, 0x2,  0x24, 0x40, 0x2,  0x24,
                        0xff, 0x0, 0x9,  0x90, 0x0,  0x9,  0x90, 0x0};
            case dawn::TextureFormat::BC6HRGBSfloat:
                return {0xe3, 0x1f, 0x0, 0x0,  0x0, 0xe0, 0x1f, 0x0,
                        0x0,  0xff, 0x0, 0xff, 0x0, 0xff, 0x0,  0xff};
            case dawn::TextureFormat::BC6HRGBUfloat:
                return {0xe3, 0x3d, 0x0, 0x0,  0x0, 0xe0, 0x3d, 0x0,
                        0x0,  0xff, 0x0, 0xff, 0x0, 0xff, 0x0,  0xff};

            default:
                UNREACHABLE();
                return {};
        }
    }

    // Return the texture data that is decoded from the result of GetOneBlockBCFormatTextureData in
    // RGBA8 formats.
    static std::vector<RGBA8> GetExpectedData(dawn::TextureFormat bcFormat,
                                              const dawn::Extent3D& testRegion) {
        constexpr RGBA8 kRed(255, 0, 0, 255);
        constexpr RGBA8 kGreen(0, 255, 0, 255);
        constexpr RGBA8 kBlack(0, 0, 0, 255);
        constexpr RGBA8 kDarkRed(198, 0, 0, 255);
        constexpr RGBA8 kDarkGreen(0, 207, 0, 255);
        constexpr RGBA8 kDarkRedSRGB(144, 0, 0, 255);
        constexpr RGBA8 kDarkGreenSRGB(0, 159, 0, 255);

        constexpr uint8_t kLeftAlpha = 0x88;
        constexpr uint8_t kRightAlpha = 0xFF;

        switch (bcFormat) {
            case dawn::TextureFormat::BC1RGBAUnorm:
            case dawn::TextureFormat::BC7RGBAUnorm:
                return FillExpectedData(testRegion, kDarkRed, kDarkGreen);

            case dawn::TextureFormat::BC2RGBAUnorm:
            case dawn::TextureFormat::BC3RGBAUnorm: {
                constexpr RGBA8 kLeftColor = RGBA8(kDarkRed.r, 0, 0, kLeftAlpha);
                constexpr RGBA8 kRightColor = RGBA8(0, kDarkGreen.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case dawn::TextureFormat::BC1RGBAUnormSrgb:
            case dawn::TextureFormat::BC7RGBAUnormSrgb:
                return FillExpectedData(testRegion, kDarkRedSRGB, kDarkGreenSRGB);

            case dawn::TextureFormat::BC2RGBAUnormSrgb:
            case dawn::TextureFormat::BC3RGBAUnormSrgb: {
                constexpr RGBA8 kLeftColor = RGBA8(kDarkRedSRGB.r, 0, 0, kLeftAlpha);
                constexpr RGBA8 kRightColor = RGBA8(0, kDarkGreenSRGB.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case dawn::TextureFormat::BC4RSnorm:
            case dawn::TextureFormat::BC4RUnorm:
                return FillExpectedData(testRegion, kRed, kBlack);

            case dawn::TextureFormat::BC5RGSnorm:
            case dawn::TextureFormat::BC5RGUnorm:
            case dawn::TextureFormat::BC6HRGBSfloat:
            case dawn::TextureFormat::BC6HRGBUfloat:
                return FillExpectedData(testRegion, kRed, kGreen);

            default:
                UNREACHABLE();
                return {};
        }
    }

    static std::vector<RGBA8> FillExpectedData(const dawn::Extent3D& testRegion,
                                               RGBA8 leftColorInBlock,
                                               RGBA8 rightColorInBlock) {
        ASSERT(testRegion.depth == 1);

        std::vector<RGBA8> expectedData(testRegion.width * testRegion.height, leftColorInBlock);
        for (uint32_t y = 0; y < testRegion.height; ++y) {
            for (uint32_t x = 0; x < testRegion.width; ++x) {
                if (x % kBCBlockWidthInTexels >= kBCBlockWidthInTexels / 2) {
                    expectedData[testRegion.width * y + x] = rightColorInBlock;
                }
            }
        }
        return expectedData;
    }

    static dawn::Extent3D GetVirtualSizeAtLevel(const CopyConfig& config) {
        return {config.textureWidthLevel0 >> config.baseMipmapLevel,
                config.textureHeightLevel0 >> config.baseMipmapLevel, 1};
    }

    static dawn::Extent3D GetPhysicalSizeAtLevel(const CopyConfig& config) {
        dawn::Extent3D sizeAtLevel = GetVirtualSizeAtLevel(config);
        sizeAtLevel.width = (sizeAtLevel.width + kBCBlockWidthInTexels - 1) /
                            kBCBlockWidthInTexels * kBCBlockWidthInTexels;
        sizeAtLevel.height = (sizeAtLevel.height + kBCBlockHeightInTexels - 1) /
                             kBCBlockHeightInTexels * kBCBlockHeightInTexels;
        return sizeAtLevel;
    }

    const std::array<dawn::TextureFormat, 14> kBCFormats = {
        dawn::TextureFormat::BC1RGBAUnorm,  dawn::TextureFormat::BC1RGBAUnormSrgb,
        dawn::TextureFormat::BC2RGBAUnorm,  dawn::TextureFormat::BC2RGBAUnormSrgb,
        dawn::TextureFormat::BC3RGBAUnorm,  dawn::TextureFormat::BC3RGBAUnormSrgb,
        dawn::TextureFormat::BC4RSnorm,     dawn::TextureFormat::BC4RUnorm,
        dawn::TextureFormat::BC5RGSnorm,    dawn::TextureFormat::BC5RGUnorm,
        dawn::TextureFormat::BC6HRGBSfloat, dawn::TextureFormat::BC6HRGBUfloat,
        dawn::TextureFormat::BC7RGBAUnorm,  dawn::TextureFormat::BC7RGBAUnormSrgb};

    // Tthe block width and height in texels are 4 for all BC formats.
    static constexpr uint32_t kBCBlockWidthInTexels = 4;
    static constexpr uint32_t kBCBlockHeightInTexels = 4;

    dawn::BindGroupLayout mBindGroupLayout;
};

// Test copying into the whole BC texture with 2x2 blocks and sampling from it.
TEST_P(CompressedTextureBCFormatTest, Basic) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = 8;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test copying into a sub-region of a texture with BC formats works correctly.
TEST_P(CompressedTextureBCFormatTest, CopyIntoSubRegion) {
    CopyConfig config;
    config.textureHeightLevel0 = 8;
    config.textureWidthLevel0 = 8;
    config.rowPitchAlignment = kTextureRowPitchAlignment;

    const dawn::Origin3D kOrigin = {4, 4, 0};
    const dawn::Extent3D kExtent3D = {4, 4, 1};
    config.copyOrigin3D = kOrigin;
    config.copyExtent3D = kExtent3D;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test using rowPitch == 0 in the copies with BC formats works correctly.
TEST_P(CompressedTextureBCFormatTest, CopyWithZeroRowPitch) {
    CopyConfig config;
    config.textureHeightLevel0 = 8;

    config.rowPitchAlignment = 0;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        config.textureWidthLevel0 = kTextureRowPitchAlignment /
                                    CompressedFormatBlockSizeInBytes(config.format) *
                                    kBCBlockWidthInTexels;
        config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test copying into the non-zero layer of a 2D array texture with BC formats works correctly.
TEST_P(CompressedTextureBCFormatTest, CopyIntoNonZeroArrayLayer) {
    CopyConfig config;
    config.textureHeightLevel0 = 8;
    config.textureWidthLevel0 = 8;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};
    config.rowPitchAlignment = kTextureRowPitchAlignment;

    constexpr uint32_t kArrayLayerCount = 3;
    config.arrayLayerCount = kArrayLayerCount;
    config.baseArrayLayer = kArrayLayerCount - 1;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test copying into a non-zero mipmap level of a texture with BC texture formats.
TEST_P(CompressedTextureBCFormatTest, CopyBufferIntoNonZeroMipmapLevel) {
    CopyConfig config;
    config.textureHeightLevel0 = 60;
    config.textureWidthLevel0 = 60;
    config.rowPitchAlignment = kTextureRowPitchAlignment;

    constexpr uint32_t kMipmapLevelCount = 3;
    config.mipmapLevelCount = kMipmapLevelCount;
    config.baseMipmapLevel = kMipmapLevelCount - 1;

    // The actual size of the texture at mipmap level == 2 is not a multiple of 4, paddings are
    // required in the copies.
    const uint32_t kActualWidthAtLevel = config.textureWidthLevel0 >> config.baseMipmapLevel;
    const uint32_t kActualHeightAtLevel = config.textureHeightLevel0 >> config.baseMipmapLevel;
    ASSERT(kActualWidthAtLevel % kBCBlockWidthInTexels != 0);
    ASSERT(kActualHeightAtLevel % kBCBlockHeightInTexels != 0);

    const uint32_t kCopyWidthAtLevel = (kActualWidthAtLevel + kBCBlockWidthInTexels - 1) /
                                       kBCBlockWidthInTexels * kBCBlockWidthInTexels;
    const uint32_t kCopyHeightAtLevel = (kActualHeightAtLevel + kBCBlockHeightInTexels - 1) /
                                        kBCBlockHeightInTexels * kBCBlockHeightInTexels;

    config.copyExtent3D = {kCopyWidthAtLevel, kCopyHeightAtLevel, 1};

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test texture-to-texture whole-size copies with BC formats.
TEST_P(CompressedTextureBCFormatTest, CopyWholeTextureSubResourceIntoNonZeroMipmapLevel) {
    // TODO(cwallez@chromium.org): This consistently fails on with the 12th pixel being opaque black
    // instead of opaque red on Win10 FYI Release (NVIDIA GeForce GTX 1660). See
    // https://bugs.chromium.org/p/chromium/issues/detail?id=981393
    DAWN_SKIP_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

    CopyConfig config;
    config.textureHeightLevel0 = 60;
    config.textureWidthLevel0 = 60;
    config.rowPitchAlignment = kTextureRowPitchAlignment;

    constexpr uint32_t kMipmapLevelCount = 3;
    config.mipmapLevelCount = kMipmapLevelCount;
    config.baseMipmapLevel = kMipmapLevelCount - 1;

    // The actual size of the texture at mipmap level == 2 is not a multiple of 4, paddings are
    // required in the copies.
    const dawn::Extent3D kVirtualSize = GetVirtualSizeAtLevel(config);
    const dawn::Extent3D kPhysicalSize = GetPhysicalSizeAtLevel(config);
    ASSERT(kVirtualSize.width % kBCBlockWidthInTexels != 0);
    ASSERT(kVirtualSize.height % kBCBlockHeightInTexels != 0);

    config.copyExtent3D = kPhysicalSize;
    for (dawn::TextureFormat format : kBCFormats) {
        // Create bcTextureSrc as the source texture and initialize it with pre-prepared BC
        // compressed data.
        config.format = format;
        dawn::Texture bcTextureSrc = Create2DTexture(
            device, config.format, config.textureWidthLevel0, config.textureHeightLevel0,
            config.arrayLayerCount, config.mipmapLevelCount,
            dawn::TextureUsageBit::CopySrc | dawn::TextureUsageBit::CopyDst);
        CopyDataIntoCompressedTexture(bcTextureSrc, config);

        // Create bcTexture and copy from the content in bcTextureSrc into it.
        dawn::Texture bcTexture = Create2DTexture(device, config.format, config.textureWidthLevel0,
                                                  config.textureHeightLevel0,
                                                  config.arrayLayerCount, config.mipmapLevelCount);
        dawn::TextureCopyView textureCopyViewSrc = utils::CreateTextureCopyView(
            bcTextureSrc, config.baseMipmapLevel, config.baseArrayLayer, config.copyOrigin3D);
        dawn::TextureCopyView textureCopyViewDst = utils::CreateTextureCopyView(
            bcTexture, config.baseMipmapLevel, config.baseArrayLayer, config.copyOrigin3D);
        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&textureCopyViewSrc, &textureCopyViewDst,
                                     &config.copyExtent3D);
        dawn::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        // Verify if we can use bcTexture as sampled textures correctly.
        dawn::BindGroup bindGroup = CreateBindGroupForTest(
            bcTexture, config.format, config.baseArrayLayer, config.baseMipmapLevel);
        dawn::RenderPipeline renderPipeline = CreateRenderPipelineForTest();

        std::vector<RGBA8> expectedData = GetExpectedData(config.format, kVirtualSize);
        VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kVirtualSize,
                                           config.copyOrigin3D, kVirtualSize, expectedData);
    }
}

// Test BC format texture-to-texture partial copies.
TEST_P(CompressedTextureBCFormatTest, CopyPartofTextureSubResourceIntoNonZeroMipmapLevel) {
    // TODO(jiawei.shao@intel.com): add workaround on the T2T copies where Extent3D fits in one
    // subresource and does not fit in another one on Vulkan. Currently this test causes an error if
    // Vulkan validation layer is enabled.
    DAWN_SKIP_TEST_IF(IsVulkan());

    CopyConfig srcConfig;
    srcConfig.textureHeightLevel0 = 60;
    srcConfig.textureWidthLevel0 = 60;
    srcConfig.rowPitchAlignment = kTextureRowPitchAlignment;
    srcConfig.mipmapLevelCount = 1;
    srcConfig.baseMipmapLevel = srcConfig.mipmapLevelCount - 1;

    const dawn::Extent3D kSrcVirtualSize = GetVirtualSizeAtLevel(srcConfig);

    CopyConfig dstConfig;
    dstConfig.textureHeightLevel0 = 60;
    dstConfig.textureWidthLevel0 = 60;
    dstConfig.rowPitchAlignment = kTextureRowPitchAlignment;

    constexpr uint32_t kMipmapLevelCount = 3;
    dstConfig.mipmapLevelCount = kMipmapLevelCount;
    dstConfig.baseMipmapLevel = kMipmapLevelCount - 1;

    // The actual size of the texture at mipmap level == 2 is not a multiple of 4, paddings are
    // required in the copies.
    const dawn::Extent3D kDstVirtualSize = GetVirtualSizeAtLevel(dstConfig);
    ASSERT(kDstVirtualSize.width % kBCBlockWidthInTexels != 0);
    ASSERT(kDstVirtualSize.height % kBCBlockHeightInTexels != 0);

    const dawn::Extent3D kDstPhysicalSize = GetPhysicalSizeAtLevel(dstConfig);

    srcConfig.copyExtent3D = dstConfig.copyExtent3D = kDstPhysicalSize;
    ASSERT(srcConfig.copyOrigin3D.x + srcConfig.copyExtent3D.width < kSrcVirtualSize.width);
    ASSERT(srcConfig.copyOrigin3D.y + srcConfig.copyExtent3D.height < kSrcVirtualSize.height);

    for (dawn::TextureFormat format : kBCFormats) {
        // Create bcTextureSrc as the source texture and initialize it with pre-prepared BC
        // compressed data.
        srcConfig.format = format;
        dawn::Texture bcTextureSrc = Create2DTexture(
            device, srcConfig.format, srcConfig.textureWidthLevel0, srcConfig.textureHeightLevel0,
            srcConfig.arrayLayerCount, srcConfig.mipmapLevelCount,
            dawn::TextureUsageBit::CopySrc | dawn::TextureUsageBit::CopyDst);
        CopyDataIntoCompressedTexture(bcTextureSrc, srcConfig);
        dawn::TextureCopyView textureCopyViewSrc =
            utils::CreateTextureCopyView(bcTextureSrc, srcConfig.baseMipmapLevel,
                                         srcConfig.baseArrayLayer, srcConfig.copyOrigin3D);

        // Create bcTexture and copy from the content in bcTextureSrc into it.
        dstConfig.format = format;
        dawn::Texture bcTexture = Create2DTexture(
            device, dstConfig.format, dstConfig.textureWidthLevel0, dstConfig.textureHeightLevel0,
            dstConfig.arrayLayerCount, dstConfig.mipmapLevelCount);
        dawn::TextureCopyView textureCopyViewDst = utils::CreateTextureCopyView(
            bcTexture, dstConfig.baseMipmapLevel, dstConfig.baseArrayLayer, dstConfig.copyOrigin3D);

        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&textureCopyViewSrc, &textureCopyViewDst,
                                     &dstConfig.copyExtent3D);
        dawn::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        // Verify if we can use bcTexture as sampled textures correctly.
        dawn::BindGroup bindGroup = CreateBindGroupForTest(
            bcTexture, dstConfig.format, dstConfig.baseArrayLayer, dstConfig.baseMipmapLevel);
        dawn::RenderPipeline renderPipeline = CreateRenderPipelineForTest();

        std::vector<RGBA8> expectedData = GetExpectedData(dstConfig.format, kDstVirtualSize);
        VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kDstVirtualSize,
                                           dstConfig.copyOrigin3D, kDstVirtualSize, expectedData);
    }
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset and texture
// extent exactly fit the RowPitch.
TEST_P(CompressedTextureBCFormatTest, BufferOffsetAndExtentFitRowPitch) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = 8;
    config.rowPitchAlignment = kTextureRowPitchAlignment;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    const uint32_t blockCountPerRow = config.textureWidthLevel0 / kBCBlockWidthInTexels;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;

        const uint32_t blockSizeInBytes = CompressedFormatBlockSizeInBytes(format);
        const uint32_t blockCountPerRowPitch = config.rowPitchAlignment / blockSizeInBytes;

        config.bufferOffset = (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes;

        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset exceeds the
// slice pitch (slicePitch = rowPitch * (imageHeightInTexels / blockHeightInTexels)). On D3D12
// backend the texelOffset.y will be greater than 0 after calcuting the texelOffset in the function
// ComputeTexelOffsets().
TEST_P(CompressedTextureBCFormatTest, BufferOffsetExceedsSlicePitch) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = 8;
    config.rowPitchAlignment = kTextureRowPitchAlignment;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    const uint32_t blockCountPerRow = config.textureWidthLevel0 / kBCBlockWidthInTexels;
    const uint32_t slicePitchInBytes =
        config.rowPitchAlignment * (config.textureHeightLevel0 / kBCBlockHeightInTexels);

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;

        const uint32_t blockSizeInBytes = CompressedFormatBlockSizeInBytes(format);
        const uint32_t blockCountPerRowPitch = config.rowPitchAlignment / blockSizeInBytes;

        config.bufferOffset = (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes +
                              config.rowPitchAlignment + slicePitchInBytes;

        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset and texture
// extent exceed the RowPitch. On D3D12 backend two copies are required for this case.
TEST_P(CompressedTextureBCFormatTest, CopyWithBufferOffsetAndExtentExceedRowPitch) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = 8;
    config.rowPitchAlignment = kTextureRowPitchAlignment;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    const uint32_t blockCountPerRow = config.textureWidthLevel0 / kBCBlockWidthInTexels;

    constexpr uint32_t kExceedRowBlockCount = 1;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;

        const uint32_t blockSizeInBytes = CompressedFormatBlockSizeInBytes(format);
        const uint32_t blockCountPerRowPitch = config.rowPitchAlignment / blockSizeInBytes;
        config.bufferOffset =
            (blockCountPerRowPitch - blockCountPerRow + kExceedRowBlockCount) * blockSizeInBytes;

        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test the special case of the B2T copies on the D3D12 backend that the slicePitch is equal to the
// rowPitch. On D3D12 backend the texelOffset.z will be greater than 0 after calcuting the
// texelOffset in the function ComputeTexelOffsets().
TEST_P(CompressedTextureBCFormatTest, RowPitchEqualToSlicePitch) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = kBCBlockHeightInTexels;
    config.rowPitchAlignment = kTextureRowPitchAlignment;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    const uint32_t blockCountPerRow = config.textureWidthLevel0 / kBCBlockWidthInTexels;
    const uint32_t slicePitchInBytes = config.rowPitchAlignment;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;

        const uint32_t blockSizeInBytes = CompressedFormatBlockSizeInBytes(format);
        const uint32_t blockCountPerRowPitch = config.rowPitchAlignment / blockSizeInBytes;

        config.bufferOffset =
            (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes + slicePitchInBytes;

        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test the workaround in the B2T copies when (bufferSize - bufferOffset < bytesPerImage *
// copyExtent.depth) on Metal backends. As copyExtent.depth can only be 1 for BC formats, on Metal
// backend we will use two copies to implement such copy.
TEST_P(CompressedTextureBCFormatTest, LargeImageHeight) {
    CopyConfig config;
    config.textureWidthLevel0 = 8;
    config.textureHeightLevel0 = 8;
    config.copyExtent3D = {config.textureWidthLevel0, config.textureHeightLevel0, 1};

    config.imageHeight = config.textureHeightLevel0 * 2;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// Test the workaround in the B2T copies when (bufferSize - bufferOffset < bytesPerImage *
// copyExtent.depth) and copyExtent needs to be clamped.
TEST_P(CompressedTextureBCFormatTest, LargeImageHeightAndClampedCopyExtent) {
    CopyConfig config;
    config.textureHeightLevel0 = 56;
    config.textureWidthLevel0 = 56;
    config.rowPitchAlignment = kTextureRowPitchAlignment;

    constexpr uint32_t kMipmapLevelCount = 3;
    config.mipmapLevelCount = kMipmapLevelCount;
    config.baseMipmapLevel = kMipmapLevelCount - 1;

    // The actual size of the texture at mipmap level == 2 is not a multiple of 4, paddings are
    // required in the copies.
    const uint32_t kActualWidthAtLevel = config.textureWidthLevel0 >> config.baseMipmapLevel;
    const uint32_t kActualHeightAtLevel = config.textureHeightLevel0 >> config.baseMipmapLevel;
    ASSERT(kActualWidthAtLevel % kBCBlockWidthInTexels != 0);
    ASSERT(kActualHeightAtLevel % kBCBlockHeightInTexels != 0);

    const uint32_t kCopyWidthAtLevel = (kActualWidthAtLevel + kBCBlockWidthInTexels - 1) /
                                       kBCBlockWidthInTexels * kBCBlockWidthInTexels;
    const uint32_t kCopyHeightAtLevel = (kActualHeightAtLevel + kBCBlockHeightInTexels - 1) /
                                        kBCBlockHeightInTexels * kBCBlockHeightInTexels;

    config.copyExtent3D = {kCopyWidthAtLevel, kCopyHeightAtLevel, 1};

    config.imageHeight = kCopyHeightAtLevel * 2;

    for (dawn::TextureFormat format : kBCFormats) {
        config.format = format;
        TestCopyRegionIntoBCFormatTextures(config);
    }
}

// TODO(jiawei.shao@intel.com): support BC formats on OpenGL backend
DAWN_INSTANTIATE_TEST(CompressedTextureBCFormatTest, D3D12Backend, MetalBackend, VulkanBackend);
