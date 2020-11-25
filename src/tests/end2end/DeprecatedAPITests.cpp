// Copyright 2020 The Dawn Authors
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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test that SetIndexBufferWithFormat is deprecated.
TEST_P(DeprecationTests, SetIndexBufferWithFormat) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Index;
    wgpu::Buffer indexBuffer = device.CreateBuffer(&bufferDesc);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    EXPECT_DEPRECATION_WARNING(
        pass.SetIndexBufferWithFormat(indexBuffer, wgpu::IndexFormat::Uint32));
    pass.EndPass();
}

// Test that using BGLEntry.multisampled = true emits a deprecation warning.
TEST_P(DeprecationTests, BGLEntryMultisampledDeprecated) {
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::SampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&desc));
}

// Test that using BGLEntry.multisampled = true with MultisampledTexture is an error.
TEST_P(DeprecationTests, BGLEntryMultisampledBooleanAndTypeIsAnError) {
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::MultisampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Test that a using BGLEntry.multisampled produces the correct state tracking.
TEST_P(DeprecationTests, BGLEntryMultisampledBooleanTracking) {
    // Create a BGL with the deprecated multisampled boolean
    wgpu::BindGroupLayoutEntry entry{};
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.type = wgpu::BindingType::SampledTexture;
    entry.multisampled = true;
    entry.binding = 0;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    wgpu::BindGroupLayout bgl;
    EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&desc));

    // Create both a multisampled and non-multisampled texture.
    wgpu::TextureDescriptor textureDesc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::Sampled;
    textureDesc.size = {1, 1, 1};
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.sampleCount = 1;
    wgpu::Texture texture1Sample = device.CreateTexture(&textureDesc);

    textureDesc.sampleCount = 4;
    wgpu::Texture texture4Sample = device.CreateTexture(&textureDesc);

    // Creating a bindgroup with that layout is only valid with multisampled = true
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, texture1Sample.CreateView()}}));
    utils::MakeBindGroup(device, bgl, {{0, texture4Sample.CreateView()}});
}

// Test that compiling a pipeline with TextureComponentType::Float in the BGL when ::DepthComparison
// is expected emits a deprecation warning but isn't an error.
TEST_P(DeprecationTests, TextureComponentTypeFloatWhenDepthComparisonIsExpected) {
    wgpu::ShaderModule module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
            #version 450
            layout(set = 0, binding = 0) uniform samplerShadow samp;
            layout(set = 0, binding = 1) uniform texture2D tex;

            void main() {
                texture(sampler2DShadow(tex, samp), vec3(0.5, 0.5, 0.5));
            }
        )");

    {
        wgpu::BindGroupLayout goodBgl = utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::ComparisonSampler},
             {1, wgpu::ShaderStage::Compute, wgpu::BindingType::SampledTexture, false, 0, false,
              wgpu::TextureViewDimension::e2D, wgpu::TextureComponentType::DepthComparison}});

        wgpu::ComputePipelineDescriptor goodDesc;
        goodDesc.layout = utils::MakeBasicPipelineLayout(device, &goodBgl);
        goodDesc.computeStage.module = module;
        goodDesc.computeStage.entryPoint = "main";

        device.CreateComputePipeline(&goodDesc);
    }

    {
        wgpu::BindGroupLayout badBgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::ComparisonSampler},
                     {1, wgpu::ShaderStage::Compute, wgpu::BindingType::SampledTexture, false, 0,
                      false, wgpu::TextureViewDimension::e2D, wgpu::TextureComponentType::Float}});

        wgpu::ComputePipelineDescriptor badDesc;
        badDesc.layout = utils::MakeBasicPipelineLayout(device, &badBgl);
        badDesc.computeStage.module = module;
        badDesc.computeStage.entryPoint = "main";

        EXPECT_DEPRECATION_WARNING(device.CreateComputePipeline(&badDesc));
    }
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class BufferCopyViewDeprecationTests : public DeprecationTests {
  protected:
    wgpu::TextureCopyView MakeTextureCopyView() {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {1, 1, 2};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::TextureCopyView copy;
        copy.texture = device.CreateTexture(&desc);
        copy.origin = {0, 0, 1};
        return copy;
    }

    wgpu::Extent3D copySize = {1, 1, 1};
};
