// Copyright 2017 The Dawn Authors
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

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class BindGroupValidationTest : public ValidationTest {
  public:
    void SetUp() override {
        // Create objects to use as resources inside test bind groups.
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::Uniform;
            mUBO = device.CreateBuffer(&descriptor);
        }
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::Storage;
            mSSBO = device.CreateBuffer(&descriptor);
        }
        {
            wgpu::SamplerDescriptor descriptor = utils::GetDefaultSamplerDescriptor();
            mSampler = device.CreateSampler(&descriptor);
        }
        {
            wgpu::TextureDescriptor descriptor;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.size = {16, 16, 1};
            descriptor.arrayLayerCount = 1;
            descriptor.sampleCount = 1;
            descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
            descriptor.mipLevelCount = 1;
            descriptor.usage = wgpu::TextureUsage::Sampled;
            mSampledTexture = device.CreateTexture(&descriptor);
            mSampledTextureView = mSampledTexture.CreateView();
        }
    }

  protected:
    wgpu::Buffer mUBO;
    wgpu::Buffer mSSBO;
    wgpu::Sampler mSampler;
    wgpu::Texture mSampledTexture;
    wgpu::TextureView mSampledTextureView;
};

// Test the validation of BindGroupDescriptor::nextInChain
TEST_F(BindGroupValidationTest, NextInChainNullptr) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.bindingCount = 0;
    descriptor.bindings = nullptr;

    // Control case: check that nextInChain = nullptr is valid
    descriptor.nextInChain = nullptr;
    device.CreateBindGroup(&descriptor);

    // Check that nextInChain != nullptr is an error.
    wgpu::ChainedStruct chainedDescriptor;
    descriptor.nextInChain = &chainedDescriptor;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
}

// Check constraints on bindingCount
TEST_F(BindGroupValidationTest, bindingCountMismatch) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    // Control case: check that a descriptor with one binding is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that bindingCount != layout.bindingCount fails.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {}));
}

// Check constraints on BindGroupBinding::binding
TEST_F(BindGroupValidationTest, WrongBindings) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    // Control case: check that a descriptor with a binding matching the layout's is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that binding must be present in the layout
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, mSampler}}));

    // Check that binding >= kMaxBindingsPerGroup fails.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{kMaxBindingsPerGroup, mSampler}}));
}

// Check that the same binding cannot be set twice
TEST_F(BindGroupValidationTest, BindingSetTwice) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler},
                 {1, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    // Control case: check that different bindings work
    utils::MakeBindGroup(device, layout, {
        {0, mSampler},
        {1, mSampler}
    });

    // Check that setting the same binding twice is invalid
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {
        {0, mSampler},
        {0, mSampler}
    }));
}

// Check that a sampler binding must contain exactly one sampler
TEST_F(BindGroupValidationTest, SamplerBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    wgpu::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the sampler works
    binding.sampler = mSampler;
    device.CreateBindGroup(&descriptor);

    // Setting the texture view as well is an error
    binding.textureView = mSampledTextureView;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;

    // Setting the buffer as well is an error
    binding.buffer = mUBO;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.buffer = nullptr;

    // Setting the sampler to an error sampler is an error.
    {
        wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
        samplerDesc.minFilter = static_cast<wgpu::FilterMode>(0xFFFFFFFF);

        wgpu::Sampler errorSampler;
        ASSERT_DEVICE_ERROR(errorSampler = device.CreateSampler(&samplerDesc));

        binding.sampler = errorSampler;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.sampler = nullptr;
    }
}

// Check that a texture binding must contain exactly a texture view
TEST_F(BindGroupValidationTest, TextureBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::SampledTexture}});

    wgpu::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the texture view works
    binding.textureView = mSampledTextureView;
    device.CreateBindGroup(&descriptor);

    // Setting the sampler as well is an error
    binding.sampler = mSampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;

    // Setting the buffer as well is an error
    binding.buffer = mUBO;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.buffer = nullptr;

    // Setting the texture view to an error texture view is an error.
    {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 0;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1000;

        wgpu::TextureView errorView;
        ASSERT_DEVICE_ERROR(errorView = mSampledTexture.CreateView(&viewDesc));

        binding.textureView = errorView;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.textureView = nullptr;
    }
}

