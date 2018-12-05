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

#include "common/Assert.h"
#include "utils/DawnHelpers.h"

constexpr static unsigned int kRTSize = 64;

class DepthStencilStateTest : public DawnTest {
    protected:
        void SetUp() override {
            DawnTest::SetUp();

            dawn::TextureDescriptor renderTargetDescriptor;
            renderTargetDescriptor.dimension = dawn::TextureDimension::e2D;
            renderTargetDescriptor.size.width = kRTSize;
            renderTargetDescriptor.size.height = kRTSize;
            renderTargetDescriptor.size.depth = 1;
            renderTargetDescriptor.arrayLayer = 1;
            renderTargetDescriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
            renderTargetDescriptor.levelCount = 1;
            renderTargetDescriptor.usage = dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::TransferSrc;
            renderTarget = device.CreateTexture(&renderTargetDescriptor);

            renderTargetView = renderTarget.CreateDefaultTextureView();

            dawn::TextureDescriptor depthDescriptor;
            depthDescriptor.dimension = dawn::TextureDimension::e2D;
            depthDescriptor.size.width = kRTSize;
            depthDescriptor.size.height = kRTSize;
            depthDescriptor.size.depth = 1;
            depthDescriptor.arrayLayer = 1;
            depthDescriptor.format = dawn::TextureFormat::D32FloatS8Uint;
            depthDescriptor.levelCount = 1;
            depthDescriptor.usage = dawn::TextureUsageBit::OutputAttachment;
            depthTexture = device.CreateTexture(&depthDescriptor);

            depthTextureView = depthTexture.CreateDefaultTextureView();

            renderpass = device.CreateRenderPassDescriptorBuilder()
                .SetColorAttachment(0, renderTargetView, dawn::LoadOp::Clear)
                .SetDepthStencilAttachment(depthTextureView, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
                .GetResult();

            vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
                #version 450
                layout(set = 0, binding = 0) uniform myBlock {
                    vec3 color;
                    float depth;
                } myUbo;
                void main() {
                    const vec2 pos[6] = vec2[6](
                        vec2(-1.f, 1.f), vec2(-1.f, -1.f), vec2(1.f, -1.f), // front-facing
                        vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(1.f, -1.f)    // back-facing
                    );
                    gl_Position = vec4(pos[gl_VertexIndex], myUbo.depth, 1.f);
                }
            )");

            fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
                #version 450
                layout(set = 0, binding = 0) uniform myBlock {
                    vec3 color;
                    float depth;
                } myUbo;
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(myUbo.color, 1.f);
                }
            )");

            bindGroupLayout = utils::MakeBindGroupLayout(
                device, {
                            {0, dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment,
                             dawn::BindingType::UniformBuffer},
                        });

            pipelineLayout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
        }

        struct TestSpec {
            const dawn::DepthStencilState& depthStencilState;
            RGBA8 color;
            float depth;
            uint32_t stencil;
        };

        // Check whether a depth comparison function works as expected
        // The less, equal, greater booleans denote wether the respective triangle should be visible based on the comparison function
        void CheckDepthCompareFunction(dawn::CompareFunction compareFunction, bool less, bool equal, bool greater) {
            dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
                .SetDepthCompareFunction(dawn::CompareFunction::Always)
                .SetDepthWriteEnabled(true)
                .GetResult();

            dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
                .SetDepthCompareFunction(compareFunction)
                .SetDepthWriteEnabled(true)
                .GetResult();

            RGBA8 baseColor = RGBA8(255, 255, 255, 255);
            RGBA8 lessColor = RGBA8(255, 0, 0, 255);
            RGBA8 equalColor = RGBA8(0, 255, 0, 255);
            RGBA8 greaterColor = RGBA8(0, 0, 255, 255);

            // Base triangle at depth 0.5, depth always, depth write enabled
            TestSpec base = { baseState, baseColor, 0.5f, 0u };

            // Draw the base triangle, then a triangle in front of the base triangle with the given depth comparison function
            DoTest({ base, { state, lessColor, 0.f, 0u } }, less ? lessColor : baseColor);

            // Draw the base triangle, then a triangle in at the same depth as the base triangle with the given depth comparison function
            DoTest({ base, { state, equalColor, 0.5f, 0u } }, equal ? equalColor : baseColor);

            // Draw the base triangle, then a triangle behind the base triangle with the given depth comparison function
            DoTest({ base, { state, greaterColor, 1.0f, 0u } }, greater ? greaterColor :  baseColor);
        }

        // Check whether a stencil comparison function works as expected
        // The less, equal, greater booleans denote wether the respective triangle should be visible based on the comparison function
        void CheckStencilCompareFunction(dawn::CompareFunction compareFunction, bool less, bool equal, bool greater) {
            dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
                .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
                .GetResult();

            dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
                .SetStencilFunction(dawn::Face::Both, compareFunction, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep)
                .GetResult();

            RGBA8 baseColor = RGBA8(255, 255, 255, 255);
            RGBA8 lessColor = RGBA8(255, 0, 0, 255);
            RGBA8 equalColor = RGBA8(0, 255, 0, 255);
            RGBA8 greaterColor = RGBA8(0, 0, 255, 255);

            // Base triangle with stencil reference 1
            TestSpec base = { baseState, baseColor, 0.0f, 1u };

            // Draw the base triangle, then a triangle with stencil reference 0 with the given stencil comparison function
            DoTest({ base, { state, lessColor, 0.f, 0u } }, less ? lessColor : baseColor);

            // Draw the base triangle, then a triangle with stencil reference 1 with the given stencil comparison function
            DoTest({ base, { state, equalColor, 0.f, 1u } }, equal ? equalColor : baseColor);

            // Draw the base triangle, then a triangle with stencil reference 2 with the given stencil comparison function
            DoTest({ base, { state, greaterColor, 0.f, 2u } }, greater ? greaterColor : baseColor);
        }

        // Given the provided `initialStencil` and `reference`, check that applying the `stencilOperation` produces the `expectedStencil`
        void CheckStencilOperation(dawn::StencilOperation stencilOperation, uint32_t initialStencil, uint32_t reference, uint32_t expectedStencil) {
            dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
                .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
                .GetResult();

            dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
                .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, stencilOperation)
                .GetResult();

            CheckStencil({
                // Wipe the stencil buffer with the initialStencil value
                { baseState, RGBA8(255, 255, 255, 255), 0.f, initialStencil },

                // Draw a triangle with the provided stencil operation and reference
                { state, RGBA8(255, 0, 0, 255), 0.f, reference },
            }, expectedStencil);
        }

        // Draw a list of test specs, and check if the stencil value is equal to the expected value
        void CheckStencil(std::vector<TestSpec> testParams, uint32_t expectedStencil) {
            dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
                .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Equal, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep)
                .GetResult();

            testParams.push_back({ state, RGBA8(0, 255, 0, 255), 0, expectedStencil });
            DoTest(testParams, RGBA8(0, 255, 0, 255));
        }

        // Each test param represents a pair of triangles with a color, depth, stencil value, and depthStencil state, one frontfacing, one backfacing
        // Draw the triangles in order and check the expected colors for the frontfaces and backfaces
        void DoTest(const std::vector<TestSpec> &testParams, const RGBA8& expectedFront, const RGBA8& expectedBack) {
            dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();

            struct TriangleData {
                float color[3];
                float depth;
            };

            dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);

            for (size_t i = 0; i < testParams.size(); ++i) {
                const TestSpec& test = testParams[i];

                TriangleData data = {
                    {  static_cast<float>(test.color.r) / 255.f, static_cast<float>(test.color.g) / 255.f, static_cast<float>(test.color.b) / 255.f },
                    test.depth,
                };
                // Upload a buffer for each triangle's depth and color data
                dawn::Buffer buffer = utils::CreateBufferFromData(device, &data, sizeof(TriangleData), dawn::BufferUsageBit::Uniform);

                dawn::BufferView view = buffer.CreateBufferViewBuilder()
                    .SetExtent(0, sizeof(TriangleData))
                    .GetResult();

                // Create a bind group for the data
                dawn::BindGroup bindGroup = utils::MakeBindGroup(device, bindGroupLayout, {{0, view}});

                // Create a pipeline for the triangles with the test spec's depth stencil state
                dawn::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
                    .SetColorAttachmentFormat(0, dawn::TextureFormat::R8G8B8A8Unorm)
                    .SetDepthStencilAttachmentFormat(dawn::TextureFormat::D32FloatS8Uint)
                    .SetLayout(pipelineLayout)
                    .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
                    .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
                    .SetDepthStencilState(test.depthStencilState)
                    .GetResult();

                pass.SetRenderPipeline(pipeline);
                pass.SetStencilReference(test.stencil);  // Set the stencil reference
                pass.SetBindGroup(0, bindGroup);         // Set the bind group which contains color and depth data
                pass.DrawArrays(6, 1, 0, 0);
            }
            pass.EndPass();

            dawn::CommandBuffer commands = builder.GetResult();
            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(expectedFront, renderTarget, kRTSize / 4, kRTSize / 2) << "Front face check failed";
            EXPECT_PIXEL_RGBA8_EQ(expectedBack, renderTarget, 3 * kRTSize / 4, kRTSize / 2) << "Back face check failed";
        }

        void DoTest(const std::vector<TestSpec> &testParams, const RGBA8& expected) {
            DoTest(testParams, expected, expected);
        }

        dawn::RenderPassDescriptor renderpass;
        dawn::Texture renderTarget;
        dawn::Texture depthTexture;
        dawn::TextureView renderTargetView;
        dawn::TextureView depthTextureView;
        dawn::ShaderModule vsModule;
        dawn::ShaderModule fsModule;
        dawn::BindGroupLayout bindGroupLayout;
        dawn::PipelineLayout pipelineLayout;
};

