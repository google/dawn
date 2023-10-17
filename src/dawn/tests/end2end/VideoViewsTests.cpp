// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/tests/end2end/VideoViewsTests.h"

#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

VideoViewsTestBackend::PlatformTexture::PlatformTexture(wgpu::Texture&& texture)
    : wgpuTexture(texture) {}
VideoViewsTestBackend::PlatformTexture::~PlatformTexture() = default;

VideoViewsTestBackend::~VideoViewsTestBackend() = default;

constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kYellowYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kWhiteYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kBlueYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kRedYUVColor;

void VideoViewsTestsBase::SetUp() {
    DawnTestWithParams<Params>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
}

std::vector<wgpu::FeatureName> VideoViewsTestsBase::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = {};
    mIsMultiPlanarFormatsSupported = SupportsFeatures({wgpu::FeatureName::DawnMultiPlanarFormats});
    if (mIsMultiPlanarFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::DawnMultiPlanarFormats);
    }
    mIsMultiPlanarFormatP010Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatP010});
    if (mIsMultiPlanarFormatP010Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatP010);
    }
    mIsNorm16TextureFormatsSupported = SupportsFeatures({wgpu::FeatureName::Norm16TextureFormats});
    if (mIsNorm16TextureFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::Norm16TextureFormats);
    }
    requiredFeatures.push_back(wgpu::FeatureName::DawnInternalUsages);
    return requiredFeatures;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatsSupported() const {
    return mIsMultiPlanarFormatsSupported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatP010Supported() const {
    return mIsMultiPlanarFormatP010Supported;
}

bool VideoViewsTestsBase::IsNorm16TextureFormatsSupported() const {
    return mIsNorm16TextureFormatsSupported;
}

bool VideoViewsTestsBase::IsFormatSupported() const {
    if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) {
        // DXGI_FORMAT_P010 can't be shared between D3D11 and D3D12.
        if (IsD3D12()) {
            return false;
        }
        // DXGI_FORMAT_P010 is not supported on WARP.
        if (IsWARP()) {
            return false;
        }
        return IsNorm16TextureFormatsSupported() && IsMultiPlanarFormatP010Supported();
    }

    return true;
}

// Returns a pre-prepared multi-planar formatted texture
// The encoded texture data represents a 4x4 converted image. When |isCheckerboard| is true,
// the top left is a 2x2 yellow block, bottom right is a 2x2 red block, top right is a 2x2
// blue block, and bottom left is a 2x2 white block. When |isCheckerboard| is false, the
// image is converted from a solid yellow 4x4 block.
// static
template <typename T>
std::vector<T> VideoViewsTestsBase::GetTestTextureData(wgpu::TextureFormat format,
                                                       bool isCheckerboard) {
    const uint8_t kLeftShiftBits = (sizeof(T) - 1) * 8;
    constexpr T Yy = kYellowYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yu = kYellowYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yv = kYellowYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T Wy = kWhiteYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wu = kWhiteYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wv = kWhiteYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T Ry = kRedYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Ru = kRedYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Rv = kRedYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T By = kBlueYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bu = kBlueYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bv = kBlueYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    switch (format) {
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // is half the number of bytes (subsampled by 2) but same bytes per line as luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
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
            DAWN_UNREACHABLE();
            return {};
    }
}

template std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureData<uint8_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard);
template std::vector<uint16_t> VideoViewsTestsBase::GetTestTextureData<uint16_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard);

