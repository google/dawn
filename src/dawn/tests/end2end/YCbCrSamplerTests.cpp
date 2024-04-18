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

class YCbCrSamplerTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if ycbcr sampler feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::StaticSamplers}) &&
            SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers})) {
            requiredFeatures.push_back(wgpu::FeatureName::YCbCrVulkanSamplers);
        }
        return requiredFeatures;
    }
};

// Test that it is possible to create the sampler with ycbcr sampler descriptor.
TEST_P(YCbCrSamplerTest, YCbCrSamplerValidWhenFeatureEnabled) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::SamplerYCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerYCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = nullptr;
    samplerYCbCrDesc.vulkanYCbCrInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

    samplerDesc.nextInChain = &samplerYCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr sampler descriptor with only vulkan
// format set.
TEST_P(YCbCrSamplerTest, YCbCrSamplerValidWithOnlyVkFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::SamplerYCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerYCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = nullptr;
    samplerYCbCrDesc.vulkanYCbCrInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

#if DAWN_PLATFORM_IS(ANDROID)
    VkExternalFormatANDROID vulkanExternalFormat = {};
    vulkanExternalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    vulkanExternalFormat.pNext = nullptr;
    // format is set as VK_FORMAT.
    vulkanExternalFormat.externalFormat = 0;

    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = &vulkanExternalFormat;
#endif  // DAWN_PLATFORM_IS(ANDROID)

    samplerDesc.nextInChain = &samplerYCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr sampler descriptor with only external
// format set.
TEST_P(YCbCrSamplerTest, YCbCrSamplerValidWithOnlyExternalFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::SamplerYCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerYCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = nullptr;
    // format is set as externalFormat.
    samplerYCbCrDesc.vulkanYCbCrInfo.format = VK_FORMAT_UNDEFINED;

#if DAWN_PLATFORM_IS(ANDROID)
    VkExternalFormatANDROID vulkanExternalFormat = {};
    vulkanExternalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    vulkanExternalFormat.pNext = nullptr;
    vulkanExternalFormat.externalFormat = 5;

    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = &vulkanExternalFormat;
#endif  // DAWN_PLATFORM_IS(ANDROID)

    samplerDesc.nextInChain = &samplerYCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is not possible to create the sampler with ycbcr sampler descriptor and no format
// set.
TEST_P(YCbCrSamplerTest, YCbCrSamplerInvalidWithNoFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::SamplerYCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerYCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = nullptr;
    samplerYCbCrDesc.vulkanYCbCrInfo.format = VK_FORMAT_UNDEFINED;

#if DAWN_PLATFORM_IS(ANDROID)
    VkExternalFormatANDROID vulkanExternalFormat = {};
    vulkanExternalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    vulkanExternalFormat.pNext = nullptr;
    vulkanExternalFormat.externalFormat = 0;

    samplerYCbCrDesc.vulkanYCbCrInfo.pNext = &vulkanExternalFormat;
#endif  // DAWN_PLATFORM_IS(ANDROID)

    samplerDesc.nextInChain = &samplerYCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

DAWN_INSTANTIATE_TEST(YCbCrSamplerTest, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
