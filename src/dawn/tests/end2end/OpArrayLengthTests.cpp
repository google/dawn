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

#include <string>

#include "dawn/common/Assert.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class OpArrayLengthTest : public DawnTest {
  protected:
    void SetUp() {
        DawnTest::SetUp();

        // Create buffers of various size to check the length() implementation
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = 4;
        bufferDesc.usage = wgpu::BufferUsage::Storage;
        mStorageBuffer4 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 256;
        mStorageBuffer256 = device.CreateBuffer(&bufferDesc);

        bufferDesc.size = 512 + 256;
        mStorageBuffer512 = device.CreateBuffer(&bufferDesc);

        // Put them all in a bind group for tests to bind them easily.
        wgpu::ShaderStage kAllStages =
            wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Compute;
        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, kAllStages, wgpu::BufferBindingType::ReadOnlyStorage},
                     {1, kAllStages, wgpu::BufferBindingType::ReadOnlyStorage},
                     {2, kAllStages, wgpu::BufferBindingType::ReadOnlyStorage}});

        mBindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                          {
                                              {0, mStorageBuffer4, 0, 4},
                                              {1, mStorageBuffer256, 0, wgpu::kWholeSize},
                                              {2, mStorageBuffer512, 256, wgpu::kWholeSize},
                                          });

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

    wgpu::Buffer mStorageBuffer4;
    wgpu::Buffer mStorageBuffer256;
    wgpu::Buffer mStorageBuffer512;

    wgpu::BindGroupLayout mBindGroupLayout;
    wgpu::BindGroup mBindGroup;
    std::string mShaderInterface;
    std::array<uint32_t, 3> mExpectedLengths;
};

// Test OpArrayLength in the compute stage
TEST_P(OpArrayLengthTest, Compute) {
    // TODO(crbug.com/dawn/197): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsNvidia() && (IsOpenGL() || IsOpenGLES()));

    // TODO(crbug.com/dawn/1292): Some Intel drivers don't seem to like the
    // (spurious but harmless) offset=64 that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL());

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
    wgpu::BindGroupLayout bgls[] = {mBindGroupLayout, resultLayout};
    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 2;
    plDesc.bindGroupLayouts = bgls;
    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.entryPoint = "main";
    pipelineDesc.compute.module = utils::CreateShaderModule(device, (R"(
        struct ResultBuffer {
            data : array<u32, 3>
        }
        @group(1) @binding(0) var<storage, read_write> result : ResultBuffer;
        )" + mShaderInterface + R"(
        @stage(compute) @workgroup_size(1) fn main() {
            result.data[0] = arrayLength(&buffer1.data);
            result.data[1] = arrayLength(&buffer2.data);
            result.data[2] = arrayLength(&buffer3.data);
        })")
                                                                        .c_str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // Run a single instance of the compute shader
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, mBindGroup);
    pass.SetBindGroup(1, resultBindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(mExpectedLengths.data(), resultBuffer, 0, 3);
}

// Test OpArrayLength in the fragment stage
TEST_P(OpArrayLengthTest, Fragment) {
    // TODO(crbug.com/dawn/197): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsNvidia() && (IsOpenGL() || IsOpenGLES()));

    // TODO(crbug.com/dawn/1292): Some Intel drivers don't seem to like the
    // (spurious but harmless) offset=64 that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        @stage(fragment) fn main() -> @location(0) vec4<f32> {
            var fragColor : vec4<f32>;
            fragColor.r = f32(arrayLength(&buffer1.data)) / 255.0;
            fragColor.g = f32(arrayLength(&buffer2.data)) / 255.0;
            fragColor.b = f32(arrayLength(&buffer3.data)) / 255.0;
            fragColor.a = 0.0;
            return fragColor;
        })")
                                                                        .c_str());

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 expectedColor = RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

// Test OpArrayLength in the vertex stage
TEST_P(OpArrayLengthTest, Vertex) {
    // TODO(crbug.com/dawn/197): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. Also failing on all GLES (NV, Intel, SwANGLE).
    DAWN_SUPPRESS_TEST_IF(IsNvidia() && IsOpenGL());
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());

    // TODO(crbug.com/dawn/1292): Some Intel drivers don't seem to like the
    // (spurious but harmless) offset=64 that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, (mShaderInterface + R"(
        struct VertexOut {
            @location(0) color : vec4<f32>,
            @builtin(position) position : vec4<f32>,
        }

        @stage(vertex) fn main() -> VertexOut {
            var output : VertexOut;
            output.color.r = f32(arrayLength(&buffer1.data)) / 255.0;
            output.color.g = f32(arrayLength(&buffer2.data)) / 255.0;
            output.color.b = f32(arrayLength(&buffer3.data)) / 255.0;
            output.color.a = 0.0;

            output.position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            return output;
        })")
                                                                        .c_str());

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @stage(fragment)
        fn main(@location(0) color : vec4<f32>) -> @location(0) vec4<f32> {
            return color;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 expectedColor = RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(OpArrayLengthTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
