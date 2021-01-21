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

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "tests/DawnTest.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class GpuMemorySyncTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // TODO(crbug.com/tint/398): GLSL builtins don't work with SPIR-V reader.
        DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator"));
    }

    wgpu::Buffer CreateBuffer() {
        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = 4;
        srcDesc.usage =
            wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
        wgpu::Buffer buffer = device.CreateBuffer(&srcDesc);

        int myData = 0;
        queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));
        return buffer;
    }

    std::tuple<wgpu::ComputePipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForCompute(
        const wgpu::Buffer& buffer) {
        wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Data {
                [[offset(0)]] a : i32;
            };
            [[group(0), binding(0)]] var<storage_buffer> data : [[access(read_write)]] Data;
            [[stage(compute)]] fn main() -> void {
                data.a = data.a + 1;
            })");

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.computeStage.module = csModule;
        cpDesc.computeStage.entryPoint = "main";
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    std::tuple<wgpu::RenderPipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForRender(
        const wgpu::Buffer& buffer,
        wgpu::TextureFormat colorFormat) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Data {
                [[offset(0)]] i : i32;
            };
            [[group(0), binding(0)]] var<storage_buffer> data : [[access(read_write)]] Data;
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                data.i = data.i + 1;
                fragColor = vec4<f32>(f32(data.i) / 255.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor rpDesc(device);
        rpDesc.vertexStage.module = vsModule;
        rpDesc.cFragmentStage.module = fsModule;
        rpDesc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        rpDesc.cColorStates[0].format = colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});
        return std::make_tuple(pipeline, bindGroup);
    }
};

// Clear storage buffer with zero. Then read data, add one, and write the result to storage buffer
// in compute pass. Iterate this read-add-write steps per compute pass a few time. The successive
// iteration reads the result in buffer from last iteration, which makes the iterations a data
// dependency chain. The test verifies that data in buffer among iterations in compute passes is
// correctly synchronized.
TEST_P(GpuMemorySyncTests, ComputePass) {
    // Create pipeline, bind group, and buffer for compute pass.
    wgpu::Buffer buffer = CreateBuffer();
    wgpu::ComputePipeline compute;
    wgpu::BindGroup bindGroup;
    std::tie(compute, bindGroup) = CreatePipelineAndBindGroupForCompute(buffer);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Iterate the read-add-write operations in compute pass a few times.
    int iteration = 3;
    for (int i = 0; i < iteration; ++i) {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(compute);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_BUFFER_U32_EQ(iteration, buffer, 0);
}

// Clear storage buffer with zero. Then read data, add one, and write the result to storage buffer
// in render pass. Iterate this read-add-write steps per render pass a few time. The successive
// iteration reads the result in buffer from last iteration, which makes the iterations a data
// dependency chain. In addition, color output by fragment shader depends on the data in storage
// buffer, so we can check color in render target to verify that data in buffer among iterations in
// render passes is correctly synchronized.
TEST_P(GpuMemorySyncTests, RenderPass) {
    // Create pipeline, bind group, and buffer for render pass.
    wgpu::Buffer buffer = CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::RenderPipeline render;
    wgpu::BindGroup bindGroup;
    std::tie(render, bindGroup) =
        CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Iterate the read-add-write operations in render pass a few times.
    int iteration = 3;
    for (int i = 0; i < iteration; ++i) {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(render);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(iteration, 0, 0, 255), renderPass.color, 0, 0);
}

// Write into a storage buffer in a render pass. Then read that data in a compute
// pass. And verify the data flow is correctly synchronized.
TEST_P(GpuMemorySyncTests, RenderPassToComputePass) {
    // Create pipeline, bind group, and buffer for render pass and compute pass.
    wgpu::Buffer buffer = CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::RenderPipeline render;
    wgpu::BindGroup bindGroup0;
    std::tie(render, bindGroup0) =
        CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);

    wgpu::ComputePipeline compute;
    wgpu::BindGroup bindGroup1;
    std::tie(compute, bindGroup1) = CreatePipelineAndBindGroupForCompute(buffer);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Write data into a storage buffer in render pass.
    wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass0.SetPipeline(render);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.Draw(1);
    pass0.EndPass();

    // Read that data in compute pass.
    wgpu::ComputePassEncoder pass1 = encoder.BeginComputePass();
    pass1.SetPipeline(compute);
    pass1.SetBindGroup(0, bindGroup1);
    pass1.Dispatch(1);
    pass1.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_BUFFER_U32_EQ(2, buffer, 0);
}

