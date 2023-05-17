// Copyright 2021 The Dawn Authors
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

#include <utility>

#include "dawn/tests/end2end/VideoViewsTests.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

VideoViewsTestBackend::PlatformTexture::PlatformTexture(wgpu::Texture&& texture)
    : wgpuTexture(texture) {}
VideoViewsTestBackend::PlatformTexture::~PlatformTexture() = default;

VideoViewsTestBackend::~VideoViewsTestBackend() = default;

constexpr std::array<utils::RGBA8, 2> VideoViewsTests::kYellowYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTests::kWhiteYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTests::kBlueYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTests::kRedYUVColor;

void VideoViewsTests::SetUp() {
    DawnTest::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());

    mBackend = VideoViewsTestBackend::Create();
    mBackend->OnSetUp(device.Get());
}

void VideoViewsTests::TearDown() {
    if (!UsesWire() && IsMultiPlanarFormatsSupported()) {
        mBackend->OnTearDown();
    }
    DawnTest::TearDown();
}

std::vector<wgpu::FeatureName> VideoViewsTests::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = {};
    mIsMultiPlanarFormatsSupported = SupportsFeatures({wgpu::FeatureName::DawnMultiPlanarFormats});
    if (mIsMultiPlanarFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::DawnMultiPlanarFormats);
    }
    requiredFeatures.push_back(wgpu::FeatureName::DawnInternalUsages);
    return requiredFeatures;
}

bool VideoViewsTests::IsMultiPlanarFormatsSupported() const {
    return mIsMultiPlanarFormatsSupported;
}

// Returns a pre-prepared multi-planar formatted texture
// The encoded texture data represents a 4x4 converted image. When |isCheckerboard| is true,
// the top left is a 2x2 yellow block, bottom right is a 2x2 red block, top right is a 2x2
// blue block, and bottom left is a 2x2 white block. When |isCheckerboard| is false, the
// image is converted from a solid yellow 4x4 block.
// static
std::vector<uint8_t> VideoViewsTests::GetTestTextureData(wgpu::TextureFormat format,
                                                         bool isCheckerboard) {
    constexpr uint8_t Yy = kYellowYUVColor[kYUVLumaPlaneIndex].r;
    constexpr uint8_t Yu = kYellowYUVColor[kYUVChromaPlaneIndex].r;
    constexpr uint8_t Yv = kYellowYUVColor[kYUVChromaPlaneIndex].g;

    constexpr uint8_t Wy = kWhiteYUVColor[kYUVLumaPlaneIndex].r;
    constexpr uint8_t Wu = kWhiteYUVColor[kYUVChromaPlaneIndex].r;
    constexpr uint8_t Wv = kWhiteYUVColor[kYUVChromaPlaneIndex].g;

    constexpr uint8_t Ry = kRedYUVColor[kYUVLumaPlaneIndex].r;
    constexpr uint8_t Ru = kRedYUVColor[kYUVChromaPlaneIndex].r;
    constexpr uint8_t Rv = kRedYUVColor[kYUVChromaPlaneIndex].g;

    constexpr uint8_t By = kBlueYUVColor[kYUVLumaPlaneIndex].r;
    constexpr uint8_t Bu = kBlueYUVColor[kYUVChromaPlaneIndex].r;
    constexpr uint8_t Bv = kBlueYUVColor[kYUVChromaPlaneIndex].g;

    switch (format) {
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // is half the number of bytes (subsampled by 2) but same bytes per line as luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Ru, Rv,  // plane 1, start + 16
                    Yu, Yv, Bu, Bv,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                };
            }
        case wgpu::TextureFormat::RGBA8Unorm:
            // Combines both NV12 planes by directly mapping back to RGB: R=Y, G=U, B=V.
            if (isCheckerboard) {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                };
            } else {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                };
            }
        default:
            UNREACHABLE();
            return {};
    }
}

