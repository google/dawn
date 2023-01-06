// Copyright 2022 The Dawn Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

wgpu::Texture Create2DTexture(wgpu::Device device,
                              uint32_t width,
                              uint32_t height,
                              wgpu::TextureFormat format,
                              wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = usage;
    return device.CreateTexture(&descriptor);
}

static constexpr uint32_t kWidth = 4;
static constexpr uint32_t kHeight = 4;

std::array<std::array<utils::RGBA8, 4>, 4> kDefaultSourceRGBA = {
    std::array<utils::RGBA8, 4>(
        {utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kRed}),
    std::array<utils::RGBA8, 4>(
        {utils::RGBA8::kBlack, utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kRed}),
    std::array<utils::RGBA8, 4>(
        {utils::RGBA8::kGreen, utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlue}),
    std::array<utils::RGBA8, 4>(
        {utils::RGBA8::kGreen, utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlue})};

template <typename Parent>
class CopyExternalTextureForBrowserTests : public Parent {
  protected:
    wgpu::ExternalTexture CreateDefaultExternalTexture() {
        // y plane
        wgpu::TextureDescriptor externalTexturePlane0Desc = {};
        externalTexturePlane0Desc.size = {kWidth, kHeight, 1};
        externalTexturePlane0Desc.usage = wgpu::TextureUsage::TextureBinding |
                                          wgpu::TextureUsage::CopyDst |
                                          wgpu::TextureUsage::RenderAttachment;
        externalTexturePlane0Desc.format = wgpu::TextureFormat::R8Unorm;
        wgpu::Texture externalTexturePlane0 =
            this->device.CreateTexture(&externalTexturePlane0Desc);

        // The value Ref to ExternalTextureTest.cpp:
        //  {0.0, .5, .5, utils::RGBA8::kBlack, 0.0f},
        //  {0.2126, 0.4172, 1.0, utils::RGBA8::kRed, 1.0f},
        //  {0.7152, 0.1402, 0.0175, utils::RGBA8::kGreen, 0.0f},
        //  {0.0722, 1.0, 0.4937, utils::RGBA8::kBlue, 0.0f},
        wgpu::ImageCopyTexture plane0 = {};
        plane0.texture = externalTexturePlane0;
        std::array<uint8_t, 16> yPlaneData = {0,   0,   54, 54, 0,   0,   54, 54,
                                              182, 182, 18, 18, 182, 182, 18, 18};

        wgpu::TextureDataLayout externalTexturePlane0DataLayout = {};
        externalTexturePlane0DataLayout.bytesPerRow = 4;

        this->queue.WriteTexture(&plane0, yPlaneData.data(),
                                 yPlaneData.size() * sizeof(yPlaneData[0]),
                                 &externalTexturePlane0DataLayout, &externalTexturePlane0Desc.size);

        // uv plane
        wgpu::TextureDescriptor externalTexturePlane1Desc = {};
        externalTexturePlane1Desc.size = {kWidth / 2, kHeight / 2, 1};
        externalTexturePlane1Desc.usage = wgpu::TextureUsage::TextureBinding |
                                          wgpu::TextureUsage::CopyDst |
                                          wgpu::TextureUsage::RenderAttachment;
        externalTexturePlane1Desc.format = wgpu::TextureFormat::RG8Unorm;
        wgpu::Texture externalTexturePlane1 =
            this->device.CreateTexture(&externalTexturePlane1Desc);

        wgpu::ImageCopyTexture plane1 = {};
        plane1.texture = externalTexturePlane1;
        std::array<uint8_t, 8> uvPlaneData = {
            128, 128, 106, 255, 36, 4, 255, 126,
        };

        wgpu::TextureDataLayout externalTexturePlane1DataLayout = {};
        externalTexturePlane1DataLayout.bytesPerRow = 4;

        this->queue.WriteTexture(&plane1, uvPlaneData.data(),
                                 uvPlaneData.size() * sizeof(uvPlaneData[0]),
                                 &externalTexturePlane1DataLayout, &externalTexturePlane1Desc.size);

        // Create an ExternalTextureDescriptor from the texture views
        wgpu::ExternalTextureDescriptor externalDesc;
        utils::ColorSpaceConversionInfo info =
            utils::GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();
        externalDesc.yuvToRgbConversionMatrix = info.yuvToRgbConversionMatrix.data();
        externalDesc.gamutConversionMatrix = info.gamutConversionMatrix.data();
        externalDesc.srcTransferFunctionParameters = info.srcTransferFunctionParameters.data();
        externalDesc.dstTransferFunctionParameters = info.dstTransferFunctionParameters.data();

        externalDesc.plane0 = externalTexturePlane0.CreateView();
        externalDesc.plane1 = externalTexturePlane1.CreateView();

        externalDesc.visibleOrigin = {0, 0};
        externalDesc.visibleSize = {kWidth, kHeight};

        // Import the external texture
        return this->device.CreateExternalTexture(&externalDesc);
    }

