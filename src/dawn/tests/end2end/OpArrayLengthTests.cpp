// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstdint>
#include <string>
#include <vector>

#include "src/dawn/common/Math.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/assert.h"

namespace dawn {
namespace {

class OpArrayLengthTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        // maxStorageBuffersInVertexStage
        supported.UnlinkedCopyTo(&required);
    }

    const uint32_t kOffset = 256;
    const uint32_t kBuffer1_size = 4;
    const uint32_t kBuffer2_size = 256;
    const uint32_t kBuffer3_size = 512;

    void SetUpBuffers(bool useDynamicOffset) {
        const uint32_t offset = useDynamicOffset ? kOffset : 0;
        const uint32_t buffer1_whole_size = kBuffer1_size + offset;
        const uint32_t buffer2_whole_size = kBuffer2_size + offset;
        const uint32_t buffer3_whole_size = kBuffer3_size + 256 + offset;

        // Create buffers of various size to check the length() implementation
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = buffer1_whole_size;
        bufferDesc.usage = wgpu::BufferUsage::Storage;
        mStorageBuffer4 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = buffer2_whole_size;
        mStorageBuffer256 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = buffer3_whole_size;
        mStorageBuffer512 = device.CreateBuffer(&bufferDesc);

        // Common shader code to use these buffers in shaders, assuming they are in bindgroup index
        // 0.
        mShaderInterface = R"(
            struct DataBuffer {
                data : array<f32>
            }

            // The length should be 1 because the buffer is 4-byte long.
            @group(0) @binding(0) var<storage, read> buffer1 : DataBuffer;

            // The length should be 64 because the buffer is 256 bytes long.
            @group(0) @binding(1) var<storage, read> buffer2 : DataBuffer;

            // The length should be (512 - 16*4) / 8 = 56 because the buffer is 512 bytes long
            // and the structure is 8 bytes big.
            struct Buffer3Data {
                a : f32,
                b : i32,
            }

            struct Buffer3 {
                @size(64) garbage : mat4x4<f32>,
                data : array<Buffer3Data>,
            }
            @group(0) @binding(2) var<storage, read> buffer3 : Buffer3;
        )";

