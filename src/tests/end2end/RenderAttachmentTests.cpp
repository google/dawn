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
#include "utils/WGPUHelpers.h"

class RenderAttachmentTest : public DawnTest {};

// Test that it is ok to have more fragment outputs than color attachments.
// There should be no backend validation errors or indexing out-of-bounds.
TEST_P(RenderAttachmentTest, MoreFragmentOutputsThanAttachments) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]]
        fn main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Output {
            [[location(0)]] color0 : vec4<f32>;
            [[location(1)]] color1 : vec4<f32>;
            [[location(2)]] color2 : vec4<f32>;
            [[location(3)]] color3 : vec4<f32>;
        };

        [[stage(fragment)]]
        fn main() -> Output {
            var output : Output;
            output.color0 = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            output.color1 = vec4<f32>(0.0, 1.0, 0.0, 1.0);
            output.color2 = vec4<f32>(0.0, 0.0, 1.0, 1.0);
            output.color3 = vec4<f32>(1.0, 1.0, 0.0, 1.0);
            return output;
        })");

    // Fragment outputs 1, 2, 3 are written in the shader, but unused by the pipeline.
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = vsModule;
    pipelineDesc.cFragment.module = fsModule;
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDesc.cFragment.targetCount = 1;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::TextureDescriptor textureDesc;
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    wgpu::Texture renderTarget = device.CreateTexture(&textureDesc);
    utils::ComboRenderPassDescriptor renderPass({renderTarget.CreateView()});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderTarget, 0, 0);
}

DAWN_INSTANTIATE_TEST(RenderAttachmentTest,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