uint32_t VideoViewsTestsBase::NumPlanes(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return 2;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

template <typename T>
std::vector<T> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex(size_t planeIndex,
                                                                     size_t bytesPerRow,
                                                                     size_t height,
                                                                     bool isCheckerboard) {
    std::vector<T> texelData = VideoViewsTestsBase::GetTestTextureData<T>(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, isCheckerboard);
    const uint32_t texelDataWidth = kYUVImageDataWidthInTexels;
    const uint32_t texelDataHeight =
        planeIndex == 0 ? kYUVImageDataHeightInTexels : kYUVImageDataHeightInTexels / 2;

    size_t rowPitch = bytesPerRow / sizeof(T);
    std::vector<T> texels(rowPitch * height, 0);
    uint32_t plane_first_texel_offset = 0;
    // The size of the test video frame is 4 x 4
    switch (planeIndex) {
        case VideoViewsTestsBase::kYUVLumaPlaneIndex:
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataWidth; ++j) {
                        texels[rowPitch * i + j] =
                            texelData[texelDataWidth * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        case VideoViewsTestsBase::kYUVChromaPlaneIndex:
            // TexelData is 4 * 6 size, first 4 * 4 is Y plane, UV plane started
            // at index 16.
            plane_first_texel_offset = 16;
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataWidth; ++j) {
                        texels[rowPitch * i + j] =
                            texelData[texelDataWidth * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        default:
            DAWN_UNREACHABLE();
            return {};
    }
}

template std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint8_t>(
    size_t planeIndex,
    size_t bytesPerRow,
    size_t height,
    bool isCheckerboard);
template std::vector<uint16_t> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint16_t>(
    size_t planeIndex,
    size_t bytesPerRow,
    size_t height,
    bool isCheckerboard);

wgpu::TextureFormat VideoViewsTestsBase::GetFormat() const {
    return GetParam().mFormat;
}

wgpu::TextureFormat VideoViewsTestsBase::GetPlaneFormat(int plane) const {
    switch (GetFormat()) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return plane == 0 ? wgpu::TextureFormat::R8Unorm : wgpu::TextureFormat::RG8Unorm;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return plane == 0 ? wgpu::TextureFormat::R16Unorm : wgpu::TextureFormat::RG16Unorm;
        default:
            DAWN_UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

// Vertex shader used to render a sampled texture into a quad.
wgpu::ShaderModule VideoViewsTestsBase::GetTestVertexShaderModule() const {
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

class VideoViewsTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

        mBackend = VideoViewsTestBackend::Create();
        mBackend->OnSetUp(device.Get());
    }

    void TearDown() override {
        if (mBackend) {
            mBackend->OnTearDown();
            mBackend = nullptr;
        }
        VideoViewsTestsBase::TearDown();
    }
    std::unique_ptr<VideoViewsTestBackend> mBackend;
};

namespace {

// Create video texture uninitialized.
TEST_P(VideoViewsTests, CreateVideoTextureWithoutInitializedData) {
    ASSERT_DEVICE_ERROR(std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
                            mBackend->CreateVideoTextureForTest(GetFormat(),
                                                                wgpu::TextureUsage::TextureBinding,
                                                                /*isCheckerboard*/ false,
                                                                /*initialized*/ false));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the luminance (Y) plane from an imported bi-planar 420 texture into a single channel of
// an RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleYtoR) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(0);
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
    EXPECT_TEXTURE_EQ(&kYellowYUVColor[kYUVLumaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the chrominance (UV) plane from an imported bi-planar 420 texture into two channels of an
// RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleUVtoRG) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(1);
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
    EXPECT_TEXTURE_EQ(&kYellowYUVColor[kYUVChromaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad, then checks the the entire
// contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGB) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
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

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad with two samplers, then checks the the
// entire contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGBMultipleSamplers) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
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

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsValidationTests : public VideoViewsTests {
  protected:
    void SetUp() override {
        VideoViewsTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test explicitly creating a multiplanar format is not allowed
TEST_P(VideoViewsValidationTests, ExplicitCreation) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test texture view creation rules.
TEST_P(VideoViewsValidationTests, CreateViewValidation) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc = {};

    // Success case: Per plane view formats unspecified.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Success case: Per plane view formats specified and aspect.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        viewDesc.format = GetPlaneFormat(0);
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        viewDesc.format = GetPlaneFormat(1);
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Correct plane index but incompatible view format.
    viewDesc.format = wgpu::TextureFormat::R8Uint;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::All;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Create a single plane texture.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    desc.size = {1, 1, 1};

    wgpu::Texture texture = device.CreateTexture(&desc);

    // Plane aspect specified with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Planar views with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from one multi-planar format into another fails.
TEST_P(VideoViewsValidationTests, T2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from one multi-planar format into another per plane fails.
TEST_P(VideoViewsValidationTests, T2TCopyPlaneAspectFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from a multi-planar format to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from multi-planar format per plane to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format fails.
TEST_P(VideoViewsValidationTests, B2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, B2TCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests which multi-planar formats are allowed to be sampled.
TEST_P(VideoViewsValidationTests, SamplingMultiPlanarTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // R8BG8Biplanar420Unorm is allowed to be sampled, if plane 0 or plane 1 is selected.
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureViewDescriptor desc = {};

    desc.aspect = wgpu::TextureAspect::Plane0Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    desc.aspect = wgpu::TextureAspect::Plane1Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests creating a texture with a multi-plane format.
TEST_P(VideoViewsValidationTests, RenderAttachmentInvalid) {
    // multi-planar formats are NOT allowed to be renderable by default and require
    // Feature::MultiPlanarRenderTargets.
    ASSERT_DEVICE_ERROR(auto platformTexture = mBackend->CreateVideoTextureForTest(
                            GetFormat(), wgpu::TextureUsage::RenderAttachment,
                            /*isCheckerboard*/ true,
                            /*initialized*/ true));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format fails.
TEST_P(VideoViewsValidationTests, WriteTextureAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 4, 4);

    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(platformTexture->wgpuTexture, 0, {0, 0, 0});

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, WriteTexturePlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 12, 4);
    wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
        platformTexture->wgpuTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsRenderTargetTests : public VideoViewsValidationTests {
  protected:
    void SetUp() override {
        VideoViewsValidationTests::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());

        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::MultiPlanarRenderTargets));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = VideoViewsTests::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::MultiPlanarRenderTargets})) {
            requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarRenderTargets);
        }
        return requiredFeatures;
    }

    // Tests for rendering to a multiplanar video texture through its views. It creates R/RG source
    // textures with data which are then read into luma and chroma texture views. Since multiplanar
    // textures don't support copy operations yet, the test renders from the luma/chroma texture
    // views into another R/RG textures which are then compared with for rendered data.
    template <typename T>
    void RenderToMultiplanarVideoTexture() {
        // Create plane texture initialized with data.
        auto CreatePlaneTexWithData = [this](int planeIndex) -> wgpu::Texture {
            auto kSubsampleFactor = planeIndex == kYUVLumaPlaneIndex ? 1 : 2;
            wgpu::Extent3D size = {kYUVImageDataWidthInTexels / kSubsampleFactor,
                                   kYUVImageDataHeightInTexels / kSubsampleFactor, 1};

            // Create source texture with plane format
            wgpu::TextureDescriptor planeTextureDesc;
            planeTextureDesc.size = size;
            planeTextureDesc.format = GetPlaneFormat(planeIndex);
            planeTextureDesc.usage =
                wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
            wgpu::Texture planeTexture = device.CreateTexture(&planeTextureDesc);

            // Copy plane (Y/UV) data to the plane source texture.
            std::vector<T> planeSrcData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
                planeIndex, kTextureBytesPerRowAlignment,
                kYUVImageDataHeightInTexels / kSubsampleFactor, false);
            wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(planeTexture);
            wgpu::TextureDataLayout textureDataLayout =
                utils::CreateTextureDataLayout(0, kTextureBytesPerRowAlignment);
            wgpu::Queue queue = device.GetQueue();
            queue.WriteTexture(&imageCopyTexture, planeSrcData.data(),
                               planeSrcData.size() * sizeof(T), &textureDataLayout, &size);

            return planeTexture;
        };

        // Create source texture with plane 0 format i.e. R8/R16Unorm.
        wgpu::Texture plane0Texture = CreatePlaneTexWithData(kYUVLumaPlaneIndex);
        ASSERT_NE(plane0Texture.Get(), nullptr);
        // Create source texture with plane 1 format i.e. RG8/RG16Unorm.
        wgpu::Texture plane1Texture = CreatePlaneTexWithData(kYUVChromaPlaneIndex);
        ASSERT_NE(plane1Texture.Get(), nullptr);

        // TODO(dawn:1337): Allow creating uninitialized texture for rendering.
        // Create a video texture to be rendered into with multiplanar format.
        auto destVideoTexture = mBackend->CreateVideoTextureForTest(
            GetFormat(), wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            /*isCheckerboard*/ false,
            /*initialized*/ true);
        ASSERT_NE(destVideoTexture.get(), nullptr);
        if (!destVideoTexture->CanWrapAsWGPUTexture()) {
            mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
            GTEST_SKIP() << "Skipped because not supported.";
        }
        auto destVideoWGPUTexture = destVideoTexture->wgpuTexture;

        // Perform plane operations for texting by creating render passes and comparing textures.
        auto PerformPlaneOperations = [this](int planeIndex, wgpu::Texture destVideoWGPUTexture,
                                             wgpu::Texture planeTextureWithData) {
            auto kSubsampleFactor = planeIndex == kYUVLumaPlaneIndex ? 1 : 2;

            utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
            renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();
            renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @group(0) @binding(0) var sampler0 : sampler;
                @group(0) @binding(1) var texture : texture_2d<f32>;

                @fragment
                fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                    return textureSample(texture, sampler0, texCoord);
                })");
            renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            renderPipelineDescriptor.cTargets[0].format = GetPlaneFormat(planeIndex);
            wgpu::RenderPipeline renderPipeline =
                device.CreateRenderPipeline(&renderPipelineDescriptor);
            wgpu::Sampler sampler = device.CreateSampler();

            // Create plane texture view from the multiplanar video texture.
            wgpu::TextureViewDescriptor planeViewDesc;
            planeViewDesc.format = GetPlaneFormat(planeIndex);
            planeViewDesc.aspect = (planeIndex == kYUVLumaPlaneIndex)
                                       ? wgpu::TextureAspect::Plane0Only
                                       : wgpu::TextureAspect::Plane1Only;
            wgpu::TextureView planeTextureView = destVideoWGPUTexture.CreateView(&planeViewDesc);

            // Render pass operations for reading the source data from planeTexture view into
            // planeTextureView created from the multiplanar video texture.
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor renderPass({planeTextureView});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(renderPipeline);
            pass.SetBindGroup(
                0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                        {{0, sampler}, {1, planeTextureWithData.CreateView()}}));
            pass.Draw(6);
            pass.End();

            // Another render pass for reading the planeTextureView into a texture of the plane's
            // format (i.e. R8/R16Unorm for Y and RG8/RG16Unorm for UV). This is needed as
            // multiplanar textures do not support copy operations.
            utils::BasicRenderPass basicRenderPass = utils::CreateBasicRenderPass(
                device, kYUVImageDataWidthInTexels / kSubsampleFactor,
                kYUVImageDataHeightInTexels / kSubsampleFactor, GetPlaneFormat(planeIndex));
            wgpu::RenderPassEncoder secondPass =
                encoder.BeginRenderPass(&basicRenderPass.renderPassInfo);
            secondPass.SetPipeline(renderPipeline);
            secondPass.SetBindGroup(
                0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                        {{0, sampler}, {1, planeTextureView}}));
            secondPass.Draw(6);
            secondPass.End();

            // Submit all commands for the encoder.
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            std::vector<T> expectedData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
                planeIndex, kYUVImageDataWidthInTexels * sizeof(T),
                kYUVImageDataHeightInTexels / kSubsampleFactor, false);
            EXPECT_TEXTURE_EQ(expectedData.data(), basicRenderPass.color, {0, 0},
                              {kYUVImageDataWidthInTexels / kSubsampleFactor,
                               kYUVImageDataHeightInTexels / kSubsampleFactor},
                              GetPlaneFormat(planeIndex));
        };

        // Perform operations for the Y plane.
        PerformPlaneOperations(kYUVLumaPlaneIndex, destVideoWGPUTexture, plane0Texture);
        // Perform operations for the UV plane.
        PerformPlaneOperations(kYUVChromaPlaneIndex, destVideoWGPUTexture, plane1Texture);

        mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
    }
};

// Tests creating a texture with a multi-plane format.
TEST_P(VideoViewsRenderTargetTests, RenderAttachmentValid) {
    // multi-planar formats should be allowed to be renderable with
    // Feature::MultiPlanarRenderTargets.
    auto platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::RenderAttachment,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests validating attachment sizes with a multi-plane format.
TEST_P(VideoViewsRenderTargetTests, RenderAttachmentSizeValidation) {
    auto platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::RenderAttachment,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    // Create luma texture view from the video texture.
    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    // Create chroma texture view from the video texture.
    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    // Create an RGBA texture with same size as chroma texture view.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::RenderAttachment;
    desc.size = {kYUVImageDataWidthInTexels / 2, kYUVImageDataHeightInTexels / 2, 1};
    wgpu::Texture rgbaTexture = device.CreateTexture(&desc);

    {
        // Render pass operations passing color attachments of same size (control case).
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({chromaTextureView, rgbaTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        encoder.Finish();
    }

    {
        // Render pass operations passing color attachments of different sizes (error case).
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({chromaTextureView, lumaTextureView});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests for rendering to a multiplanar video texture through its views.
TEST_P(VideoViewsRenderTargetTests, RenderToMultiplanarVideoTexture) {
    if (GetFormat() == wgpu::TextureFormat::R8BG8Biplanar420Unorm) {
        RenderToMultiplanarVideoTexture<uint8_t>();
    } else if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) {
        RenderToMultiplanarVideoTexture<uint16_t>();
    } else {
        DAWN_UNREACHABLE();
    }
}

class VideoViewsExtendedUsagesTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

        DAWN_TEST_UNSUPPORTED_IF(
            !device.HasFeature(wgpu::FeatureName::MultiPlanarFormatExtendedUsages));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures =
            VideoViewsTestsBase::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatExtendedUsages);
        }
        return requiredFeatures;
    }

    wgpu::Texture CreateMultiPlanarTexture(wgpu::TextureFormat format,
                                           wgpu::TextureUsage usage,
                                           bool isCheckerboard = false,
                                           bool initialized = true) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return CreateMultiPlanarTextureImpl<uint8_t>(format, usage, isCheckerboard,
                                                             initialized);
            case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
                return CreateMultiPlanarTextureImpl<uint16_t>(format, usage, isCheckerboard,
                                                              initialized);
            default:
                DAWN_UNREACHABLE();
                return nullptr;
        }
    }

    template <typename ComponentType>
    wgpu::Texture CreateMultiPlanarTextureImpl(wgpu::TextureFormat format,
                                               wgpu::TextureUsage usage,
                                               bool isCheckerboard,
                                               bool initialized) {
        wgpu::TextureDescriptor desc;
        desc.format = format;
        desc.size = {VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                     VideoViewsTestsBase::kYUVImageDataHeightInTexels, 1};
        desc.usage = usage;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
        desc.nextInChain = &internalDesc;

        auto texture = device.CreateTexture(&desc);
        if (texture == nullptr) {
            return nullptr;
        }

        if (initialized) {
            size_t numPlanes = VideoViewsTestsBase::NumPlanes(format);

            wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc;
            encoderInternalDesc.useInternalUsages = true;
            wgpu::CommandEncoderDescriptor encoderDesc;
            encoderDesc.nextInChain = &encoderInternalDesc;

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

            for (size_t plane = 0; plane < numPlanes; ++plane) {
                size_t bytesPerRow =
                    VideoViewsTestsBase::kYUVImageDataWidthInTexels * sizeof(ComponentType);
                bytesPerRow = Align(bytesPerRow, 256);

                wgpu::ImageCopyTexture copyDst =
                    utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});

                wgpu::Extent3D copySize{VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                                        VideoViewsTestsBase::kYUVImageDataHeightInTexels, 1};
                switch (plane) {
                    case VideoViewsTestsBase::kYUVLumaPlaneIndex:
                        copyDst.aspect = wgpu::TextureAspect::Plane0Only;
                        break;
                    case VideoViewsTestsBase::kYUVChromaPlaneIndex:
                        copyDst.aspect = wgpu::TextureAspect::Plane1Only;
                        copySize.width /= 2;
                        copySize.height /= 2;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                }

                // Staging buffer.
                wgpu::BufferDescriptor bufferDesc;
                bufferDesc.size = bytesPerRow * copySize.height;
                bufferDesc.mappedAtCreation = true;
                bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

                auto buffer = device.CreateBuffer(&bufferDesc);

                std::vector<ComponentType> data = GetTestTextureDataWithPlaneIndex<ComponentType>(
                    plane, bytesPerRow, VideoViewsTestsBase::kYUVImageDataHeightInTexels,
                    isCheckerboard);

                memcpy(buffer.GetMappedRange(), data.data(), bufferDesc.size);
                buffer.Unmap();

                wgpu::ImageCopyBuffer copySrc =
                    utils::CreateImageCopyBuffer(buffer, 0, bytesPerRow);

                encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
            }  // for plane

            auto cmdBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &cmdBuffer);
        }  // initialized

        return texture;
    }

    template <typename ComponentType>
    void RunT2BCopyPlaneAspectsTest();
};