    std::vector<utils::RGBA8> GetDefaultExpectedData(bool flipY,
                                                     wgpu::Origin3D origin,
                                                     wgpu::Extent3D rect) {
        std::vector<utils::RGBA8> expected;
        for (uint32_t row = origin.y; row < origin.y + rect.height; ++row) {
            for (uint32_t col = origin.x; col < origin.x + rect.width; ++col) {
                if (flipY) {
                    uint32_t flippedRow = kHeight - row - 1;
                    expected.push_back(kDefaultSourceRGBA[flippedRow][col]);
                } else {
                    expected.push_back(kDefaultSourceRGBA[row][col]);
                }
            }
        }

        return expected;
    }
};

using FlipY = bool;
using SrcOrigin = wgpu::Origin3D;
using DstOrigin = wgpu::Origin3D;

std::ostream& operator<<(std::ostream& o, wgpu::Origin3D origin) {
    o << origin.x << ", " << origin.y << ", " << origin.z;
    return o;
}

DAWN_TEST_PARAM_STRUCT(CopyTestParams, SrcOrigin, DstOrigin, FlipY);

class CopyExternalTextureForBrowserTests_Basic
    : public CopyExternalTextureForBrowserTests<DawnTestWithParams<CopyTestParams>> {
  protected:
    void DoBasicCopyTest(const wgpu::Origin3D& srcOrigin,
                         const wgpu::Origin3D& dstOrigin,
                         const wgpu::Extent3D& copySize,
                         const wgpu::CopyTextureForBrowserOptions options = {}) {
        wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
        wgpu::ImageCopyExternalTexture srcImageCopyExternalTexture;
        srcImageCopyExternalTexture.externalTexture = externalTexture;
        srcImageCopyExternalTexture.origin = srcOrigin;

        wgpu::Texture dstTexture =
            Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                                wgpu::TextureUsage::CopyDst);
        wgpu::ImageCopyTexture dstImageCopyTexture =
            utils::CreateImageCopyTexture(dstTexture, 0, dstOrigin);

        queue.CopyExternalTextureForBrowser(&srcImageCopyExternalTexture, &dstImageCopyTexture,
                                            &copySize, &options);
        std::vector<utils::RGBA8> expected =
            GetDefaultExpectedData(options.flipY, srcOrigin, copySize);

        EXPECT_TEXTURE_EQ(expected.data(), dstTexture, dstOrigin, copySize);
    }
};
}  // anonymous namespace

TEST_P(CopyExternalTextureForBrowserTests_Basic, FullCopy) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = GetParam().mFlipY;

    wgpu::Origin3D srcOrigin = GetParam().mSrcOrigin;
    wgpu::Origin3D dstOrigin = GetParam().mDstOrigin;

    wgpu::Extent3D copySize = {kWidth, kHeight};

    if (srcOrigin.x != 0 || srcOrigin.y != 0 || dstOrigin.x != 0 || dstOrigin.y != 0) {
        copySize.width = kWidth / 2;
        copySize.height = kHeight / 2;
    }

    DoBasicCopyTest(srcOrigin, dstOrigin, copySize, options);
}

DAWN_INSTANTIATE_TEST_P(CopyExternalTextureForBrowserTests_Basic,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         VulkanBackend()},
                        std::vector<wgpu::Origin3D>({{0, 0}, {2, 0}, {0, 2}, {2, 2}}),
                        std::vector<wgpu::Origin3D>({{0, 0}, {2, 0}, {0, 2}, {2, 2}}),
                        std::vector<bool>({false, true}));
