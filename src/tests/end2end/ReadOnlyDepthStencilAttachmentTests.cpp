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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TextureUtils.h"
#include "utils/WGPUHelpers.h"

constexpr static uint32_t kSize = 4;

namespace {
    using TextureFormat = wgpu::TextureFormat;
    DAWN_TEST_PARAM_STRUCT(ReadOnlyDepthStencilAttachmentTestsParams, TextureFormat)
}  // namespace

class ReadOnlyDepthStencilAttachmentTests
    : public DawnTestWithParams<ReadOnlyDepthStencilAttachmentTestsParams> {
  protected:
    wgpu::RenderPipeline CreateRenderPipeline(wgpu::TextureFormat format) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        // Draw a rectangle via two triangles. The depth value of the top of the rectangle is 0.4.
        // The depth value of the bottom is 0.0. The depth value gradually change from 0.4 to 0.0
        // from the top to the bottom. The top part will compare with the depth values and fail to
        // pass the depth test. The bottom part will compare with the depth values in depth buffer
        // and pass the depth test, and sample from the depth buffer in fragment shader in the same
        // pipeline.
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            [[stage(vertex)]]
            fn main([[builtin(vertex_index)]] VertexIndex : u32) -> [[builtin(position)]] vec4<f32> {
                var pos = array<vec3<f32>, 6>(
                    vec3<f32>(-1.0,  1.0, 0.4),
                    vec3<f32>(-1.0, -1.0, 0.0),
                    vec3<f32>( 1.0,  1.0, 0.4),
                    vec3<f32>( 1.0,  1.0, 0.4),
                    vec3<f32>(-1.0, -1.0, 0.0),
                    vec3<f32>( 1.0, -1.0, 0.0));
                return vec4<f32>(pos[VertexIndex], 1.0);
            })");

        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var samp : sampler;
        [[group(0), binding(1)]] var tex : texture_depth_2d;

        [[stage(fragment)]]
        fn main([[builtin(position)]] FragCoord : vec4<f32>) -> [[location(0)]] vec4<f32> {
            return vec4<f32>(textureSample(tex, samp, FragCoord.xy), 0.0, 0.0, 0.0);
        })");

        // Enable depth test. But depth write is not enabled.
        wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil(format);
        depthStencil->depthCompare = wgpu::CompareFunction::LessEqual;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::Texture CreateTexture(wgpu::TextureFormat format, wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor descriptor = {};
        descriptor.size = {kSize, kSize, 1};
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    void TestDepth(wgpu::TextureFormat format, wgpu::Texture colorTexture) {
        wgpu::Texture depthStencilTexture = CreateTexture(
            format, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);

        // Note that we can only select one single aspect for texture view used in pipeline.
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        wgpu::TextureView depthStencilViewInPipeline = depthStencilTexture.CreateView(&viewDesc);

        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::RenderPipeline pipeline = CreateRenderPipeline(format);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, sampler}, {1, depthStencilViewInPipeline}});
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Create a render pass to initialize the depth attachment.
        // Note that we must encompass all aspects for texture view used in attachment.
        wgpu::TextureView depthStencilViewInAttachment = depthStencilTexture.CreateView();
        utils::ComboRenderPassDescriptor passDescriptorInit({}, depthStencilViewInAttachment);
        passDescriptorInit.cDepthStencilAttachmentInfo.clearDepth = 0.2;
        wgpu::RenderPassEncoder passInit = commandEncoder.BeginRenderPass(&passDescriptorInit);
        passInit.EndPass();

        // Create a render pass with readonly depth attachment. The readonly depth attachment
        // has already been initialized. The pipeline in this render pass will sample from the
        // depth attachment. The pipeline will read from the depth attachment to do depth test too.
        utils::ComboRenderPassDescriptor passDescriptor({colorTexture.CreateView()},
                                                        depthStencilViewInAttachment);
        passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
        // Set stencilReadOnly if the format has both depth and stencil aspects.
        if (format == wgpu::TextureFormat::Depth24PlusStencil8) {
            passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
            passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
        }
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.EndPass();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }
};

TEST_P(ReadOnlyDepthStencilAttachmentTests, Depth) {
    wgpu::Texture colorTexture =
        CreateTexture(wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    wgpu::TextureFormat depthStencilFormat = GetParam().mTextureFormat;
    TestDepth(depthStencilFormat, colorTexture);

    // The top part is not rendered by the pipeline. Its color is the default clear color for
    // color attachment.
    const std::vector<RGBA8> kExpectedTopColors(kSize * kSize / 2, {0, 0, 0, 0});
    // The bottom part is rendered, whose red channel is sampled from depth attachment, which
    // is initialized into 0.2.
    const std::vector<RGBA8> kExpectedBottomColors(kSize * kSize / 2,
                                                   {static_cast<uint8_t>(0.2 * 255), 0, 0, 0});
    EXPECT_TEXTURE_EQ(kExpectedTopColors.data(), colorTexture, {0, 0}, {kSize, kSize / 2});
    EXPECT_TEXTURE_EQ(kExpectedBottomColors.data(), colorTexture, {0, kSize / 2},
                      {kSize, kSize / 2});
}

DAWN_INSTANTIATE_TEST_P(ReadOnlyDepthStencilAttachmentTests,
                        {D3D12Backend()},
                        std::vector<wgpu::TextureFormat>(utils::kDepthStencilFormats.begin(),
                                                         utils::kDepthStencilFormats.end()));
