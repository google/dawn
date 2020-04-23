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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class DeprecationTests : public DawnTest {
  protected:
    void TestSetUp() override {
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());
    }

    void TearDown() override {
        if (!UsesWire()) {
            EXPECT_EQ(mLastWarningCount,
                      dawn_native::GetDeprecationWarningCountForTesting(device.Get()));
        }
    }

    size_t mLastWarningCount = 0;
};

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                        \
            EXPECT_EQ(warningsAfter, warningsBefore + 1);                        \
            mLastWarningCount = warningsAfter;                                   \
        }                                                                        \
    } while (0)

// Tests for Device::CreateQueue -> Device::GetDefaultQueue.

// Test that using CreateQueue produces a deprecation warning
TEST_P(DeprecationTests, CreateQueueIsDeprecated) {
    EXPECT_DEPRECATION_WARNING(device.CreateQueue());
}

// Test that queues created from CreateQueue can be used for things
TEST_P(DeprecationTests, CreateQueueReturnsFunctionalQueue) {
    wgpu::Queue q;
    EXPECT_DEPRECATION_WARNING(q = device.CreateQueue());

    q.Submit(0, nullptr);
}

// Tests for BindGroupLayoutEntry::textureDimension -> viewDimension

// Test that creating a BGL with textureDimension produces a deprecation warning.
TEST_P(DeprecationTests, BGLEntryTextureDimensionIsDeprecated) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::SampledTexture,
        .textureDimension = wgpu::TextureViewDimension::e2D,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .entryCount = 1,
        .entries = &entryDesc,
    };
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with default viewDimension and textureDimension doesn't emit a warning
TEST_P(DeprecationTests, BGLEntryTextureDimensionAndViewUndefinedEmitsNoWarning) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::Sampler,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .entryCount = 1,
        .entries = &entryDesc,
    };
    device.CreateBindGroupLayout(&bglDesc);
}
// Test that creating a BGL with both textureDimension and viewDimension is an error
TEST_P(DeprecationTests, BGLEntryTextureAndViewDimensionIsInvalid) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::SampledTexture,
        .textureDimension = wgpu::TextureViewDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2D,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .entryCount = 1,
        .entries = &entryDesc,
    };
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both textureDimension still does correct state tracking
TEST_P(DeprecationTests, BGLEntryTextureDimensionStateTracking) {
    // Create a BGL that expects a cube map
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::SampledTexture,
        .textureDimension = wgpu::TextureViewDimension::Cube,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .entryCount = 1,
        .entries = &entryDesc,
    };
    wgpu::BindGroupLayout layout;
    EXPECT_DEPRECATION_WARNING(layout = device.CreateBindGroupLayout(&bglDesc));

    // Create a 2D array view and a cube view
    wgpu::TextureDescriptor textureDesc = {
        .usage = wgpu::TextureUsage::Sampled,
        .size = {1, 1, 1},
        .arrayLayerCount = 6,
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::TextureViewDescriptor viewDesc = {
        .dimension = wgpu::TextureViewDimension::e2DArray,
        .baseArrayLayer = 0,
        .arrayLayerCount = 6,
    };
    wgpu::TextureView arrayView = texture.CreateView(&viewDesc);

    viewDesc.dimension = wgpu::TextureViewDimension::Cube;
    wgpu::TextureView cubeView = texture.CreateView(&viewDesc);

    // textureDimension is correctly taken into account and only the BindGroup with the Cube view is
    // valid.
    utils::MakeBindGroup(device, layout, {{0, cubeView}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, arrayView}}));
}

// Test for BindGroupLayout::bindings/bindingCount -> entries/entryCount

// Test that creating a BGL with bindings emits a deprecation warning.
TEST_P(DeprecationTests, BGLDescBindingIsDeprecated) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::Sampler,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .bindingCount = 1,
        .bindings = &entryDesc,
    };
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both entries and bindings is an error
TEST_P(DeprecationTests, BGLDescBindingAndEntriesIsInvalid) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .type = wgpu::BindingType::Sampler,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .bindingCount = 1,
        .bindings = &entryDesc,
        .entryCount = 1,
        .entries = &entryDesc,
    };
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Test that creating a BGL with both entries and bindings to 0 doesn't emit warnings
TEST_P(DeprecationTests, BGLDescBindingAndEntriesBothZeroEmitsNoWarning) {
    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .bindingCount = 0,
        .bindings = nullptr,
        .entryCount = 0,
        .entries = nullptr,
    };
    device.CreateBindGroupLayout(&bglDesc);
}