uint32_t VideoViewsTests::NumPlanes(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return 2;
        default:
            UNREACHABLE();
            return 0;
    }
}
std::vector<uint8_t> VideoViewsTests::GetTestTextureDataWithPlaneIndex(size_t planeIndex,
                                                                       size_t bytesPerRow,
                                                                       size_t height,
                                                                       bool isCheckerboard) {
    std::vector<uint8_t> texelData = VideoViewsTests::GetTestTextureData(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, isCheckerboard);
    const uint32_t texelDataRowBytes = kYUVImageDataWidthInTexels;
    const uint32_t texelDataHeight =
        planeIndex == 0 ? kYUVImageDataHeightInTexels : kYUVImageDataHeightInTexels / 2;

    std::vector<uint8_t> texels(bytesPerRow * height, 0);
    uint32_t plane_first_texel_offset = 0;
    // The size of the test video frame is 4 x 4
    switch (planeIndex) {
        case VideoViewsTests::kYUVLumaPlaneIndex:
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataRowBytes; ++j) {
                        texels[bytesPerRow * i + j] =
                            texelData[texelDataRowBytes * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        case VideoViewsTests::kYUVChromaPlaneIndex:
            // TexelData is 4 * 6 size, first 4 * 4 is Y plane, UV plane started
            // at index 16.
            plane_first_texel_offset = 16;
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataRowBytes; ++j) {
                        texels[bytesPerRow * i + j] =
                            texelData[texelDataRowBytes * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        default:
            UNREACHABLE();
            return {};
    }
}

// Vertex shader used to render a sampled texture into a quad.
wgpu::ShaderModule VideoViewsTests::GetTestVertexShaderModule() const {
    return utils::CreateShaderModule(device, R"(
                struct VertexOut {
                    @location(0) texCoord : vec2 <f32>,
                    @builtin(position) position : vec4f,
                }

                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                    var pos = array(
                        vec2f(-1.0, 1.0),
                        vec2f(-1.0, -1.0),
                        vec2f(1.0, -1.0),
                        vec2f(-1.0, 1.0),
                        vec2f(1.0, -1.0),
                        vec2f(1.0, 1.0)
                    );
                    var output : VertexOut;
                    output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                    output.texCoord = vec2f(output.position.xy * 0.5) + vec2f(0.5, 0.5);
                    return output;
            })");
}

// Create video texture uninitialized.
TEST_P(VideoViewsTests, CreateVideoTextureWithoutInitializedData) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ false);
    ASSERT_NE(platformTexture.get(), nullptr);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

namespace {

// Samples the luminance (Y) plane from an imported NV12 texture into a single channel of an RGBA
// output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, NV12SampleYtoR) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = wgpu::TextureFormat::R8Unorm;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(texture, sampler0, texCoord).r;
               return vec4f(y, 0.0, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the luma plane in the top left corner of RGB image.
    EXPECT_PIXEL_RGBA8_EQ(kYellowYUVColor[kYUVLumaPlaneIndex], renderPass.color, 0, 0);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the chrominance (UV) plane from an imported texture into two channels of an RGBA output
// attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, NV12SampleUVtoRG) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = wgpu::TextureFormat::RG8Unorm;
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let u : f32 = textureSample(texture, sampler0, texCoord).r;
               let v : f32 = textureSample(texture, sampler0, texCoord).g;
               return vec4f(u, v, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the chroma plane in the top left corner of RGB image.
    EXPECT_PIXEL_RGBA8_EQ(kYellowYUVColor[kYUVChromaPlaneIndex], renderPass.color, 0, 0);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a NV12 "checkerboard" texture into a RGB quad, then checks the the entire
// contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, NV12SampleYUVtoRGB) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = wgpu::TextureFormat::R8Unorm;
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = wgpu::TextureFormat::RG8Unorm;
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels});
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a NV12 "checkerboard" texture into a RGB quad with two samplers, then checks the the
// entire contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, NV12SampleYUVtoRGBMultipleSamplers) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = wgpu::TextureFormat::R8Unorm;
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = wgpu::TextureFormat::RG8Unorm;
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(3) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler1, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler1, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler0 = device.CreateSampler();
    wgpu::Sampler sampler1 = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(
                   device, renderPipeline.GetBindGroupLayout(0),
                   {{0, sampler0}, {1, sampler1}, {2, lumaTextureView}, {3, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels});
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

DAWN_INSTANTIATE_TEST(VideoViewsTests, VideoViewsTestBackend::Backend());

}  // anonymous namespace
}  // namespace dawn
