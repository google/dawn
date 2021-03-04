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
#include "common/VertexFormatUtils.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <cmath>

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
    binding.type = wgpu::BindingType::UniformBuffer;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&descriptor));

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

// Test that the deprecated BGLEntry path correctly handles the defaulting of viewDimension.
// This is a regression test for crbug.com/dawn/620
TEST_P(DeprecationTests, BindGroupLayoutEntryViewDimensionDefaulting) {
    wgpu::BindGroupLayoutEntry binding;
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Vertex;
    binding.type = wgpu::BindingType::SampledTexture;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &binding;

    wgpu::BindGroupLayout bgl;

    // Check that the default viewDimension is 2D.
    {
        binding.viewDimension = wgpu::TextureViewDimension::Undefined;
        EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&bglDesc));

        wgpu::TextureDescriptor desc;
        desc.usage = wgpu::TextureUsage::Sampled;
        desc.size = {1, 1, 1};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        wgpu::Texture texture = device.CreateTexture(&desc);

        // Success, the default is 2D and we give it a 2D view.
        utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}});
    }

    // Check that setting a non-default viewDimension works.
    {
        binding.viewDimension = wgpu::TextureViewDimension::e2DArray;
        EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&bglDesc));

        wgpu::TextureDescriptor desc;
        desc.usage = wgpu::TextureUsage::Sampled;
        desc.size = {1, 1, 4};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        wgpu::Texture texture = device.CreateTexture(&desc);

        // Success, the view will be 2DArray and the BGL expects a 2DArray.
        utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}});
    }
}

// Test Device::GetDefaultQueue deprecation.
TEST_P(DeprecationTests, GetDefaultQueueDeprecation) {
    // Using GetDefaultQueue emits a warning.
    wgpu::Queue deprecatedQueue;
    EXPECT_DEPRECATION_WARNING(deprecatedQueue = device.GetDefaultQueue());

    // Using GetQueue doesn't emit a warning.
    wgpu::Queue queue = device.GetQueue();

    // Both objects are the same, even with dawn_wire.
    EXPECT_EQ(deprecatedQueue.Get(), queue.Get());
}

// Test that fences are deprecated.
TEST_P(DeprecationTests, CreateFence) {
    EXPECT_DEPRECATION_WARNING(queue.CreateFence());
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class ImageCopyBufferDeprecationTests : public DeprecationTests {
  protected:
    wgpu::ImageCopyTexture MakeImageCopyTexture() {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {1, 1, 2};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::ImageCopyTexture copy;
        copy.texture = device.CreateTexture(&desc);
        copy.origin = {0, 0, 1};
        return copy;
    }

    wgpu::Extent3D copySize = {1, 1, 1};
};

// Tests that deprecated vertex formats properly raise a deprecation warning when used
class VertexFormatDeprecationTests : public DeprecationTests {
  protected:
    // Runs the test
    void DoTest(const wgpu::VertexFormat vertexFormat, bool deprecated) {
        std::string attribute = "[[location(0)]] var<in> a : ";
        attribute += dawn::GetWGSLVertexFormatType(vertexFormat);
        attribute += ";";

        std::string attribAccess = dawn::VertexFormatNumComponents(vertexFormat) > 1
                                       ? "vec4<f32>(a.x, 0.0, 0.0, 1.0)"
                                       : "vec4<f32>(a, 0.0, 0.0, 1.0)";

        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, (attribute + R"(
                [[builtin(position)]] var<out> Position : vec4<f32>;

                [[stage(vertex)]] fn main() -> void {
                    Position = )" + attribAccess + R"(;
                    return;
                }
            )")
                                                                                    .c_str());
        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
                [[location(0)]] var<out> outColor : vec4<f32>;

                [[stage(fragment)]] fn main() -> void {
                    outColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
                    return;
                }
            )");

        utils::ComboVertexStateDescriptor vertexState;
        vertexState.vertexBufferCount = 1;
        vertexState.cVertexBuffers[0].arrayStride = 32;
        vertexState.cVertexBuffers[0].attributeCount = 1;
        vertexState.cAttributes[0].format = vertexFormat;
        vertexState.cAttributes[0].offset = 0;
        vertexState.cAttributes[0].shaderLocation = 0;

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        descriptor.vertexState = &vertexState;
        descriptor.cColorStates[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
    }
};

TEST_P(VertexFormatDeprecationTests, NewVertexFormats) {
    // Using the new vertex formats does not emit a warning.
    for (auto& format : dawn::kAllVertexFormats) {
        DoTest(format, false);
    }
}

TEST_P(VertexFormatDeprecationTests, DeprecatedVertexFormats) {
    // Using deprecated vertex formats does emit a warning.
    for (auto& format : dawn::kAllDeprecatedVertexFormats) {
        DoTest(format, true);
    }
}

DAWN_INSTANTIATE_TEST(VertexFormatDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());