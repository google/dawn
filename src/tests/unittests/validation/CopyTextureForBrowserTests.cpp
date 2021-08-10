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

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "tests/unittests/validation/ValidationTest.h"
#include "utils/TestUtils.h"
#include "utils/TextureUtils.h"
#include "utils/WGPUHelpers.h"

class CopyTextureForBrowserTest : public ValidationTest {
  protected:
    wgpu::Texture Create2DTexture(uint32_t width,
                                  uint32_t height,
                                  uint32_t mipLevelCount,
                                  uint32_t arrayLayerCount,
                                  wgpu::TextureFormat format,
                                  wgpu::TextureUsage usage,
                                  uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depthOrArrayLayers = arrayLayerCount;
        descriptor.sampleCount = sampleCount;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;
        wgpu::Texture tex = device.CreateTexture(&descriptor);
        return tex;
    }

    void TestCopyTextureForBrowser(utils::Expectation expectation,
                                   wgpu::Texture srcTexture,
                                   uint32_t srcLevel,
                                   wgpu::Origin3D srcOrigin,
                                   wgpu::Texture dstTexture,
                                   uint32_t dstLevel,
                                   wgpu::Origin3D dstOrigin,
                                   wgpu::Extent3D extent3D,
                                   wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcTexture, srcLevel, srcOrigin, aspect);
        wgpu::ImageCopyTexture dstImageCopyTexture =
            utils::CreateImageCopyTexture(dstTexture, dstLevel, dstOrigin, aspect);
        wgpu::CopyTextureForBrowserOptions options = {};

        if (expectation == utils::Expectation::Success) {
            device.GetQueue().CopyTextureForBrowser(&srcImageCopyTexture, &dstImageCopyTexture,
                                                    &extent3D, &options);
        } else {
            ASSERT_DEVICE_ERROR(device.GetQueue().CopyTextureForBrowser(
                &srcImageCopyTexture, &dstImageCopyTexture, &extent3D, &options));
        }
    }
};

// Tests should be Success
TEST_F(CopyTextureForBrowserTest, Success) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Different copies, including some that touch the OOB condition
    {
        // Copy a region along top left boundary
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy entire texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // Copy a region along bottom right boundary
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {8, 8, 0}, destination, 0,
                                  {8, 8, 0}, {8, 8, 1});

        // Copy region into mip
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 2,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy mip into region
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 2, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy between slices
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 1}, {16, 16, 1});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 1});

        // An empty copy with depth = 0
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 0});

        // An empty copy touching the side of the source texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {16, 16, 0}, {0, 0, 1});

        // An empty copy touching the side of the destination texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {16, 16, 0}, {0, 0, 1});
    }
}

// Test source or destination texture has wrong usages
TEST_F(CopyTextureForBrowserTest, IncorrectUsage) {
    wgpu::Texture validSource =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture validDestination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture noSampledUsageSource =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture noRenderAttachmentUsageDestination =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);
    wgpu::Texture noCopySrcUsageSource = Create2DTexture(
        16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::TextureBinding);
    wgpu::Texture noCopyDstUsageSource = Create2DTexture(
        16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::RenderAttachment);

    // Incorrect source usage causes failure : lack |Sampled| usage
    TestCopyTextureForBrowser(utils::Expectation::Failure, noSampledUsageSource, 0, {0, 0, 0},
                              validDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect destination usage causes failure: lack |RenderAttachement| usage.
    TestCopyTextureForBrowser(utils::Expectation::Failure, validSource, 0, {0, 0, 0},
                              noRenderAttachmentUsageDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect source usage causes failure : lack |CopySrc| usage
    TestCopyTextureForBrowser(utils::Expectation::Failure, noCopySrcUsageSource, 0, {0, 0, 0},
                              validDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect destination usage causes failure: lack |CopyDst| usage.
    TestCopyTextureForBrowser(utils::Expectation::Failure, validSource, 0, {0, 0, 0},
                              noCopyDstUsageSource, 0, {0, 0, 0}, {16, 16, 1});
}

// Test non-zero value origin in source and OOB copy rects.
TEST_F(CopyTextureForBrowserTest, OutOfBounds) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // OOB on source
    {
        // x + width overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {1, 0, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // y + height overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 1, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // non-zero mip overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {9, 9, 1});

        // copy to multiple slices
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 2}, {16, 16, 2});

        // copy origin z value is non-zero.
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 1}, destination, 0,
                                  {0, 0, 2}, {16, 16, 1});
    }

    // OOB on destination
    {
        // x + width overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {1, 0, 0}, {16, 16, 1});

        // y + height overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 1, 0}, {16, 16, 1});

        // non-zero mip overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 1,
                                  {0, 0, 0}, {9, 9, 1});

        // arrayLayer + depth OOB
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 4}, {16, 16, 1});

        // empty copy on non-existent mip fails
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 6,
                                  {0, 0, 0}, {0, 0, 1});

        // empty copy on non-existent slice fails
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 4}, {0, 0, 1});
    }
}

// Test destination texture has format that not supported by CopyTextureForBrowser().
TEST_F(CopyTextureForBrowserTest, InvalidDstFormat) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RG8Uint,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Not supported dst texture format.
    TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                              {0, 0, 0}, {0, 0, 1});
}

// Test source or destination texture are multisampled.
TEST_F(CopyTextureForBrowserTest, InvalidSampleCount) {
    wgpu::Texture sourceMultiSampled1x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding, 1);
    wgpu::Texture destinationMultiSampled1x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 1);
    wgpu::Texture sourceMultiSampled4x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding, 4);
    wgpu::Texture destinationMultiSampled4x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 4);

    // An empty copy with dst texture sample count > 1 failure.
    TestCopyTextureForBrowser(utils::Expectation::Failure, sourceMultiSampled1x, 0, {0, 0, 0},
                              destinationMultiSampled4x, 0, {0, 0, 0}, {0, 0, 1});

    // A empty copy with source texture sample count > 1 failure
    TestCopyTextureForBrowser(utils::Expectation::Failure, sourceMultiSampled4x, 0, {0, 0, 0},
                              destinationMultiSampled1x, 0, {0, 0, 0}, {0, 0, 1});
}
