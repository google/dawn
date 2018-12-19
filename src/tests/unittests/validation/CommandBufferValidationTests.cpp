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

#include "utils/DawnHelpers.h"

class CommandBufferValidationTest : public ValidationTest {
};

// Test for an empty command buffer
TEST_F(CommandBufferValidationTest, Empty) {
    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .GetResult();
}

// Test that a command buffer cannot be ended mid render pass
TEST_F(CommandBufferValidationTest, EndedMidRenderPass) {
    dawn::RenderPassDescriptor renderpass = CreateSimpleRenderPass();

    // Control case, command buffer ended after the pass is ended.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.EndPass();
        builder.GetResult();
    }

    // Error case, command buffer ended mid-pass.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        builder.GetResult();
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after GetResult
    // should fail too.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        builder.GetResult();
        // TODO(cwallez@chromium.org) this should probably be a device error, but currently it
        // produces a builder error.
        pass.EndPass();
    }
}

// Test that a command buffer cannot be ended mid compute pass
TEST_F(CommandBufferValidationTest, EndedMidComputePass) {
    // Control case, command buffer ended after the pass is ended.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.EndPass();
        builder.GetResult();
    }

    // Error case, command buffer ended mid-pass.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        builder.GetResult();
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after GetResult
    // should fail too.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        builder.GetResult();
        // TODO(cwallez@chromium.org) this should probably be a device error, but currently it
        // produces a builder error.
        pass.EndPass();
    }
}

// Test that using a single buffer in multiple read usages in the same pass is allowed.
TEST_F(CommandBufferValidationTest, BufferWithMultipleReadUsage) {
    // Create a buffer used as both vertex and index buffer.
    dawn::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = dawn::BufferUsageBit::Vertex | dawn::BufferUsageBit::Index;
    bufferDescriptor.size = 4;
    dawn::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Use the buffer as both index and vertex in the same pass
    uint32_t zero = 0;
    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    auto renderpass = CreateSimpleRenderPass();
    dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
    pass.SetIndexBuffer(buffer, 0);
    pass.SetVertexBuffers(0, 1, &buffer, &zero);
    pass.EndPass();
    builder.GetResult();
}

// Test that using the same buffer as both readable and writable in the same pass is disallowed
TEST_F(CommandBufferValidationTest, BufferWithReadAndWriteUsage) {
    // Create a buffer that will be used as an index buffer and as a storage buffer
    dawn::BufferDescriptor bufferDescriptor;
    bufferDescriptor.usage = dawn::BufferUsageBit::Storage | dawn::BufferUsageBit::Index;
    bufferDescriptor.size = 4;
    dawn::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    // Create the bind group to use the buffer as storage
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(device, {{
        0, dawn::ShaderStageBit::Vertex, dawn::BindingType::StorageBuffer
    }});
    dawn::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

    // Use the buffer as both index and storage in the same pass
    dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
    auto renderpass = CreateSimpleRenderPass();
    dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
    pass.SetIndexBuffer(buffer, 0);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    builder.GetResult();
}

// Test that using the same texture as both readable and writable in the same pass is disallowed
TEST_F(CommandBufferValidationTest, TextureWithReadAndWriteUsage) {
    // Create a texture that will be used both as a sampled texture and a render target
    dawn::TextureDescriptor textureDescriptor;
    textureDescriptor.usage = dawn::TextureUsageBit::Sampled | dawn::TextureUsageBit::OutputAttachment;
    textureDescriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
    textureDescriptor.dimension = dawn::TextureDimension::e2D;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.arraySize = 1;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.levelCount = 1;
    dawn::Texture texture = device.CreateTexture(&textureDescriptor);
    dawn::TextureView view = texture.CreateDefaultTextureView();

    // Create the bind group to use the texture as sampled
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(device, {{
        0, dawn::ShaderStageBit::Vertex, dawn::BindingType::SampledTexture
    }});
    dawn::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view}});

    // Create the render pass that will use the texture as an output attachment
    dawn::RenderPassColorAttachmentDescriptor colorAttachment;
    colorAttachment.attachment = view;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    colorAttachment.loadOp = dawn::LoadOp::Load;
    colorAttachment.storeOp = dawn::StoreOp::Store;
    dawn::RenderPassDescriptor renderPass = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachments(1, &colorAttachment)
        .GetResult();

    // Use the texture as both sampeld and output attachment in the same pass
    dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
    dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass);
    pass.SetBindGroup(0, bg);
    pass.EndPass();
    builder.GetResult();
}
