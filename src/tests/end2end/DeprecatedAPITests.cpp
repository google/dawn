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

#include <cmath>

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
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

// Test that setting computeStage in a ComputePipelineDescriptor is deprecated.
TEST_P(DeprecationTests, ComputeStage) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[stage(compute), workgroup_size(1)]] fn main() {
        })");
    csDesc.computeStage.entryPoint = "main";

    wgpu::ComputePipeline pipeline;
    EXPECT_DEPRECATION_WARNING(pipeline = device.CreateComputePipeline(&csDesc));
}

// Test that StoreOp::Clear is deprecated.
TEST_P(DeprecationTests, StoreOpClear) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass;

    // Check that a storeOp of Clear for color attachments raises a validation warning.
    renderPass.renderPassInfo.cColorAttachments[0].storeOp = wgpu::StoreOp::Clear;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.EndPass();

    // Check that a storeOp of Clear for depth/stencil attachments raises a validation warning.
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
    depthAttachment->view = depthStencil.CreateView();

    renderPass.renderPassInfo.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    depthAttachment->depthStoreOp = wgpu::StoreOp::Clear;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.EndPass();

    depthAttachment->depthStoreOp = wgpu::StoreOp::Discard;
    depthAttachment->stencilStoreOp = wgpu::StoreOp::Clear;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.EndPass();
}

// Test that readonly storage textures are deprecated
TEST_P(DeprecationTests, ReadOnlyStorageTextures) {
    // Control case: WriteOnly storage textures are allowed.
    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly,
                  wgpu::TextureFormat::R32Float}});

    // Error case: ReadOnly storage textures are not allowed.
    EXPECT_DEPRECATION_WARNING(utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::ReadOnly,
                  wgpu::TextureFormat::R32Float}}));
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

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];

        descriptor.cBlends[0].color.srcFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::One;

        descriptor.cBlends[0].color.dstFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Zero;

        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::One;

        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
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
