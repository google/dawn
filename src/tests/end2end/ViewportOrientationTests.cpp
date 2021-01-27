// Copyright 2017 The Dawn Authors
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

class ViewportOrientationTests : public DawnTest {};

// Test that the pixel in viewport coordinate (-1, -1) matches texel (0, 0)
TEST_P(ViewportOrientationTests, OriginAt0x0) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 2, 2);

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            Position = vec4<f32>(-0.5, 0.5, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor = vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;
    descriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
    descriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 0, 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kZero, renderPass.color, 1, 1);
}

DAWN_INSTANTIATE_TEST(ViewportOrientationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