// Check that a buffer binding must contain exactly a buffer
TEST_F(BindGroupValidationTest, BufferBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::UniformBuffer}});

    wgpu::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the buffer works
    binding.buffer = mUBO;
    device.CreateBindGroup(&descriptor);

    // Setting the texture view as well is an error
    binding.textureView = mSampledTextureView;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;

    // Setting the sampler as well is an error
    binding.sampler = mSampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.sampler = nullptr;

    // Setting the buffer to an error buffer is an error.
    {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = 1024;
        bufferDesc.usage = static_cast<wgpu::BufferUsage>(0xFFFFFFFF);

        wgpu::Buffer errorBuffer;
        ASSERT_DEVICE_ERROR(errorBuffer = device.CreateBuffer(&bufferDesc));

        binding.buffer = errorBuffer;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.buffer = nullptr;
    }
}

// Check that a texture must have the correct usage
TEST_F(BindGroupValidationTest, TextureUsage) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::SampledTexture}});

    // Control case: setting a sampleable texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make an output attachment texture and try to set it for a SampledTexture binding
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.arrayLayerCount = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::OutputAttachment;
    wgpu::Texture outputTexture = device.CreateTexture(&descriptor);
    wgpu::TextureView outputTextureView = outputTexture.CreateView();
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, outputTextureView}}));
}

// Check that a texture must have the correct component type
TEST_F(BindGroupValidationTest, TextureComponentType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::SampledTexture, false, false,
                  wgpu::TextureViewDimension::e2D, wgpu::TextureComponentType::Float}});

    // Control case: setting a Float typed texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make a Uint component typed texture and try to set it to a Float component binding.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.arrayLayerCount = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::Sampled;
    wgpu::Texture uintTexture = device.CreateTexture(&descriptor);
    wgpu::TextureView uintTextureView = uintTexture.CreateView();

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, uintTextureView}}));
}

// Check that a texture must have the correct dimension
TEST_F(BindGroupValidationTest, TextureDimension) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::SampledTexture, false, false,
                  wgpu::TextureViewDimension::e2D, wgpu::TextureComponentType::Float}});

    // Control case: setting a 2D texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make a 2DArray texture and try to set it to a 2D binding.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.arrayLayerCount = 2;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::Sampled;
    wgpu::Texture arrayTexture = device.CreateTexture(&descriptor);
    wgpu::TextureView arrayTextureView = arrayTexture.CreateView();

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, arrayTextureView}}));
}

// Check that a UBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageUBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::UniformBuffer}});

    // Control case: using a buffer with the uniform usage works
    utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}});

    // Using a buffer without the uniform usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}}));
}

// Check that a SSBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageSSBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer}});

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a buffer without the storage usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}}));
}

// Check that a readonly SSBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageReadonlySSBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::ReadonlyStorageBuffer}});

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a buffer without the storage usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}}));
}

// Tests constraints on the buffer offset for bind groups.
TEST_F(BindGroupValidationTest, BufferOffsetAlignment) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                });

    // Check that offset 0 is valid
    utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 512}});

    // Check that offset 256 (aligned) is valid
    utils::MakeBindGroup(device, layout, {{0, mUBO, 256, 256}});

    // Check cases where unaligned buffer offset is invalid
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 1, 256}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 128, 256}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 255, 256}}));
}

// Tests constraints to be sure the buffer binding fits in the buffer
TEST_F(BindGroupValidationTest, BufferBindingOOB) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                });

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 1024;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    // Success case, touching the start of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256}});

    // Success case, touching the end of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 3*256, 256}});
    utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 0}});

    // Success case, touching the full buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 1024}});
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, wgpu::kWholeSize}});

    // Error case, offset is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256*5, 0}}));

    // Error case, size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256*5}}));

    // Error case, offset+size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 256}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256, wgpu::kWholeSize}}));

    // Error case, offset+size overflows to be 0
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256, uint32_t(0) - uint32_t(256)}}));
}

// Test what happens when the layout is an error.
TEST_F(BindGroupValidationTest, ErrorLayout) {
    wgpu::BindGroupLayout goodLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                });

    wgpu::BindGroupLayout errorLayout;
    ASSERT_DEVICE_ERROR(
        errorLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                        {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                    }));

    // Control case, creating with the good layout works
    utils::MakeBindGroup(device, goodLayout, {{0, mUBO, 0, 256}});

    // Creating with an error layout fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, errorLayout, {{0, mUBO, 0, 256}}));
}

