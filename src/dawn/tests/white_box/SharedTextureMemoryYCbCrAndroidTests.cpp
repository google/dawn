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

#include <span>
#include <utility>
#include <vector>

#include "dawn/common/Algebra.h"
#include "dawn/common/Assert.h"
#include "dawn/common/ColorSpace.h"
#include "dawn/common/Range.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "vulkan/vulkan_core.h"

namespace dawn {
namespace {

// Make an AHB with the YCbCr data, note that Cb and Cr are a quarter the size of Y.
AHardwareBuffer* MakeY8Cb8Cr8AHB(uint32_t width,
                                 uint32_t height,
                                 std::span<const uint8_t> yData,
                                 std::span<const uint8_t> cbData,
                                 std::span<const uint8_t> crData) {
    DAWN_ASSERT(width % 2 == 0 && height % 2 == 0);

    AHardwareBuffer_Desc ahbDesc = {
        .width = width,
        .height = height,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420,
        .usage = AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY | AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                 AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };

    AHardwareBuffer* ahb = nullptr;
    EXPECT_EQ(AHardwareBuffer_allocate(&ahbDesc, &ahb), 0);

    AHardwareBuffer_Planes ahbPlanes;
    EXPECT_EQ(AHardwareBuffer_lockPlanes(ahb, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY, -1, nullptr,
                                         &ahbPlanes),
              0);

    auto CopyPlane = [](std::span<const uint8_t> src, uint32_t srcWidth, uint32_t srcHeight,
                        const AHardwareBuffer_Plane& dst) {
        uint8_t* dstData = static_cast<uint8_t*>(dst.data);
        for (uint32_t x : Range(srcWidth)) {
            for (uint32_t y : Range(srcHeight)) {
                dstData[y * dst.rowStride + x * dst.pixelStride] = src[y * srcWidth + x];
            }
        }
    };

    CopyPlane(yData, width, height, ahbPlanes.planes[0]);
    CopyPlane(cbData, width / 2, height / 2, ahbPlanes.planes[1]);
    CopyPlane(crData, width / 2, height / 2, ahbPlanes.planes[2]);

    EXPECT_EQ(AHardwareBuffer_unlock(ahb, nullptr), 0);

    return ahb;
}

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

// Define a test parameter struct to check all combinations of Vulkan YCbCr sampler models. Use enum
// class so that the ostream overloads match, otherwise testnames just contain the value of the
// Vulkan enum and not its name.
enum class VkYCbCrModel { YCbCrIdentity, RGBIdentity, Rec601, Rec709, Rec2020 };
std::ostream& operator<<(std::ostream& o, VkYCbCrModel model) {
    switch (model) {
        case VkYCbCrModel::YCbCrIdentity:
            o << "ycbcrIdentity";
            break;
        case VkYCbCrModel::RGBIdentity:
            o << "rgbIdentity";
            break;
        case VkYCbCrModel::Rec601:
            o << "601";
            break;
        case VkYCbCrModel::Rec709:
            o << "709";
            break;
        case VkYCbCrModel::Rec2020:
            o << "2020";
            break;
    }
    return o;
}
VkSamplerYcbcrModelConversion ToVk(VkYCbCrModel model) {
    switch (model) {
        case VkYCbCrModel::RGBIdentity:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
        case VkYCbCrModel::YCbCrIdentity:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY;
        case VkYCbCrModel::Rec601:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
        case VkYCbCrModel::Rec709:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
        case VkYCbCrModel::Rec2020:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
    }
}
enum class VkYCbCrRange { Full, Narrow };
std::ostream& operator<<(std::ostream& o, VkYCbCrRange range) {
    switch (range) {
        case VkYCbCrRange::Full:
            o << "full";
            break;
        case VkYCbCrRange::Narrow:
            o << "narrow";
            break;
    }
    return o;
}
VkSamplerYcbcrRange ToVk(VkYCbCrRange range) {
    switch (range) {
        case VkYCbCrRange::Full:
            return VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
        case VkYCbCrRange::Narrow:
            return VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    }
}
DAWN_TEST_PARAM_STRUCT(VkYCbCrParams, VkYCbCrModel, VkYCbCrRange);

class SharedTextureMemoryVulkanYCbCrParamsTests : public DawnTestWithParams<VkYCbCrParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceSyncFD, wgpu::FeatureName::StaticSamplers,
                wgpu::FeatureName::YCbCrVulkanSamplers};
    }

    math::Mat4x3f ComputeYCbCrToRGB() {
        // Vulkan has CrYCb (because Cr is "red", Y is "green" and Cb is "blue") but we want YCbCr.
        // Stay in 4D to undo the swizzle as it is before the application of the range, but have the
        // redo in 3D because it is after the range is applied.
        constexpr math::Mat4x4f kUndoVulkanSwizzle = {
            {0, 0, 1, 0},
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 0, 1},
        };
        constexpr math::Mat3x3f kRedoVulkanSwizzle = {
            {0, 1, 0},
            {0, 0, 1},
            {1, 0, 0},
        };

        math::Mat4x3f rangeTransform;
        switch (GetParam().mVkYCbCrRange) {
            case VkYCbCrRange::Full:
                rangeTransform = kYCbCrRange_Full;
                break;
            case VkYCbCrRange::Narrow:
                rangeTransform = kYCbCrRange_Narrow;
                break;
        }

        math::Mat3x3f ycbcrToRgb;
        switch (GetParam().mVkYCbCrModel) {
            case VkYCbCrModel::RGBIdentity:
                // Directly return without multiplying with the range transform as that's support to
                // be what Vulkan does with RGB_IDENTITY. Sampled YCbCr values are returned raw.
                return math::Mat4x3f::CropOrExpandFrom(math::Mat3x3f::Identity());

            case VkYCbCrModel::YCbCrIdentity:
                // Redo the swizzle since no YCbCr to RGB conversion happens and Vulkan will have
                // CrYCb.
                ycbcrToRgb = kRedoVulkanSwizzle;
                break;
            case VkYCbCrModel::Rec601:
                ycbcrToRgb = kYCbCrToRGB_Rec601;
                break;
            case VkYCbCrModel::Rec709:
                ycbcrToRgb = kYCbCrToRGB_Rec709;
                break;
            case VkYCbCrModel::Rec2020:
                ycbcrToRgb = kYCbCrToRGB_Rec2020;
                break;
        }

        return Mul(ycbcrToRgb, Mul(rangeTransform, kUndoVulkanSwizzle));
    }
};

