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

// Test that BindGroupLayoutEntry cannot have a type if buffer, sampler, texture, or storageTexture
// are defined.
TEST_P(DeprecationTests, BindGroupLayoutEntryTypeConflict) {
    wgpu::BindGroupLayoutEntry binding;
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Succeeds with only a type.
    // Will soon emit a deprecation warning.
    binding.type = wgpu::BindingType::UniformBuffer;
    device.CreateBindGroupLayout(&descriptor);

    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a buffer.type.
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a buffer.type are specified.
    binding.type = wgpu::BindingType::UniformBuffer;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.buffer.type = wgpu::BufferBindingType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a sampler.type.
    binding.sampler.type = wgpu::SamplerBindingType::Filtering;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a sampler.type are specified.
    binding.type = wgpu::BindingType::Sampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.sampler.type = wgpu::SamplerBindingType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a texture.sampleType.
    binding.texture.sampleType = wgpu::TextureSampleType::Float;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a texture.sampleType are specified.
    binding.type = wgpu::BindingType::SampledTexture;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.texture.sampleType = wgpu::TextureSampleType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a storageTexture.access.
    binding.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
    binding.storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a storageTexture.access are specified.
    binding.type = wgpu::BindingType::ReadonlyStorageTexture;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
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