// Test compilation and usage of the fixture
TEST_P(DepthStencilStateTest, Basic) {
    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .GetResult();

    DoTest({
        { state, RGBA8(0, 255, 0, 255), 0.5f, 0u },
    }, RGBA8(0, 255, 0, 255));
}

// Test defaults: depth and stencil tests disabled
TEST_P(DepthStencilStateTest, DepthStencilDisabled) {
    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .GetResult();

    TestSpec specs[3] = {
        { state, RGBA8(255, 0, 0, 255), 0.0f, 0u },
        { state, RGBA8(0, 255, 0, 255), 0.5f, 0u },
        { state, RGBA8(0, 0, 255, 255), 1.0f, 0u },
    };

    // Test that for all combinations, the last triangle drawn is the one visible
    // We check against three triangles because the stencil test may modify results
    for (uint32_t last = 0; last < 3; ++last) {
        uint32_t i = (last + 1) % 3;
        uint32_t j = (last + 2) % 3;
        DoTest({ specs[i], specs[j], specs[last] }, specs[last].color);
        DoTest({ specs[j], specs[i], specs[last] }, specs[last].color);
    }
}

// The following tests check that each depth comparison function works
TEST_P(DepthStencilStateTest, DepthAlways) {
    CheckDepthCompareFunction(dawn::CompareFunction::Always , true, true, true);
}

