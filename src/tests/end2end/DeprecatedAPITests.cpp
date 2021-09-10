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

// Test that setting attachment rather than view for render pass color and depth/stencil attachments
// is deprecated.
TEST_P(DeprecationTests, SetAttachmentDescriptorAttachment) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass;

    // Check that using .attachment with color attachments gives the warning.
    wgpu::RenderPassColorAttachment* colorAttachment =
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

    wgpu::RenderPassDepthStencilAttachment* depthAttachment =
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

    wgpu::RenderPassDepthStencilAttachment* depthAttachment =
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
