// Copyright 2017 The NXT Authors
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

#include "tests/NXTTest.h"

#include "utils/NXTHelpers.h"

class ViewportOrientationTests : public NXTTest {};

// Test that the pixel in viewport coordinate (-1, -1) matches texel (0, 0)
TEST_P(ViewportOrientationTests, OriginAt0x0) {
    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 2, 2);

    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        void main() {
            gl_Position = vec4(-0.5f, -0.5f, 0.0f, 1.0f);
        })");

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        })");

    nxt::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
        .SetSubpass(fb.renderPass, 0)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetPrimitiveTopology(nxt::PrimitiveTopology::PointList)
        .GetResult();

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
        .BeginRenderSubpass()
        .SetRenderPipeline(pipeline)
        .DrawArrays(1, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), fb.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 0, 1);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), fb.color, 1, 1);
}

NXT_INSTANTIATE_TEST(ViewportOrientationTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
