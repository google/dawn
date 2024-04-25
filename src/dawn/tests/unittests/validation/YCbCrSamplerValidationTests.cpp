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

#include <vulkan/vulkan.h>
#include <vector>

#include "dawn/native/VulkanBackend.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class YCbCrSamplerWithoutFeatureValidationTest : public ValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Tests that creating a sampler with a valid ycbcr sampler descriptor raises an error
// if the required feature is not enabled.
TEST_F(YCbCrSamplerWithoutFeatureValidationTest, YCbCrSamplerNotSupported) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::YCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerDesc.nextInChain = &samplerYCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

class YCbCrSamplerWithFeatureValidationTest : public YCbCrSamplerWithoutFeatureValidationTest {
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        wgpu::FeatureName requiredFeatures[2] = {wgpu::FeatureName::StaticSamplers,
                                                 wgpu::FeatureName::YCbCrVulkanSamplers};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeatureCount = 2;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Tests that creating a sampler with a valid ycbcr sampler descriptor succeeds if the
// required feature is enabled.
TEST_F(YCbCrSamplerWithFeatureValidationTest, YCbCrSamplerSupported) {
    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::YCbCrVulkanDescriptor samplerYCbCrDesc = {};
    samplerYCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerDesc.nextInChain = &samplerYCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Tests that creating a bind group layout with a valid static sampler succeeds if the
// required feature is enabled.
TEST_F(YCbCrSamplerWithFeatureValidationTest, CreateBindGroupWithYCbCrSamplerSupported) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::YCbCrVulkanDescriptor yCbCrDesc = {};
    yCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerDesc.nextInChain = &yCbCrDesc;

    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 0;

    device.CreateBindGroup(&descriptor);
}

// Verifies that creation of a correctly-specified bind group for a layout that
// has a sampler and a static sampler succeeds.
TEST_F(YCbCrSamplerWithFeatureValidationTest, CreateBindGroupWithSamplerAndStaticSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::YCbCrVulkanDescriptor yCbCrDesc = {};
    yCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerDesc.nextInChain = &yCbCrDesc;

    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    binding1.nextInChain = &staticSamplerBinding;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc0;
    samplerDesc0.minFilter = wgpu::FilterMode::Linear;
    utils::MakeBindGroup(device, layout, {{0, device.CreateSampler(&samplerDesc0)}});
}

// Verifies that creation of a bind group with the correct number of entries for a layout that has a
// sampler and a static sampler raises an error if the entry is specified at the
// index of the static sampler rather than that of the sampler.
TEST_F(YCbCrSamplerWithFeatureValidationTest, BindGroupCreationForSamplerBindingTypeCausesError) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc = {};
    native::vulkan::YCbCrVulkanDescriptor yCbCrDesc = {};
    yCbCrDesc.vulkanYCbCrInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerDesc.nextInChain = &yCbCrDesc;

    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    binding1.nextInChain = &staticSamplerBinding;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc0;
    samplerDesc0.minFilter = wgpu::FilterMode::Linear;
    ASSERT_DEVICE_ERROR(
        utils::MakeBindGroup(device, layout, {{1, device.CreateSampler(&samplerDesc0)}}));
}

}  // anonymous namespace
}  // namespace dawn
