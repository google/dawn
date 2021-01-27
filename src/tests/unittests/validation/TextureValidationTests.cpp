// Copyright 2018 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

namespace {

    class TextureValidationTest : public ValidationTest {
      protected:
        void SetUp() override {
            ValidationTest::SetUp();

            queue = device.GetDefaultQueue();
        }

        wgpu::TextureDescriptor CreateDefaultTextureDescriptor() {
            wgpu::TextureDescriptor descriptor;
            descriptor.size.width = kWidth;
            descriptor.size.height = kHeight;
            descriptor.size.depth = kDefaultDepth;
            descriptor.mipLevelCount = kDefaultMipLevels;
            descriptor.sampleCount = kDefaultSampleCount;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.format = kDefaultTextureFormat;
            descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::Sampled;
            return descriptor;
        }

        wgpu::Queue queue;

      private:
        static constexpr uint32_t kWidth = 32;
        static constexpr uint32_t kHeight = 32;
        static constexpr uint32_t kDefaultDepth = 1;
        static constexpr uint32_t kDefaultMipLevels = 1;
        static constexpr uint32_t kDefaultSampleCount = 1;

        static constexpr wgpu::TextureFormat kDefaultTextureFormat =
            wgpu::TextureFormat::RGBA8Unorm;
    };

    // Test the validation of sample count
    TEST_F(TextureValidationTest, SampleCount) {
        wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

        // sampleCount == 1 is allowed.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 1;

            device.CreateTexture(&descriptor);
        }

        // sampleCount == 4 is allowed.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 4;