        // See comments in the shader for an explanation of these values
        mExpectedLengths = {1, 64, 56};
    }

    wgpu::BindGroupLayout MakeBindGroupLayout(wgpu::ShaderStage stages, bool useDynamicOffset) {
        // Put them all in a bind group for tests to bind them easily.
        return utils::MakeBindGroupLayout(
            device,
            {{0, stages, wgpu::BufferBindingType::ReadOnlyStorage, useDynamicOffset, kBuffer1_size},
             {1, stages, wgpu::BufferBindingType::ReadOnlyStorage, useDynamicOffset, kBuffer2_size},
             {2, stages, wgpu::BufferBindingType::ReadOnlyStorage, useDynamicOffset,
              kBuffer3_size}});
    }

    wgpu::BindGroup MakeBindGroup(wgpu::BindGroupLayout bindGroupLayout) {
        return utils::MakeBindGroup(device, bindGroupLayout,
                                    {
                                        {0, mStorageBuffer4, 0, kBuffer1_size},
                                        {1, mStorageBuffer256, 0, kBuffer2_size},
                                        {2, mStorageBuffer512, 256, kBuffer3_size},
                                    });
    }

    void ComputeTest(bool useDynamicOffset) {
        SetUpBuffers(useDynamicOffset);

        // Create a buffer to hold the result sizes and create a bindgroup for it.
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        bufferDesc.size = sizeof(uint32_t) * mExpectedLengths.size();
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

        wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

        wgpu::BindGroup resultBindGroup =
            utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, wgpu::kWholeSize}});

        // Create the compute pipeline that stores the length()s in the result buffer.
        wgpu::BindGroupLayout bindGroupLayout =
            MakeBindGroupLayout(wgpu::ShaderStage::Compute, useDynamicOffset);
        wgpu::BindGroupLayout bgls[] = {bindGroupLayout, resultLayout};
        wgpu::PipelineLayoutDescriptor plDesc;
        plDesc.bindGroupLayoutCount = 2;
        plDesc.bindGroupLayouts = bgls;
        wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc;
        pipelineDesc.layout = pl;
        pipelineDesc.compute.module = utils::CreateShaderModule(device, (R"(
        struct ResultBuffer {
            data : array<u32, 3>
        }
        @group(1) @binding(0) var<storage, read_write> result : ResultBuffer;
        )" + mShaderInterface + R"(
        @compute @workgroup_size(1) fn main() {
            result.data[0] = arrayLength(&buffer1.data);
            result.data[1] = arrayLength(&buffer2.data);
            result.data[2] = arrayLength(&buffer3.data);
        })")
                                                                            .c_str());
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);
        wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

        std::vector<uint32_t> offsets;
        if (useDynamicOffset) {
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
        }

        // Run a single instance of the compute shader
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        pass.SetBindGroup(1, resultBindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(mExpectedLengths.data(), resultBuffer, 0, 3);
    }

    void FragmentTest(bool useDynamicOffset) {
        SetUpBuffers(useDynamicOffset);
        // TODO(crbug.com/408042465): investigate this failure on Pixel 6 OpenGLES
        DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM() &&
                              HasToggleEnabled("gl_use_array_length_from_immediate"));

        DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 3);

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

        // Create the pipeline that computes the length of the buffers and writes it to the only
        // render pass pixel.
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        @fragment fn main() -> @location(0) vec4f {
            var fragColor : vec4f;
            fragColor.r = f32(arrayLength(&buffer1.data)) / 255.0;
            fragColor.g = f32(arrayLength(&buffer2.data)) / 255.0;
            fragColor.b = f32(arrayLength(&buffer3.data)) / 255.0;
            fragColor.a = 0.0;
            return fragColor;
        })")
                                                                            .c_str());

        wgpu::BindGroupLayout bindGroupLayout =
            MakeBindGroupLayout(wgpu::ShaderStage::Fragment, useDynamicOffset);

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        descriptor.cTargets[0].format = renderPass.colorFormat;
        descriptor.layout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

        std::vector<uint32_t> offsets;
        if (useDynamicOffset) {
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
        }

        // "Draw" the lengths to the texture.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
            pass.Draw(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectedColor =
            utils::RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
    }

    void VertexTest(bool useDynamicOffset) {
        SetUpBuffers(useDynamicOffset);
        DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 3);

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

        // Create the pipeline that computes the length of the buffers and writes it to the only
        // render pass pixel.
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        struct VertexOut {
            @location(0) color : vec4f,
            @builtin(position) position : vec4f,
        }

        @vertex fn main() -> VertexOut {
            var output : VertexOut;
            output.color.r = f32(arrayLength(&buffer1.data)) / 255.0;
            output.color.g = f32(arrayLength(&buffer2.data)) / 255.0;
            output.color.b = f32(arrayLength(&buffer3.data)) / 255.0;
            output.color.a = 0.0;

            output.position = vec4f(0.0, 0.0, 0.0, 1.0);
            return output;
        })")
                                                                            .c_str());

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) color : vec4f) -> @location(0) vec4f {
            return color;
        })");

        wgpu::BindGroupLayout bindGroupLayout =
            MakeBindGroupLayout(wgpu::ShaderStage::Vertex, useDynamicOffset);

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        descriptor.cTargets[0].format = renderPass.colorFormat;
        descriptor.layout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::BindGroup bindGroup = MakeBindGroup(bindGroupLayout);

        std::vector<uint32_t> offsets;
        if (useDynamicOffset) {
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
            offsets.push_back(kOffset);
        }

        // "Draw" the lengths to the texture.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
            pass.Draw(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectedColor =
            utils::RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
    }

    wgpu::Buffer mStorageBuffer4;
    wgpu::Buffer mStorageBuffer256;
    wgpu::Buffer mStorageBuffer512;

    std::string mShaderInterface;
    std::array<uint32_t, 3> mExpectedLengths{};
};

// Test OpArrayLength in the compute stage
TEST_P(OpArrayLengthTest, Compute) {
    ComputeTest(false);
}
TEST_P(OpArrayLengthTest, Compute_DynamicOffset) {
    // TODO(b/535703448): fails on Pixel 10
    DAWN_SUPPRESS_TEST_IF(IsImgTec());
    ComputeTest(true);
}

// Test OpArrayLength in the fragment stage
TEST_P(OpArrayLengthTest, Fragment) {
    FragmentTest(false);
}
TEST_P(OpArrayLengthTest, Fragment_DynamicOffset) {
    // TODO(b/535703448): fails on Pixel 10
    DAWN_SUPPRESS_TEST_IF(IsImgTec());
    FragmentTest(true);
}