// Write into a storage buffer in a compute pass. Then read that data in a render
// pass. And verify the data flow is correctly synchronized.
TEST_P(GpuMemorySyncTests, ComputePassToRenderPass) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    wgpu::Buffer buffer = CreateBuffer();
    wgpu::ComputePipeline compute;
    wgpu::BindGroup bindGroup1;
    std::tie(compute, bindGroup1) = CreatePipelineAndBindGroupForCompute(buffer);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::RenderPipeline render;
    wgpu::BindGroup bindGroup0;
    std::tie(render, bindGroup0) =
        CreatePipelineAndBindGroupForRender(buffer, renderPass.colorFormat);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Write data into a storage buffer in compute pass.
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, bindGroup1);
    pass0.Dispatch(1);
    pass0.EndPass();

    // Read that data in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, bindGroup0);
    pass1.Draw(1);
    pass1.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(2, 0, 0, 255), renderPass.color, 0, 0);
}

// Use an image as both sampled and readonly storage in a compute pass. This is a regression test
// for the Vulkan backend choosing different layouts for Sampled and ReadOnlyStorage.
TEST_P(GpuMemorySyncTests, SampledAndROStorageTextureInComputePass) {
    // TODO(dawn:483): Test is skipped on OpenGL because it uses WriteTexture which is
    // unimplemented.
    DAWN_SKIP_TEST_IF(IsOpenGL() || IsOpenGLES());

    // Create a storage + sampled texture of one texel initialized to 1
    wgpu::TextureDescriptor texDesc;
    texDesc.format = wgpu::TextureFormat::R32Uint;
    texDesc.size = {1, 1, 1};
    texDesc.usage =
        wgpu::TextureUsage::Storage | wgpu::TextureUsage::Sampled | wgpu::TextureUsage::CopyDst;
    wgpu::Texture tex = device.CreateTexture(&texDesc);

    wgpu::TextureCopyView copyView;
    copyView.texture = tex;
    wgpu::TextureDataLayout layout;
    wgpu::Extent3D copySize = {1, 1, 1};
    uint32_t kOne = 1;
    queue.WriteTexture(&copyView, &kOne, sizeof(kOne), &layout, &copySize);

    // Create a pipeline that loads the texture from both the sampled and storage paths.
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.computeStage.entryPoint = "main";
    pipelineDesc.computeStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Output {
            [[offset(0)]] sampledOut: u32;
            [[offset(4)]] storageOut: u32;
        };
        [[group(0), binding(0)]] var<storage_buffer> output : [[access(write)]] Output;
        [[group(0), binding(1)]] var<uniform_constant> sampledTex : texture_2d<u32>;
        [[group(0), binding(2)]] var<uniform_constant> storageTex : [[access(read)]] texture_storage_2d<r32uint>;

        [[stage(compute)]] fn main() -> void {
            output.sampledOut = textureLoad(sampledTex, vec2<i32>(0, 0), 0).x;
            output.storageOut = textureLoad(storageTex, vec2<i32>(0, 0)).x;
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // Run the compute pipeline and store the result in the buffer.
    wgpu::BufferDescriptor outputDesc;
    outputDesc.size = 8;
    outputDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer outputBuffer = device.CreateBuffer(&outputDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, outputBuffer},
                                                  {1, tex.CreateView()},
                                                  {2, tex.CreateView()},
                                              });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.Dispatch(1);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check the buffer's content is what we expect.
    EXPECT_BUFFER_U32_EQ(1, outputBuffer, 0);
    EXPECT_BUFFER_U32_EQ(1, outputBuffer, 4);
}

