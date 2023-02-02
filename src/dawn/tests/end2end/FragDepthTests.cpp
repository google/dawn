// Copyright 2022 The Dawn Authors
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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr wgpu::TextureFormat kDepthFormat = wgpu::TextureFormat::Depth32Float;

class FragDepthTests : public DawnTest {};

// Test that when writing to FragDepth the result is clamped to the viewport.
TEST_P(FragDepthTests, FragDepthIsClampedToViewport) {
    // TODO(dawn:1125): Add the shader transform to clamp the frag depth to the GL backend.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.5, 1.0);
        }

        @fragment fn fs() -> @builtin(frag_depth) f32 {
            return 1.0;
        }
    )");

    // Create the pipeline that uses frag_depth to output the depth.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.vertex.entryPoint = "vs";
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.entryPoint = "fs";
    pDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* pDescDS = pDesc.EnableDepthStencil(kDepthFormat);
    pDescDS->depthWriteEnabled = true;
    pDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw a point with a skewed viewport, so 1.0 depth gets clamped to 0.5.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetViewport(0, 0, 1, 1, 0.0, 0.5);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// Test for the push constant logic for ClampFragDepth in Vulkan to check that changing the
// pipeline layout doesn't invalidate the push constants that were set.
TEST_P(FragDepthTests, ChangingPipelineLayoutDoesntInvalidateViewport) {
    // TODO(dawn:1125): Add the shader transform to clamp the frag depth to the GL backend.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.5, 1.0);
        }

        @group(0) @binding(0) var<uniform> uniformDepth : f32;
        @fragment fn fsUniform() -> @builtin(frag_depth) f32 {
            return uniformDepth;
        }

        @group(0) @binding(0) var<storage, read> storageDepth : f32;
        @fragment fn fsStorage() -> @builtin(frag_depth) f32 {
            return storageDepth;
        }
    )");

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor upDesc;
    upDesc.vertex.module = module;
    upDesc.vertex.entryPoint = "vs";
    upDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    upDesc.cFragment.module = module;
    upDesc.cFragment.entryPoint = "fsUniform";
    upDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* upDescDS = upDesc.EnableDepthStencil(kDepthFormat);
    upDescDS->depthWriteEnabled = true;
    upDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline uniformPipeline = device.CreateRenderPipeline(&upDesc);

    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Uniform, {0.0});
    wgpu::BindGroup uniformBG =
        utils::MakeBindGroup(device, uniformPipeline.GetBindGroupLayout(0), {{0, uniformBuffer}});

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor spDesc;
    spDesc.vertex.module = module;
    spDesc.vertex.entryPoint = "vs";
    spDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    spDesc.cFragment.module = module;
    spDesc.cFragment.entryPoint = "fsStorage";
    spDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* spDescDS = spDesc.EnableDepthStencil(kDepthFormat);
    spDescDS->depthWriteEnabled = true;
    spDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline storagePipeline = device.CreateRenderPipeline(&spDesc);

    wgpu::Buffer storageBuffer =
        utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Storage, {1.0});
    wgpu::BindGroup storageBG =
        utils::MakeBindGroup(device, storagePipeline.GetBindGroupLayout(0), {{0, storageBuffer}});

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw two point with a different pipeline layout to check Vulkan's behavior.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetViewport(0, 0, 1, 1, 0.0, 0.5);

    // Writes 0.0.
    pass.SetPipeline(uniformPipeline);
    pass.SetBindGroup(0, uniformBG);
    pass.Draw(1);

    // Writes 1.0 clamped to 0.5.
    pass.SetPipeline(storagePipeline);
    pass.SetBindGroup(0, storageBG);
    pass.Draw(1);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// Check that if the fragment is outside of the viewport during rasterization, it is clipped
// even if it output @builtin(frag_depth).
TEST_P(FragDepthTests, RasterizationClipBeforeFS) {
    // TODO(dawn:1616): Metal too needs to clamping of @builtin(frag_depth) to the viewport.
    DAWN_SUPPRESS_TEST_IF(IsMetal());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 5.0, 1.0);
        }

        @fragment fn fs() -> @builtin(frag_depth) f32 {
            return 0.5;
        }
    )");

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.vertex.entryPoint = "vs";
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.entryPoint = "fs";
    pDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* pDescDS = pDesc.EnableDepthStencil(kDepthFormat);
    pDescDS->depthWriteEnabled = true;
    pDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline uniformPipeline = device.CreateRenderPipeline(&pDesc);

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.depthClearValue = 0.0f;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw a point with a depth outside of the viewport. It should get discarded.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetPipeline(uniformPipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The fragment should be discarded so the depth stayed 0.0, the depthClearValue.
    EXPECT_PIXEL_FLOAT_EQ(0.0f, depthTexture, 0, 0);
}

DAWN_INSTANTIATE_TEST(FragDepthTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
