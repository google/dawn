// Copyright 2019 The Dawn Authors
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
#include "utils/DawnHelpers.h"

constexpr uint32_t kRTSize = 400;
constexpr uint32_t kBufferElementsCount = kMinDynamicBufferOffsetAlignment / sizeof(uint32_t) + 2;
constexpr uint32_t kBufferSize = kBufferElementsCount * sizeof(uint32_t);
constexpr uint32_t kBindingSize = 8;

class DynamicBufferOffsetTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Mix up dynamic and non dynamic resources in one bind group and using not continuous
        // binding number to cover more cases.
        std::array<uint32_t, kBufferElementsCount> uniformData = {0};
        uniformData[0] = 1;
        uniformData[1] = 2;

        mUniformBuffer = utils::CreateBufferFromData(device, uniformData.data(), kBufferSize,
                                                     dawn::BufferUsageBit::Uniform);

        uniformData[uniformData.size() - 2] = 5;
        uniformData[uniformData.size() - 1] = 6;

        mDynamicUniformBuffer = utils::CreateBufferFromData(device, uniformData.data(), kBufferSize,
                                                            dawn::BufferUsageBit::Uniform);

        dawn::BufferDescriptor storageBufferDescriptor;
        storageBufferDescriptor.size = kBufferSize;
        storageBufferDescriptor.usage = dawn::BufferUsageBit::Storage |
                                        dawn::BufferUsageBit::CopyDst |
                                        dawn::BufferUsageBit::CopySrc;

        mStorageBuffer = device.CreateBuffer(&storageBufferDescriptor);

        mDynamicStorageBuffer = device.CreateBuffer(&storageBufferDescriptor);

        mDefaultBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStageBit::Compute | dawn::ShaderStageBit::Fragment,
                      dawn::BindingType::UniformBuffer},
                     {1, dawn::ShaderStageBit::Compute | dawn::ShaderStageBit::Fragment,
                      dawn::BindingType::StorageBuffer},
                     {3, dawn::ShaderStageBit::Compute | dawn::ShaderStageBit::Fragment,
                      dawn::BindingType::UniformBuffer, true},
                     {4, dawn::ShaderStageBit::Compute | dawn::ShaderStageBit::Fragment,
                      dawn::BindingType::StorageBuffer, true}});

        mDefaultPipelineLayout = utils::MakeBasicPipelineLayout(device, &mDefaultBindGroupLayout);

        mBindGroup = utils::MakeBindGroup(device, mDefaultBindGroupLayout,
                                          {{0, mUniformBuffer, 0, kBindingSize},
                                           {1, mStorageBuffer, 0, kBindingSize},
                                           {3, mDynamicUniformBuffer, 0, kBindingSize},
                                           {4, mDynamicStorageBuffer, 0, kBindingSize}});
    }
    // Create objects to use as resources inside test bind groups.

    dawn::BindGroup mBindGroup;
    dawn::BindGroupLayout mDefaultBindGroupLayout;
    dawn::PipelineLayout mDefaultPipelineLayout;
    dawn::Buffer mUniformBuffer;
    dawn::Buffer mStorageBuffer;
    dawn::Buffer mDynamicUniformBuffer;
    dawn::Buffer mDynamicStorageBuffer;
    dawn::Texture mColorAttachment;

    dawn::ShaderModule CreateDefaultCsModule() {
        return utils::CreateShaderModule(device, utils::ShaderStage::Compute, R"(
                #version 450
                layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                layout(std140, set = 0, binding = 0) uniform uBufferNotDynamic {
                    uvec2 notDynamicValue;
                };
                layout(std140, set = 0, binding = 1) buffer sBufferNotDynamic {
                    uvec2 notDynamicResult;
                } mid;
                layout(std140, set = 0, binding = 3) uniform uBuffer {
                     uvec2 value;
                };
                layout(std140, set = 0, binding = 4) buffer SBuffer {
                     uvec2 result;
                } sBuffer;

                void main() {
                    mid.notDynamicResult.xy = notDynamicValue.xy;
                    sBuffer.result.xy = value.xy + mid.notDynamicResult.xy;
                })");
    }

    dawn::ShaderModule CreateDefaultVsModule() {
        return utils::CreateShaderModule(device, utils::ShaderStage::Vertex, R"(
                #version 450
                void main() {
                    const vec2 pos[3] = vec2[3](vec2(-1.0f, 0.0f), vec2(-1.0f, -1.0f), vec2(0.0f, -1.0f));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
                })");
    }

    dawn::ShaderModule CreateDefaultFsModule() {
        return utils::CreateShaderModule(device, utils::ShaderStage::Fragment, R"(
                #version 450
                layout(std140, set = 0, binding = 0) uniform uBufferNotDynamic {
                    uvec2 notDynamicValue;
                };
                layout(std140, set = 0, binding = 1) buffer sBufferNotDynamic {
                    uvec2 notDynamicResult;
                } mid;
                layout(std140, set = 0, binding = 3) uniform uBuffer {
                     uvec2 value;
                };
                layout(std140, set = 0, binding = 4) buffer SBuffer {
                     uvec2 result;
                } sBuffer;
                layout(location = 0) out vec4 fragColor;
                void main() {
                    mid.notDynamicResult.xy = notDynamicValue.xy;
                    sBuffer.result.xy = value.xy + mid.notDynamicResult.xy;
                    fragColor = vec4(value.x / 255.0f, value.y / 255.0f, 1.0f, 1.0f);
                })");
    }

    dawn::ShaderModule CreateInheritCsModule() {
        return utils::CreateShaderModule(device, utils::ShaderStage::Compute, R"(
                #version 450
                layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
                layout(std140, set = 0, binding = 0) uniform uBufferNotDynamic {
                    uvec2 notDynamicValue;
                };
                layout(std140, set = 0, binding = 1) buffer sBufferNotDynamic {
                    uvec2 notDynamicResult;
                } mid;
                layout(std140, set = 0, binding = 3) uniform uBuffer {
                     uvec2 value;
                };
                layout(std140, set = 0, binding = 4) buffer SBuffer {
                     uvec2 result;
                } sBuffer;
                layout(std140, set = 1, binding = 0) uniform paddingBlock {
                    uvec2 padding;
                };

                void main() {
                    mid.notDynamicResult.xy = notDynamicValue.xy;
                    sBuffer.result.xy = 2 * (value.xy + mid.notDynamicResult.xy);
                })");
    }

    dawn::ShaderModule CreateInheritFsModule() {
        return utils::CreateShaderModule(device, utils::ShaderStage::Fragment, R"(
                #version 450
                layout(std140, set = 0, binding = 0) uniform uBufferNotDynamic {
                    uvec2 notDynamicValue;
                };
                layout(std140, set = 0, binding = 1) buffer sBufferNotDynamic {
                    uvec2 notDynamicResult;
                } mid;
                layout(std140, set = 0, binding = 3) uniform uBuffer {
                     uvec2 value;
                };
                layout(std140, set = 0, binding = 4) buffer SBuffer {
                     uvec2 result;
                } sBuffer;
                layout(std140, set = 1, binding = 0) uniform paddingBlock {
                    uvec2 padding;
                };
                layout(location = 0) out vec4 fragColor;

                void main() {
                    mid.notDynamicResult.xy = notDynamicValue.xy;
                    sBuffer.result.xy = 2 * (value.xy + mid.notDynamicResult.xy);
                    fragColor = vec4(value.x / 255.0f, value.y / 255.0f, 1.0f, 1.0f);
                })");
    }

    dawn::RenderPipeline CreateRenderPipeline(dawn::PipelineLayout layout,
                                              dawn::ShaderModule vs,
                                              dawn::ShaderModule fs) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.cVertexStage.module = vs;
        pipelineDescriptor.cFragmentStage.module = fs;
        pipelineDescriptor.cColorStates[0]->format = dawn::TextureFormat::RGBA8Unorm;
        pipelineDescriptor.layout = layout;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    dawn::ComputePipeline CreateComputePipeline(dawn::PipelineLayout layout,
                                                dawn::ShaderModule cs) {
        dawn::ComputePipelineDescriptor csDesc;
        csDesc.layout = layout;

        dawn::PipelineStageDescriptor computeStage;
        computeStage.module = cs;
        computeStage.entryPoint = "main";
        csDesc.computeStage = &computeStage;

        return device.CreateComputePipeline(&csDesc);
    }

    dawn::RenderPipeline CreateDefaultRenderPipeline() {
        dawn::ShaderModule vs = this->CreateDefaultVsModule();
        dawn::ShaderModule fs = this->CreateDefaultFsModule();

        return this->CreateRenderPipeline(mDefaultPipelineLayout, vs, fs);
    }

    dawn::ComputePipeline CreateDefaultComputePipeline() {
        dawn::ShaderModule cs = this->CreateDefaultCsModule();

        return this->CreateComputePipeline(mDefaultPipelineLayout, cs);
    }
};

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicRenderPipeline) {
    dawn::RenderPipeline pipeline = CreateDefaultRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {0, 0};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer, 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffestsRenderPipeline) {
    dawn::RenderPipeline pipeline = CreateDefaultRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer,
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicComputePipeline) {
    dawn::ComputePipeline pipeline = CreateDefaultComputePipeline();

    std::array<uint64_t, 2> offsets = {0, 0};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer, 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffestsComputePipeline) {
    dawn::ComputePipeline pipeline = CreateDefaultComputePipeline();

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer,
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on render pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffestsRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    dawn::RenderPipeline pipeline = CreateDefaultRenderPipeline();

    dawn::ShaderModule testVs = CreateDefaultVsModule();
    dawn::ShaderModule testFs = CreateInheritFsModule();
    dawn::BindGroupLayout bgl[2];
    bgl[0] = mDefaultBindGroupLayout;
    bgl[1] = utils::MakeBindGroupLayout(
        device, {{0, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 2;
    descriptor.bindGroupLayouts = bgl;
    dawn::PipelineLayout layout = device.CreatePipelineLayout(&descriptor);
    dawn::RenderPipeline testPipeline = CreateRenderPipeline(layout, testVs, testFs);

    std::array<uint32_t, kBufferElementsCount> uniformData = {0};

    dawn::Buffer uniformBuffer = utils::CreateBufferFromData(
        device, uniformData.data(), kBufferSize, dawn::BufferUsageBit::Uniform);
    dawn::BindGroup bindGroup =
        utils::MakeBindGroup(device, bgl[1], {{0, uniformBuffer, 0, kBindingSize}});

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.SetPipeline(testPipeline);
    renderPassEncoder.SetBindGroup(1, bindGroup, 0, nullptr);
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer,
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on compute pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffestsComputePipeline) {
    dawn::ComputePipeline pipeline = CreateDefaultComputePipeline();

    dawn::ShaderModule testCs = CreateInheritCsModule();
    dawn::BindGroupLayout bgl[2];
    bgl[0] = mDefaultBindGroupLayout;
    bgl[1] = utils::MakeBindGroupLayout(
        device, {{0, dawn::ShaderStageBit::Compute, dawn::BindingType::UniformBuffer}});
    dawn::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 2;
    descriptor.bindGroupLayouts = bgl;
    dawn::PipelineLayout layout = device.CreatePipelineLayout(&descriptor);
    dawn::ComputePipeline testPipeline = CreateComputePipeline(layout, testCs);

    std::array<uint32_t, kBufferElementsCount> uniformData = {0};

    dawn::Buffer uniformBuffer = utils::CreateBufferFromData(
        device, uniformData.data(), kBufferSize, dawn::BufferUsageBit::Uniform);
    dawn::BindGroup bindGroup =
        utils::MakeBindGroup(device, bgl[1], {{0, uniformBuffer, 0, kBindingSize}});

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.SetPipeline(testPipeline);
    computePassEncoder.SetBindGroup(1, bindGroup, 0, nullptr);
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer,
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one render pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffestsMultipleTimesRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    dawn::RenderPipeline pipeline = CreateDefaultRenderPipeline();

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    std::array<uint64_t, 2> testOffsets = {0, 0};

    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.SetBindGroup(0, mBindGroup, testOffsets.size(), testOffsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer, 0, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one compute pass.
// TODO(shaobo.yan@intel.com) : enable this test after resolving dawn issue 198.
TEST_P(DynamicBufferOffsetTests, DISABLED_UpdateDynamicOffestsMultipleTimesComputePipeline) {
    dawn::ComputePipeline pipeline = CreateDefaultComputePipeline();

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    std::array<uint64_t, 2> testOffsets = {0, 0};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroup, offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.SetBindGroup(0, mBindGroup, testOffsets.size(), testOffsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mDynamicStorageBuffer, 0, expectedData.size());
}

DAWN_INSTANTIATE_TEST(DynamicBufferOffsetTests,
                      D3D12Backend,
                      MetalBackend,
                      OpenGLBackend,
                      VulkanBackend);
