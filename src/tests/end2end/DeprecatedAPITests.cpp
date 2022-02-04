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

#include "dawn/common/Constants.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

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
TEST_P(DeprecationTests, ReadOnlyDepthStencilStoreLoadOpsAttachment) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass;

    // Check that setting load/store ops with read only depth/stencil attachments gives a warning.
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
    depthAttachment->depthReadOnly = true;
    depthAttachment->stencilReadOnly = true;

    depthAttachment->depthLoadOp = wgpu::LoadOp::Load;
    depthAttachment->depthStoreOp = wgpu::StoreOp::Store;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));

    depthAttachment->depthLoadOp = wgpu::LoadOp::Undefined;
    depthAttachment->depthStoreOp = wgpu::StoreOp::Undefined;
    depthAttachment->stencilLoadOp = wgpu::LoadOp::Load;
    depthAttachment->stencilStoreOp = wgpu::StoreOp::Store;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));

    pass.EndPass();
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