// Test OpArrayLength in the vertex stage
TEST_P(OpArrayLengthTest, Vertex) {
    VertexTest(false);
}
TEST_P(OpArrayLengthTest, Vertex_DynamicOffset) {
    // TODO(b/535703448): fails on Pixel 10
    DAWN_SUPPRESS_TEST_IF(IsImgTec());
    VertexTest(true);
}

// Verify rebinding a compute storage buffer updates its array-length metadata.
TEST_P(OpArrayLengthTest, ComputeBindGroupSwitch) {
    // TODO(crbug.com/366291600): Suspected native GLES/Vulkan arrayLength bug on Pixel 4
    // Android bots. Keep GLES coverage when array lengths are provided via immediates.
    DAWN_SUPPRESS_TEST_IF(
        IsAndroid() &&
        (IsVulkan() || (IsOpenGLES() && !HasToggleEnabled("gl_use_array_length_from_immediate"))));

    wgpu::BufferDescriptor resultDesc;
    resultDesc.size = sizeof(uint32_t);
    resultDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&resultDesc);

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read> data : array<u32>;
        @group(1) @binding(0) var<storage, read_write> result : array<u32>;

        @compute @workgroup_size(1) fn main() {
            result[0] = arrayLength(&data);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroupLayout dataLayout = pipeline.GetBindGroupLayout(0);
    wgpu::BindGroupLayout resultLayout = pipeline.GetBindGroupLayout(1);
    wgpu::BufferDescriptor dataDesc;
    dataDesc.size = kBuffer2_size;
    dataDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer dataBuffer = device.CreateBuffer(&dataDesc);
    wgpu::BindGroup smallData =
        utils::MakeBindGroup(device, dataLayout, {{0, dataBuffer, 0, kBuffer1_size}});
    wgpu::BindGroup largeData =
        utils::MakeBindGroup(device, dataLayout, {{0, dataBuffer, 0, kBuffer2_size}});
    wgpu::BindGroup result =
        utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, wgpu::kWholeSize}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(1, result);
    pass.SetBindGroup(0, smallData);
    pass.DispatchWorkgroups(1);
    pass.SetBindGroup(0, largeData);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(kBuffer2_size / sizeof(uint32_t), resultBuffer, 0);
}

// Verify storage buffers in different bind groups use distinct size metadata slots
// and that array lengths reflect bound ranges rather than whole buffer allocations.
TEST_P(OpArrayLengthTest, ComputeStorageBuffersAcrossBindGroups) {
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(2) var<storage, read> data : array<u32>;
        @group(1) @binding(3) var<storage, read_write> result : array<u32>;

        @compute @workgroup_size(1) fn main() {
            result[0] = arrayLength(&data);
            result[1] = arrayLength(&result);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 512;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer dataBuffer = device.CreateBuffer(&bufferDesc);
    bufferDesc.size = 2 * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup data =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{2, dataBuffer, 256, 256}});
    wgpu::BindGroup result =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1), {{3, resultBuffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, data);
    pass.SetBindGroup(1, result);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(64u, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(2u, resultBuffer, sizeof(uint32_t));
}

// OpenGL passes firstInstance as an internal immediate. Direct/indirect/direct draws must
// update it without rebinding or switching pipelines, while preserving storage buffer sizes.
TEST_P(OpArrayLengthTest, RenderIndirectSwitch) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 5, 1);
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(position) position : vec4f,
            @location(0) instanceIndex : f32,
        }
        @vertex fn main(@builtin(vertex_index) vertexIndex : u32,
                        @builtin(instance_index) instanceIndex : u32) -> Output {
            let position = (f32(vertexIndex) + 0.5) * 2.0 / 5.0 - 1.0;
            return Output(vec4f(position, 0.0, 0.0, 1.0), f32(instanceIndex));
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read> data : array<u32>;
        @fragment fn main(@location(0) instanceIndex : f32) -> @location(0) vec4f {
            return vec4f(f32(arrayLength(&data)), instanceIndex, 0.0, 255.0) / 255.0;
        }
    )");
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 256;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    wgpu::BindGroup data =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});
    wgpu::Buffer indirect =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {1, 1, 1, 0});
    wgpu::Buffer indexedIndirect =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {1, 1, 1, 0, 0});
    wgpu::Buffer index =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {2, 3});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, data);
    pass.SetIndexBuffer(index, wgpu::IndexFormat::Uint32);
    pass.Draw(1, 1, 0, 1);
    pass.DrawIndirect(indirect, 0);
    pass.DrawIndexed(1, 1, 0, 0, 2);
    pass.DrawIndexedIndirect(indexedIndirect, 0);
    pass.Draw(1, 1, 4, 3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 1, 0, 255), renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 0, 0, 255), renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 2, 0, 255), renderPass.color, 2, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 0, 0, 255), renderPass.color, 3, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 3, 0, 255), renderPass.color, 4, 0);
}

