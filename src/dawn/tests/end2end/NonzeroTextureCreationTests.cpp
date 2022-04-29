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

#include <algorithm>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

    using Format = wgpu::TextureFormat;
    using Aspect = wgpu::TextureAspect;
    using Usage = wgpu::TextureUsage;
    using Dimension = wgpu::TextureDimension;
    using DepthOrArrayLayers = uint32_t;
    using MipCount = uint32_t;
    using Mip = uint32_t;
    using SampleCount = uint32_t;

    DAWN_TEST_PARAM_STRUCT(Params,
                           Format,
                           Aspect,
                           Usage,
                           Dimension,
                           DepthOrArrayLayers,
                           MipCount,
                           Mip,
                           SampleCount);

    template <typename T>
    class ExpectNonZero : public detail::CustomTextureExpectation {
      public:
        uint32_t DataSize() override {
            return sizeof(T);
        }

        testing::AssertionResult Check(const void* data, size_t size) override {
            ASSERT(size % DataSize() == 0 && size > 0);
            const T* actual = static_cast<const T*>(data);
            T value = *actual;
            if (value == T(0)) {
                return testing::AssertionFailure()
                       << "Expected data to be non-zero, was " << value << std::endl;
            }
            for (size_t i = 0; i < size / DataSize(); ++i) {
                if (actual[i] != value) {
                    return testing::AssertionFailure()
                           << "Expected data[" << i << "] to match non-zero value " << value
                           << ", actual " << actual[i] << std::endl;
                }
            }

            return testing::AssertionSuccess();
        }
    };

