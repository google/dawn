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

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class NonzeroTextureCreationTests : public DawnTest {
  protected:
    constexpr static uint32_t kSize = 128;
    constexpr static uint32_t kDepthOrArrayLayers = 7;
};

// Test that texture clears 0xFF because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, TextureCreationClears) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kSize;
    descriptor.size.height = kSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    // 2D
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        std::vector<RGBA8> expected(kSize * kSize, RGBA8(255, 255, 255, 255));
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0}, {kSize, kSize});
    }

    // 2D Array
    {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        std::vector<RGBA8> expected(kSize * kSize * kDepthOrArrayLayers, RGBA8(255, 255, 255, 255));
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, 0}, {kSize, kSize, kDepthOrArrayLayers});
    }
}

// Test that 3D texture clears to nonzero because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, Texture3DCreationClears) {
    // TODO(crbug.com/dawn/547): 3D texture copies not fully implemented on D3D12.
    // TODO(crbug.com/angleproject/5967): This texture readback hits an assert in ANGLE.
    DAWN_SKIP_TEST_IF(IsANGLE() || IsD3D12());

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size.width = kSize;
    descriptor.size.height = kSize;
    descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    std::vector<RGBA8> expected(kSize * kSize * kDepthOrArrayLayers, RGBA8(255, 255, 255, 255));
    EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, 0}, {kSize, kSize, kDepthOrArrayLayers});
}

// Test that a depth texture clears 0xFF because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, Depth32TextureCreationDepthClears) {
    // Copies from depth textures not fully supported on the OpenGL backend right now.
    DAWN_SKIP_TEST_IF(IsOpenGL() || IsOpenGLES());

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kSize;
    descriptor.size.height = kSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    descriptor.format = wgpu::TextureFormat::Depth32Float;

    // We can only really test Depth32Float here because Depth24Plus(Stencil8)? may be in an unknown
    // format.
    // TODO(crbug.com/dawn/145): Test other formats via sampling.

    // 2D
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::vector<float> expected(kSize * kSize, 1.f);
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0}, {kSize, kSize});
    }

    // 2D Array
    {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::vector<float> expected(kSize * kSize * kDepthOrArrayLayers, 1.f);
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, 0}, {kSize, kSize, kDepthOrArrayLayers});
    }
}

// Test that non-zero mip level clears 0xFF because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, MipMapClears) {
    constexpr uint32_t mipLevels = 4;

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kSize;
    descriptor.size.height = kSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = mipLevels;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    // 2D
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        uint32_t mipSize = kSize >> 2;
        std::vector<RGBA8> expected(mipSize * mipSize, RGBA8(255, 255, 255, 255));
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0}, {mipSize, mipSize}, 2);
    }

    // 2D Array
    {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        uint32_t mipSize = kSize >> 2;
        std::vector<RGBA8> expected(mipSize * mipSize * kDepthOrArrayLayers,
                                    RGBA8(255, 255, 255, 255));
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, 0},
                          {mipSize, mipSize, kDepthOrArrayLayers}, 2);
    }

    // 3D
    {
        descriptor.dimension = wgpu::TextureDimension::e3D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        uint32_t mipSize = kSize >> 2;
        uint32_t mipDepth = kDepthOrArrayLayers >> 2;
        std::vector<RGBA8> expected(mipSize * mipSize * mipDepth, RGBA8(255, 255, 255, 255));
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, 0}, {mipSize, mipSize, mipDepth}, 2);
    }
}

// Test that nonrenderable texture formats clear 0x01 because toggle is enabled
TEST_P(NonzeroTextureCreationTests, NonrenderableTextureFormat) {
    // TODO(crbug.com/dawn/667): Work around the fact that some platforms do not support reading
    // from Snorm textures.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("disable_snorm_read"));

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kSize;
    descriptor.size.height = kSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Snorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::CopySrc;

    // 2D
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Set buffer with dirty data so we know it is cleared by the lazy cleared texture copy
        uint32_t bufferSize = kSize * kSize;
        std::vector<uint8_t> data(sizeof(uint32_t) * bufferSize, 100);
        wgpu::Buffer bufferDst = utils::CreateBufferFromData(
            device, data.data(), static_cast<uint32_t>(data.size()), wgpu::BufferUsage::CopySrc);

        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(bufferDst, 0, kSize * 4);
        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {kSize, kSize, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expectedBytes = IsVulkan() ? 0x7F7F7F7F : 0x01010101;
        std::vector<uint32_t> expected(bufferSize, expectedBytes);
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), bufferDst, 0, expected.size());
    }

    // 2D Array
    {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Set buffer with dirty data so we know it is cleared by the lazy cleared texture copy
        uint32_t bufferSize = kSize * kSize * kDepthOrArrayLayers;
        std::vector<uint8_t> data(sizeof(uint32_t) * bufferSize, 100);
        wgpu::Buffer bufferDst = utils::CreateBufferFromData(
            device, data.data(), static_cast<uint32_t>(data.size()), wgpu::BufferUsage::CopySrc);

        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(bufferDst, 0, kSize * 4, kSize);
        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {kSize, kSize, kDepthOrArrayLayers};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expectedBytes = IsVulkan() ? 0x7F7F7F7F : 0x01010101;
        std::vector<uint32_t> expected(bufferSize, expectedBytes);
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), bufferDst, 0, expected.size());
    }

    // 3D
    {
        descriptor.dimension = wgpu::TextureDimension::e3D;
        descriptor.size.depthOrArrayLayers = kDepthOrArrayLayers;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Set buffer with dirty data so we know it is cleared by the lazy cleared texture copy
        uint32_t bufferSize = kSize * kSize * kDepthOrArrayLayers;
        std::vector<uint8_t> data(sizeof(uint32_t) * bufferSize, 100);
        wgpu::Buffer bufferDst = utils::CreateBufferFromData(
            device, data.data(), static_cast<uint32_t>(data.size()), wgpu::BufferUsage::CopySrc);

        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(bufferDst, 0, kSize * 4, kSize);
        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {kSize, kSize, kDepthOrArrayLayers};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expectedBytes = IsVulkan() ? 0x7F7F7F7F : 0x01010101;
        std::vector<uint32_t> expected(bufferSize, expectedBytes);
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), bufferDst, 0, expected.size());
    }
}

DAWN_INSTANTIATE_TEST(NonzeroTextureCreationTests,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}));
