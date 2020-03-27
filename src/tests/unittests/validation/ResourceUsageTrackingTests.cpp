// Copyright 2020 The Dawn Authors
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

namespace {

    class ResourceUsageTrackingTest : public ValidationTest {
      protected:
        wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage) {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = usage;

            return device.CreateBuffer(&descriptor);
        }

        wgpu::Texture CreateTexture(wgpu::TextureUsage usage, wgpu::TextureFormat format) {
            wgpu::TextureDescriptor descriptor;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.size = {1, 1, 1};
            descriptor.arrayLayerCount = 1;
            descriptor.sampleCount = 1;
            descriptor.mipLevelCount = 1;
            descriptor.usage = usage;
            descriptor.format = format;

            return device.CreateTexture(&descriptor);
        }
    };

    // Test that using a single buffer in multiple read usages in the same pass is allowed.
    TEST_F(ResourceUsageTrackingTest, BufferWithMultipleReadUsage) {
        // Test render pass
        {
            // Create a buffer, and use the buffer as both vertex and index buffer.
            wgpu::Buffer buffer =
                CreateBuffer(4, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            DummyRenderPass dummyRenderPass(device);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
            pass.SetIndexBuffer(buffer);
            pass.SetVertexBuffer(0, buffer);
            pass.EndPass();
            encoder.Finish();
        }

        // Test compute pass
        {
            // Create buffer and bind group
            wgpu::Buffer buffer =
                CreateBuffer(4, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device,
                {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer},
                 {1, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer}});
            wgpu::BindGroup bg =
                utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}, {1, buffer, 0, 4}});

            // Use the buffer as both uniform and readonly storage buffer in compute pass.
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg);
            pass.EndPass();
            encoder.Finish();
        }
    }

    // Test that using the same buffer as both readable and writable in the same pass is disallowed
    TEST_F(ResourceUsageTrackingTest, BufferWithReadAndWriteUsage) {
        // test render pass for index buffer and storage buffer
        {
            // Create buffer and bind group
            wgpu::Buffer buffer =
                CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer}});
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}});

            // Use the buffer as both index and storage in render pass
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            DummyRenderPass dummyRenderPass(device);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
            pass.SetIndexBuffer(buffer);
            pass.SetBindGroup(0, bg);
            pass.EndPass();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        // test compute pass
        {
            // Create buffer and bind group
            wgpu::Buffer buffer = CreateBuffer(512, wgpu::BufferUsage::Storage);

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device,
                {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                 {1, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer}});
            wgpu::BindGroup bg =
                utils::MakeBindGroup(device, bgl, {{0, buffer, 0, 4}, {1, buffer, 256, 4}});

            // Use the buffer as both storage and readonly storage in compute pass
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg);
            pass.EndPass();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    // Test that using the same buffer as copy src/dst and writable/readable usage is allowed.
    TEST_F(ResourceUsageTrackingTest, BufferCopyAndBufferUsageInPass) {
        // Create buffers that will be used as an copy src/dst buffer and as a storage buffer
        wgpu::Buffer bufferSrc =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
        wgpu::Buffer bufferDst =
            CreateBuffer(4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

        // Create the bind group to use the buffer as storage
        wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BindingType::StorageBuffer}});
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, bufferSrc, 0, 4}});
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl1, {{0, bufferDst, 0, 4}});

        // Use the buffer as both copy src and storage in render pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(bufferSrc, 0, bufferDst, 0, 4);
            DummyRenderPass dummyRenderPass(device);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&dummyRenderPass);
            pass.SetBindGroup(0, bg0);
            pass.EndPass();
            encoder.Finish();
        }

        // Use the buffer as both copy dst and readonly storage in compute pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(bufferSrc, 0, bufferDst, 0, 4);
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg1);
            pass.EndPass();
            encoder.Finish();
        }
    }

    // Test that using the same texture as both readable and writable in the same pass is disallowed
    TEST_F(ResourceUsageTrackingTest, TextureWithReadAndWriteUsage) {
        // Test render pass
        {
            // Create a texture that will be used both as a sampled texture and a render target
            wgpu::Texture texture =
                CreateTexture(wgpu::TextureUsage::Sampled | wgpu::TextureUsage::OutputAttachment,
                              wgpu::TextureFormat::RGBA8Unorm);
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

        // TODO(yunchao.he@intel.com) Test compute pass, which depends on writeonly storage buffer
    }

    // Test that using a single texture as copy src/dst and writable/readable usage in pass is
    // allowed.
    TEST_F(ResourceUsageTrackingTest, TextureCopyAndTextureUsageInPass) {
        // Create a texture that will be used both as a sampled texture and a render target
        wgpu::Texture texture0 =
            CreateTexture(wgpu::TextureUsage::CopySrc, wgpu::TextureFormat::RGBA8Unorm);
        wgpu::Texture texture1 =
            CreateTexture(wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled |
                              wgpu::TextureUsage::OutputAttachment,
                          wgpu::TextureFormat::RGBA8Unorm);
        wgpu::TextureView view0 = texture0.CreateView();
        wgpu::TextureView view1 = texture1.CreateView();

        wgpu::TextureCopyView srcView = utils::CreateTextureCopyView(texture0, 0, 0, {0, 0, 0});
        wgpu::TextureCopyView dstView = utils::CreateTextureCopyView(texture1, 0, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {1, 1, 1};

        // Use the texture as both copy dst and output attachment in render pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToTexture(&srcView, &dstView, &copySize);
            utils::ComboRenderPassDescriptor renderPass({view1});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.EndPass();
            encoder.Finish();
        }

        // Use the texture as both copy dst and readable usage in compute pass
        {
            // Create the bind group to use the texture as sampled
            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::BindingType::SampledTexture}});
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, view1}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToTexture(&srcView, &dstView, &copySize);
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, bg);
            pass.EndPass();
            encoder.Finish();
        }
    }

    // TODO (yunchao.he@intel.com):
    // 1. Add tests for overritten tests:
    //     1) multiple SetBindGroup on the same index
    //     2) multiple SetVertexBuffer on the same index
    //     3) multiple SetIndexBuffer
    // 2. useless bindings in bind groups. For example, a bind group includes bindings for compute
    // stage, but the bind group is used in render pass.
    // 3. more read write tracking tests for texture which need readonly storage texture and
    // writeonly storage texture support
    // 4. resource write and read dependency
    //     1) across passes (render + render, compute + compute, compute and render mixed) is valid
    //     2) across draws/dispatches is invalid

}  // anonymous namespace