TEST_P(OpArrayLengthTest, RenderBindGroupSwitch) {
    // TODO(crbug.com/366291600): Suspected native GLES/Vulkan arrayLength bug on Pixel 4
    // Android bots. Keep GLES coverage when array lengths are provided via immediates.
    DAWN_SUPPRESS_TEST_IF(
        IsAndroid() &&
        (IsVulkan() || (IsOpenGLES() && !HasToggleEnabled("gl_use_array_length_from_immediate"))));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read> data : array<u32>;
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(f32(arrayLength(&data)) / 255.0, 0.0, 0.0, 1.0);
        }
    )");
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);
    wgpu::BindGroupLayout dataLayout = pipeline.GetBindGroupLayout(0);
    wgpu::BufferDescriptor dataDesc;
    dataDesc.size = kBuffer2_size;
    dataDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer dataBuffer = device.CreateBuffer(&dataDesc);
    wgpu::BindGroup smallData =
        utils::MakeBindGroup(device, dataLayout, {{0, dataBuffer, 0, kBuffer1_size}});
    wgpu::BindGroup largeData =
        utils::MakeBindGroup(device, dataLayout, {{0, dataBuffer, 0, kBuffer2_size}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, smallData);
    pass.Draw(1);
    pass.SetBindGroup(0, largeData);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(64, 0, 0, 255), renderPass.color, 0, 0);
}