class BindGroupLayoutValidationTest : public ValidationTest {
  public:
    void TestCreateBindGroupLayout(wgpu::BindGroupLayoutBinding* binding,
                                   uint32_t count,
                                   bool expected) {
        wgpu::BindGroupLayoutDescriptor descriptor;

        descriptor.bindingCount = count;
        descriptor.bindings = binding;

        if (!expected) {
            ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
        } else {
            device.CreateBindGroupLayout(&descriptor);
        }
    }

    void TestCreatePipelineLayout(wgpu::BindGroupLayout* bgl, uint32_t count, bool expected) {
        wgpu::PipelineLayoutDescriptor descriptor;

        descriptor.bindGroupLayoutCount = count;
        descriptor.bindGroupLayouts = bgl;

        if (!expected) {
            ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&descriptor));
        } else {
            device.CreatePipelineLayout(&descriptor);
        }
    }
};

// Tests setting storage buffer and readonly storage buffer bindings in vertex and fragment shader.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutStorageBindingsInVertexShader) {
    // Checks that storage buffer binding is not supported in vertex shader.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BindingType::StorageBuffer}}));

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BindingType::ReadonlyStorageBuffer}});

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer}});

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::ReadonlyStorageBuffer}});
}

// Tests setting that bind group layout bindings numbers may be >= kMaxBindingsPerGroup.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutBindingUnbounded) {
    // Checks that kMaxBindingsPerGroup is valid.
    utils::MakeBindGroupLayout(device, {{kMaxBindingsPerGroup, wgpu::ShaderStage::Vertex,
                                         wgpu::BindingType::UniformBuffer}});

    // Checks that kMaxBindingsPerGroup + 1 is valid.
    utils::MakeBindGroupLayout(device, {{kMaxBindingsPerGroup + 1, wgpu::ShaderStage::Vertex,
                                         wgpu::BindingType::UniformBuffer}});
}

// Test that there can't be more than kMaxBindingPerGroup bindings per group
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutMaxBindings) {
    wgpu::BindGroupLayoutBinding bindings[kMaxBindingsPerGroup + 1];

    for (uint32_t i = 0; i < kMaxBindingsPerGroup + 1; i++) {
        bindings[i].type = wgpu::BindingType::UniformBuffer;
        bindings[i].binding = i;
        bindings[i].visibility = wgpu::ShaderStage::Compute;
    }

    wgpu::BindGroupLayoutDescriptor desc;
    desc.bindings = bindings;

    // Control case: kMaxBindingsPerGroup bindings is allowed.
    desc.bindingCount = kMaxBindingsPerGroup;
    device.CreateBindGroupLayout(&desc);

    // Error case: kMaxBindingsPerGroup + 1 bindings is not allowed.
    desc.bindingCount = kMaxBindingsPerGroup + 1;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// This test verifies that the BindGroupLayout bindings are correctly validated, even if the
// binding ids are out-of-order.
TEST_F(BindGroupLayoutValidationTest, BindGroupBinding) {
    utils::MakeBindGroupLayout(device,
                               {
                                   {1, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                                   {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                               });
}

// Check that dynamic = true is only allowed with buffer bindings.
TEST_F(BindGroupLayoutValidationTest, DynamicAndTypeCompatibility) {
    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer, true},
                });

    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer, true},
                });

    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer, true},
                });

    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::SampledTexture, true},
                }));

    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::Sampler, true},
                }));
}

// This test verifies that visibility of bindings in BindGroupLayout can be none
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutVisibilityNone) {
    utils::MakeBindGroupLayout(device,
                               {
                                   {0, wgpu::ShaderStage::Vertex, wgpu::BindingType::UniformBuffer},
                               });

    wgpu::BindGroupLayoutBinding binding = {0, wgpu::ShaderStage::None,
                                            wgpu::BindingType::UniformBuffer};
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.bindingCount = 1;
    descriptor.bindings = &binding;
    device.CreateBindGroupLayout(&descriptor);
}

