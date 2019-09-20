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

        mUniformBuffers[0] = utils::CreateBufferFromData(device, uniformData.data(), kBufferSize,
                                                         dawn::BufferUsage::Uniform);

        uniformData[uniformData.size() - 2] = 5;
        uniformData[uniformData.size() - 1] = 6;

        // Dynamic uniform buffer
        mUniformBuffers[1] = utils::CreateBufferFromData(device, uniformData.data(), kBufferSize,
                                                         dawn::BufferUsage::Uniform);

        dawn::BufferDescriptor storageBufferDescriptor;
        storageBufferDescriptor.size = kBufferSize;
        storageBufferDescriptor.usage =
            dawn::BufferUsage::Storage | dawn::BufferUsage::CopyDst | dawn::BufferUsage::CopySrc;

        mStorageBuffers[0] = device.CreateBuffer(&storageBufferDescriptor);

        // Dynamic storage buffer
        mStorageBuffers[1] = device.CreateBuffer(&storageBufferDescriptor);

        // Default bind group layout
        mBindGroupLayouts[0] = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStage::Compute | dawn::ShaderStage::Fragment,
                      dawn::BindingType::UniformBuffer},
                     {1, dawn::ShaderStage::Compute | dawn::ShaderStage::Fragment,
                      dawn::BindingType::StorageBuffer},
                     {3, dawn::ShaderStage::Compute | dawn::ShaderStage::Fragment,
                      dawn::BindingType::UniformBuffer, true},
                     {4, dawn::ShaderStage::Compute | dawn::ShaderStage::Fragment,
                      dawn::BindingType::StorageBuffer, true}});

        // Default bind group
        mBindGroups[0] = utils::MakeBindGroup(device, mBindGroupLayouts[0],
                                              {{0, mUniformBuffers[0], 0, kBindingSize},
                                               {1, mStorageBuffers[0], 0, kBindingSize},
                                               {3, mUniformBuffers[1], 0, kBindingSize},
                                               {4, mStorageBuffers[1], 0, kBindingSize}});

        // Extra uniform buffer for inheriting test
        mUniformBuffers[2] = utils::CreateBufferFromData(device, uniformData.data(), kBufferSize,
                                                         dawn::BufferUsage::Uniform);

        // Bind group layout for inheriting test
        mBindGroupLayouts[1] = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStage::Compute | dawn::ShaderStage::Fragment,
                      dawn::BindingType::UniformBuffer}});

        // Bind group for inheriting test
        mBindGroups[1] = utils::MakeBindGroup(device, mBindGroupLayouts[1],
                                              {{0, mUniformBuffers[2], 0, kBindingSize}});
    }
    // Create objects to use as resources inside test bind groups.

    dawn::BindGroup mBindGroups[2];
    dawn::BindGroupLayout mBindGroupLayouts[2];
    dawn::Buffer mUniformBuffers[3];
    dawn::Buffer mStorageBuffers[2];
    dawn::Texture mColorAttachment;

    dawn::RenderPipeline CreateRenderPipeline(bool isInheritedPipeline = false) {
        dawn::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
                #version 450
                void main() {
                    const vec2 pos[3] = vec2[3](vec2(-1.0f, 0.0f), vec2(-1.0f, 1.0f), vec2(0.0f, 1.0f));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
                })");

        // Construct fragment shader source
        std::ostringstream fs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        fs << R"(
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
        )";

        if (isInheritedPipeline) {
            fs << R"(
                layout(std140, set = 1, binding = 0) uniform paddingBlock {
                    uvec2 padding;
                };
            )";
        }

        fs << " layout(location = 0) out vec4 fragColor;\n";
        fs << " void main() {\n";
        fs << "     mid.notDynamicResult.xy = notDynamicValue.xy;\n";
        fs << "     sBuffer.result.xy = " << multipleNumber
           << " * (value.xy + mid.notDynamicResult.xy);\n";
        fs << "     fragColor = vec4(value.x / 255.0f, value.y / 255.0f, 1.0f, 1.0f);\n";
        fs << " }\n";

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, fs.str().c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.cColorStates[0]->format = dawn::TextureFormat::RGBA8Unorm;

        dawn::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        pipelineDescriptor.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    dawn::ComputePipeline CreateComputePipeline(bool isInheritedPipeline = false) {
        // Construct compute shader source
        std::ostringstream cs;
        std::string multipleNumber = isInheritedPipeline ? "2" : "1";
        cs << R"(
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
        )";

        if (isInheritedPipeline) {
            cs << R"(
                layout(std140, set = 1, binding = 0) uniform paddingBlock {
                    uvec2 padding;
                };
            )";
        }

        cs << " void main() {\n";
        cs << "     mid.notDynamicResult.xy = notDynamicValue.xy;\n";
        cs << "     sBuffer.result.xy = " << multipleNumber
           << " * (value.xy + mid.notDynamicResult.xy);\n";
        cs << " }\n";

        dawn::ShaderModule csModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, cs.str().c_str());

        dawn::ComputePipelineDescriptor csDesc;
        csDesc.computeStage.module = csModule;
        csDesc.computeStage.entryPoint = "main";

        dawn::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        if (isInheritedPipeline) {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        } else {
            pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
        }
        pipelineLayoutDescriptor.bindGroupLayouts = mBindGroupLayouts;
        csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        return device.CreateComputePipeline(&csDesc);
    }
};

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicRenderPipeline) {
    dawn::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {0, 0};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffestsRenderPipeline) {
    dawn::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Dynamic offsets are all zero and no effect to result.
TEST_P(DynamicBufferOffsetTests, BasicComputePipeline) {
    dawn::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint64_t, 2> offsets = {0, 0};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Have non-zero dynamic offsets.
TEST_P(DynamicBufferOffsetTests, SetDynamicOffestsComputePipeline) {
    dawn::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {6, 8};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on render pipeline
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffestsRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    dawn::RenderPipeline pipeline = CreateRenderPipeline();
    dawn::RenderPipeline testPipeline = CreateRenderPipeline(true);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.SetPipeline(testPipeline);
    renderPassEncoder.SetBindGroup(1, mBindGroups[1], 0, nullptr);
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(5, 6, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Test inherit dynamic offsets on compute pipeline
// TODO(shaobo.yan@intel.com) : Try this test on GTX1080 and cannot reproduce the failure.
// Suspect it is due to dawn doesn't handle sync between two dispatch and disable this case.
// Will double check root cause after got GTX1660.
TEST_P(DynamicBufferOffsetTests, InheritDynamicOffestsComputePipeline) {
    DAWN_SKIP_TEST_IF(IsWindows());
    dawn::ComputePipeline pipeline = CreateComputePipeline();
    dawn::ComputePipeline testPipeline = CreateComputePipeline(true);

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.SetPipeline(testPipeline);
    computePassEncoder.SetBindGroup(1, mBindGroups[1], 0, nullptr);
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {12, 16};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1],
                               kMinDynamicBufferOffsetAlignment, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one render pass.
TEST_P(DynamicBufferOffsetTests, UpdateDynamicOffestsMultipleTimesRenderPipeline) {
    // Using default pipeline and setting dynamic offsets
    dawn::RenderPipeline pipeline = CreateRenderPipeline();

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    std::array<uint64_t, 2> testOffsets = {0, 0};

    dawn::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    renderPassEncoder.Draw(3, 1, 0, 0);
    renderPassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 255, 255), renderPass.color, 0, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

// Setting multiple dynamic offsets for the same bindgroup in one compute pass.
// TODO(shaobo.yan@intel.com) : enable this test after resolving dawn issue 198.
TEST_P(DynamicBufferOffsetTests, DISABLED_UpdateDynamicOffestsMultipleTimesComputePipeline) {
    dawn::ComputePipeline pipeline = CreateComputePipeline();

    std::array<uint64_t, 2> offsets = {kMinDynamicBufferOffsetAlignment,
                                       kMinDynamicBufferOffsetAlignment};
    std::array<uint64_t, 2> testOffsets = {0, 0};

    dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.SetBindGroup(0, mBindGroups[0], testOffsets.size(), testOffsets.data());
    computePassEncoder.Dispatch(1, 1, 1);
    computePassEncoder.EndPass();
    dawn::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expectedData = {2, 4};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), mStorageBuffers[1], 0, expectedData.size());
}

DAWN_INSTANTIATE_TEST(DynamicBufferOffsetTests,
                      D3D12Backend,
                      MetalBackend,
                      OpenGLBackend,
                      VulkanBackend);