TEST_P(OpArrayLengthTest, ComputeIgnoresFragmentOnlyStorageBuffers) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage},
                 {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(1) var<storage, read_write> data : array<u32>;
        @compute @workgroup_size(1) fn main() {
            data[0] = arrayLength(&data);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 256;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer fragmentBuffer = device.CreateBuffer(&bufferDesc);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, bindGroupLayout, {{0, fragmentBuffer}, {1, buffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(64u, buffer, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_immediate"}),
                      VulkanBackend(),
                      WebGPUBackend());

// Verify that a fragment-only storage binding does not affect the array length
// reported for a compute-visible binding with a different bound range.
class OpArrayLengthVisibilityCollisionTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

TEST_P(OpArrayLengthVisibilityCollisionTest, ComputeWithFragmentOnlyBufferInLayout) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    // Buffer A: large, bound at full size to a FRAGMENT-only storage slot.
    constexpr uint32_t kBufASize = 1u << 20;  // 1 MiB → arrayLength<u32> = 262144
    wgpu::BufferDescriptor descA;
    descA.size = kBufASize;
    descA.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer bufA = device.CreateBuffer(&descA);

    // Bind only 64 bytes of B so its array length differs from both A's bound
    // range and B's full allocation. Observe the length using an in-bounds write.
    constexpr uint32_t kBufBSize = 16384;  // 16 KiB underlying allocation
    constexpr uint32_t kBufBBound = 64;    // 64 bytes bound → arrayLength = 16
    std::vector<uint32_t> initialData(kBufBSize / 4, 0u);
    wgpu::Buffer bufB = utils::CreateBufferFromData(
        device, initialData.data(), initialData.size() * sizeof(uint32_t),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // PipelineLayoutGL assigns ssboIndex by iterating groups in order, so:
    //   group 0 (A, FRAGMENT-only) → ssboIndex 0
    //   group 1 (B, COMPUTE)       → ssboIndex 1  → glIndex_B = 1
    // Choose A's WGSL @binding == glIndex_B (= 1) so that A's pre-remap key
    // {0,1} equals B's post-remap key {0, glIndex_B}.
    wgpu::BindGroupLayout bglA = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroupLayout bglB = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    const char* shaderSource = R"(
        @group(1) @binding(0) var<storage, read_write> B : array<u32>;
        @compute @workgroup_size(1) fn main() {
            B[0] = arrayLength(&B);
        })";

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakePipelineLayout(device, {bglA, bglB});
    pipelineDesc.compute.module = utils::CreateShaderModule(device, shaderSource);
    pipelineDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bgA = utils::MakeBindGroup(device, bglA, {{1, bufA, 0, kBufASize}});
    wgpu::BindGroup bgB = utils::MakeBindGroup(device, bglB, {{0, bufB, 0, kBufBBound}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bgA);
    pass.SetBindGroup(1, bgB);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(kBufBBound / 4, bufB, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthVisibilityCollisionTest,
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_immediate"}),
                      VulkanBackend());

enum class TieredLimits {
    No,
    Yes,
};

std::ostream& operator<<(std::ostream& o, TieredLimits tieredLimits) {
    switch (tieredLimits) {
        case TieredLimits::No:
            o << "NoTieredLimits";
            break;
        case TieredLimits::Yes:
            o << "TieredLimits";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(MaxArrayLengthTestParams, TieredLimits);

class MaxArrayLengthTest : public DawnTestWithParams<MaxArrayLengthTestParams> {
  protected:
    bool GetRequireUseTieredLimits() override {
        return GetParam().mTieredLimits == TieredLimits::Yes;
    }

    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }

    void SetUp() override {
        DawnTestWithParams<MaxArrayLengthTestParams>::SetUp();

        // Will fail with 'VK_ERROR_OUT_OF_DEVICE_MEMORY' due to the maxStorageBufferBindingSize
        // portion of the test
        DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode() || IsSwiftshader() || IsANGLESwiftShader() ||
                              IsOpenGLES());


        // TODO(crbug.com/473894293): [Capture] buffer mapping: investigate.
        DAWN_SUPPRESS_TEST_IF(IsCaptureReplayCheckingEnabled());

        // x86 has issues with OpArrayLength.
        DAWN_SUPPRESS_TEST_IF(IsX86());

        // TODO(crbug.com/485946556): Dawn native will report the wrong arrayLength for Apple
        // hardware.
        DAWN_SUPPRESS_TEST_IF(IsApple() && !GetRequireUseTieredLimits());

        auto maxBindingSizeSize = AlignDown(GetSupportedLimits().maxStorageBufferBindingSize, 4);

        // Create buffers of various sizes to check the length() implementation
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = maxBindingSizeSize;
        bufferDesc.usage = wgpu::BufferUsage::Storage;
        mStorageBufferMax = device.CreateBuffer(&bufferDesc);

        mExpectedLength = static_cast<uint32_t>(maxBindingSizeSize / 4);
    }

    wgpu::Buffer mStorageBufferMax;
    uint32_t mExpectedLength = 0;
};

// Test OpArrayLength in the compute stage
TEST_P(MaxArrayLengthTest, Compute) {
    // Create a buffer to hold the result sizes and create a bindgroup for it.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    bufferDesc.size = sizeof(uint32_t);
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroupLayout resultLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    wgpu::BindGroup resultBindGroup =
        utils::MakeBindGroup(device, resultLayout, {{0, resultBuffer, 0, wgpu::kWholeSize}});

    // Create the compute pipeline that stores the length()s in the result buffer.
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});

    wgpu::BindGroupLayout bgls[] = {bindGroupLayout, resultLayout};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 2;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(1) @binding(0) var<storage, read_write> result : u32;

        struct Buffer {
            data : array<u32>,
        }
        @group(0) @binding(0) var<storage, read> buffer1 : Buffer;

        @compute @workgroup_size(1) fn main() {
            result  = arrayLength(&buffer1.data);
        })");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                                     {{0, mStorageBufferMax, 0, wgpu::kWholeSize}});

    // Run a single instance of the compute shader
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.SetBindGroup(1, resultBindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(mExpectedLength, resultBuffer, 0);
}

DAWN_INSTANTIATE_TEST_P(MaxArrayLengthTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), OpenGLESBackend({"gl_use_array_length_from_immediate"}),
                         VulkanBackend(), WebGPUBackend()},
                        {TieredLimits::No, TieredLimits::Yes});

class OpArrayLengthVertexStorageTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

TEST_P(OpArrayLengthVertexStorageTest, VertexAndFragmentShareStorageBufferSizes) {
    // TODO(crbug.com/366291600): Suspected native GLES/Vulkan arrayLength bug on Pixel 4
    // Android bots. Keep GLES coverage when array lengths are provided via immediates.
    DAWN_SUPPRESS_TEST_IF(
        IsAndroid() &&
        (IsVulkan() || (IsOpenGLES() && !HasToggleEnabled("gl_use_array_length_from_immediate"))));

    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 2);
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 2);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read> vertexData : array<u32>;
        @group(0) @binding(1) var<storage, read> sharedData : array<u32>;
        struct Output {
            @builtin(position) position : vec4f,
            @location(0) length : f32,
        }
        @vertex fn main() -> Output {
            return Output(vec4f(0.0, 0.0, 0.0, 1.0),
                          f32(arrayLength(&vertexData) + arrayLength(&sharedData)));
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(1) var<storage, read> sharedData : array<u32>;
        @group(0) @binding(2) var<storage, read> fragmentData : array<u32>;
        @fragment fn main(@location(0) vertexLength : f32) -> @location(0) vec4f {
            return vec4f(vertexLength,
                         f32(arrayLength(&sharedData) + arrayLength(&fragmentData)),
                         f32(arrayLength(&sharedData)), 255.0) / 255.0;
        }
    )");
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 256;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, buffer, 0, 64}, {1, buffer, 0, 128}, {2, buffer, 0, 256}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(48, 96, 32, 255), renderPass.color, 0, 0);
}