TEST_P(SharedTextureMemoryVulkanYCbCrParamsTests, SampleY8Cb8Cr8AHB) {
    // Make the AHB and import it as an STM and wgpu::Texture.
    const std::array<uint8_t, 4> yData = {50, 100, 150, 200};
    const std::array<uint8_t, 1> cbData = {130};
    const std::array<uint8_t, 1> crData = {140};
    AHardwareBuffer* ahb = MakeY8Cb8Cr8AHB(2, 2, yData, cbData, crData);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc{};
    stmAHardwareBufferDesc.handle = ahb;
    wgpu::SharedTextureMemoryDescriptor stmDesc{};
    stmDesc.nextInChain = &stmAHardwareBufferDesc;
    wgpu::SharedTextureMemory stm = device.ImportSharedTextureMemory(&stmDesc);
    wgpu::Texture ycbcrTex = stm.CreateTexture();

    AHardwareBuffer_release(ahb);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc{};
    beginDesc.initialized = true;
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    beginDesc.nextInChain = &beginLayout;
    EXPECT_EQ(stm.BeginAccess(ycbcrTex, &beginDesc), wgpu::Status::Success);

    // Get the YCbCrVkDescriptor
    wgpu::SharedTextureMemoryProperties stmProperties{};
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties{};
    stmProperties.nextInChain = &ahbProperties;
    stm.GetProperties(&stmProperties);
    wgpu::YCbCrVkDescriptor yCbCrDesc = ahbProperties.yCbCrInfo;

    // Override the YCbCr descriptor to force it to use the YCbCr model and range in test params.
    yCbCrDesc.vkYCbCrModel = ToVk(GetParam().mVkYCbCrModel);
    yCbCrDesc.vkYCbCrRange = ToVk(GetParam().mVkYCbCrRange);
    yCbCrDesc.vkComponentSwizzleRed = VK_COMPONENT_SWIZZLE_R;
    yCbCrDesc.vkComponentSwizzleGreen = VK_COMPONENT_SWIZZLE_G;
    yCbCrDesc.vkComponentSwizzleBlue = VK_COMPONENT_SWIZZLE_B;
    yCbCrDesc.vkComponentSwizzleAlpha = VK_COMPONENT_SWIZZLE_A;
    yCbCrDesc.vkChromaFilter = wgpu::FilterMode::Nearest;

    // Create the BindGroupLayout with a static YCbCr sampler, and the BindGroup.
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.nextInChain = &yCbCrDesc;

    wgpu::StaticSamplerBindingLayout staticSamplerBinding{};
    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    staticSamplerBinding.sampledTextureBinding = 1;

    std::array<wgpu::BindGroupLayoutEntry, 2> bglEntries;
    bglEntries[0].binding = 0;
    bglEntries[0].visibility = wgpu::ShaderStage::Fragment;
    bglEntries[0].nextInChain = &staticSamplerBinding;
    bglEntries[1].binding = 1;
    bglEntries[1].visibility = wgpu::ShaderStage::Fragment;
    bglEntries[1].texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;

    wgpu::BindGroupLayoutDescriptor bglDesc{
        .entryCount = bglEntries.size(),
        .entries = bglEntries.data(),
    };
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.nextInChain = &yCbCrDesc;
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{1, ycbcrTex.CreateView(&viewDesc)}});

    // Create the pipeline that copies from the YCbCr texture to an RGBA one.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn quad(@builtin(vertex_index) i : u32) -> @builtin(position) vec4f {
            const pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[i], 0.0, 1.0);
        }

        @group(0) @binding(0) var s : sampler;
        @group(0) @binding(1) var t : texture_2d<f32>;
        @fragment fn fs(@builtin(position) pos : vec4f) -> @location(0) vec4f {
            return textureSample(t, s, pos.xy / 2);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.layout = utils::MakePipelineLayout(device, {bgl});
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Do the copy.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, 2, 2, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bg);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Replicate the computation that should be done by the YCbCr vulkan sampler and check that the
    // copied data is within a small tolerance.
    math::Mat4x3f ycbcrToRgb = ComputeYCbCrToRGB();
    auto CheckPixel = [&](uint8_t y, uint8_t cb, uint8_t cr, uint32_t pixelX, uint32_t pixelY) {
        auto ycbcr = math::Vec4f(cr / 255.0, y / 255.0, cb / 255.0, 1.0);
        auto rgb = math::Mul(ycbcrToRgb, ycbcr);

        auto expected = utils::RGBA8(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255, 255);
        auto bottom = utils::RGBA8(expected.r - 1, expected.g - 1, expected.b - 1, 255);
        auto top = utils::RGBA8(expected.r + 1, expected.g + 1, expected.b + 1, 255);

        EXPECT_PIXEL_RGBA8_BETWEEN(bottom, top, renderPass.color, pixelX, pixelY);
    };

    CheckPixel(yData[0], cbData[0], crData[0], 0, 0);
    CheckPixel(yData[1], cbData[0], crData[0], 1, 0);
    CheckPixel(yData[2], cbData[0], crData[0], 0, 1);
    CheckPixel(yData[3], cbData[0], crData[0], 1, 1);
}

DAWN_INSTANTIATE_TEST_P(SharedTextureMemoryVulkanYCbCrParamsTests,
                        {VulkanBackend()},
                        {
                            VkYCbCrModel::RGBIdentity,
                            VkYCbCrModel::YCbCrIdentity,
                            VkYCbCrModel::Rec601,
                            VkYCbCrModel::Rec709,
                            VkYCbCrModel::Rec2020,
                        },
                        {VkYCbCrRange::Full, VkYCbCrRange::Narrow});

}  // anonymous namespace
}  // namespace dawn