// Test that creating a BGL with bindings still does correct state tracking
TEST_P(DeprecationTests, BGLDescBindingStateTracking) {
    wgpu::BindGroupLayoutEntry entryDesc = {
        .binding = 0,
        .type = wgpu::BindingType::Sampler,
    };

    wgpu::BindGroupLayoutDescriptor bglDesc = {
        .bindingCount = 1,
        .bindings = &entryDesc,
    };
    wgpu::BindGroupLayout layout;
    EXPECT_DEPRECATION_WARNING(layout = device.CreateBindGroupLayout(&bglDesc));

    // Test a case where if |bindings| wasn't taken into account, no validation error would happen
    // because the layout would be empty
    wgpu::BindGroupDescriptor badBgDesc = {
        .layout = layout,
        .entryCount = 0,
        .entries = nullptr,
    };
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&badBgDesc));
}

// Test for BindGroup::bindings/bindingCount -> entries/entryCount

// Test that creating a BG with bindings emits a deprecation warning.
TEST_P(DeprecationTests, BGDescBindingIsDeprecated) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    wgpu::BindGroupEntry entryDesc = {
        .binding = 0,
        .sampler = sampler,
    };

    wgpu::BindGroupDescriptor bgDesc = {
        .layout = layout,
        .bindingCount = 1,
        .bindings = &entryDesc,
    };
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroup(&bgDesc));
}

// Test that creating a BG with both entries and bindings is an error
TEST_P(DeprecationTests, BGDescBindingAndEntriesIsInvalid) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::Sampler}});

    wgpu::BindGroupEntry entryDesc = {
        .binding = 0,
        .sampler = sampler,
    };

    wgpu::BindGroupDescriptor bgDesc = {
        .layout = layout,
        .bindingCount = 1,
        .bindings = &entryDesc,
        .entryCount = 1,
        .entries = &entryDesc,
    };
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

// Test that creating a BG with both entries and bindings to 0 doesn't emit warnings
TEST_P(DeprecationTests, BGDescBindingAndEntriesBothZeroEmitsNoWarning) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor bgDesc = {
        .layout = layout,
        .bindingCount = 0,
        .bindings = nullptr,
        .entryCount = 0,
        .entries = nullptr,
    };
    device.CreateBindGroup(&bgDesc);
}

// Test that creating a BG with bindings still does correct state tracking
TEST_P(DeprecationTests, BGDescBindingStateTracking) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    // Test a case where if |bindings| wasn't taken into account, no validation error would happen
    // because it would match the empty layout.
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::BindGroupEntry entryDesc = {
        .binding = 0,
        .sampler = sampler,
    };

    wgpu::BindGroupDescriptor bgDesc = {
        .layout = layout,
        .bindingCount = 1,
        .bindings = &entryDesc,
    };
    EXPECT_DEPRECATION_WARNING(ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc)));
}

// Tests for ShaderModuleDescriptor.code/codeSize -> ShaderModuleSPIRVDescriptor

static const char kEmptyShader[] = R"(#version 450
void main() {
})";

// That creating a ShaderModule without the chained descriptor gives a warning.
TEST_P(DeprecationTests, ShaderModuleNoSubDescriptorIsDeprecated) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleDescriptor descriptor = {
        .codeSize = static_cast<uint32_t>(spirv.size()),
        .code = spirv.data(),
    };
    EXPECT_DEPRECATION_WARNING(device.CreateShaderModule(&descriptor));
}

// That creating a ShaderModule with both inline code and the chained descriptor is an error.
TEST_P(DeprecationTests, ShaderModuleBothInlinedAndChainedIsInvalid) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    spirvDesc.codeSize = static_cast<uint32_t>(spirv.size());
    spirvDesc.code = spirv.data();

    wgpu::ShaderModuleDescriptor descriptor = {
        .nextInChain = &spirvDesc,
        .codeSize = static_cast<uint32_t>(spirv.size()),
        .code = spirv.data(),
    };
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&descriptor));
}

// That creating a ShaderModule with both inline code still does correct state tracking
TEST_P(DeprecationTests, ShaderModuleInlinedCodeStateTracking) {
    std::vector<uint32_t> spirv =
        CompileGLSLToSpirv(utils::SingleShaderStage::Compute, kEmptyShader);

    wgpu::ShaderModuleDescriptor descriptor = {
        .codeSize = static_cast<uint32_t>(spirv.size()),
        .code = spirv.data(),
    };
    wgpu::ShaderModule module;
    EXPECT_DEPRECATION_WARNING(module = device.CreateShaderModule(&descriptor));

    // Creating a compute pipeline works, because it is a compute module.
    wgpu::ComputePipelineDescriptor computePipelineDesc = {
        .computeStage =
            {
                .module = module,
                .entryPoint = "main",
            },
    };
    device.CreateComputePipeline(&computePipelineDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDesc(device);
    renderPipelineDesc.vertexStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, kEmptyShader);
    renderPipelineDesc.cFragmentStage.module = module;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&renderPipelineDesc));
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
