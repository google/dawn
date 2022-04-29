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

#include <cmath>

#include "dawn/tests/DawnTest.h"

#include "dawn/common/Constants.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

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

    pass.End();
}

// Test that setting the clearColor, clearDepth, or clearStencil values for render pass attachments
// is deprecated. (dawn:1269)
TEST_P(DeprecationTests, AttachmentClearColor) {
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
    depthAttachment->depthLoadOp = wgpu::LoadOp::Clear;
    depthAttachment->stencilLoadOp = wgpu::LoadOp::Clear;

    // A pass that uses none of the deprecated value should be fine.
    pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.End();

    depthAttachment->clearStencil = 1;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.End();

    depthAttachment->clearStencil = 0;
    depthAttachment->depthClearValue = 0.0f;
    depthAttachment->clearDepth = 1.0f;

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.End();

    renderPass.renderPassInfo.depthStencilAttachment = nullptr;
    renderPass.renderPassInfo.cColorAttachments[0].clearColor = {1.0, 2.0, 3.0, 4.0};
    renderPass.renderPassInfo.cColorAttachments[0].clearValue = {5.0, 4.0, 3.0, 2.0};

    EXPECT_DEPRECATION_WARNING(pass = encoder.BeginRenderPass(&renderPass.renderPassInfo));
    pass.End();
}

// Test that endPass() is deprecated for both render and compute passes.
TEST_P(DeprecationTests, EndPass) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        EXPECT_DEPRECATION_WARNING(pass.EndPass());
    }

    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        EXPECT_DEPRECATION_WARNING(pass.EndPass());
    }
}

// Test that dispatch() and dispatchIndirect() is deprecated.
TEST_P(DeprecationTests, Dispatch) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @stage(compute) @workgroup_size(1, 1, 1)
            fn main() {
            })");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    std::array<uint32_t, 3> indirectBufferData = {1, 0, 0};

    wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
        device, &indirectBufferData[0], indirectBufferData.size() * sizeof(uint32_t),
        wgpu::BufferUsage::Indirect);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    EXPECT_DEPRECATION_WARNING(pass.Dispatch(1));

    EXPECT_DEPRECATION_WARNING(pass.DispatchIndirect(indirectBuffer, 0));

    pass.End();
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