DAWN_INSTANTIATE_TEST(GpuMemorySyncTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class StorageToUniformSyncTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // TODO(crbug.com/tint/398): GLSL builtins don't work with SPIR-V reader.
        DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator"));
    }

    void CreateBuffer() {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform;
        mBuffer = device.CreateBuffer(&bufferDesc);
    }

    std::tuple<wgpu::ComputePipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForCompute() {
        wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Data {
                [[offset(0)]] a : f32;
            };
            [[group(0), binding(0)]] var<storage_buffer> data : [[access(read_write)]] Data;
            [[stage(compute)]] fn main() -> void {
                data.a = 1.0;
            })");

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.computeStage.module = csModule;
        cpDesc.computeStage.entryPoint = "main";
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, mBuffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    std::tuple<wgpu::RenderPipeline, wgpu::BindGroup> CreatePipelineAndBindGroupForRender(
        wgpu::TextureFormat colorFormat) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Contents {
                [[offset(0)]] color : f32;
            };
            [[group(0), binding(0)]] var<uniform> contents : Contents;

            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(contents.color, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor rpDesc(device);
        rpDesc.vertexStage.module = vsModule;
        rpDesc.cFragmentStage.module = fsModule;
        rpDesc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        rpDesc.cColorStates[0].format = colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, mBuffer}});
        return std::make_tuple(pipeline, bindGroup);
    }

    wgpu::Buffer mBuffer;
};

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the same command buffer.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithSameCommandBuffer) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1);
    pass0.EndPass();

    // Read that data in render pass.
    wgpu::RenderPassEncoder pass1 = encoder0.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.EndPass();

    wgpu::CommandBuffer commands = encoder0.Finish();
    queue.Submit(1, &commands);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue in one shot.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentCommandBuffers) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1);
    pass0.EndPass();
    cb[0] = encoder0.Finish();

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.EndPass();

    cb[1] = encoder1.Finish();
    queue.Submit(2, cb);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