TEST_P(OpArrayLengthVertexStorageTest, RenderStorageBufferSizesExceedInlineCapacity) {
    // TODO(crbug.com/366291600): Suspected native GLES/Vulkan arrayLength bug on Pixel 4
    // Android bots. Keep GLES coverage when array lengths are provided via immediates.
    DAWN_SUPPRESS_TEST_IF(
        IsAndroid() &&
        (IsVulkan() || (IsOpenGLES() && !HasToggleEnabled("gl_use_array_length_from_immediate"))));

    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersPerShaderStage < 9);
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 9);
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 9);

    constexpr uint32_t kVertexBufferCount = 9;
    constexpr uint32_t kFragmentBufferCount = 9;
    constexpr uint32_t kVertexBinding = 0;
    constexpr uint32_t kFragmentBinding = kVertexBufferCount;
    constexpr uint32_t kBindingCount = kVertexBufferCount + kFragmentBufferCount;

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto storageBufferSource = [](uint32_t firstBinding, uint32_t bufferCount) {
        std::string source;
        std::string totalLength = "0u";
        for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex) {
            std::string binding = std::to_string(firstBinding + bufferIndex);
            source += "@group(0) @binding(" + binding + ") var<storage, read> data" + binding +
                      " : array<u32>;\n";
            totalLength += " + arrayLength(&data" + binding + ")";
        }
        return source + "fn totalLength() -> u32 { return " + totalLength + "; }\n";
    };

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(
        device, storageBufferSource(kVertexBinding, kVertexBufferCount) + R"(
        struct Output {
            @builtin(position) position : vec4f,
            @location(0) length : f32,
        }
        @vertex fn main() -> Output {
            var output : Output;
            output.position = vec4f(0.0, 0.0, 0.0, 1.0);
            output.length = f32(totalLength()) / 255.0;
            return output;
        })");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(
        device, storageBufferSource(kFragmentBinding, kFragmentBufferCount) + R"(
        @fragment fn main(@location(0) vertex_length : f32) -> @location(0) vec4f {
            return vec4f(vertex_length, f32(totalLength()) / 255.0, 0.0, 1.0);
        })");
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer smallBuffer = device.CreateBuffer(&bufferDesc);
    bufferDesc.size = 256;
    wgpu::Buffer vertexBuffer = device.CreateBuffer(&bufferDesc);
    bufferDesc.size = 512;
    wgpu::Buffer fragmentBuffer = device.CreateBuffer(&bufferDesc);

    std::vector<wgpu::BindGroupEntry> bindings(kBindingCount);
    for (uint32_t i = 0; i < kBindingCount; ++i) {
        bindings[i].binding = i;
        bindings[i].buffer = smallBuffer;
    }
    bindings[kVertexBinding].buffer = vertexBuffer;
    bindings[kFragmentBinding].buffer = fragmentBuffer;
    wgpu::BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = pipeline.GetBindGroupLayout(0);
    bindGroupDesc.entryCount = bindings.size();
    bindGroupDesc.entries = bindings.data();
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bindGroupDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(72, 136, 0, 255), renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthVertexStorageTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_immediate"}),
                      VulkanBackend(),
                      WebGPUBackend());