TEST_P(DepthStencilStateTest, DepthEqual) {
    CheckDepthCompareFunction(dawn::CompareFunction::Equal, false, true, false);
}

TEST_P(DepthStencilStateTest, DepthGreater) {
    CheckDepthCompareFunction(dawn::CompareFunction::Greater, false, false, true);
}

TEST_P(DepthStencilStateTest, DepthGreaterEqual) {
    CheckDepthCompareFunction(dawn::CompareFunction::GreaterEqual, false, true, true);
}

TEST_P(DepthStencilStateTest, DepthLess) {
    CheckDepthCompareFunction(dawn::CompareFunction::Less, true, false, false);
}

TEST_P(DepthStencilStateTest, DepthLessEqual) {
    CheckDepthCompareFunction(dawn::CompareFunction::LessEqual, true, true, false);
}

TEST_P(DepthStencilStateTest, DepthNever) {
    CheckDepthCompareFunction(dawn::CompareFunction::Never, false, false, false);
}

TEST_P(DepthStencilStateTest, DepthNotEqual) {
    CheckDepthCompareFunction(dawn::CompareFunction::NotEqual, true, false, true);
}

// Test that disabling depth writes works and leaves the depth buffer unchanged
TEST_P(DepthStencilStateTest, DepthWriteDisabled) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Always)
        .SetDepthWriteEnabled(true)
        .GetResult();

    dawn::DepthStencilState noDepthWrite = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Always)
        .SetDepthWriteEnabled(false)
        .GetResult();

    dawn::DepthStencilState checkState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Equal)
        .GetResult();

    DoTest({
        { baseState, RGBA8(255, 255, 255, 255), 1.f, 0u }, // Draw a base triangle with depth enabled
        { noDepthWrite, RGBA8(0, 0, 0, 255), 0.f, 0u }, // Draw a second triangle without depth enabled
        { checkState, RGBA8(0, 255, 0, 255), 1.f, 0u }, // Draw a third triangle which should occlude the second even though it is behind it
    }, RGBA8(0, 255, 0, 255));
}

