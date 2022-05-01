// Copyright 2021 The Dawn Authors
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

#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

#include "dawn/tests/unittests/validation/ValidationTest.h"

constexpr static uint32_t kSize = 4;
// Note that format Depth24PlusStencil8 has both depth and stencil aspects, so parameters
// depthReadOnly and stencilReadOnly should be the same in render pass and render bundle.
wgpu::TextureFormat kFormat = wgpu::TextureFormat::Depth24PlusStencil8;

namespace {

class RenderPipelineAndPassCompatibilityTests : public ValidationTest {
  public:
    wgpu::RenderPipeline CreatePipeline(wgpu::TextureFormat format,
                                        bool enableDepthWrite,
                                        bool enableStencilWrite) {
        // Create a NoOp pipeline
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
                @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
                    return vec4<f32>();
                })");
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @stage(fragment) fn main() {
                })");
        pipelineDescriptor.cFragment.targets = nullptr;
        pipelineDescriptor.cFragment.targetCount = 0;

        // Enable depth/stencil write if needed
        wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil(format);
        if (enableDepthWrite) {
            depthStencil->depthWriteEnabled = true;
        }
        if (enableStencilWrite) {
            depthStencil->stencilFront.failOp = wgpu::StencilOperation::Replace;
        }
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    utils::ComboRenderPassDescriptor CreateRenderPassDescriptor(wgpu::TextureFormat format,
                                                                bool depthReadOnly,
                                                                bool stencilReadOnly) {
        wgpu::TextureDescriptor textureDescriptor = {};
        textureDescriptor.size = {kSize, kSize, 1};
        textureDescriptor.format = format;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture depthStencilTexture = device.CreateTexture(&textureDescriptor);

        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        if (depthReadOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        }

        if (stencilReadOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        }

        return passDescriptor;
    }
};

// Test depthWrite/stencilWrite in DepthStencilState in render pipeline vs
// depthReadOnly/stencilReadOnly in DepthStencilAttachment in render pass.
TEST_F(RenderPipelineAndPassCompatibilityTests, WriteAndReadOnlyConflictForDepthStencil) {
    for (bool depthStencilReadOnlyInPass : {true, false}) {
        for (bool depthWriteInPipeline : {true, false}) {
            for (bool stencilWriteInPipeline : {true, false}) {
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                utils::ComboRenderPassDescriptor passDescriptor = CreateRenderPassDescriptor(
                    kFormat, depthStencilReadOnlyInPass, depthStencilReadOnlyInPass);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
                wgpu::RenderPipeline pipeline =
                    CreatePipeline(kFormat, depthWriteInPipeline, stencilWriteInPipeline);
                pass.SetPipeline(pipeline);
                pass.Draw(3);
                pass.End();
                if (depthStencilReadOnlyInPass &&
                    (depthWriteInPipeline || stencilWriteInPipeline)) {
                    ASSERT_DEVICE_ERROR(encoder.Finish());
                } else {
                    encoder.Finish();
                }
            }
        }
    }
}

// Test depthWrite/stencilWrite in DepthStencilState in render pipeline vs
// depthReadOnly/stencilReadOnly in RenderBundleEncoderDescriptor in render bundle.
TEST_F(RenderPipelineAndPassCompatibilityTests,
       WriteAndReadOnlyConflictForDepthStencilBetweenPipelineAndBundle) {
    for (bool depthStencilReadOnlyInBundle : {true, false}) {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.depthStencilFormat = kFormat;
        desc.depthReadOnly = depthStencilReadOnlyInBundle;
        desc.stencilReadOnly = depthStencilReadOnlyInBundle;

        for (bool depthWriteInPipeline : {true, false}) {
            for (bool stencilWriteInPipeline : {true, false}) {
                wgpu::RenderBundleEncoder renderBundleEncoder =
                    device.CreateRenderBundleEncoder(&desc);
                wgpu::RenderPipeline pipeline =
                    CreatePipeline(kFormat, depthWriteInPipeline, stencilWriteInPipeline);
                renderBundleEncoder.SetPipeline(pipeline);
                renderBundleEncoder.Draw(3);
                if (depthStencilReadOnlyInBundle &&
                    (depthWriteInPipeline || stencilWriteInPipeline)) {
                    ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
                } else {
                    renderBundleEncoder.Finish();
                }
            }
        }
    }
}

// Test depthReadOnly/stencilReadOnly in RenderBundleEncoderDescriptor in render bundle vs
// depthReadOnly/stencilReadOnly in DepthStencilAttachment in render pass.
TEST_F(RenderPipelineAndPassCompatibilityTests,
       WriteAndReadOnlyConflictForDepthStencilBetweenBundleAndPass) {
    for (bool depthStencilReadOnlyInPass : {true, false}) {
        for (bool depthStencilReadOnlyInBundle : {true, false}) {
            for (bool emptyBundle : {true, false}) {
                // Create render bundle, with or without a pipeline
                utils::ComboRenderBundleEncoderDescriptor desc = {};
                desc.depthStencilFormat = kFormat;
                desc.depthReadOnly = depthStencilReadOnlyInBundle;
                desc.stencilReadOnly = depthStencilReadOnlyInBundle;
                wgpu::RenderBundleEncoder renderBundleEncoder =
                    device.CreateRenderBundleEncoder(&desc);
                if (!emptyBundle) {
                    wgpu::RenderPipeline pipeline = CreatePipeline(
                        kFormat, !depthStencilReadOnlyInBundle, !depthStencilReadOnlyInBundle);
                    renderBundleEncoder.SetPipeline(pipeline);
                    renderBundleEncoder.Draw(3);
                }
                wgpu::RenderBundle bundle = renderBundleEncoder.Finish();

                // Create render pass and call ExecuteBundles()
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                utils::ComboRenderPassDescriptor passDescriptor = CreateRenderPassDescriptor(
                    kFormat, depthStencilReadOnlyInPass, depthStencilReadOnlyInPass);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
                pass.ExecuteBundles(1, &bundle);
                pass.End();
                if (!depthStencilReadOnlyInPass || depthStencilReadOnlyInBundle) {
                    encoder.Finish();
                } else {
                    ASSERT_DEVICE_ERROR(encoder.Finish());
                }
            }
        }
    }
}

// TODO(dawn:485): add more tests. For example:
//   - depth/stencil attachment should be designated if depth/stencil test is enabled.
//   - pipeline and pass compatibility tests for color attachment(s).
//   - pipeline and pass compatibility tests for compute.

}  // anonymous namespace