// Check that dynamic buffer numbers exceed maximum value in one bind group layout.
TEST_F(BindGroupLayoutValidationTest, DynamicBufferNumberLimit) {
    wgpu::BindGroupLayout bgl[2];
    std::vector<wgpu::BindGroupLayoutBinding> maxUniformDB;
    std::vector<wgpu::BindGroupLayoutBinding> maxStorageDB;
    std::vector<wgpu::BindGroupLayoutBinding> maxReadonlyStorageDB;

    for (uint32_t i = 0; i < kMaxDynamicUniformBufferCount; ++i) {
        maxUniformDB.push_back(
            {i, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer, true});
    }

    for (uint32_t i = 0; i < kMaxDynamicStorageBufferCount; ++i) {
        maxStorageDB.push_back(
            {i, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer, true});
    }

    for (uint32_t i = 0; i < kMaxDynamicStorageBufferCount; ++i) {
        maxReadonlyStorageDB.push_back(
            {i, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer, true});
    }

    auto MakeBindGroupLayout = [&](wgpu::BindGroupLayoutBinding* binding,
                                   uint32_t count) -> wgpu::BindGroupLayout {
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.bindingCount = count;
        descriptor.bindings = binding;
        return device.CreateBindGroupLayout(&descriptor);
    };

    {
        bgl[0] = MakeBindGroupLayout(maxUniformDB.data(), maxUniformDB.size());
        bgl[1] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());

        TestCreatePipelineLayout(bgl, 2, true);
    }

    {
        bgl[0] = MakeBindGroupLayout(maxUniformDB.data(), maxUniformDB.size());
        bgl[1] = MakeBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size());

        TestCreatePipelineLayout(bgl, 2, true);
    }

    // Check dynamic uniform buffers exceed maximum in pipeline layout.
    {
        bgl[0] = MakeBindGroupLayout(maxUniformDB.data(), maxUniformDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer, true},
                    });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic storage buffers exceed maximum in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer, true},
                    });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic readonly storage buffers exceed maximum in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device,
            {
                {0, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer, true},
            });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic storage buffers + dynamic readonly storage buffers exceed maximum storage
    // buffers in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device,
            {
                {0, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer, true},
            });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic uniform buffers exceed maximum in bind group layout.
    {
        maxUniformDB.push_back({kMaxDynamicUniformBufferCount, wgpu::ShaderStage::Compute,
                                wgpu::BindingType::UniformBuffer, true});
        TestCreateBindGroupLayout(maxUniformDB.data(), maxUniformDB.size(), false);
    }

    // Check dynamic storage buffers exceed maximum in bind group layout.
    {
        maxStorageDB.push_back({kMaxDynamicStorageBufferCount, wgpu::ShaderStage::Compute,
                                wgpu::BindingType::StorageBuffer, true});
        TestCreateBindGroupLayout(maxStorageDB.data(), maxStorageDB.size(), false);
    }

    // Check dynamic readonly storage buffers exceed maximum in bind group layout.
    {
        maxReadonlyStorageDB.push_back({kMaxDynamicStorageBufferCount, wgpu::ShaderStage::Compute,
                                        wgpu::BindingType::ReadonlyStorageBuffer, true});
        TestCreateBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size(), false);
    }
}

constexpr uint64_t kBufferSize = 3 * kMinDynamicBufferOffsetAlignment + 8;
constexpr uint32_t kBindingSize = 9;