// Write into a storage buffer in compute pass in a command buffer. Then read that data in a render
// pass. The two passes use the different command buffers. The command buffers are submitted to the
// queue separately.
TEST_P(StorageToUniformSyncTests, ReadAfterWriteWithDifferentQueueSubmits) {
    // Create pipeline, bind group, and buffer for compute pass and render pass.
    CreateBuffer();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::ComputePipeline compute;
    wgpu::BindGroup computeBindGroup;
    std::tie(compute, computeBindGroup) = CreatePipelineAndBindGroupForCompute();
    wgpu::RenderPipeline render;
    wgpu::BindGroup renderBindGroup;
    std::tie(render, renderBindGroup) = CreatePipelineAndBindGroupForRender(renderPass.colorFormat);

    // Write data into a storage buffer in compute pass.
    wgpu::CommandBuffer cb[2];
    wgpu::CommandEncoder encoder0 = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder0.BeginComputePass();
    pass0.SetPipeline(compute);
    pass0.SetBindGroup(0, computeBindGroup);
    pass0.Dispatch(1);
    pass0.EndPass();
    cb[0] = encoder0.Finish();
    queue.Submit(1, &cb[0]);

    // Read that data in render pass.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass1 = encoder1.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(render);
    pass1.SetBindGroup(0, renderBindGroup);
    pass1.Draw(1);
    pass1.EndPass();

    cb[1] = encoder1.Finish();
    queue.Submit(1, &cb[1]);

    // Verify the rendering result.
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kRed, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(StorageToUniformSyncTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

constexpr int kRTSize = 8;
constexpr int kVertexBufferStride = 4 * sizeof(float);

class MultipleWriteThenMultipleReadTests : public DawnTest {
  protected:
    wgpu::Buffer CreateZeroedBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = size;
        srcDesc.usage = usage;
        wgpu::Buffer buffer = device.CreateBuffer(&srcDesc);

        std::vector<uint8_t> zeros(size, 0);
        queue.WriteBuffer(buffer, 0, zeros.data(), size);

        return buffer;
    }
};

// Write into a few storage buffers in compute pass. Then read that data in a render pass. The
// readonly buffers in render pass include vertex buffer, index buffer, uniform buffer, and readonly
// storage buffer. Data to be read in all of these buffers in render pass depend on the write
// operation in compute pass.
TEST_P(MultipleWriteThenMultipleReadTests, SeparateBuffers) {
    // Create pipeline, bind group, and different buffers for compute pass.
    wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct VBContents {
            [[offset(0)]] pos : [[stride(16)]] array<vec4<f32>, 4>;
        };
        [[group(0), binding(0)]] var<storage_buffer> vbContents : [[access(read_write)]] VBContents;

        [[block]] struct IBContents {
            [[offset(0)]] indices : [[stride(16)]] array<vec4<i32>, 2>;
        };
        [[group(0), binding(1)]] var<storage_buffer> ibContents : [[access(read_write)]] IBContents;

        // TODO(crbug.com/tint/386): Use the same struct.
        [[block]] struct ColorContents1 {
            [[offset(0)]] color : f32;
        };
        [[block]] struct ColorContents2 {
            [[offset(0)]] color : f32;
        };
        [[group(0), binding(2)]] var<storage_buffer> uniformContents : [[access(read_write)]] ColorContents1;
        [[group(0), binding(3)]] var<storage_buffer> storageContents : [[access(read_write)]] ColorContents2;

        [[stage(compute)]] fn main() -> void {
            vbContents.pos[0] = vec4<f32>(-1.0, 1.0, 0.0, 1.0);
            vbContents.pos[1] = vec4<f32>(1.0, 1.0, 0.0, 1.0);
            vbContents.pos[2] = vec4<f32>(1.0, -1.0, 0.0, 1.0);
            vbContents.pos[3] = vec4<f32>(-1.0, -1.0, 0.0, 1.0);
            const dummy : i32 = 0;
            ibContents.indices[0] = vec4<i32>(0, 1, 2, 0);
            ibContents.indices[1] = vec4<i32>(2, 3, dummy, dummy);
            uniformContents.color = 1.0;
            storageContents.color = 1.0;
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.computeStage.module = csModule;
    cpDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);
    wgpu::Buffer vertexBuffer = CreateZeroedBuffer(
        kVertexBufferStride * 4,
        wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    wgpu::Buffer indexBuffer = CreateZeroedBuffer(
        sizeof(int) * 4 * 2,
        wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    wgpu::Buffer uniformBuffer =
        CreateZeroedBuffer(sizeof(float), wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage |
                                              wgpu::BufferUsage::CopyDst);
    wgpu::Buffer storageBuffer =
        CreateZeroedBuffer(sizeof(float), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
        device, cp.GetBindGroupLayout(0),
        {{0, vertexBuffer}, {1, indexBuffer}, {2, uniformBuffer}, {3, storageBuffer}});
    // Write data into storage buffers in compute pass.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(cp);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.Dispatch(1);
    pass0.EndPass();

    // Create pipeline, bind group, and reuse buffers in render pass.
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<in> pos : vec4<f32>;
        [[builtin(position)]] var<out> Position: vec4<f32>;
        [[stage(vertex)]] fn main() -> void {
            Position = pos;
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buf {
            [[offset(0)]] color : f32;
        };

        [[group(0), binding(0)]] var<uniform> uniformBuffer : Buf;
        [[group(0), binding(1)]] var<storage_buffer> storageBuffer : [[access(read)]] Buf;

        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor = vec4<f32>(uniformBuffer.color, storageBuffer.color, 0.0, 1.0);
        })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor rpDesc(device);
    rpDesc.vertexStage.module = vsModule;
    rpDesc.cFragmentStage.module = fsModule;
    rpDesc.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
    rpDesc.cVertexState.vertexBufferCount = 1;
    rpDesc.cVertexState.cVertexBuffers[0].arrayStride = kVertexBufferStride;
    rpDesc.cVertexState.cVertexBuffers[0].attributeCount = 1;
    rpDesc.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
    rpDesc.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline rp = device.CreateRenderPipeline(&rpDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, rp.GetBindGroupLayout(0),
                                                      {{0, uniformBuffer}, {1, storageBuffer}});

    // Read data in buffers in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(rp);
    pass1.SetVertexBuffer(0, vertexBuffer);
    pass1.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
    pass1.SetBindGroup(0, bindGroup1);
    pass1.DrawIndexed(6);
    pass1.EndPass();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Verify the rendering result.
    int min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, max, max);
}

// Write into a storage buffer in compute pass. Then read that data in buffer in a render pass. The
// buffer is composed of vertices, indices, uniforms and readonly storage. Data to be read in the
// buffer in render pass depend on the write operation in compute pass.
TEST_P(MultipleWriteThenMultipleReadTests, OneBuffer) {
    // Create pipeline, bind group, and a complex buffer for compute pass.
    wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Contents {
            [[offset(0)]] pos : [[stride(16)]] array<vec4<f32>, 4>;
            [[offset(256)]] indices : [[stride(16)]] array<vec4<i32>, 2>;
            [[offset(512)]] color0 : f32;
            [[offset(768)]] color1 : f32;
        };

        [[group(0), binding(0)]] var<storage_buffer> contents : [[access(read_write)]] Contents;

        [[stage(compute)]] fn main() -> void {
            contents.pos[0] = vec4<f32>(-1.0, 1.0, 0.0, 1.0);
            contents.pos[1] = vec4<f32>(1.0, 1.0, 0.0, 1.0);
            contents.pos[2] = vec4<f32>(1.0, -1.0, 0.0, 1.0);
            contents.pos[3] = vec4<f32>(-1.0, -1.0, 0.0, 1.0);
            const dummy : i32 = 0;
            contents.indices[0] = vec4<i32>(0, 1, 2, 0);
            contents.indices[1] = vec4<i32>(2, 3, dummy, dummy);
            contents.color0 = 1.0;
            contents.color1 = 1.0;
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.computeStage.module = csModule;
    cpDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);
    struct Data {
        float pos[4][4];
        char padding0[256 - sizeof(float) * 16];
        int indices[2][4];
        char padding1[256 - sizeof(int) * 8];
        float color0;
        char padding2[256 - sizeof(float)];
        float color1;
    };
    wgpu::Buffer buffer = CreateZeroedBuffer(
        sizeof(Data), wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                          wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage |
                          wgpu::BufferUsage::CopyDst);
    wgpu::BindGroup bindGroup0 =
        utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer}});

    // Write various data (vertices, indices, and uniforms) into the buffer in compute pass.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
    pass0.SetPipeline(cp);
    pass0.SetBindGroup(0, bindGroup0);
    pass0.Dispatch(1);
    pass0.EndPass();

    // Create pipeline, bind group, and reuse the buffer in render pass.
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<in> pos : vec4<f32>;
        [[builtin(position)]] var<out> Position : vec4<f32>;
        [[stage(vertex)]] fn main() -> void {
            Position = pos;
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buf {
            [[offset(0)]] color : f32;
        };
        [[group(0), binding(0)]] var<uniform> uniformBuffer : Buf;
        [[group(0), binding(1)]] var<storage_buffer> storageBuffer : [[access(read)]] Buf;

        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor = vec4<f32>(uniformBuffer.color, storageBuffer.color, 0.0, 1.0);
        })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    utils::ComboRenderPipelineDescriptor rpDesc(device);
    rpDesc.vertexStage.module = vsModule;
    rpDesc.cFragmentStage.module = fsModule;
    rpDesc.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
    rpDesc.cVertexState.vertexBufferCount = 1;
    rpDesc.cVertexState.cVertexBuffers[0].arrayStride = kVertexBufferStride;
    rpDesc.cVertexState.cVertexBuffers[0].attributeCount = 1;
    rpDesc.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
    rpDesc.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline rp = device.CreateRenderPipeline(&rpDesc);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, rp.GetBindGroupLayout(0),
                             {{0, buffer, offsetof(Data, color0), sizeof(float)},
                              {1, buffer, offsetof(Data, color1), sizeof(float)}});

    // Read various data in the buffer in render pass.
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass1.SetPipeline(rp);
    pass1.SetVertexBuffer(0, buffer);
    pass1.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, offsetof(Data, indices));
    pass1.SetBindGroup(0, bindGroup1);
    pass1.DrawIndexed(6);
    pass1.EndPass();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Verify the rendering result.
    int min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kYellow, renderPass.color, max, max);
}

DAWN_INSTANTIATE_TEST(MultipleWriteThenMultipleReadTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
