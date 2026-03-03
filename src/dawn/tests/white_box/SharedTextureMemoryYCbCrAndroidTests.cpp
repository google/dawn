// Copyright 2026 The Dawn & Tint Authors
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

#include <android/hardware_buffer.h>
#include <webgpu/webgpu_cpp.h>

#include <utility>
#include <vector>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class SharedTextureMemoryYCbCrVulkanSamplersTests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceSyncFD, wgpu::FeatureName::YCbCrVulkanSamplers};
    }
};

// Test validation of an incorrectly-configured SharedTextureMemoryAHardwareBufferProperties
// instance.
TEST_P(SharedTextureMemoryYCbCrVulkanSamplersTests,
       InvalidSharedTextureMemoryAHardwareBufferProperties) {
    // TODO(crbug.com/40238674): Fails on Pixel 10 vulkan.
    DAWN_SUPPRESS_TEST_IF(IsImgTec() && IsVulkan());
    // TODO(crbug.com/444741058): Fails on Intel-based brya devices running Android Desktop.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsIntel() && IsAndroid());

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc;

    // Chaining anything onto the passed-in YCbCrVkDescriptor is invalid.
    yCbCrDesc.nextInChain = &stmAHardwareBufferDesc;
    ahbProperties.yCbCrInfo = yCbCrDesc;
    properties.nextInChain = &ahbProperties;

    ASSERT_DEVICE_ERROR(memory.GetProperties(&properties));
}

// Test querying YCbCr info from the Device.
TEST_P(SharedTextureMemoryYCbCrVulkanSamplersTests, QueryYCbCrInfoFromDevice) {
    // TODO(crbug.com/40238674): Fails on Pixel 10 vulkan.
    DAWN_SUPPRESS_TEST_IF(IsImgTec() && IsVulkan());

    // TODO(crbug.com/444741058): Fails on Intel-based brya devices running Android Desktop.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsIntel() && IsAndroid());

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of this AHB via the Device.
    wgpu::AHardwareBufferProperties ahbProperties;
    device.GetAHardwareBufferProperties(aHardwareBuffer, &ahbProperties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    EXPECT_EQ(bufferFormatProperties.format, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    EXPECT_EQ(bufferFormatProperties.externalFormat, yCbCrInfo.externalFormat);
}

// Test querying YCbCr info from the SharedTextureMemory without external format.
TEST_P(SharedTextureMemoryYCbCrVulkanSamplersTests, QueryYCbCrInfoWithoutYCbCrFormat) {
    // TODO(crbug.com/40238674): Fails on Pixel 10 vulkan.
    DAWN_SUPPRESS_TEST_IF(IsImgTec() && IsVulkan());

    // TODO(crbug.com/444741058): Fails on Intel-based brya devices running Android Desktop.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsIntel() && IsAndroid());

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of a SharedTextureMemory created from this
    // AHB.
    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    properties.nextInChain = &ahbProperties;
    memory.GetProperties(&properties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    EXPECT_EQ(bufferFormatProperties.format, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    uint64_t expectedExternalFormat = 0u;
    EXPECT_EQ(expectedExternalFormat, yCbCrInfo.externalFormat);
}

// Test querying YCbCr info from the SharedTextureMemory with external format.
TEST_P(SharedTextureMemoryYCbCrVulkanSamplersTests, QueryYCbCrInfoWithExternalFormat) {
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of a SharedTextureMemory created from this
    // AHB.
    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    properties.nextInChain = &ahbProperties;
    memory.GetProperties(&properties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    VkFormat expectedVkFormat = VK_FORMAT_UNDEFINED;
    EXPECT_EQ(expectedVkFormat, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    EXPECT_EQ(bufferFormatProperties.externalFormat, yCbCrInfo.externalFormat);
}

// Test BeginAccess on an uninitialized texture with external format fails.
TEST_P(SharedTextureMemoryYCbCrVulkanSamplersTests,
       GPUReadForUninitializedTextureWithExternalFormatFails) {
    const AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    const wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.size.depthOrArrayLayers = 1u;
    descriptor.sampleCount = 1u;
    descriptor.format = wgpu::TextureFormat::OpaqueYCbCrAndroid;
    descriptor.mipLevelCount = 1u;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    auto texture = memory.CreateTexture(&descriptor);
    AHardwareBuffer_release(aHardwareBuffer);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = false;
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    beginDesc.nextInChain = &beginLayout;

    ASSERT_DEVICE_ERROR(memory.BeginAccess(texture, &beginDesc));
}

DAWN_INSTANTIATE_TEST(SharedTextureMemoryYCbCrVulkanSamplersTests, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