#define EXPECT_TEXTURE_NONZERO(T, ...) \
    AddTextureExpectation(__FILE__, __LINE__, new ExpectNonZero<T>(), __VA_ARGS__)

    class NonzeroTextureCreationTests : public DawnTestWithParams<Params> {
      protected:
        constexpr static uint32_t kSize = 128;

        std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
            if (GetParam().mFormat == wgpu::TextureFormat::BC1RGBAUnorm &&
                SupportsFeatures({wgpu::FeatureName::TextureCompressionBC})) {
                return {wgpu::FeatureName::TextureCompressionBC};
            }
            return {};
        }

        void Run() {
            DAWN_TEST_UNSUPPORTED_IF(GetParam().mFormat == wgpu::TextureFormat::BC1RGBAUnorm &&
                                     !SupportsFeatures({wgpu::FeatureName::TextureCompressionBC}));

            // TODO(crbug.com/dawn/667): Work around the fact that some platforms do not support
            // reading from Snorm textures.
            DAWN_TEST_UNSUPPORTED_IF(GetParam().mFormat == wgpu::TextureFormat::RGBA8Snorm &&
                                     HasToggleEnabled("disable_snorm_read"));

            // TODO(crbug.com/dawn/791): Determine Intel specific platforms this occurs on, and
            // implement a workaround on all backends (happens on Windows too, but not on our test
            // machines).
            DAWN_SUPPRESS_TEST_IF(
                (GetParam().mFormat == wgpu::TextureFormat::Depth32Float ||
                 GetParam().mFormat == wgpu::TextureFormat::Depth24PlusStencil8) &&
                IsMetal() && IsIntel() && GetParam().mMip != 0);

            // TODO(crbug.com/dawn/1071): Implement a workaround on Intel/Metal backends.
            DAWN_SUPPRESS_TEST_IF((GetParam().mFormat == wgpu::TextureFormat::R8Unorm ||
                                   GetParam().mFormat == wgpu::TextureFormat::RG8Unorm) &&
                                  GetParam().mMipCount > 1 &&
                                  HasToggleEnabled("disable_r8_rg8_mipmaps"));

            // TODO(crbug.com/dawn/667): ANGLE claims to support NV_read_stencil, but won't read
            // correctly from a DEPTH32F_STENCIL8 texture.
            DAWN_SUPPRESS_TEST_IF(GetParam().mFormat == wgpu::TextureFormat::Depth24PlusStencil8 &&
                                  GetParam().mAspect == wgpu::TextureAspect::StencilOnly &&
                                  IsANGLE());

            // TODO(crbug.com/dawn/667): Work around the fact that some platforms do not support
            // reading depth.
            DAWN_TEST_UNSUPPORTED_IF(GetParam().mAspect == wgpu::TextureAspect::DepthOnly &&
                                     HasToggleEnabled("disable_depth_read"));

            // TODO(crbug.com/dawn/667): Work around the fact that some platforms do not support
            // reading stencil.
            DAWN_TEST_UNSUPPORTED_IF(GetParam().mAspect == wgpu::TextureAspect::StencilOnly &&
                                     HasToggleEnabled("disable_stencil_read"));

            // GL may support the feature, but reading data back is not implemented.
            DAWN_TEST_UNSUPPORTED_IF(GetParam().mFormat == wgpu::TextureFormat::BC1RGBAUnorm &&
                                     (IsOpenGL() || IsOpenGLES()));

            wgpu::TextureDescriptor descriptor;
            descriptor.dimension = GetParam().mDimension;
            descriptor.size.width = kSize;
            descriptor.size.height = kSize;
            descriptor.size.depthOrArrayLayers = GetParam().mDepthOrArrayLayers;
            descriptor.sampleCount = GetParam().mSampleCount;
            descriptor.format = GetParam().mFormat;
            descriptor.usage = GetParam().mUsage;
            descriptor.mipLevelCount = GetParam().mMipCount;

            wgpu::Texture texture = device.CreateTexture(&descriptor);

            uint32_t mip = GetParam().mMip;
            uint32_t mipSize = std::max(kSize >> mip, 1u);
            uint32_t depthOrArrayLayers = GetParam().mDimension == wgpu::TextureDimension::e3D
                                              ? std::max(GetParam().mDepthOrArrayLayers >> mip, 1u)
                                              : GetParam().mDepthOrArrayLayers;
            switch (GetParam().mFormat) {
                case wgpu::TextureFormat::R8Unorm: {
                    if (GetParam().mSampleCount > 1) {
                        ExpectMultisampledFloatData(texture, mipSize, mipSize, 1,
                                                    GetParam().mSampleCount, 0, mip,
                                                    new ExpectNonZero<float>());
                    } else {
                        EXPECT_TEXTURE_EQ(new ExpectNonZero<uint8_t>(), texture, {0, 0, 0},
                                          {mipSize, mipSize, depthOrArrayLayers}, mip);
                    }
                    break;
                }
                case wgpu::TextureFormat::RG8Unorm: {
                    if (GetParam().mSampleCount > 1) {
                        ExpectMultisampledFloatData(texture, mipSize, mipSize, 2,
                                                    GetParam().mSampleCount, 0, mip,
                                                    new ExpectNonZero<float>());
                    } else {
                        EXPECT_TEXTURE_EQ(new ExpectNonZero<uint16_t>(), texture, {0, 0, 0},
                                          {mipSize, mipSize, depthOrArrayLayers}, mip);
                    }
                    break;
                }
                case wgpu::TextureFormat::RGBA8Unorm:
                case wgpu::TextureFormat::RGBA8Snorm: {
                    if (GetParam().mSampleCount > 1) {
                        ExpectMultisampledFloatData(texture, mipSize, mipSize, 4,
                                                    GetParam().mSampleCount, 0, mip,
                                                    new ExpectNonZero<float>());
                    } else {
                        EXPECT_TEXTURE_EQ(new ExpectNonZero<uint32_t>(), texture, {0, 0, 0},
                                          {mipSize, mipSize, depthOrArrayLayers}, mip);
                    }
                    break;
                }
                case wgpu::TextureFormat::Depth32Float: {
                    EXPECT_TEXTURE_EQ(new ExpectNonZero<float>(), texture, {0, 0, 0},
                                      {mipSize, mipSize, depthOrArrayLayers}, mip);
                    break;
                }
                case wgpu::TextureFormat::Depth24PlusStencil8: {
                    switch (GetParam().mAspect) {
                        case wgpu::TextureAspect::DepthOnly: {
                            for (uint32_t arrayLayer = 0;
                                 arrayLayer < GetParam().mDepthOrArrayLayers; ++arrayLayer) {
                                ExpectSampledDepthData(texture, mipSize, mipSize, arrayLayer, mip,
                                                       new ExpectNonZero<float>())
                                    << "arrayLayer " << arrayLayer;
                            }
                            break;
                        }
                        case wgpu::TextureAspect::StencilOnly: {
                            uint32_t texelCount = mipSize * mipSize * depthOrArrayLayers;
                            std::vector<uint8_t> expectedStencil(texelCount, 1);
                            EXPECT_TEXTURE_EQ(expectedStencil.data(), texture, {0, 0, 0},
                                              {mipSize, mipSize, depthOrArrayLayers}, mip,
                                              wgpu::TextureAspect::StencilOnly);

                            break;
                        }
                        default:
                            UNREACHABLE();
                    }
                    break;
                }
                case wgpu::TextureFormat::BC1RGBAUnorm: {
                    // Set buffer with dirty data so we know it is cleared by the lazy cleared
                    // texture copy
                    uint32_t blockWidth = utils::GetTextureFormatBlockWidth(GetParam().mFormat);
                    uint32_t blockHeight = utils::GetTextureFormatBlockHeight(GetParam().mFormat);
                    wgpu::Extent3D copySize = {Align(mipSize, blockWidth),
                                               Align(mipSize, blockHeight), depthOrArrayLayers};

                    uint32_t bytesPerRow =
                        utils::GetMinimumBytesPerRow(GetParam().mFormat, copySize.width);
                    uint32_t rowsPerImage = copySize.height / blockHeight;

                    uint64_t bufferSize = utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage,
                                                                     copySize, GetParam().mFormat);

                    std::vector<uint8_t> data(bufferSize, 100);
                    wgpu::Buffer bufferDst = utils::CreateBufferFromData(
                        device, data.data(), bufferSize, wgpu::BufferUsage::CopySrc);

                    wgpu::ImageCopyBuffer imageCopyBuffer =
                        utils::CreateImageCopyBuffer(bufferDst, 0, bytesPerRow, rowsPerImage);
                    wgpu::ImageCopyTexture imageCopyTexture =
                        utils::CreateImageCopyTexture(texture, mip, {0, 0, 0});

                    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                    encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
                    wgpu::CommandBuffer commands = encoder.Finish();
                    queue.Submit(1, &commands);

                    uint32_t copiedWidthInBytes =
                        utils::GetTexelBlockSizeInBytes(GetParam().mFormat) * copySize.width /
                        blockWidth;
                    uint8_t* d = data.data();
                    for (uint32_t z = 0; z < depthOrArrayLayers; ++z) {
                        for (uint32_t row = 0; row < copySize.height / blockHeight; ++row) {
                            std::fill_n(d, copiedWidthInBytes, 1);
                            d += bytesPerRow;
                        }
                    }
                    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), bufferDst, 0, bufferSize);
                    break;
                }
                default:
                    UNREACHABLE();
            }
        }
    };

    class NonzeroNonrenderableTextureCreationTests : public NonzeroTextureCreationTests {};
    class NonzeroCompressedTextureCreationTests : public NonzeroTextureCreationTests {};
    class NonzeroDepthTextureCreationTests : public NonzeroTextureCreationTests {};
    class NonzeroDepthStencilTextureCreationTests : public NonzeroTextureCreationTests {};
    class NonzeroMultisampledTextureCreationTests : public NonzeroTextureCreationTests {};

}  // anonymous namespace

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroNonrenderableTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroCompressedTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroDepthTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroDepthStencilTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroMultisampledTextureCreationTests, TextureCreationClears) {
    Run();
}