class GLArrayLengthOverflowTest : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

// Test that using more than 96 ShaderStage::None bind group entries
// (which don't count against Dawn's validation limit) don't cause GL
// errors and failed buffer transfers.
TEST_P(GLArrayLengthOverflowTest, VisibilityNoneOverflowsArrayLengthBuffer) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxBindGroups < 4);

    constexpr uint32_t kLargeSize = 512u;
    constexpr uint32_t kSmallSize = 256u;
    constexpr uint32_t kPadPerGroup = 33;

    wgpu::BufferDescriptor bd;
    bd.size = kLargeSize;
    bd.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer largeBuf = device.CreateBuffer(&bd);

    // Get the arrayLength() of the passed-in storage buffer, and store it into
    // the first element of the array.
    wgpu::ComputePipelineDescriptor primeDesc;
    primeDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read_write> a : array<u32>;
        @compute @workgroup_size(1) fn main() {
            a[0] = arrayLength(&a);
        })");
    wgpu::ComputePipeline primePipeline = device.CreateComputePipeline(&primeDesc);
    wgpu::BindGroupLayout primeBGL = primePipeline.GetBindGroupLayout(0);
    wgpu::BindGroup primeBG = utils::MakeBindGroup(device, primeBGL, {{0, largeBuf}});

    {
        wgpu::CommandEncoder enc = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = enc.BeginComputePass();
        pass.SetPipeline(primePipeline);
        pass.SetBindGroup(0, primeBG);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer cb = enc.Finish();
        queue.Submit(1, &cb);
    }

    bd.size = kSmallSize;
    bd.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer smallBuf = device.CreateBuffer(&bd);

    bd.size = 4;
    bd.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer tinyBuffer = device.CreateBuffer(&bd);

    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    std::vector<wgpu::BindGroupLayoutEntry> padEntries(kPadPerGroup);
    for (uint32_t i = 0; i < kPadPerGroup; i++) {
        padEntries[i].binding = i;
        padEntries[i].visibility = wgpu::ShaderStage::None;
        padEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    }
    wgpu::BindGroupLayoutDescriptor padDesc;
    padDesc.entryCount = padEntries.size();
    padDesc.entries = padEntries.data();
    wgpu::BindGroupLayout bglPad = device.CreateBindGroupLayout(&padDesc);

    wgpu::BindGroupLayout bgls[] = {bgl0, bglPad, bglPad, bglPad};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 4;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout manyBindingsPL = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor manyBindingsDesc;
    manyBindingsDesc.layout = manyBindingsPL;
    manyBindingsDesc.compute.module = primeDesc.compute.module;
    wgpu::ComputePipeline manyBindingsPipeline = device.CreateComputePipeline(&manyBindingsDesc);

    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, smallBuf}});

    std::vector<wgpu::BindGroupEntry> padBinds(kPadPerGroup);
    for (uint32_t i = 0; i < kPadPerGroup; i++) {
        padBinds[i].binding = i;
        padBinds[i].buffer = tinyBuffer;
    }
    wgpu::BindGroupDescriptor bgPadDesc;
    bgPadDesc.layout = bglPad;
    bgPadDesc.entryCount = padBinds.size();
    bgPadDesc.entries = padBinds.data();
    wgpu::BindGroup bgPad = device.CreateBindGroup(&bgPadDesc);

    {
        wgpu::CommandEncoder enc = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = enc.BeginComputePass();
        pass.SetPipeline(manyBindingsPipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bgPad);
        pass.SetBindGroup(2, bgPad);
        pass.SetBindGroup(3, bgPad);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer cb = enc.Finish();
        queue.Submit(1, &cb);
    }

    // Check that the stored arrayLength is the (new) small buffer length
    // and not the (stale) large buffer length.
    EXPECT_BUFFER_U32_EQ(kSmallSize / 4u, smallBuf, 0);
}

DAWN_INSTANTIATE_TEST(GLArrayLengthOverflowTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_use_array_length_from_immediate"}),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