// Test that creating multi-planar texture should success if device is created with
// MultiPlanarFormatExtendedUsages feature enabled.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureSucceeds) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    auto texture = device.CreateTexture(&descriptor);
    EXPECT_NE(texture, nullptr);
}

// Test that creating multi-planar texture should fail if the specified descriptor is mipmapped.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsIfMipmapped) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 2;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));
}

// Test that creating multi-planar texture should fail if the specified descriptor is not 2D.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsIfNot2D) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // Texture must not be 3D.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));

    // Texture must not be array.
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.depthOrArrayLayers = 2;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));
}

// Test that creating multiplanar texture with view formats should fail.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsWithViewFormats) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    {
        std::vector<wgpu::TextureFormat> viewFormats = {
            wgpu::TextureFormat::RGBA8Unorm,
        };
        descriptor.viewFormatCount = viewFormats.size();
        descriptor.viewFormats = viewFormats.data();
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
    {
        std::vector<wgpu::TextureFormat> viewFormats = {
            GetPlaneFormat(0),
            GetPlaneFormat(1),
        };
        descriptor.viewFormatCount = viewFormats.size();
        descriptor.viewFormats = viewFormats.data();
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Tests sampling a multi-planar texture.
TEST_P(VideoViewsExtendedUsagesTests, SamplingMultiPlanarTexture) {
    // TODO(crbug.com/dawn/1998): Failure on Intel's Vulkan device.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    auto texture = CreateMultiPlanarTexture(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    EXPECT_NE(texture, nullptr);

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = texture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = texture.CreateView(&chromaViewDesc);

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

    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, kYUVImageDataWidthInTexels,
                                     kYUVImageDataHeightInTexels, wgpu::TextureFormat::RGBA8Unorm);
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

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard*/ true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
}

