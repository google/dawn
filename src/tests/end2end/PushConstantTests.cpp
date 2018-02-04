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

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/NXTHelpers.h"

#include <array>

class PushConstantTest: public NXTTest {
    protected:
        // Layout, bind group and friends to store results for compute tests, can have an extra buffer
        // so that two different pipeline layout can be created.
        struct TestBindings {
            nxt::PipelineLayout layout;
            nxt::BindGroup bindGroup;
            nxt::Buffer resultBuffer;
        };
        TestBindings MakeTestBindings(bool extraBuffer) {
            nxt::Buffer buf1 = device.CreateBufferBuilder()
                .SetSize(4)
                .SetAllowedUsage(nxt::BufferUsageBit::Storage | nxt::BufferUsageBit::TransferSrc | nxt::BufferUsageBit::TransferDst)
                .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
                .GetResult();
            uint32_t one = 1;
            buf1.SetSubData(0, sizeof(one), reinterpret_cast<uint8_t*>(&one));

            nxt::Buffer buf2 = device.CreateBufferBuilder()
                .SetSize(4)
                .SetAllowedUsage(nxt::BufferUsageBit::Storage)
                .GetResult();
            buf2.FreezeUsage(nxt::BufferUsageBit::Storage);

            nxt::ShaderStageBit kAllStages = nxt::ShaderStageBit::Compute | nxt::ShaderStageBit::Fragment | nxt::ShaderStageBit::Vertex;
            nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
                .SetBindingsType(kAllStages, nxt::BindingType::StorageBuffer, 0, extraBuffer ? 2 : 1)
                .GetResult();

            nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
                .SetBindGroupLayout(0, bgl)
                .GetResult();

            nxt::BufferView views[2] = {
                buf1.CreateBufferViewBuilder().SetExtent(0, 4).GetResult(),
                buf2.CreateBufferViewBuilder().SetExtent(0, 4).GetResult(),
            };

            nxt::BindGroup bg = device.CreateBindGroupBuilder()
                .SetLayout(bgl)
                .SetUsage(nxt::BindGroupUsage::Frozen)
                .SetBufferViews(0, extraBuffer ? 2 : 1, views)
                .GetResult();

            return {std::move(pl), std::move(bg), std::move(buf1)};
        }

        // A test spec is a bunch of push constant types and expected values
        enum PushConstantType {
            Float,
            Int,
            UInt,
        };
        struct PushConstantSpecItem {
            PushConstantType type;
            int value;
        };
        using PushConstantSpec = std::vector<PushConstantSpecItem>;

        PushConstantSpec MakeAllZeroSpec() const {
            PushConstantSpec allZeros;
            for (uint32_t i = 0; i < kMaxPushConstants; ++i) {
                allZeros.push_back({Int, 0});
            }
            return allZeros;
        }

        // The GLSL code to define the push constant block for a given test spec
        std::string MakePushConstantBlock(PushConstantSpec spec) {
            std::string block = "layout(push_constant) uniform ConstantsBlock {\n";
            for (size_t i = 0; i < spec.size(); ++i) {
                block += "    ";
                switch (spec[i].type) {
                    case Float:
                        block += "float";
                        break;
                    case Int:
                        block += "int";
                        break;
                    case UInt:
                        block += "uint";
                        break;
                }
                block += " val" + std::to_string(i) + ";\n";
            }
            block += "} c;\n";
            return block;
        }

        // The GLSL code to define the push constant test for a given test spec
        std::string MakePushConstantTest(PushConstantSpec spec, std::string varName) {
            std::string test = "bool " + varName + " = true;\n";
            for (size_t i = 0; i < spec.size(); ++i) {
                test += varName + " = " + varName + " && (c.val" + std::to_string(i) + " == ";
                switch (spec[i].type) {
                    case Float:
                        test += "float";
                        break;
                    case Int:
                        test += "int";
                        break;
                    case UInt:
                        test += "uint";
                        break;
                }
                test += "(" + std::to_string(spec[i].value) + "));\n";
            }
            return test;
        }

        // The compute pipeline ANDs the result of the test in the SSBO
        nxt::ComputePipeline MakeTestComputePipeline(const nxt::PipelineLayout& pl, PushConstantSpec spec) {
            nxt::ShaderModule module = utils::CreateShaderModule(device, nxt::ShaderStage::Compute, (R"(
                #version 450
                layout(set = 0, binding = 0) buffer Result {
                    int success;
                } result;
                )" + MakePushConstantBlock(spec) + R"(
                void main() {
                    )" + MakePushConstantTest(spec, "success") + R"(
                    if (success && result.success == 1) {
                        result.success = 1;
                    } else {
                        result.success = 0;
                    }
                })").c_str()
            );

            return device.CreateComputePipelineBuilder()
                .SetLayout(pl)
                .SetStage(nxt::ShaderStage::Compute, module, "main")
                .GetResult();
        }

