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

#include <algorithm>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

using Format = wgpu::TextureFormat;
enum class Check {
    CopyStencil,
    StencilTest,
    CopyDepth,
    DepthTest,
    SampleDepth,
};

std::ostream& operator<<(std::ostream& o, Check check) {
    switch (check) {
        case Check::CopyStencil:
            o << "CopyStencil";
            break;
        case Check::StencilTest:
            o << "StencilTest";
            break;
        case Check::CopyDepth:
            o << "CopyDepth";
            break;
        case Check::DepthTest:
            o << "DepthTest";
            break;
        case Check::SampleDepth:
            o << "SampleDepth";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(DepthStencilLoadOpTestParams, Format, Check);

constexpr static uint32_t kRTSize = 16;
constexpr uint32_t kMipLevelCount = 2u;
constexpr std::array<float, kMipLevelCount> kDepthValues = {0.125f, 0.875f};
constexpr std::array<uint16_t, kMipLevelCount> kU16DepthValues = {8192u, 57343u};
constexpr std::array<uint8_t, kMipLevelCount> kStencilValues = {7u, 3u};

class DepthStencilLoadOpTests : public DawnTestWithParams<DepthStencilLoadOpTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<DepthStencilLoadOpTestParams>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);

        // Readback of Depth/Stencil textures not fully supported on GL right now.
        // Also depends on glTextureView which is not supported on ES.
        DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

        wgpu::TextureDescriptor descriptor;
        descriptor.size = {kRTSize, kRTSize};
        descriptor.format = GetParam().mFormat;
        descriptor.mipLevelCount = kMipLevelCount;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                           wgpu::TextureUsage::TextureBinding;

        texture = device.CreateTexture(&descriptor);

        wgpu::TextureViewDescriptor textureViewDesc = {};
        textureViewDesc.mipLevelCount = 1;

        for (uint32_t mipLevel = 0; mipLevel < kMipLevelCount; ++mipLevel) {
            textureViewDesc.baseMipLevel = mipLevel;
            textureViews[mipLevel] = texture.CreateView(&textureViewDesc);

            utils::ComboRenderPassDescriptor renderPassDescriptor({}, textureViews[mipLevel]);
            renderPassDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mFormat);
            renderPassDescriptor.cDepthStencilAttachmentInfo.depthClearValue =
                kDepthValues[mipLevel];
            renderPassDescriptor.cDepthStencilAttachmentInfo.stencilClearValue =
                kStencilValues[mipLevel];
            renderPassDescriptors.push_back(renderPassDescriptor);
        }
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mFormat) {
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

    void CheckMipLevel(uint32_t mipLevel) {
        uint32_t mipSize = std::max(kRTSize >> mipLevel, 1u);

        switch (GetParam().mCheck) {
            case Check::SampleDepth: {
                std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                ExpectSampledDepthData(
                    texture, mipSize, mipSize, 0, mipLevel,
                    new detail::ExpectEq<float>(expectedDepth.data(), expectedDepth.size(), 0.0001))
                    << "sample depth mip " << mipLevel;
                break;
            }

            case Check::CopyDepth: {
                if (GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm) {
                    std::vector<uint16_t> expectedDepth(mipSize * mipSize,
                                                        kU16DepthValues[mipLevel]);
                    EXPECT_TEXTURE_EQ(expectedDepth.data(), texture, {0, 0}, {mipSize, mipSize},
                                      mipLevel, wgpu::TextureAspect::DepthOnly)
                        << "copy depth mip " << mipLevel;
                } else {
                    std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                    EXPECT_TEXTURE_EQ(expectedDepth.data(), texture, {0, 0}, {mipSize, mipSize},
                                      mipLevel, wgpu::TextureAspect::DepthOnly)
                        << "copy depth mip " << mipLevel;
                }

                break;
            }

            case Check::CopyStencil: {
                std::vector<uint8_t> expectedStencil(mipSize * mipSize, kStencilValues[mipLevel]);
                EXPECT_TEXTURE_EQ(expectedStencil.data(), texture, {0, 0}, {mipSize, mipSize},
                                  mipLevel, wgpu::TextureAspect::StencilOnly)
                    << "copy stencil mip " << mipLevel;
                break;
            }

            case Check::DepthTest: {
                std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                ExpectAttachmentDepthTestData(texture, GetParam().mFormat, mipSize, mipSize, 0,
                                              mipLevel, expectedDepth)
                    << "depth test mip " << mipLevel;
                break;
            }

            case Check::StencilTest: {
                ExpectAttachmentStencilTestData(texture, GetParam().mFormat, mipSize, mipSize, 0,
                                                mipLevel, kStencilValues[mipLevel])
                    << "stencil test mip " << mipLevel;
                break;
            }
        }
    }

    wgpu::Texture texture;
    std::array<wgpu::TextureView, kMipLevelCount> textureViews;
    // Vector instead of array because there is no default constructor.
    std::vector<utils::ComboRenderPassDescriptor> renderPassDescriptors;

  private:
    bool mIsFormatSupported = false;
};

}  // anonymous namespace