class SetBindGroupValidationTest : public ValidationTest {
  public:
    void SetUp() override {
        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BindingType::UniformBuffer, true},
                     {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BindingType::UniformBuffer, false},
                     {2, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BindingType::StorageBuffer, true},
                     {3, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BindingType::ReadonlyStorageBuffer, true}});
    }

    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    wgpu::BindGroupLayout mBindGroupLayout;

    wgpu::RenderPipeline CreateRenderPipeline() {
        wgpu::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
                #version 450
                void main() {
                })");

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(std140, set = 0, binding = 0) uniform uBufferDynamic {
                    vec2 value0;
                };
                layout(std140, set = 0, binding = 1) uniform uBuffer {
                    vec2 value1;
                };
                layout(std140, set = 0, binding = 2) buffer SBufferDynamic {
                    vec2 value2;
                } sBuffer;
                layout(std140, set = 0, binding = 3) readonly buffer RBufferDynamic {
                    vec2 value3;
                } rBuffer;
                layout(location = 0) out vec4 fragColor;
                void main() {
                })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
        pipelineDescriptor.layout = pipelineLayout;
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline() {
        wgpu::ShaderModule csModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
                #version 450
                const uint kTileSize = 4;
                const uint kInstances = 11;

                layout(local_size_x = kTileSize, local_size_y = kTileSize, local_size_z = 1) in;
                layout(std140, set = 0, binding = 0) uniform UniformBufferDynamic {
                    float value0;
                };
                layout(std140, set = 0, binding = 1) uniform UniformBuffer {
                    float value1;
                };
                layout(std140, set = 0, binding = 2) buffer SBufferDynamic {
                    float value2;
                } dst;
                layout(std140, set = 0, binding = 3) readonly buffer RBufferDynamic {
                    readonly float value3;
                } rdst;
                void main() {
                })");

        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pipelineLayout;
        csDesc.computeStage.module = csModule;
        csDesc.computeStage.entryPoint = "main";

        return device.CreateComputePipeline(&csDesc);
    }

    void TestRenderPassBindGroup(wgpu::BindGroup bindGroup,
                                 uint32_t* offsets,
                                 uint32_t count,
                                 bool expectation) {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipeline();
        DummyRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup, count, offsets);
        renderPassEncoder.Draw(3, 1, 0, 0);
        renderPassEncoder.EndPass();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

    void TestComputePassBindGroup(wgpu::BindGroup bindGroup,
                                  uint32_t* offsets,
                                  uint32_t count,
                                  bool expectation) {
        wgpu::ComputePipeline computePipeline = CreateComputePipeline();

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        computePassEncoder.SetBindGroup(0, bindGroup, count, offsets);
        computePassEncoder.Dispatch(1, 1, 1);
        computePassEncoder.EndPass();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }
};

// This is the test case that should work.
TEST_F(SetBindGroupValidationTest, Basic) {
    // Set up the bind group.
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    std::array<uint32_t, 3> offsets = {512, 256, 0};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, true);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, true);
}

// Test cases that test dynamic offsets count mismatch with bind group layout.
TEST_F(SetBindGroupValidationTest, DynamicOffsetsMismatch) {
    // Set up bind group.
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    // Number of offsets mismatch.
    std::array<uint32_t, 4> mismatchOffsets = {768, 512, 256, 0};

    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 1, false);
    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 2, false);
    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 4, false);

    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 1, false);
    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 2, false);
    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 4, false);
}

// Test cases that test dynamic offsets not aligned
TEST_F(SetBindGroupValidationTest, DynamicOffsetsNotAligned) {
    // Set up bind group.
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    // Dynamic offsets are not aligned.
    std::array<uint32_t, 3> notAlignedOffsets = {512, 128, 0};

    TestRenderPassBindGroup(bindGroup, notAlignedOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, notAlignedOffsets.data(), 3, false);
}

// Test cases that test dynamic uniform buffer out of bound situation.
TEST_F(SetBindGroupValidationTest, OffsetOutOfBoundDynamicUniformBuffer) {
    // Set up bind group.
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    // Dynamic offset + offset is larger than buffer size.
    std::array<uint32_t, 3> overFlowOffsets = {1024, 256, 0};

    TestRenderPassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);
}

// Test cases that test dynamic storage buffer out of bound situation.
TEST_F(SetBindGroupValidationTest, OffsetOutOfBoundDynamicStorageBuffer) {
    // Set up bind group.
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    // Dynamic offset + offset is larger than buffer size.
    std::array<uint32_t, 3> overFlowOffsets = {0, 256, 1024};

    TestRenderPassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);
}

// Test cases that test dynamic uniform buffer out of bound situation because of binding size.
TEST_F(SetBindGroupValidationTest, BindingSizeOutOfBoundDynamicUniformBuffer) {
    // Set up bind group, but binding size is larger than
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    // Dynamic offset + offset isn't larger than buffer size.
    // But with binding size, it will trigger OOB error.
    std::array<uint32_t, 3> offsets = {768, 256, 0};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, false);
}

// Test cases that test dynamic storage buffer out of bound situation because of binding size.
TEST_F(SetBindGroupValidationTest, BindingSizeOutOfBoundDynamicStorageBuffer) {
    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});
    // Dynamic offset + offset isn't larger than buffer size.
    // But with binding size, it will trigger OOB error.
    std::array<uint32_t, 3> offsets = {0, 256, 768};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, false);
}

// Test that an error is produced (and no ASSERTs fired) when using an error bindgroup in
// SetBindGroup
TEST_F(SetBindGroupValidationTest, ErrorBindGroup) {
    // Bindgroup creation fails because not all bindings are specified.
    wgpu::BindGroup bindGroup;
    ASSERT_DEVICE_ERROR(bindGroup = utils::MakeBindGroup(device, mBindGroupLayout, {}));

    TestRenderPassBindGroup(bindGroup, nullptr, 0, false);

    TestComputePassBindGroup(bindGroup, nullptr, 0, false);
}

class SetBindGroupPersistenceValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        mVsModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
            #version 450
            void main() {
            })");
    }

    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    // Generates bind group layouts and a pipeline from a 2D list of binding types.
    std::tuple<std::vector<wgpu::BindGroupLayout>, wgpu::RenderPipeline> SetUpLayoutsAndPipeline(
        std::vector<std::vector<wgpu::BindingType>> layouts) {
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts(layouts.size());

        // Iterate through the desired bind group layouts.
        for (uint32_t l = 0; l < layouts.size(); ++l) {
            const auto& layout = layouts[l];
            std::vector<wgpu::BindGroupLayoutBinding> bindings(layout.size());

            // Iterate through binding types and populate a list of BindGroupLayoutBindings.
            for (uint32_t b = 0; b < layout.size(); ++b) {
                bindings[b] = {b, wgpu::ShaderStage::Fragment, layout[b], false};
            }

            // Create the bind group layout.
            wgpu::BindGroupLayoutDescriptor bglDescriptor;
            bglDescriptor.bindingCount = static_cast<uint32_t>(bindings.size());
            bglDescriptor.bindings = bindings.data();
            bindGroupLayouts[l] = device.CreateBindGroupLayout(&bglDescriptor);
        }

        // Create a pipeline layout from the list of bind group layouts.
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount =
            static_cast<uint32_t>(bindGroupLayouts.size());
        pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts.data();

        wgpu::PipelineLayout pipelineLayout =
            device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        std::stringstream ss;
        ss << "#version 450\n";

        // Build a shader which has bindings that match the pipeline layout.
        for (uint32_t l = 0; l < layouts.size(); ++l) {
            const auto& layout = layouts[l];

            for (uint32_t b = 0; b < layout.size(); ++b) {
                wgpu::BindingType binding = layout[b];
                ss << "layout(std140, set = " << l << ", binding = " << b << ") ";
                switch (binding) {
                    case wgpu::BindingType::StorageBuffer:
                        ss << "buffer SBuffer";
                        break;
                    case wgpu::BindingType::UniformBuffer:
                        ss << "uniform UBuffer";
                        break;
                    default:
                        UNREACHABLE();
                }
                ss << l << "_" << b << " { vec2 set" << l << "_binding" << b << "; };\n";
            }
        }

        ss << "layout(location = 0) out vec4 fragColor;\n";
        ss << "void main() { fragColor = vec4(0.0, 1.0, 0.0, 1.0); }\n";

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, ss.str().c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = mVsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.layout = pipelineLayout;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        return std::make_tuple(bindGroupLayouts, pipeline);
    }

  private:
    wgpu::ShaderModule mVsModule;
};

