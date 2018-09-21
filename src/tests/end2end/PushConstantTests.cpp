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
#include "common/Constants.h"
#include "utils/DawnHelpers.h"

#include <array>

class PushConstantTest: public DawnTest {
    protected:
        // Layout, bind group and friends to store results for compute tests, can have an extra buffer
        // so that two different pipeline layout can be created.
        struct TestBindings {
            dawn::PipelineLayout layout;
            dawn::BindGroup bindGroup;
            dawn::Buffer resultBuffer;
        };
        TestBindings MakeTestBindings(bool extraBuffer) {
            uint32_t one = 1;
            dawn::Buffer buf1 = utils::CreateBufferFromData(device, &one, 4, dawn::BufferUsageBit::Storage |
                                                                             dawn::BufferUsageBit::TransferSrc |
                                                                             dawn::BufferUsageBit::TransferDst);
            dawn::BufferDescriptor buf2Desc;
            buf2Desc.size = 4;
            buf2Desc.usage = dawn::BufferUsageBit::Storage;
            dawn::Buffer buf2 = device.CreateBuffer(&buf2Desc);

            dawn::ShaderStageBit kAllStages = dawn::ShaderStageBit::Compute | dawn::ShaderStageBit::Fragment | dawn::ShaderStageBit::Vertex;
            constexpr dawn::ShaderStageBit kNoStages{};

            dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device,
                {
                    {0, kAllStages, dawn::BindingType::StorageBuffer},
                    {1, extraBuffer ? kAllStages : kNoStages, dawn::BindingType::StorageBuffer},
                });

            dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

            dawn::BufferView views[2] = {
                buf1.CreateBufferViewBuilder().SetExtent(0, 4).GetResult(),
                buf2.CreateBufferViewBuilder().SetExtent(0, 4).GetResult(),
            };

            dawn::BindGroup bg = device.CreateBindGroupBuilder()
                .SetLayout(bgl)
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
        dawn::ComputePipeline MakeTestComputePipeline(const dawn::PipelineLayout& pl, PushConstantSpec spec) {
            dawn::ShaderModule module = utils::CreateShaderModule(device, dawn::ShaderStage::Compute, (R"(
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

            dawn::ComputePipelineDescriptor descriptor;
            descriptor.module = module.Clone();
            descriptor.entryPoint = "main";
            descriptor.layout = pl.Clone();
            return device.CreateComputePipeline(&descriptor);
        }

        dawn::PipelineLayout MakeEmptyLayout() {
            return utils::MakeBasicPipelineLayout(device, nullptr);
        }

        // The render pipeline adds one to the red channel for successful vertex push constant test
        // and adds one to green for the frgament test.
        dawn::RenderPipeline MakeTestRenderPipeline(dawn::PipelineLayout& layout, PushConstantSpec vsSpec, PushConstantSpec fsSpec) {
            dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, (R"(
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
            dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, (R"(
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

            dawn::BlendState blendState = device.CreateBlendStateBuilder()
                .SetBlendEnabled(true)
                .SetColorBlend(dawn::BlendOperation::Add, dawn::BlendFactor::One, dawn::BlendFactor::One)
                .SetAlphaBlend(dawn::BlendOperation::Add, dawn::BlendFactor::One, dawn::BlendFactor::One)
                .GetResult();

            return device.CreateRenderPipelineBuilder()
                .SetColorAttachmentFormat(0, dawn::TextureFormat::R8G8B8A8Unorm)
                .SetLayout(layout)
                .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
                .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
                .SetPrimitiveTopology(dawn::PrimitiveTopology::PointList)
                .SetColorAttachmentBlendState(0, blendState)
                .GetResult();
        }
};

// Test that push constants default to zero at the beginning of every compute passes.
TEST_P(PushConstantTest, ComputePassDefaultsToZero) {
    auto binding = MakeTestBindings(false);

    // Expect push constants to be zero in all dispatches of this test.
    dawn::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, MakeAllZeroSpec());

    uint32_t notZero = 42;
    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();

        // Test compute push constants are set to zero by default.
        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, binding.bindGroup);
        pass.Dispatch(1, 1, 1);
        // Set push constants to non-zero value to check they will be reset to zero
        // on the next BeginComputePass
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, &notZero);

        pass.EndPass();
    }
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();

        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, binding.bindGroup);
        pass.Dispatch(1, 1, 1);

        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding.resultBuffer, 0);
}