            device.CreateTexture(&descriptor);
        }

        // It is an error to create a texture with an invalid sampleCount.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 3;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // It is an error to create a multisampled texture with mipLevelCount > 1.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 4;
            descriptor.mipLevelCount = 2;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Currently we do not support multisampled 2D array textures.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 4;
            descriptor.size.depth = 2;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // It is an error to set TextureUsage::Storage when sampleCount > 1.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.sampleCount = 4;
            descriptor.usage |= wgpu::TextureUsage::Storage;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }

    // Test the validation of the mip level count
    TEST_F(TextureValidationTest, MipLevelCount) {
        wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

        // mipLevelCount == 1 is allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 32;
            descriptor.size.height = 32;
            descriptor.mipLevelCount = 1;

            device.CreateTexture(&descriptor);
        }

        // mipLevelCount == 0 is an error
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 32;
            descriptor.size.height = 32;
            descriptor.mipLevelCount = 0;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Full mip chains are allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 32;
            descriptor.size.height = 32;
            // Mip level sizes: 32, 16, 8, 4, 2, 1
            descriptor.mipLevelCount = 6;

            device.CreateTexture(&descriptor);
        }

        // Test non-power-of-two width
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            // Mip level width: 31, 15, 7, 3, 1
            descriptor.size.width = 31;
            descriptor.size.height = 4;

            // Full mip chains on non-power-of-two width are allowed
            descriptor.mipLevelCount = 5;
            device.CreateTexture(&descriptor);

            // Too big mip chains on non-power-of-two width are disallowed
            descriptor.mipLevelCount = 6;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Test non-power-of-two height
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 4;
            // Mip level height: 31, 15, 7, 3, 1
            descriptor.size.height = 31;

            // Full mip chains on non-power-of-two height are allowed
            descriptor.mipLevelCount = 5;
            device.CreateTexture(&descriptor);

            // Too big mip chains on non-power-of-two height are disallowed
            descriptor.mipLevelCount = 6;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Undefined shift check if miplevel is bigger than the integer bit width.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 32;
            descriptor.size.height = 32;
            descriptor.mipLevelCount = 100;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Non square mip map halves the resolution until a 1x1 dimension.
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 32;
            descriptor.size.height = 8;
            // Mip maps: 32 * 8, 16 * 4, 8 * 2, 4 * 1, 2 * 1, 1 * 1
            descriptor.mipLevelCount = 6;

            device.CreateTexture(&descriptor);
        }

        // Mip level exceeding kMaxTexture2DMipLevels not allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = 1 >> kMaxTexture2DMipLevels;
            descriptor.size.height = 1 >> kMaxTexture2DMipLevels;
            descriptor.mipLevelCount = kMaxTexture2DMipLevels + 1u;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }
    // Test the validation of array layer count
    TEST_F(TextureValidationTest, ArrayLayerCount) {
        wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

        // Array layer count exceeding kMaxTextureArrayLayers is not allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.depth = kMaxTextureArrayLayers + 1u;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Array layer count less than kMaxTextureArrayLayers is allowed;
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.depth = kMaxTextureArrayLayers >> 1;

            device.CreateTexture(&descriptor);
        }

        // Array layer count equal to kMaxTextureArrayLayers is allowed;
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.depth = kMaxTextureArrayLayers;

            device.CreateTexture(&descriptor);
        }
    }

    // Test the validation of texture size
    TEST_F(TextureValidationTest, TextureSize) {
        wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

        // Texture size exceeding kMaxTextureDimension2D is not allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = kMaxTextureDimension2D + 1u;
            descriptor.size.height = kMaxTextureDimension2D + 1u;

            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Texture size less than kMaxTextureDimension2D is allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = kMaxTextureDimension2D >> 1;
            descriptor.size.height = kMaxTextureDimension2D >> 1;

            device.CreateTexture(&descriptor);
        }

        // Texture equal to kMaxTextureDimension2D is allowed
        {
            wgpu::TextureDescriptor descriptor = defaultDescriptor;
            descriptor.size.width = kMaxTextureDimension2D;
            descriptor.size.height = kMaxTextureDimension2D;

            device.CreateTexture(&descriptor);
        }
    }

    // Test that it is valid to destroy a texture
    TEST_F(TextureValidationTest, DestroyTexture) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        texture.Destroy();
    }

    // Test that it's valid to destroy a destroyed texture
    TEST_F(TextureValidationTest, DestroyDestroyedTexture) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        texture.Destroy();
        texture.Destroy();
    }

    // Test that it's invalid to submit a destroyed texture in a queue
    // in the case of destroy, encode, submit
    TEST_F(TextureValidationTest, DestroyEncodeSubmit) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        wgpu::TextureView textureView = texture.CreateView();

        utils::ComboRenderPassDescriptor renderPass({textureView});

        // Destroy the texture
        texture.Destroy();

        wgpu::CommandEncoder encoder_post_destroy = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder_post_destroy.BeginRenderPass(&renderPass);
            pass.EndPass();
        }
        wgpu::CommandBuffer commands = encoder_post_destroy.Finish();

        // Submit should fail due to destroyed texture
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }

    // Test that it's invalid to submit a destroyed texture in a queue
    // in the case of encode, destroy, submit
    TEST_F(TextureValidationTest, EncodeDestroySubmit) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        wgpu::TextureView textureView = texture.CreateView();

        utils::ComboRenderPassDescriptor renderPass({textureView});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.EndPass();
        }
        wgpu::CommandBuffer commands = encoder.Finish();

        // Destroy the texture
        texture.Destroy();

        // Submit should fail due to destroyed texture
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }

    // Test it is an error to create an RenderAttachment texture with a non-renderable format.
    TEST_F(TextureValidationTest, NonRenderableAndRenderAttachment) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {1, 1, 1};
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;

        // Succeeds because RGBA8Unorm is renderable
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        device.CreateTexture(&descriptor);

        wgpu::TextureFormat nonRenderableFormats[] = {
            wgpu::TextureFormat::RG11B10Ufloat,
            wgpu::TextureFormat::R8Snorm,
            wgpu::TextureFormat::RG8Snorm,
            wgpu::TextureFormat::RGBA8Snorm,
        };

        for (wgpu::TextureFormat format : nonRenderableFormats) {
            // Fails because `format` is non-renderable
            descriptor.format = format;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }

    // Test it is an error to create a Storage texture with any format that doesn't support
    // TextureUsage::Storage texture usages.
    TEST_F(TextureValidationTest, TextureFormatNotSupportTextureUsageStorage) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {1, 1, 1};
        descriptor.usage = wgpu::TextureUsage::Storage;

        for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
            descriptor.format = format;
            if (utils::TextureFormatSupportsStorageTexture(format)) {
                device.CreateTexture(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
            }
        }
    }

    // Test it is an error to create a texture with format "Undefined".
    TEST_F(TextureValidationTest, TextureFormatUndefined) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = wgpu::TextureFormat::Undefined;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Test that the creation of a texture with BC format will fail when the extension
    // textureCompressionBC is not enabled.
    TEST_F(TextureValidationTest, UseBCFormatWithoutEnablingExtension) {
        for (wgpu::TextureFormat format : utils::kBCFormats) {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }

    // TODO(jiawei.shao@intel.com): add tests to verify we cannot create 1D or 3D textures with
    // compressed texture formats.
    class CompressedTextureFormatsValidationTests : public TextureValidationTest {
      protected:
        WGPUDevice CreateTestDevice() override {
            dawn_native::DeviceDescriptor descriptor;
            descriptor.requiredExtensions = {"texture_compression_bc"};
            return adapter.CreateDevice(&descriptor);
        }

        wgpu::TextureDescriptor CreateDefaultTextureDescriptor() {
            wgpu::TextureDescriptor descriptor =
                TextureValidationTest::CreateDefaultTextureDescriptor();
            descriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                               wgpu::TextureUsage::Sampled;
            return descriptor;
        }
    };

    // Test the validation of texture size when creating textures in compressed texture formats.
    TEST_F(CompressedTextureFormatsValidationTests, TextureSize) {
        // Test that it is invalid to use a number that is not a multiple of 4 (the compressed block
        // width and height of all BC formats) as the width or height of textures in BC formats.
        for (wgpu::TextureFormat format : utils::kBCFormats) {
            {
                wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
                descriptor.format = format;
                ASSERT_TRUE(descriptor.size.width % 4 == 0 && descriptor.size.height % 4 == 0);
                device.CreateTexture(&descriptor);
            }

            {
                wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
                descriptor.format = format;
                descriptor.size.width = 31;
                ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
            }

            {
                wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
                descriptor.format = format;
                descriptor.size.height = 31;
                ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
            }

            {
                wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
                descriptor.format = format;
                descriptor.size.width = 12;
                descriptor.size.height = 32;
                device.CreateTexture(&descriptor);
            }
        }
    }

    // Test the validation of texture usages when creating textures in compressed texture formats.
    TEST_F(CompressedTextureFormatsValidationTests, TextureUsage) {
        // Test that only CopySrc, CopyDst and Sampled are accepted as the texture usage of the
        // textures in BC formats.
        wgpu::TextureUsage invalidUsages[] = {
            wgpu::TextureUsage::RenderAttachment,
            wgpu::TextureUsage::Storage,
            wgpu::TextureUsage::Present,
        };
        for (wgpu::TextureFormat format : utils::kBCFormats) {
            for (wgpu::TextureUsage usage : invalidUsages) {
                wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
                descriptor.format = format;
                descriptor.usage = usage;
                ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
            }
        }
    }

    // Test the validation of sample count when creating textures in compressed texture formats.
    TEST_F(CompressedTextureFormatsValidationTests, SampleCount) {
        // Test that it is invalid to specify SampleCount > 1 when we create a texture in BC
        // formats.
        for (wgpu::TextureFormat format : utils::kBCFormats) {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.sampleCount = 4;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }

    // Test the validation of creating 2D array textures in compressed texture formats.
    TEST_F(CompressedTextureFormatsValidationTests, 2DArrayTexture) {
        // Test that it is allowed to create a 2D array texture in BC formats.
        for (wgpu::TextureFormat format : utils::kBCFormats) {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.size.depth = 6;
            device.CreateTexture(&descriptor);
        }
    }

}  // namespace