// The following tests check that each stencil comparison function works
TEST_P(DepthStencilStateTest, StencilAlways) {
    CheckStencilCompareFunction(dawn::CompareFunction::Always, true, true, true);
}

TEST_P(DepthStencilStateTest, StencilEqual) {
    CheckStencilCompareFunction(dawn::CompareFunction::Equal, false, true, false);
}

TEST_P(DepthStencilStateTest, StencilGreater) {
    CheckStencilCompareFunction(dawn::CompareFunction::Greater, false, false, true);
}

TEST_P(DepthStencilStateTest, StencilGreaterEqual) {
    CheckStencilCompareFunction(dawn::CompareFunction::GreaterEqual, false, true, true);
}

TEST_P(DepthStencilStateTest, StencilLess) {
    CheckStencilCompareFunction(dawn::CompareFunction::Less, true, false, false);
}

TEST_P(DepthStencilStateTest, StencilLessEqual) {
    CheckStencilCompareFunction(dawn::CompareFunction::LessEqual, true, true, false);
}

TEST_P(DepthStencilStateTest, StencilNever) {
    CheckStencilCompareFunction(dawn::CompareFunction::Never, false, false, false);
}

TEST_P(DepthStencilStateTest, StencilNotEqual) {
    CheckStencilCompareFunction(dawn::CompareFunction::NotEqual, true, false, true);
}

// The following tests check that each stencil operation works
TEST_P(DepthStencilStateTest, StencilKeep) {
    CheckStencilOperation(dawn::StencilOperation::Keep, 1, 3, 1);
}

TEST_P(DepthStencilStateTest, StencilZero) {
    CheckStencilOperation(dawn::StencilOperation::Zero, 1, 3, 0);
}

TEST_P(DepthStencilStateTest, StencilReplace) {
    CheckStencilOperation(dawn::StencilOperation::Replace, 1, 3, 3);
}

TEST_P(DepthStencilStateTest, StencilInvert) {
    CheckStencilOperation(dawn::StencilOperation::Invert, 0xf0, 3, 0x0f);
}

TEST_P(DepthStencilStateTest, StencilIncrementClamp) {
    CheckStencilOperation(dawn::StencilOperation::IncrementClamp, 1, 3, 2);
    CheckStencilOperation(dawn::StencilOperation::IncrementClamp, 0xff, 3, 0xff);
}

TEST_P(DepthStencilStateTest, StencilIncrementWrap) {
    CheckStencilOperation(dawn::StencilOperation::IncrementWrap, 1, 3, 2);
    CheckStencilOperation(dawn::StencilOperation::IncrementWrap, 0xff, 3, 0);
}

TEST_P(DepthStencilStateTest, StencilDecrementClamp) {
    CheckStencilOperation(dawn::StencilOperation::DecrementClamp, 1, 3, 0);
    CheckStencilOperation(dawn::StencilOperation::DecrementClamp, 0, 3, 0);
}

TEST_P(DepthStencilStateTest, StencilDecrementWrap) {
    CheckStencilOperation(dawn::StencilOperation::DecrementWrap, 1, 3, 0);
    CheckStencilOperation(dawn::StencilOperation::DecrementWrap, 0, 3, 0xff);
}