// Test it is valid to set bind groups before setting the pipeline.
TEST_F(SetBindGroupPersistenceValidationTest, BindGroupBeforePipeline) {
    std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
    wgpu::RenderPipeline pipeline;
    std::tie(bindGroupLayouts, pipeline) = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BindingType::UniformBuffer,
            wgpu::BindingType::UniformBuffer,
        }},
        {{
            wgpu::BindingType::StorageBuffer,
            wgpu::BindingType::UniformBuffer,
        }},
    }});

    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
        device, bindGroupLayouts[0],
        {{0, uniformBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
        device, bindGroupLayouts[1],
        {{0, storageBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    DummyRenderPass renderPass(device);
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);

    renderPassEncoder.SetBindGroup(0, bindGroup0);
    renderPassEncoder.SetBindGroup(1, bindGroup1);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.Draw(3, 1, 0, 0);

    renderPassEncoder.EndPass();
    commandEncoder.Finish();
}

// Dawn does not have a concept of bind group inheritance though the backing APIs may.
// Test that it is valid to draw with bind groups that are not "inherited". They persist
// after a pipeline change.
TEST_F(SetBindGroupPersistenceValidationTest, NotVulkanInheritance) {
    std::vector<wgpu::BindGroupLayout> bindGroupLayoutsA;
    wgpu::RenderPipeline pipelineA;
    std::tie(bindGroupLayoutsA, pipelineA) = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BindingType::UniformBuffer,
            wgpu::BindingType::StorageBuffer,
        }},
        {{
            wgpu::BindingType::UniformBuffer,
            wgpu::BindingType::UniformBuffer,
        }},
    }});

    std::vector<wgpu::BindGroupLayout> bindGroupLayoutsB;
    wgpu::RenderPipeline pipelineB;
    std::tie(bindGroupLayoutsB, pipelineB) = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BindingType::StorageBuffer,
            wgpu::BindingType::UniformBuffer,
        }},
        {{
            wgpu::BindingType::UniformBuffer,
            wgpu::BindingType::UniformBuffer,
        }},
    }});

    wgpu::Buffer uniformBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(kBufferSize, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroupA0 = utils::MakeBindGroup(
        device, bindGroupLayoutsA[0],
        {{0, uniformBuffer, 0, kBindingSize}, {1, storageBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroupA1 = utils::MakeBindGroup(
        device, bindGroupLayoutsA[1],
        {{0, uniformBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroupB0 = utils::MakeBindGroup(
        device, bindGroupLayoutsB[0],
        {{0, storageBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    DummyRenderPass renderPass(device);
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);

    renderPassEncoder.SetPipeline(pipelineA);
    renderPassEncoder.SetBindGroup(0, bindGroupA0);
    renderPassEncoder.SetBindGroup(1, bindGroupA1);
    renderPassEncoder.Draw(3, 1, 0, 0);

    renderPassEncoder.SetPipeline(pipelineB);
    renderPassEncoder.SetBindGroup(0, bindGroupB0);
    // This draw is valid.
    // Bind group 1 persists even though it is not "inherited".
    renderPassEncoder.Draw(3, 1, 0, 0);

    renderPassEncoder.EndPass();
    commandEncoder.Finish();
}

class BindGroupLayoutCompatibilityTest : public ValidationTest {
  public:
    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(wgpu::BindGroupLayout* bindGroupLayout) {
        wgpu::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
                #version 450
                void main() {
                })");

        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(std140, set = 0, binding = 0) buffer SBuffer {
                    vec2 value2;
                } sBuffer;
                layout(std140, set = 0, binding = 1) readonly buffer RBuffer {
                    vec2 value3;
                } rBuffer;
                layout(location = 0) out vec4 fragColor;
                void main() {
                })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, bindGroupLayout);
        pipelineDescriptor.layout = pipelineLayout;
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline(wgpu::BindGroupLayout* bindGroupLayout) {
        wgpu::ShaderModule csModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, R"(
                #version 450
                const uint kTileSize = 4;
                const uint kInstances = 11;

                layout(local_size_x = kTileSize, local_size_y = kTileSize, local_size_z = 1) in;
                layout(std140, set = 0, binding = 0) buffer SBuffer {
                    float value2;
                } dst;
                layout(std140, set = 0, binding = 1) readonly buffer RBuffer {
                    readonly float value3;
                } rdst;
                void main() {
                })");

        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, bindGroupLayout);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pipelineLayout;
        csDesc.computeStage.module = csModule;
        csDesc.computeStage.entryPoint = "main";

        return device.CreateComputePipeline(&csDesc);
    }
};

// Test cases that test bind group layout mismatch with shader. The second item in bind group layout
// is a writable storage buffer, but the second item in shader is a readonly storage buffer. It is
// valid.
TEST_F(BindGroupLayoutCompatibilityTest, RWStorageInBGLWithROStorageInShader) {
    // Set up the bind group layout.
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BindingType::StorageBuffer, true},
                 {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BindingType::StorageBuffer, true}});

    CreateRenderPipeline(&bindGroupLayout);

    CreateComputePipeline(&bindGroupLayout);
}

// Test cases that test bind group layout mismatch with shader. The first item in bind group layout
// is a readonly storage buffer, but the first item in shader is a writable storage buffer. It is
// invalid.
TEST_F(BindGroupLayoutCompatibilityTest, ROStorageInBGLWithRWStorageInShader) {
    // Set up the bind group layout.
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BindingType::ReadonlyStorageBuffer, true},
                 {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BindingType::ReadonlyStorageBuffer, true}});

    ASSERT_DEVICE_ERROR(CreateRenderPipeline(&bindGroupLayout));

    ASSERT_DEVICE_ERROR(CreateComputePipeline(&bindGroupLayout));
}
