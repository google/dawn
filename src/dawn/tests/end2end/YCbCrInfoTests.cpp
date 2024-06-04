// Copyright 2024 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/native/VulkanBackend.h"
#include "dawn/tests/DawnTest.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/hardware_buffer.h>
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn {
namespace {

constexpr uint32_t kDefaultMipLevels = 1u;
constexpr uint32_t kDefaultLayerCount = 1u;
constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::External;

wgpu::Texture Create2DTexture(wgpu::Device& device,
                              wgpu::TextureFormat format = kDefaultTextureFormat) {
#if DAWN_PLATFORM_IS(ANDROID)
    constexpr uint32_t kWidth = 32u;
    constexpr uint32_t kHeight = 32u;
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = kWidth,
        .height = kHeight,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;
    stmAHardwareBufferDesc.useExternalFormat = true;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kWidth;
    descriptor.size.height = kHeight;
    descriptor.size.depthOrArrayLayers = kDefaultLayerCount;
    descriptor.sampleCount = 1u;
    descriptor.format = kDefaultTextureFormat;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    auto texture = memory.CreateTexture(&descriptor);
    AHardwareBuffer_release(aHardwareBuffer);
    return texture;
#else
    return {};
#endif
}

wgpu::TextureViewDescriptor CreateDefaultViewDescriptor(wgpu::TextureViewDimension dimension) {
    wgpu::TextureViewDescriptor descriptor;
    descriptor.format = kDefaultTextureFormat;
    descriptor.dimension = dimension;
    descriptor.baseMipLevel = 0;
    if (dimension != wgpu::TextureViewDimension::e1D) {
        descriptor.mipLevelCount = kDefaultMipLevels;
    }
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = kDefaultLayerCount;
    return descriptor;
}

class YCbCrInfoTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip tests if platform is not Android.
        DAWN_TEST_UNSUPPORTED_IF(!DAWN_PLATFORM_IS(ANDROID));
        // Skip all tests if ycbcr sampler feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::StaticSamplers}) &&
            SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}) &&
            SupportsFeatures({wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer})) {
            requiredFeatures.push_back(wgpu::FeatureName::YCbCrVulkanSamplers);
            requiredFeatures.push_back(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer);
        }
        return requiredFeatures;
    }
};

// Test that it is possible to create the sampler with ycbcr vulkan descriptor.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWhenFeatureEnabled) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr vulkan descriptor with only vulkan
// format set.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWithOnlyVkFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    // format is set as VK_FORMAT.
    yCbCrDesc.externalFormat = 0;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr vulkan descriptor with only external
// format set.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWithOnlyExternalFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    // format is set as externalFormat.
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is NOT possible to create the sampler with ycbcr vulkan descriptor and no format
// set.
TEST_P(YCbCrInfoTest, YCbCrSamplerInvalidWithNoFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 0;
    samplerDesc.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

// Test that it is invalid to create texture view with formats other than External.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithoutWgpuFormatExternal) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    // Pass RGBA8Unorm instead of External format.
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    descriptor.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithWgpuFormatExternal) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor with only vulkan
// format set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithOnlyVkFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    // format is set as VK_FORMAT.
    yCbCrDesc.externalFormat = 0;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor with only external
// format set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithOnlyExternalFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    // format is set as externalFormat.
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is NOT possible to create texture view with ycbcr vulkan descriptor and no format
// set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithNoFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 0;
    descriptor.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

// Test that it is NOT possible to create texture view from texture created with
// TextureFormat::External but NO ycbcr vulkan descriptor passed.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithNoYCbCrDescriptor) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

DAWN_INSTANTIATE_TEST(YCbCrInfoTest, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
