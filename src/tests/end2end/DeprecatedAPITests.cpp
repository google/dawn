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

// Test that SetBlendColor is deprecated.
TEST_P(DeprecationTests, SetSetBlendColor) {
    wgpu::Color blendColor{1.0, 0.0, 0.0, 1.0};

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    EXPECT_DEPRECATION_WARNING(pass.SetBlendColor(&blendColor));
    pass.EndPass();
}

// Test that setting attachment rather than view for render pass color and depth/stencil attachments
// is deprecated.
TEST_P(DeprecationTests, SetAttachmentDescriptorAttachment) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass;

    // Check that using .attachment with color attachments gives the warning.
    wgpu::RenderPassColorAttachmentDescriptor* colorAttachment =
        &renderPass.renderPassInfo.cColorAttachments[0];
    colorAttachment->attachment = colorAttachment->view;
    colorAttachment->view = nullptr;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.EndPass();

    colorAttachment->view = colorAttachment->attachment;
    colorAttachment->attachment = nullptr;

    // Check that using .attachment with depth/stencil attachments gives the warning.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {1, 1, 1};
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture depthStencil = device.CreateTexture(&descriptor);

    wgpu::RenderPassDepthStencilAttachmentDescriptor* depthAttachment =
        &renderPass.renderPassInfo.cDepthStencilAttachmentInfo;
    renderPass.renderPassInfo.depthStencilAttachment = depthAttachment;
    depthAttachment->attachment = depthStencil.CreateView();

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
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
        std::string attribute = "[[location(0)]] a : ";
        attribute += dawn::GetWGSLVertexFormatType(vertexFormat);

        std::string attribAccess = dawn::VertexFormatNumComponents(vertexFormat) > 1
                                       ? "vec4<f32>(f32(a.x), 0.0, 0.0, 1.0)"
                                       : "vec4<f32>(f32(a), 0.0, 0.0, 1.0)";

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, (R"(
                [[stage(vertex)]] fn main()" + attribute + R"() -> [[builtin(position)]] vec4<f32> {
                    return )" + attribAccess + R"(;
                }
            )")
                                                                            .c_str());
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
                }
            )");

        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 32;
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = vertexFormat;
        descriptor.cAttributes[0].offset = 0;
        descriptor.cAttributes[0].shaderLocation = 0;
        descriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
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

// Tests that deprecated blend factors properly raise a deprecation warning when used
class BlendFactorDeprecationTests : public DeprecationTests {
  protected:
    // Runs the test
    void DoTest(const wgpu::BlendFactor blendFactor, bool deprecated) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
                    return vec4<f32>(0.0, 0.0, 0.0, 1.0);
                }
            )");
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
                }
            )");

        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];

        descriptor.cBlends[0].color.srcFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
        }
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::One;

        descriptor.cBlends[0].color.dstFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
        }
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Zero;

        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
        }
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::One;

        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
        }
        descriptor.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::Zero;
    }
};

static constexpr std::array<wgpu::BlendFactor, 13> kBlendFactors = {
    wgpu::BlendFactor::Zero,
    wgpu::BlendFactor::One,
    wgpu::BlendFactor::Src,
    wgpu::BlendFactor::OneMinusSrc,
    wgpu::BlendFactor::SrcAlpha,
    wgpu::BlendFactor::OneMinusSrcAlpha,
    wgpu::BlendFactor::Dst,
    wgpu::BlendFactor::OneMinusDst,
    wgpu::BlendFactor::DstAlpha,
    wgpu::BlendFactor::OneMinusDstAlpha,
    wgpu::BlendFactor::SrcAlphaSaturated,
    wgpu::BlendFactor::Constant,
    wgpu::BlendFactor::OneMinusConstant,
};

TEST_P(BlendFactorDeprecationTests, CurrentBlendFactors) {
    // Using the new blend factors does not emit a warning.
    for (auto& format : kBlendFactors) {
        DoTest(format, false);
    }
}

static constexpr std::array<wgpu::BlendFactor, 6> kDeprecatedBlendFactors = {
    wgpu::BlendFactor::SrcColor,   wgpu::BlendFactor::OneMinusSrcColor,
    wgpu::BlendFactor::DstColor,   wgpu::BlendFactor::OneMinusDstColor,
    wgpu::BlendFactor::BlendColor, wgpu::BlendFactor::OneMinusBlendColor,
};

TEST_P(BlendFactorDeprecationTests, DeprecatedBlendFactors) {
    // Using deprecated blend factors does emit a warning.
    for (auto& format : kDeprecatedBlendFactors) {
        DoTest(format, true);
    }
}

DAWN_INSTANTIATE_TEST(BlendFactorDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