// Test that push constants default to zero at the beginning of render passes.
TEST_P(PushConstantTest, RenderPassDefaultsToZero) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Expect push constants to be zero in all draws of this test.
    PushConstantSpec allZeros = MakeAllZeroSpec();
    dawn::PipelineLayout layout = MakeEmptyLayout();
    dawn::RenderPipeline pipeline = MakeTestRenderPipeline(layout, MakeAllZeroSpec(), MakeAllZeroSpec());

    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPassInfo);
        // Test render push constants are set to zero by default.
        pass.SetRenderPipeline(pipeline);
        pass.DrawArrays(1, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 1, 0, 0), renderPass.color, 0, 0);
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
    dawn::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, spec);


    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();

        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 3, reinterpret_cast<uint32_t*>(&values));
        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, binding.bindGroup);
        pass.Dispatch(1, 1, 1);

        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
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
    dawn::ComputePipeline pipeline1 = MakeTestComputePipeline(binding1.layout, spec1);
    dawn::ComputePipeline pipeline2 = MakeTestComputePipeline(binding2.layout, spec2);

    uint32_t one = 1;
    uint32_t two = 2;
    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();

        // Set Push constant before there is a pipeline set
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, &one);
        pass.SetComputePipeline(pipeline1);
        pass.SetBindGroup(0, binding1.bindGroup);
        pass.Dispatch(1, 1, 1);
        // Change the push constant before changing pipeline layout
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, &two);
        pass.SetComputePipeline(pipeline2);
        pass.SetBindGroup(0, binding2.bindGroup);
        pass.Dispatch(1, 1, 1);

        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
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
    dawn::ComputePipeline pipeline = MakeTestComputePipeline(binding.layout, spec);

    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();

        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, kMaxPushConstants, &values[0]);
        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, binding.bindGroup);
        pass.Dispatch(1, 1, 1);

        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, binding.resultBuffer, 0);
}

// Try setting separate push constants for vertex and fragment stage
TEST_P(PushConstantTest, SeparateVertexAndFragmentConstants) {
    PushConstantSpec vsSpec = {{Int, 1}};
    PushConstantSpec fsSpec = {{Int, 2}};

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    dawn::PipelineLayout layout = MakeEmptyLayout();
    dawn::RenderPipeline pipeline = MakeTestRenderPipeline(layout, vsSpec, fsSpec);

    uint32_t one = 1;
    uint32_t two = 2;
    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPassInfo);
        pass.SetPushConstants(dawn::ShaderStageBit::Vertex, 0, 1, &one);
        pass.SetPushConstants(dawn::ShaderStageBit::Fragment, 0, 1, &two);
        pass.SetRenderPipeline(pipeline);
        pass.DrawArrays(1, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 1, 0, 0), renderPass.color, 0, 0);
}

// Try setting push constants for vertex and fragment stage simulteanously
TEST_P(PushConstantTest, SimultaneousVertexAndFragmentConstants) {
    PushConstantSpec spec = {{Int, 2}};

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    dawn::PipelineLayout layout = MakeEmptyLayout();
    dawn::RenderPipeline pipeline = MakeTestRenderPipeline(layout, spec, spec);

    uint32_t two = 2;
    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPassInfo);
        pass.SetPushConstants(dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment, 0, 1, &two);
        pass.SetRenderPipeline(pipeline);
        pass.DrawArrays(1, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 1, 0, 0), renderPass.color, 0, 0);
}
DAWN_INSTANTIATE_TEST(PushConstantTest, MetalBackend, OpenGLBackend)