// Check that the setting a stencil read mask works
TEST_P(DepthStencilStateTest, StencilReadMask) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .GetResult();

    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Equal, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep)
        .SetStencilMask(0x2, 0xff)
        .GetResult();

    RGBA8 baseColor = RGBA8(255, 255, 255, 255);
    RGBA8 red = RGBA8(255, 0, 0, 255);
    RGBA8 green = RGBA8(0, 255, 0, 255);

    TestSpec base = { baseState, baseColor, 0.5f, 3u };     // Base triangle to set the stencil to 3
    DoTest({ base, { state, red, 0.f, 1u } }, baseColor);   // Triangle with stencil reference 1 and read mask 2 does not draw because (3 & 2 != 1)
    DoTest({ base, { state, green, 0.f, 2u } }, green);     // Triangle with stencil reference 2 and read mask 2 draws because (3 & 2 == 2)
}

// Check that setting a stencil write mask works
TEST_P(DepthStencilStateTest, StencilWriteMask) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .SetStencilMask(0xff, 0x1)
        .GetResult();

    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Equal, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep)
        .GetResult();

    RGBA8 baseColor = RGBA8(255, 255, 255, 255);
    RGBA8 green = RGBA8(0, 255, 0, 255);

    TestSpec base = { baseState, baseColor, 0.5f, 3u };         // Base triangle with stencil reference 3 and mask 1 to set the stencil 1
    DoTest({ base, { state, green, 0.f, 2u } }, baseColor);     // Triangle with stencil reference 2 does not draw because 2 != (3 & 1)
    DoTest({ base, { state, green, 0.f, 1u } }, green);         // Triangle with stencil reference 1 draws because 1 == (3 & 1)
}

// Test that the stencil operation is executed on stencil fail
TEST_P(DepthStencilStateTest, StencilFail) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .GetResult();

    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Less, dawn::StencilOperation::Replace, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep)
        .GetResult();

    CheckStencil({
        { baseState, RGBA8(255, 255, 255, 255), 1.f, 1 },   // Triangle to set stencil value to 1
        { state, RGBA8(0, 0, 0, 255), 0.f, 2 }              // Triangle with stencil reference 2 fails the Less comparison function
    }, 2);                                                  // Replace the stencil on failure, so it should be 2
}

// Test that the stencil operation is executed on stencil pass, depth fail
TEST_P(DepthStencilStateTest, StencilDepthFail) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .SetDepthWriteEnabled(true)
        .GetResult();

    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Greater, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace, dawn::StencilOperation::Keep)
        .SetDepthWriteEnabled(true)
        .SetDepthCompareFunction(dawn::CompareFunction::Less)
        .GetResult();

    CheckStencil({
        { baseState, RGBA8(255, 255, 255, 255), 0.f, 1 },   // Triangle to set stencil value to 1. Depth is 0
        { state, RGBA8(0, 0, 0, 255), 1.f, 2 } },           // Triangle with stencil reference 2 passes the Greater comparison function. At depth 1, it fails the Less depth test
    2);                                                     // Replace the stencil on stencil pass, depth failure, so it should be 2
}

// Test that the stencil operation is executed on stencil pass, depth pass
TEST_P(DepthStencilStateTest, StencilDepthPass) {
    dawn::DepthStencilState baseState = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .SetDepthWriteEnabled(true)
        .GetResult();

    dawn::DepthStencilState state = device.CreateDepthStencilStateBuilder()
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Greater, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .SetDepthWriteEnabled(true)
        .SetDepthCompareFunction(dawn::CompareFunction::Less)
        .GetResult();

    CheckStencil({
        { baseState, RGBA8(255, 255, 255, 255), 1.f, 1 },   // Triangle to set stencil value to 1. Depth is 0
        { state, RGBA8(0, 0, 0, 255), 0.f, 2 } },           // Triangle with stencil reference 2 passes the Greater comparison function. At depth 0, it pass the Less depth test
2);                                                         // Replace the stencil on stencil pass, depth pass, so it should be 2
}

DAWN_INSTANTIATE_TEST(DepthStencilStateTest,
                     D3D12Backend,
                     MetalBackend,
                     OpenGLBackend,
                     VulkanBackend)
