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

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "tests/DawnTest.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

// Vertex buffer robustness tests that clamping is applied on vertex attributes. This would happen
// on backends where vertex pulling is enabled, such as Metal.
class VertexBufferRobustnessTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
    }

    // Creates a vertex module that tests an expression with given attributes. If successful, the
    // point drawn would be moved out of the viewport. On failure, the point is kept inside the
    // viewport.
    wgpu::ShaderModule CreateVertexModule(const std::string& attributes,
                                          const std::string& successExpression) {
        return utils::CreateShaderModuleFromWGSL(device, (attributes + R"(
                [[builtin(position)]] var<out> Position : vec4<f32>;

                [[stage(vertex)]] fn main() -> void {
                    if ()" + successExpression + R"() {
                        // Success case, move the vertex out of the viewport
                        Position = vec4<f32>(-10.0, 0.0, 0.0, 1.0);
                    } else {
                        // Failure case, move the vertex inside the viewport
                        Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                    }
                    return;
                }
            )")
                                                             .c_str());
    }

    // Runs the test, a true |expectation| meaning success
    void DoTest(const std::string& attributes,
                const std::string& successExpression,
                const utils::ComboVertexStateDescriptor& vertexState,
                wgpu::Buffer vertexBuffer,
                uint64_t bufferOffset,
                bool expectation) {
        wgpu::ShaderModule vsModule = CreateVertexModule(attributes, successExpression);
        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
                [[location(0)]] var<out> outColor : vec4<f32>;

                [[stage(fragment)]] fn main() -> void {
                    outColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
                    return;
                }
            )");

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        descriptor.vertexState = &vertexState;
        descriptor.cColorStates[0].format = renderPass.colorFormat;
        renderPass.renderPassInfo.cColorAttachments[0].clearColor = {0, 0, 0, 1};

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer, bufferOffset);
        pass.Draw(1000);
        pass.EndPass();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        RGBA8 noOutput(0, 0, 0, 255);
        RGBA8 someOutput(255, 255, 255, 255);
        EXPECT_PIXEL_RGBA8_EQ(expectation ? noOutput : someOutput, renderPass.color, 0, 0);
    }
};

TEST_P(VertexBufferRobustnessTest, DetectInvalidValues) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(float);
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Float;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 0, so we see 111.0, leading to failure
    float kVertices[] = {111.0, 473.0, 473.0};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : f32;", "a == 473.0", vertexState, vertexBuffer, 0, false);
}

TEST_P(VertexBufferRobustnessTest, FloatClamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(float);
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Float;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 4, so we clamp to only values containing 473.0
    float kVertices[] = {111.0, 473.0, 473.0};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : f32;", "a == 473.0", vertexState, vertexBuffer, 4, true);
}

TEST_P(VertexBufferRobustnessTest, IntClamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(int32_t);
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Int;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 4, so we clamp to only values containing 473
    int32_t kVertices[] = {111, 473, 473};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : i32;", "a == 473", vertexState, vertexBuffer, 4, true);
}

TEST_P(VertexBufferRobustnessTest, UIntClamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(uint32_t);
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::UInt;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 4, so we clamp to only values containing 473
    uint32_t kVertices[] = {111, 473, 473};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : u32;", "a == 473", vertexState, vertexBuffer, 4, true);
}

TEST_P(VertexBufferRobustnessTest, Float2Clamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(float) * 2;
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Float2;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 8, so we clamp to only values containing 473.0
    float kVertices[] = {111.0, 111.0, 473.0, 473.0};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : vec2<f32>;", "a[0] == 473.0 && a[1] == 473.0",
           std::move(vertexState), vertexBuffer, 8, true);
}

TEST_P(VertexBufferRobustnessTest, Float3Clamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(float) * 3;
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Float3;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 12, so we clamp to only values containing 473.0
    float kVertices[] = {111.0, 111.0, 111.0, 473.0, 473.0, 473.0};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : vec3<f32>;",
           "a[0] == 473.0 && a[1] == 473.0 && a[2] == 473.0", vertexState, vertexBuffer, 12, true);
}

TEST_P(VertexBufferRobustnessTest, Float4Clamp) {
    utils::ComboVertexStateDescriptor vertexState;
    vertexState.vertexBufferCount = 1;
    vertexState.cVertexBuffers[0].arrayStride = sizeof(float) * 4;
    vertexState.cVertexBuffers[0].attributeCount = 1;
    vertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
    vertexState.cAttributes[0].offset = 0;
    vertexState.cAttributes[0].shaderLocation = 0;

    // Bind at an offset of 16, so we clamp to only values containing 473.0
    float kVertices[] = {111.0, 111.0, 111.0, 111.0, 473.0, 473.0, 473.0, 473.0};
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                            wgpu::BufferUsage::Vertex);

    DoTest("[[location(0)]] var<in> a : vec4<f32>;",
           "a[0] == 473.0 && a[1] == 473.0 && a[2] == 473.0 && a[3] == 473.0", vertexState,
           vertexBuffer, 16, true);
}

DAWN_INSTANTIATE_TEST(VertexBufferRobustnessTest, MetalBackend({"metal_enable_vertex_pulling"}));