// Check that clearing a mip level works at all.
TEST_P(DepthStencilLoadOpTests, ClearMip0) {
    // TODO(https://issuetracker.google.com/issues/204919030): SwiftShader does not clear
    // Depth16Unorm correctly with some values.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() &&
                          GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
}

// Check that clearing a non-zero mip level works at all.
TEST_P(DepthStencilLoadOpTests, ClearMip1) {
    // TODO(crbug.com/dawn/838): Sampling from the non-zero mip does not work.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && GetParam().mCheck == Check::SampleDepth);

    // TODO(crbug.com/dawn/838): Copying from the non-zero mip here sometimes returns uninitialized
    // data! (from mip 0 of a previous test run).
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && GetParam().mCheck == Check::CopyDepth);
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && GetParam().mCheck == Check::CopyStencil);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(1u);
}

// Clear first mip then the second mip.  Check both mip levels.
TEST_P(DepthStencilLoadOpTests, ClearBothMip0Then1) {
    // TODO(crbug.com/dawn/838): Sampling from the non-zero mip does not work.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && GetParam().mCheck == Check::SampleDepth);

    // TODO(https://issuetracker.google.com/issues/204919030): SwiftShader does not clear
    // Depth16Unorm correctly with some values.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() &&
                          GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
    CheckMipLevel(1u);
}

// Clear second mip then the first mip. Check both mip levels.
TEST_P(DepthStencilLoadOpTests, ClearBothMip1Then0) {
    // TODO(crbug.com/dawn/838): Sampling from the non-zero mip does not work.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && GetParam().mCheck == Check::SampleDepth);

    // TODO(https://issuetracker.google.com/issues/204919030): SwiftShader does not clear
    // Depth16Unorm correctly with some values.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() &&
                          GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
    CheckMipLevel(1u);
}

namespace {

auto GenerateParams() {
    auto params1 = MakeParamGenerator<DepthStencilLoadOpTestParams>(
        {D3D12Backend(), D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
        {wgpu::TextureFormat::Depth32Float, wgpu::TextureFormat::Depth16Unorm},
        {Check::CopyDepth, Check::DepthTest, Check::SampleDepth});

    auto params2 = MakeParamGenerator<DepthStencilLoadOpTestParams>(
        {D3D12Backend(), D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
        {wgpu::TextureFormat::Depth24PlusStencil8, wgpu::TextureFormat::Depth24UnormStencil8,
         wgpu::TextureFormat::Depth32FloatStencil8},
        {Check::CopyStencil, Check::StencilTest, Check::DepthTest, Check::SampleDepth});

    std::vector<DepthStencilLoadOpTestParams> allParams;
    allParams.insert(allParams.end(), params1.begin(), params1.end());
    allParams.insert(allParams.end(), params2.begin(), params2.end());

    return allParams;
}

INSTANTIATE_TEST_SUITE_P(,
                         DepthStencilLoadOpTests,
                         ::testing::ValuesIn(GenerateParams()),
                         DawnTestBase::PrintToStringParamName("DepthStencilLoadOpTests"));
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthStencilLoadOpTests);

}  // namespace
