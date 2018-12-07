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

#include "common/Constants.h"
#include "utils/DawnHelpers.h"

class BindGroupValidationTest : public ValidationTest {
  public:
    void SetUp() override {
        // Create objects to use as resources inside test bind groups.
        {
            dawn::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = dawn::BufferUsageBit::Uniform;
            mUBO = device.CreateBuffer(&descriptor);
        }
        {
            dawn::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = dawn::BufferUsageBit::Storage;
            mSSBO = device.CreateBuffer(&descriptor);
        }
        {
            dawn::SamplerDescriptor descriptor = utils::GetDefaultSamplerDescriptor();
            mSampler = device.CreateSampler(&descriptor);
        }
        {
            dawn::TextureDescriptor descriptor;
            descriptor.dimension = dawn::TextureDimension::e2D;
            descriptor.size = {16, 16, 1};
            descriptor.arrayLayer = 1;
            descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
            descriptor.levelCount = 1;
            descriptor.usage = dawn::TextureUsageBit::Sampled;
            mSampledTexture = device.CreateTexture(&descriptor);
            mSampledTextureView = mSampledTexture.CreateDefaultTextureView();
        }
    }

  protected:
    dawn::Buffer mUBO;
    dawn::Buffer mSSBO;
    dawn::Sampler mSampler;
    dawn::Texture mSampledTexture;
    dawn::TextureView mSampledTextureView;
};

// Test the validation of BindGroupDescriptor::nextInChain
TEST_F(BindGroupValidationTest, NextInChainNullptr) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    dawn::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.numBindings = 0;
    descriptor.bindings = nullptr;

    // Control case: check that nextInChain = nullptr is valid
    descriptor.nextInChain = nullptr;
    device.CreateBindGroup(&descriptor);

    // Check that nextInChain != nullptr is an error.
    descriptor.nextInChain = static_cast<void*>(&descriptor);
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
}

// Check constraints on numBindings
TEST_F(BindGroupValidationTest, NumBindingsMismatch) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler}
    });

    // Control case: check that a descriptor with one binding is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that numBindings != layout.numBindings fails.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {}));
}

// Check constraints on BindGroupBinding::binding
TEST_F(BindGroupValidationTest, WrongBindings) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler}
    });

    // Control case: check that a descriptor with a binding matching the layout's is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that binding must be present in the layout
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, mSampler}}));

    // Check that binding >= kMaxBindingsPerGroup fails.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{kMaxBindingsPerGroup, mSampler}}));
}

// Check that the same binding cannot be set twice
TEST_F(BindGroupValidationTest, BindingSetTwice) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
        {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler}
    });

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
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler}
    });

    dawn::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    dawn::BindGroupDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.layout = layout;
    descriptor.numBindings = 1;
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
}

// Check that a texture binding must contain exactly a texture view
TEST_F(BindGroupValidationTest, TextureBindingType) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture}
    });

    dawn::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    dawn::BindGroupDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.layout = layout;
    descriptor.numBindings = 1;
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
}

// Check that a buffer binding must contain exactly a buffer
TEST_F(BindGroupValidationTest, BufferBindingType) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}
    });

    dawn::BindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    dawn::BindGroupDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.layout = layout;
    descriptor.numBindings = 1;
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
}

// Check that a texture must have the correct usage
TEST_F(BindGroupValidationTest, TextureUsage) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture}
    });

    // Control case: setting a sampleable texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make an output attachment texture and try to set it for a SampledTexture binding
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.arrayLayer = 1;
    descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
    descriptor.levelCount = 1;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;
    dawn::Texture outputTexture = device.CreateTexture(&descriptor);
    dawn::TextureView outputTextureView = outputTexture.CreateDefaultTextureView();
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, outputTextureView}}));
}

// Check that a UBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageUBO) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}
    });

    // Control case: using a buffer with the uniform usage works
    utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}});

    // Using a buffer without the uniform usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}}));
}

// Check that a SSBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageSSBO) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::StorageBuffer}
    });

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a buffer without the storage usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}}));
}

// Tests constraints on the buffer offset for bind groups.
TEST_F(BindGroupValidationTest, BufferOffsetAlignment) {
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
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
    dawn::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {
        {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
    });

    dawn::BufferDescriptor descriptor;
    descriptor.size = 1024;
    descriptor.usage = dawn::BufferUsageBit::Uniform;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    // Success case, touching the start of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256}});

    // Success case, touching the end of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 3*256, 256}});
    utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 0}});

    // Success case, touching the full buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 1024}});

    // Error case, offset is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256*5, 0}}));

    // Error case, size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256*5}}));

    // Error case, offset+size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 1}}));

    // Error case, offset+size overflows to be 0
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256, uint32_t(0) - uint32_t(256)}}));
}

class BindGroupLayoutValidationTest : public ValidationTest {
};

// Tests setting OOB checks for kMaxBindingsPerGroup in bind group layouts.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutBindingOOB) {
    // Checks that kMaxBindingsPerGroup - 1 is valid.
    utils::MakeBindGroupLayout(device, {
        {kMaxBindingsPerGroup - 1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}
    });

    // Checks that kMaxBindingsPerGroup is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(device, {
        {kMaxBindingsPerGroup, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}
    }));
}

// This test verifies that the BindGroupLayout bindings are correctly validated, even if the
// binding ids are out-of-order.
TEST_F(BindGroupLayoutValidationTest, BindGroupBinding) {
    auto layout = utils::MakeBindGroupLayout(
        device, {
                    {1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });
}


// This test verifies that the BindGroupLayout cache is successfully caching/deduplicating objects.
//
// NOTE: This test only works currently because unittests are run without the wire - so the returned
// BindGroupLayout pointers are actually visibly equivalent. With the wire, this would not be true.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutCache) {
    auto layout1 = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });
    auto layout2 = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });

    // Caching should cause these to be the same.
    ASSERT_EQ(layout1.Get(), layout2.Get());
}