DAWN_INSTANTIATE_TEST_P(
    NonzeroTextureCreationTests,
    {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RGBA8Unorm},
    {wgpu::TextureAspect::All},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc),
     wgpu::TextureUsage::CopySrc},
    {wgpu::TextureDimension::e2D},
    {1u, 7u},          // depth or array layers
    {4u},              // mip count
    {0u, 1u, 2u, 3u},  // mip
    {1u}               // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroNonrenderableTextureCreationTests,
                        {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::RGBA8Snorm},
                        {wgpu::TextureAspect::All},
                        {wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D, wgpu::TextureDimension::e3D},
                        {1u, 7u},          // depth or array layers
                        {4u},              // mip count
                        {0u, 1u, 2u, 3u},  // mip
                        {1u}               // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroCompressedTextureCreationTests,
                        {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::BC1RGBAUnorm},
                        {wgpu::TextureAspect::All},
                        {wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D},
                        {1u, 7u},          // depth or array layers
                        {4u},              // mip count
                        {0u, 1u, 2u, 3u},  // mip
                        {1u}               // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroDepthTextureCreationTests,
                        {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::Depth32Float},
                        {wgpu::TextureAspect::DepthOnly},
                        {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment |
                                            wgpu::TextureUsage::CopySrc),
                         wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D},
                        {1u, 7u},          // depth or array layers
                        {4u},              // mip count
                        {0u, 1u, 2u, 3u},  // mip
                        {1u}               // sample count
);

DAWN_INSTANTIATE_TEST_P(
    NonzeroDepthStencilTextureCreationTests,
    {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::Depth24PlusStencil8},
    {wgpu::TextureAspect::DepthOnly, wgpu::TextureAspect::StencilOnly},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::TextureBinding),
     wgpu::TextureUsage(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc)},
    {wgpu::TextureDimension::e2D},
    {1u, 7u},          // depth or array layers
    {4u},              // mip count
    {0u, 1u, 2u, 3u},  // mip
    {1u}               // sample count
);

DAWN_INSTANTIATE_TEST_P(
    NonzeroMultisampledTextureCreationTests,
    {D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RGBA8Unorm},
    {wgpu::TextureAspect::All},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding),
     wgpu::TextureUsage::TextureBinding},
    {wgpu::TextureDimension::e2D},
    {1u},  // depth or array layers
    {1u},  // mip count
    {0u},  // mip
    {4u}   // sample count
);