        nxt::PipelineLayout MakeEmptyLayout() {
            return device.CreatePipelineLayoutBuilder().GetResult();
        }

        // The render pipeline adds one to the red channel for successful vertex push constant test
        // and adds one to green for the frgament test.
        nxt::RenderPipeline MakeTestRenderPipeline(nxt::PipelineLayout& layout, nxt::RenderPass& renderPass, uint32_t subpass,
                                                   PushConstantSpec vsSpec, PushConstantSpec fsSpec) {
            nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, (R"(
                #version 450
                )" + MakePushConstantBlock(vsSpec) + R"(
                layout(location = 0) out float red;
                void main() {
                    red = 0.0f;
                    )" + MakePushConstantTest(vsSpec, "success") + R"(
                    if (success) {red = 1.0f / 255.0f;}
                    gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                })").c_str()
            );
            nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, (R"(
                #version 450
                )" + MakePushConstantBlock(fsSpec) + R"(
                layout(location = 0) out vec4 color;
                layout(location = 0) in float red;
                void main() {
                    color = vec4(red, 0.0f, 0.0f, 0.0f);
                    )" + MakePushConstantTest(fsSpec, "success") + R"(
                    if (success) {color.g = 1.0f / 255.0f;}
                })").c_str()
            );

            nxt::BlendState blendState = device.CreateBlendStateBuilder()
                .SetBlendEnabled(true)
                .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
                .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
                .GetResult();

            return device.CreateRenderPipelineBuilder()
                .SetSubpass(renderPass, subpass)
                .SetLayout(layout)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetPrimitiveTopology(nxt::PrimitiveTopology::PointList)
                .SetColorAttachmentBlendState(0, blendState)
                .GetResult();
        }
};

// Test that push constants default to zero at the beginning of every compute passes.
TEST_P(PushConstantTest, ComputePassDefaultsToZero) {
    auto binding = MakeTestBindings(false);

    // Expect push constants to be zero in all dispatches of this test.
    nxt::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, MakeAllZeroSpec());

    uint32_t notZero = 42;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(binding.resultBuffer, nxt::BufferUsageBit::Storage)
        .BeginComputePass()
            // Test compute push constants are set to zero by default.
            .SetComputePipeline(pipeline)
            .SetBindGroup(0, binding.bindGroup)
            .Dispatch(1, 1, 1)
            // Set push constants to non-zero value to check they will be reset to zero
            // on the next BeginComputePass
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, &notZero)
        .EndComputePass()
        .BeginComputePass()
            .SetComputePipeline(pipeline)
            .SetBindGroup(0, binding.bindGroup)
            .Dispatch(1, 1, 1)
        .EndComputePass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding.resultBuffer, 0);
}

// Test that push constants default to zero at the beginning of every render subpasses.
TEST_P(PushConstantTest, RenderSubpassDefaultsToZero) {
    // Change the renderpass to be a two subpass renderpass just for this test.
    nxt::RenderPass renderPass = device.CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .AttachmentSetColorLoadOp(0, nxt::LoadOp::Clear)
        .SetSubpassCount(2)
        .SubpassSetColorAttachment(0, 0, 0)
        .SubpassSetColorAttachment(1, 0, 0)
        .GetResult();

    nxt::Texture renderTarget = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(1, 1, 1)
        .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::TransferSrc)
        .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
        .GetResult();

    nxt::TextureView renderTargetView = renderTarget.CreateTextureViewBuilder().GetResult();

    nxt::Framebuffer framebuffer = device.CreateFramebufferBuilder()
        .SetRenderPass(renderPass)
        .SetAttachment(0, renderTargetView)
        .SetDimensions(1, 1)
        .GetResult();

    // Expect push constants to be zero in all draws of this test.
    PushConstantSpec allZeros = MakeAllZeroSpec();
    nxt::PipelineLayout layout = MakeEmptyLayout();
    nxt::RenderPipeline pipeline1 = MakeTestRenderPipeline(layout, renderPass, 0, MakeAllZeroSpec(), MakeAllZeroSpec());
    nxt::RenderPipeline pipeline2 = MakeTestRenderPipeline(layout, renderPass, 1, MakeAllZeroSpec(), MakeAllZeroSpec());

    uint32_t notZero = 42;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderPass, framebuffer)
            .BeginRenderSubpass()
                // Test render push constants are set to zero by default.
                .SetRenderPipeline(pipeline1)
                .DrawArrays(1, 1, 0, 0)
                // Set push constants to non-zero value to check they will be reset to zero
                // on the next subpass. This tests both fragment and vertex as they write to different
                // color channels on error.
                .SetPushConstants(nxt::ShaderStageBit::Fragment | nxt::ShaderStageBit::Vertex, 0, 1, &notZero)
            .EndRenderSubpass()
            .BeginRenderSubpass()
                .SetRenderPipeline(pipeline2)
                .DrawArrays(1, 1, 0, 0)
            .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(2, 2, 0, 0), renderTarget, 0, 0);
}

