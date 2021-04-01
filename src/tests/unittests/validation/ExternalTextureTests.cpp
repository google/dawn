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

#include "tests/unittests/validation/ValidationTest.h"

namespace {
    class ExternalTextureTest : public ValidationTest {
      public:
        wgpu::TextureDescriptor CreateDefaultTextureDescriptor() {
            wgpu::TextureDescriptor descriptor;
            descriptor.size.width = kWidth;
            descriptor.size.height = kHeight;
            descriptor.size.depth = kDefaultDepth;
            descriptor.mipLevelCount = kDefaultMipLevels;
            descriptor.sampleCount = kDefaultSampleCount;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.format = kDefaultTextureFormat;
            descriptor.usage = wgpu::TextureUsage::Sampled;
            return descriptor;
        }

      protected:
        static constexpr uint32_t kWidth = 32;
        static constexpr uint32_t kHeight = 32;
        static constexpr uint32_t kDefaultDepth = 1;
        static constexpr uint32_t kDefaultMipLevels = 1;
        static constexpr uint32_t kDefaultSampleCount = 1;

        static constexpr wgpu::TextureFormat kDefaultTextureFormat =
            wgpu::TextureFormat::RGBA8Unorm;
    };

    TEST_F(ExternalTextureTest, CreateExternalTextureValidation) {
        wgpu::TextureDescriptor textureDescriptor = CreateDefaultTextureDescriptor();
        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.format = kDefaultTextureFormat;

        // Creating an external texture from a 2D, single-subresource texture should succeed.
        {
            wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = texture.CreateView();
            device.CreateExternalTexture(&externalDesc);
        }

        // Creating an external texture with a mismatched texture view format should fail.
        {
            textureDescriptor.format = wgpu::TextureFormat::RGBA8Uint;
            wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = texture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }

        // Creating an external texture from a non-2D texture should fail.
        {
            textureDescriptor.dimension = wgpu::TextureDimension::e3D;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = internalTexture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }

        // Creating an external texture from a texture with mip count > 1 should fail.
        {
            textureDescriptor.mipLevelCount = 2;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = internalTexture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }

        // Creating an external texture from a texture without TextureUsage::Sampled should fail.
        {
            textureDescriptor.mipLevelCount = 2;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = internalTexture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }

        // Creating an external texture with an unsupported format should fail.
        {
            constexpr wgpu::TextureFormat kUnsupportedFormat = wgpu::TextureFormat::R8Uint;
            textureDescriptor.format = kUnsupportedFormat;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);
            externalDesc.plane0 = internalTexture.CreateView();
            externalDesc.format = kUnsupportedFormat;
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

}  // namespace