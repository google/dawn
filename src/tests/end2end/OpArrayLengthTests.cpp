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

#include "common/Assert.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class OpArrayLengthTest : public DawnTest {
  protected:
    void SetUp() {
        DawnTest::SetUp();

        // Create buffers of various size to check the length() implementation
        dawn::BufferDescriptor bufferDesc;
        bufferDesc.size = 4;
        bufferDesc.usage = dawn::BufferUsage::Storage;
        mStorageBuffer4 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 256;
        mStorageBuffer256 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 512;
        mStorageBuffer512 = device.CreateBuffer(&bufferDesc);

        // Put them all in a bind group for tests to bind them easily.
        dawn::ShaderStage kAllStages =
            dawn::ShaderStage::Fragment | dawn::ShaderStage::Vertex | dawn::ShaderStage::Compute;
        mBindGroupLayout =
            utils::MakeBindGroupLayout(device, {{0, kAllStages, dawn::BindingType::StorageBuffer},
                                                {1, kAllStages, dawn::BindingType::StorageBuffer},
                                                {2, kAllStages, dawn::BindingType::StorageBuffer}});

        mBindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                          {
                                              {0, mStorageBuffer4, 0, 4},
                                              {1, mStorageBuffer256, 0, dawn::kWholeSize},
                                              {2, mStorageBuffer512, 0, 512},
                                          });

        // Common shader code to use these buffers in shaders, assuming they are in bindgroup index
        // 0.
        mShaderInterface = R"(
            // The length should be 1 because the buffer is 4-byte long.
            layout(std430, set = 0, binding = 0) buffer Buffer1 {
                float data[];
            } buffer1;

            // The length should be 64 because the buffer is 256 bytes long.
            layout(std430, set = 0, binding = 1) buffer Buffer2 {
                float data[];
            } buffer2;

            // The length should be (512 - 16*4) / 8 = 56 because the buffer is 512 bytes long
            // and the structure is 8 bytes big.
            struct Buffer3Data {float a; int b;};
            layout(std430, set = 0, binding = 2) buffer Buffer3 {
                mat4 garbage;
                Buffer3Data data[];
            } buffer3;
        )";

        // See comments in the shader for an explanation of these values
        mExpectedLengths = {1, 64, 56};
    }

    dawn::Buffer mStorageBuffer4;
    dawn::Buffer mStorageBuffer256;
    dawn::Buffer mStorageBuffer512;

    dawn::BindGroupLayout mBindGroupLayout;
    dawn::BindGroup mBindGroup;
    std::string mShaderInterface;
    std::array<uint32_t, 3> mExpectedLengths;
};

// Test OpArrayLength in the compute stage
TEST_P(OpArrayLengthTest, Compute) {
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. See https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());

    // Create a buffer to hold the result sizes and create a bindgroup for it.
    dawn::BufferDescriptor bufferDesc;
    bufferDesc.usage = dawn::BufferUsage::Storage | dawn::BufferUsage::CopySrc;
    bufferDesc.size = sizeof(uint32_t) * mExpectedLengths.size();
    dawn::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    dawn::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
        device, {{0, dawn::ShaderStage::Compute, dawn::BindingType::StorageBuffer}});

    dawn::BindGroup resultBindGroup =
        utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, dawn::kWholeSize}});

    // Create the compute pipeline that stores the length()s in the result buffer.
    dawn::BindGroupLayout bgls[] = {mBindGroupLayout, resultLayout};
    dawn::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 2;
    plDesc.bindGroupLayouts = bgls;
    dawn::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    dawn::PipelineStageDescriptor computeStage;
    computeStage.entryPoint = "main";
    computeStage.module = utils::CreateShaderModule(device, utils::SingleShaderStage::Compute,
                                                    (R"(#version 450
            layout(std430, set = 1, binding = 0) buffer ResultBuffer {
                uint result[3];
            };
            )" + mShaderInterface + R"(
            void main() {
                result[0] = buffer1.data.length();
                result[1] = buffer2.data.length();
                result[2] = buffer3.data.length();
            })")
                                                        .c_str());

    dawn::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.computeStage = &computeStage;
    dawn::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // Run a single instance of the compute shader
    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    dawn::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, mBindGroup, 0, nullptr);
    pass.SetBindGroup(1, resultBindGroup, 0, nullptr);
    pass.Dispatch(1, 1, 1);
    pass.EndPass();

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(mExpectedLengths.data(), resultBuffer, 0, 3);
}

// Test OpArrayLength in the fragment stage
TEST_P(OpArrayLengthTest, Fragment) {
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. See https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    dawn::ShaderModule vsModule =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            gl_PointSize = 1.0;
        })");

    dawn::ShaderModule fsModule =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment,
                                  (R"(
        #version 450
        )" + mShaderInterface + R"(
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor.r = buffer1.data.length() / 255.0f;
            fragColor.g = buffer2.data.length() / 255.0f;
            fragColor.b = buffer3.data.length() / 255.0f;
            fragColor.a = 0.0f;
        })")
                                      .c_str());

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;
    descriptor.primitiveTopology = dawn::PrimitiveTopology::PointList;
    descriptor.cColorStates[0]->format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup, 0, nullptr);
        pass.Draw(1, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 expectedColor = RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

// Test OpArrayLength in the vertex stage
TEST_P(OpArrayLengthTest, Vertex) {
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. See https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    dawn::ShaderModule vsModule =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex,
                                  (R"(
        #version 450
        )" + mShaderInterface + R"(
        layout(location = 0) out vec4 pointColor;
        void main() {
            pointColor.r = buffer1.data.length() / 255.0f;
            pointColor.g = buffer2.data.length() / 255.0f;
            pointColor.b = buffer3.data.length() / 255.0f;
            pointColor.a = 0.0f;

            gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            gl_PointSize = 1.0;
        })")
                                      .c_str());

    dawn::ShaderModule fsModule =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
        #version 450
        layout(location = 0) out vec4 fragColor;
        layout(location = 0) in vec4 pointColor;
        void main() {
            fragColor = pointColor;
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;
    descriptor.primitiveTopology = dawn::PrimitiveTopology::PointList;
    descriptor.cColorStates[0]->format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup, 0, nullptr);
        pass.Draw(1, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 expectedColor = RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