// Test setting push constants of various 32bit types.
TEST_P(PushConstantTest, VariousConstantTypes) {

    struct {
        int32_t v1;
        uint32_t v2;
        float v3;
    } values = {-1, 3, 4.0f};
    static_assert(sizeof(values) == 3 * sizeof(uint32_t), "");

    auto binding = MakeTestBindings(false);
    PushConstantSpec spec = {{Int, -1}, {UInt, 3}, {Float, 4}};
    nxt::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, spec);


    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(binding.resultBuffer, nxt::BufferUsageBit::Storage)
        .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 3, reinterpret_cast<uint32_t*>(&values))
            .SetComputePipeline(pipeline)
            .SetBindGroup(0, binding.bindGroup)
            .Dispatch(1, 1, 1)
        .EndComputePass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding.resultBuffer, 0);
}

// Test that the push constants stay in between pipeline layout changes.
TEST_P(PushConstantTest, InheritThroughPipelineLayoutChange) {
    // These bindings will have a different pipeline layout because binding 2 has an extra buffer.
    auto binding1 = MakeTestBindings(false);
    auto binding2 = MakeTestBindings(true);
    PushConstantSpec spec1 = {{Int, 1}};
    PushConstantSpec spec2 = {{Int, 2}};
    nxt::ComputePipeline pipeline1 = MakeTestComputePipeline(binding1.layout, spec1);
    nxt::ComputePipeline pipeline2 = MakeTestComputePipeline(binding2.layout, spec2);

    uint32_t one = 1;
    uint32_t two = 2;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(binding1.resultBuffer, nxt::BufferUsageBit::Storage)
        .TransitionBufferUsage(binding2.resultBuffer, nxt::BufferUsageBit::Storage)
        .BeginComputePass()
            // Set Push constant before there is a pipeline set
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, &one)
            .SetComputePipeline(pipeline1)
            .SetBindGroup(0, binding1.bindGroup)
            .Dispatch(1, 1, 1)
            // Change the push constant before changing pipeline layout
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, &two)
            .SetComputePipeline(pipeline2)
            .SetBindGroup(0, binding2.bindGroup)
            .Dispatch(1, 1, 1)
        .EndComputePass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding1.resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(1, binding2.resultBuffer, 0);
}

// Try setting all push constants
TEST_P(PushConstantTest, SetAllConstantsToNonZero) {
    PushConstantSpec spec;
    std::array<uint32_t, kMaxPushConstants> values;
    for (uint32_t i = 0; i < kMaxPushConstants; ++i) {
        spec.push_back({Int, static_cast<int>(i + 1)});
        values[i] = i + 1;
    }

    auto binding = MakeTestBindings(false);
    nxt::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, spec);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(binding.resultBuffer, nxt::BufferUsageBit::Storage)
        .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, kMaxPushConstants, &values[0])
            .SetComputePipeline(pipeline)
            .SetBindGroup(0, binding.bindGroup)
            .Dispatch(1, 1, 1)
        .EndComputePass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding.resultBuffer, 0);
}

// Try setting separate push constants for vertex and fragment stage
TEST_P(PushConstantTest, SeparateVertexAndFragmentConstants) {
    PushConstantSpec vsSpec = {{Int, 1}};
    PushConstantSpec fsSpec = {{Int, 2}};

    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 1, 1);

    nxt::PipelineLayout layout = MakeEmptyLayout();
    nxt::RenderPipeline pipeline = MakeTestRenderPipeline(layout, fb.renderPass, 0, vsSpec, fsSpec);

    uint32_t one = 1;
    uint32_t two = 2;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
            .BeginRenderSubpass()
                .SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 1, &one)
                .SetPushConstants(nxt::ShaderStageBit::Fragment, 0, 1, &two)
                .SetRenderPipeline(pipeline)
                .DrawArrays(1, 1, 0, 0)
            .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 1, 0, 0), fb.color, 0, 0);
}

// Try setting push constants for vertex and fragment stage simulteanously
TEST_P(PushConstantTest, SimultaneousVertexAndFragmentConstants) {
    PushConstantSpec spec = {{Int, 2}};

    utils::BasicFramebuffer fb = utils::CreateBasicFramebuffer(device, 1, 1);

    nxt::PipelineLayout layout = MakeEmptyLayout();
    nxt::RenderPipeline pipeline = MakeTestRenderPipeline(layout, fb.renderPass, 0, spec, spec);

    uint32_t two = 2;
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(fb.renderPass, fb.framebuffer)
            .BeginRenderSubpass()
                .SetPushConstants(nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment, 0, 1, &two)
                .SetRenderPipeline(pipeline)
                .DrawArrays(1, 1, 0, 0)
            .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 1, 0, 0), fb.color, 0, 0);
}
NXT_INSTANTIATE_TEST(PushConstantTest, MetalBackend, OpenGLBackend)
