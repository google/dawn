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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/WGPUHelpers.h"

class CommandBufferValidationTest : public ValidationTest {
};

// Test for an empty command buffer
TEST_F(CommandBufferValidationTest, Empty) {
    device.CreateCommandEncoder().Finish();
}

// Test that a command buffer cannot be ended mid render pass
TEST_F(CommandBufferValidationTest, EndedMidRenderPass) {
    DummyRenderPass dummyRenderPass(device);

    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
        pass.EndPass();
        encoder.Finish();
    }

    // Error case, command buffer ended mid-pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after Finish
    // should fail too.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
        ASSERT_DEVICE_ERROR(encoder.Finish());
        ASSERT_DEVICE_ERROR(pass.EndPass());
    }
}

// Test that a command buffer cannot be ended mid compute pass
TEST_F(CommandBufferValidationTest, EndedMidComputePass) {
    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.EndPass();
        encoder.Finish();
    }

    // Error case, command buffer ended mid-pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after Finish
    // should fail too.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
        ASSERT_DEVICE_ERROR(pass.EndPass());
    }
}

// Test that a render pass cannot be ended twice
TEST_F(CommandBufferValidationTest, RenderPassEndedTwice) {
    DummyRenderPass dummyRenderPass(device);

    // Control case, pass is ended once
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
        pass.EndPass();
        encoder.Finish();
    }

    // Error case, pass ended twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
        pass.EndPass();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that a compute pass cannot be ended twice
TEST_F(CommandBufferValidationTest, ComputePassEndedTwice) {
    // Control case, pass is ended once.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.EndPass();
        encoder.Finish();
    }

    // Error case, pass ended twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.EndPass();
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that beginning a compute pass before ending the previous pass causes an error.
TEST_F(CommandBufferValidationTest, BeginComputePassBeforeEndPreviousPass) {
    DummyRenderPass dummyRenderPass(device);

    // Beginning a compute pass before ending a render pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&dummyRenderPass);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.EndPass();
        renderPass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Beginning a compute pass before ending a compute pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass1 = encoder.BeginComputePass();
        wgpu::ComputePassEncoder computePass2 = encoder.BeginComputePass();
        computePass2.EndPass();
        computePass1.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that encoding command after a successful finish produces an error
TEST_F(CommandBufferValidationTest, CallsAfterASuccessfulFinish) {
    // A buffer that can be used in CopyBufferToBuffer
    wgpu::BufferDescriptor copyBufferDesc;
    copyBufferDesc.size = 16;
    copyBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer copyBuffer = device.CreateBuffer(&copyBufferDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.Finish();

    ASSERT_DEVICE_ERROR(encoder.CopyBufferToBuffer(copyBuffer, 0, copyBuffer, 0, 0));
}

// Test that encoding command after a failed finish produces an error
TEST_F(CommandBufferValidationTest, CallsAfterAFailedFinish) {
    // A buffer that can be used in CopyBufferToBuffer
    wgpu::BufferDescriptor copyBufferDesc;
    copyBufferDesc.size = 16;
    copyBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer copyBuffer = device.CreateBuffer(&copyBufferDesc);

    // A buffer that can't be used in CopyBufferToBuffer
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(buffer, 0, buffer, 0, 0);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    ASSERT_DEVICE_ERROR(encoder.CopyBufferToBuffer(copyBuffer, 0, copyBuffer, 0, 0));
}

// Test that using a single buffer in multiple read usages in the same pass is allowed.
TEST_F(CommandBufferValidationTest, BufferWithMultipleReadUsage) {
    // Create a buffer used as both vertex and index buffer.
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index;
    bufferDescriptor.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Use the buffer as both index and vertex in the same pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    DummyRenderPass dummyRenderPass(device);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
    pass.SetIndexBuffer(buffer);
    pass.SetVertexBuffer(0, buffer);
    pass.EndPass();
    encoder.Finish();
}

// Test that using the same buffer as both readable and writable in the same pass is disallowed
TEST_F(CommandBufferValidationTest, BufferWithReadAndWriteUsage) {
    // Create a buffer that will be used as an index buffer and as a storage buffer
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index;
    bufferDescriptor.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Create the bind group to use the buffer as storage
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer}});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

    // Use the buffer as both index and storage in the same pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    DummyRenderPass dummyRenderPass(device);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
    pass.SetIndexBuffer(buffer);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that using a single buffer in multiple read usages which include readonly storage usage in
// the same pass is allowed.
TEST_F(CommandBufferValidationTest, BufferWithMultipleReadAndReadOnlyStorageUsage) {
    // Create a buffer that will be used as an index buffer and as a storage buffer
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index;
    bufferDescriptor.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Create the bind group to use the buffer as storage
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BindingType::ReadonlyStorageBuffer}});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

    // Use the buffer as both index and storage in the same pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    DummyRenderPass dummyRenderPass(device);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
    pass.SetIndexBuffer(buffer);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    encoder.Finish();
}

// Test that using the same storage buffer as both readable and writable in the same pass is
// disallowed
TEST_F(CommandBufferValidationTest, BufferWithReadAndWriteStorageBufferUsage) {
    // Create a buffer that will be used as an storage buffer and as a readonly storage buffer
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = wgpu::BufferUsage::Storage;
    bufferDescriptor.size = 512;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Create the bind group to use the buffer as storage
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer},
                 {1, wgpu::ShaderStage::Fragment, wgpu::BindingType::ReadonlyStorageBuffer}});
    wgpu::BindGroup bg =
        utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}, {1, buffer, 256, 4}});

    // Use the buffer as both index and storage in the same pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    DummyRenderPass dummyRenderPass(device);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that using the same texture as both readable and writable in the same pass is disallowed
TEST_F(CommandBufferValidationTest, TextureWithReadAndWriteUsage) {
    // Create a texture that will be used both as a sampled texture and a render target
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.usage = wgpu::TextureUsage::Sampled | wgpu::TextureUsage::OutputAttachment;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.arrayLayerCount = 1;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.mipLevelCount = 1;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView view = texture.CreateView();

    // Create the bind group to use the texture as sampled
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BindingType::SampledTexture}});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

    // Create the render pass that will use the texture as an output attachment
    utils::ComboRenderPassDescriptor renderPass({view});

    // Use the texture as both sampeld and output attachment in the same pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}