// Test copying from multi-planar format per plane to a buffer succeeds.
TEST_P(VideoViewsExtendedUsagesTests, T2BCopyPlaneAspectsSucceeds) {
    switch (GetFormat()) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            RunT2BCopyPlaneAspectsTest<uint8_t>();
            break;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            RunT2BCopyPlaneAspectsTest<uint16_t>();
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

template <typename ComponentType>
void VideoViewsExtendedUsagesTests::RunT2BCopyPlaneAspectsTest() {
    auto srcTexture = CreateMultiPlanarTexture(
        GetFormat(), wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
        /*isCheckerboard*/ false,
        /*initialized*/ true);
    EXPECT_NE(srcTexture, nullptr);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    // Plane0
    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    {
        std::vector<ComponentType> expectedData =
            VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<ComponentType>(
                kYUVLumaPlaneIndex, kYUVImageDataWidthInTexels * sizeof(ComponentType),
                kYUVImageDataHeightInTexels, false);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        // Convert 1st pixel's luma component to array of 8 bits bytes.
        uint8_t expectedYDataAsU8[sizeof(ComponentType)];
        memcpy(expectedYDataAsU8, &expectedData[0], sizeof(expectedYDataAsU8));

        EXPECT_BUFFER_U8_RANGE_EQ(expectedYDataAsU8, dstBuffer, 0, sizeof(expectedYDataAsU8));
    }

    // Plane1
    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);
    {
        std::vector<ComponentType> expectedData =
            VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<ComponentType>(
                kYUVChromaPlaneIndex, kYUVImageDataWidthInTexels * sizeof(ComponentType),
                kYUVImageDataHeightInTexels / 2, false);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        // Convert 1st pixel's chroma component to array of 8 bits bytes.
        uint8_t expectedUVDataAsU8[sizeof(ComponentType) * 2];
        memcpy(expectedUVDataAsU8, expectedData.data(), sizeof(expectedUVDataAsU8));

        EXPECT_BUFFER_U8_RANGE_EQ(expectedUVDataAsU8, dstBuffer, 0, sizeof(expectedUVDataAsU8));
    }
}

// Test copying from a multi-planar format to a buffer fails if texture aspect is not single plane.
TEST_P(VideoViewsExtendedUsagesTests, T2BCopyAllAspectsFails) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto srcTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), testing::HasSubstr("More than a single aspect"));
}

// Test copying from one multi-planar formatted texture into another fails even if
// MultiPlanarFormatExtendedUsages is enabled.
TEST_P(VideoViewsExtendedUsagesTests, T2TCopyPlaneAspectFails) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto srcTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);
    auto dstTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), testing::HasSubstr("not allowed"));
}

DAWN_INSTANTIATE_TEST_B(VideoViewsTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());
DAWN_INSTANTIATE_TEST_B(VideoViewsValidationTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());
DAWN_INSTANTIATE_TEST_B(VideoViewsRenderTargetTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());

DAWN_INSTANTIATE_TEST_B(VideoViewsExtendedUsagesTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                         wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm});

}  // anonymous namespace
}  // namespace dawn
