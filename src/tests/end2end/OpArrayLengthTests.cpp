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
#include "utils/WGPUHelpers.h"

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
            // TODO(crbug.com/tint/386): Use the same struct.
            [[block]] struct DataBuffer1 {
                [[offset(0)]] data : [[stride(4)]] array<f32>;
            };

            [[block]] struct DataBuffer2 {
                [[offset(0)]] data : [[stride(4)]] array<f32>;
            };

            // The length should be 1 because the buffer is 4-byte long.
            [[group(0), binding(0)]] var<storage_buffer> buffer1 : [[access(read)]] DataBuffer1;

            // The length should be 64 because the buffer is 256 bytes long.
            [[group(0), binding(1)]] var<storage_buffer> buffer2 : [[access(read)]] DataBuffer2;

            // The length should be (512 - 16*4) / 8 = 56 because the buffer is 512 bytes long
            // and the structure is 8 bytes big.
            struct Buffer3Data {
                [[offset(0)]] a : f32;
                [[offset(4)]] b : i32;
            };

            [[block]] struct Buffer3 {
                [[offset(0)]] garbage : mat4x4<f32>;
                [[offset(64)]] data : [[stride(8)]] array<Buffer3Data>;
            };
            [[group(0), binding(2)]] var<storage_buffer> buffer3 : [[access(read)]] Buffer3;
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
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. See https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGLES());

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
    pipelineDesc.computeStage.entryPoint = "main";
    pipelineDesc.computeStage.module = utils::CreateShaderModuleFromWGSL(device, (R"(
        [[block]] struct ResultBuffer {
            [[offset(0)]] data : [[stride(4)]] array<u32, 3>;
        };
        [[group(1), binding(0)]] var<storage_buffer> result : [[access(read_write)]] ResultBuffer;
        )" + mShaderInterface + R"(
        [[stage(compute)]] fn main() -> void {
            result.data[0] = arrayLength(buffer1.data);
            result.data[1] = arrayLength(buffer2.data);
            result.data[2] = arrayLength(buffer3.data);
        })")
                                                                                     .c_str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // Run a single instance of the compute shader
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, mBindGroup);
    pass.SetBindGroup(1, resultBindGroup);
    pass.Dispatch(1);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(mExpectedLengths.data(), resultBuffer, 0, 3);
}

// Test OpArrayLength in the fragment stage
TEST_P(OpArrayLengthTest, Fragment) {
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. See https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGLES());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(position)]] var<out> Position : vec4<f32>;
        [[stage(vertex)]] fn main() -> void {
            Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, (mShaderInterface + R"(
        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor.r = f32(arrayLength(buffer1.data)) / 255.0;
            fragColor.g = f32(arrayLength(buffer2.data)) / 255.0;
            fragColor.b = f32(arrayLength(buffer3.data)) / 255.0;
            fragColor.a = 0.0;
        })")
                                                                                .c_str());

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;
    descriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
    descriptor.cColorStates[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup);
        pass.Draw(1);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 expectedColor = RGBA8(mExpectedLengths[0], mExpectedLengths[1], mExpectedLengths[2], 0);
    EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
}

// Test OpArrayLength in the vertex stage
TEST_P(OpArrayLengthTest, Vertex) {
    // TODO(cwallez@chromium.org): The computations for length() of unsized buffer is broken on
    // Nvidia OpenGL. Also failing on SwANGLE. See
    // https://bugs.chromium.org/p/dawn/issues/detail?id=197
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGL());
    DAWN_SKIP_TEST_IF(IsNvidia() && IsOpenGLES() || IsANGLE());
    // TODO(crbug.com/dawn/657): Returned data is slightly incorrect in this case.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator") && IsIntel() && IsOpenGL());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Create the pipeline that computes the length of the buffers and writes it to the only render
    // pass pixel.
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, (mShaderInterface + R"(
        [[location(0)]] var<out> pointColor : vec4<f32>;
        [[builtin(position)]] var<out> Position : vec4<f32>;
        [[stage(vertex)]] fn main() -> void {
            pointColor.r = f32(arrayLength(buffer1.data)) / 255.0;
            pointColor.g = f32(arrayLength(buffer2.data)) / 255.0;
            pointColor.b = f32(arrayLength(buffer3.data)) / 255.0;
            pointColor.a = 0.0;

            Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })")
                                                                                .c_str());

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[location(0)]] var<in> pointColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor = pointColor;
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;
    descriptor.primitiveTopology = wgpu::PrimitiveTopology::PointList;
    descriptor.cColorStates[0].format = renderPass.colorFormat;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // "Draw" the lengths to the texture.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, mBindGroup);
        pass.Draw(1);
        pass.EndPass();
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
