// Copyright 2018 The Dawn Authors
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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

    class QueueSubmitValidationTest : public ValidationTest {};

    // Test submitting with a mapped buffer is disallowed
    TEST_F(QueueSubmitValidationTest, SubmitWithMappedBuffer) {
        // Create a map-write buffer.
        const uint64_t kBufferSize = 4;
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        descriptor.size = kBufferSize;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        // Create a fake copy destination buffer
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer targetBuffer = device.CreateBuffer(&descriptor);

        // Create a command buffer that reads from the mappable buffer.
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        wgpu::Queue queue = device.GetQueue();

        // Submitting when the buffer has never been mapped should succeed
        queue.Submit(1, &commands);

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Map the buffer, submitting when the buffer is mapped should fail
        buffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, nullptr, nullptr);

        // Try submitting before the callback is fired.
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        WaitForAllOperations(device);

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Try submitting after the callback is fired.
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        // Unmap the buffer, queue submit should succeed
        buffer.Unmap();
        queue.Submit(1, &commands);
    }

    // Test it is invalid to submit a command buffer twice
    TEST_F(QueueSubmitValidationTest, CommandBufferSubmittedTwice) {
        wgpu::CommandBuffer commandBuffer = device.CreateCommandEncoder().Finish();
        wgpu::Queue queue = device.GetQueue();

        // Should succeed
        queue.Submit(1, &commandBuffer);

        // Should fail because command buffer was already submitted
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commandBuffer));
    }

    // Test resubmitting failed command buffers
    TEST_F(QueueSubmitValidationTest, CommandBufferSubmittedFailed) {
        // Create a map-write buffer
        const uint64_t kBufferSize = 4;
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        descriptor.size = kBufferSize;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        // Create a destination buffer for the b2b copy
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        descriptor.size = kBufferSize;
        wgpu::Buffer targetBuffer = device.CreateBuffer(&descriptor);

        // Create a command buffer that reads from the mappable buffer
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToBuffer(buffer, 0, targetBuffer, 0, kBufferSize);
            commands = encoder.Finish();
        }

        wgpu::Queue queue = device.GetQueue();

        // Map the source buffer to force a failure
        buffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, nullptr, nullptr);

        // Submitting a command buffer with a mapped buffer should fail
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));

        // Unmap buffer to fix the failure
        buffer.Unmap();

        // Resubmitting any command buffer, even if the problem was fixed, should fail
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }

    // Test that submitting in a buffer mapping callback doesn't cause re-entrance problems.
    TEST_F(QueueSubmitValidationTest, SubmitInBufferMapCallback) {
        // Create a buffer for mapping, to run our callback.
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        struct CallbackData {
            wgpu::Device device;
            wgpu::Buffer buffer;
        } callbackData = {device, buffer};

        const auto callback = [](WGPUBufferMapAsyncStatus status, void* userdata) {
            CallbackData* data = reinterpret_cast<CallbackData*>(userdata);

            data->buffer.Unmap();

            wgpu::Queue queue = data->device.GetQueue();
            queue.Submit(0, nullptr);
        };

        buffer.MapAsync(wgpu::MapMode::Write, 0, descriptor.size, callback, &callbackData);

        WaitForAllOperations(device);
    }

    // Test that submitting in a render pipeline creation callback doesn't cause re-entrance
    // problems.
    TEST_F(QueueSubmitValidationTest, SubmitInCreateRenderPipelineAsyncCallback) {
        struct CallbackData {
            wgpu::Device device;
        } callbackData = {device};

        const auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline,
                                 char const* message, void* userdata) {
            CallbackData* data = reinterpret_cast<CallbackData*>(userdata);

            wgpuRenderPipelineRelease(pipeline);

            wgpu::Queue queue = data->device.GetQueue();
            queue.Submit(0, nullptr);
        };

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        device.CreateRenderPipelineAsync(&descriptor, callback, &callbackData);

        WaitForAllOperations(device);
    }

    // Test that submitting in a compute pipeline creation callback doesn't cause re-entrance
    // problems.
    TEST_F(QueueSubmitValidationTest, SubmitInCreateComputePipelineAsyncCallback) {
        struct CallbackData {
            wgpu::Device device;
        } callbackData = {device};

        const auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline,
                                 char const* message, void* userdata) {
            CallbackData* data = reinterpret_cast<CallbackData*>(userdata);

            wgpuComputePipelineRelease(pipeline);

            wgpu::Queue queue = data->device.GetQueue();
            queue.Submit(0, nullptr);
        };

        wgpu::ComputePipelineDescriptor descriptor;
        descriptor.compute.module = utils::CreateShaderModule(device, R"(
            @stage(compute) @workgroup_size(1) fn main() {
            })");
        descriptor.compute.entryPoint = "main";
        device.CreateComputePipelineAsync(&descriptor, callback, &callbackData);

        WaitForAllOperations(device);
    }

    // Test that buffers in unused compute pass bindgroups are still checked for in
    // Queue::Submit validation.
    TEST_F(QueueSubmitValidationTest, SubmitWithUnusedComputeBuffer) {
        wgpu::Queue queue = device.GetQueue();

        wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
        wgpu::BindGroup emptyBG = utils::MakeBindGroup(device, emptyBGL, {});

        wgpu::BindGroupLayout testBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

        // In this test we check that BindGroup 1 is checked, the texture test will check
        // BindGroup 2. This is to provide coverage of for loops in validation code.
        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.layout = utils::MakePipelineLayout(device, {emptyBGL, testBGL});
        cpDesc.compute.entryPoint = "main";
        cpDesc.compute.module =
            utils::CreateShaderModule(device, "@stage(compute) @workgroup_size(1) fn main() {}");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::BufferDescriptor bufDesc;
        bufDesc.size = 4;
        bufDesc.usage = wgpu::BufferUsage::Storage;

        // Test that completely unused bindgroups still have their buffers checked.
        for (bool destroy : {true, false}) {
            wgpu::Buffer unusedBuffer = device.CreateBuffer(&bufDesc);
            wgpu::BindGroup unusedBG = utils::MakeBindGroup(device, testBGL, {{0, unusedBuffer}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(1, unusedBG);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (destroy) {
                unusedBuffer.Destroy();
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }

        // Test that unused bindgroups because they were replaced still have their buffers checked.
        for (bool destroy : {true, false}) {
            wgpu::Buffer unusedBuffer = device.CreateBuffer(&bufDesc);
            wgpu::BindGroup unusedBG = utils::MakeBindGroup(device, testBGL, {{0, unusedBuffer}});

            wgpu::Buffer usedBuffer = device.CreateBuffer(&bufDesc);
            wgpu::BindGroup usedBG = utils::MakeBindGroup(device, testBGL, {{0, unusedBuffer}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, emptyBG);
            pass.SetBindGroup(1, unusedBG);
            pass.SetBindGroup(1, usedBG);
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (destroy) {
                unusedBuffer.Destroy();
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }
    }

    // Test that textures in unused compute pass bindgroups are still checked for in
    // Queue::Submit validation.
    TEST_F(QueueSubmitValidationTest, SubmitWithUnusedComputeTextures) {
        wgpu::Queue queue = device.GetQueue();

        wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
        wgpu::BindGroup emptyBG = utils::MakeBindGroup(device, emptyBGL, {});

        wgpu::BindGroupLayout testBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});

        wgpu::ComputePipelineDescriptor cpDesc;
        cpDesc.layout = utils::MakePipelineLayout(device, {emptyBGL, emptyBGL, testBGL});
        cpDesc.compute.entryPoint = "main";
        cpDesc.compute.module =
            utils::CreateShaderModule(device, "@stage(compute) @workgroup_size(1) fn main() {}");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&cpDesc);

        wgpu::TextureDescriptor texDesc;
        texDesc.size = {1, 1, 1};
        texDesc.usage = wgpu::TextureUsage::TextureBinding;
        texDesc.format = wgpu::TextureFormat::RGBA8Unorm;

        // Test that completely unused bindgroups still have their buffers checked.
        for (bool destroy : {true, false}) {
            wgpu::Texture unusedTexture = device.CreateTexture(&texDesc);
            wgpu::BindGroup unusedBG =
                utils::MakeBindGroup(device, testBGL, {{0, unusedTexture.CreateView()}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(2, unusedBG);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (destroy) {
                unusedTexture.Destroy();
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }

        // Test that unused bindgroups because they were replaced still have their buffers checked.
        for (bool destroy : {true, false}) {
            wgpu::Texture unusedTexture = device.CreateTexture(&texDesc);
            wgpu::BindGroup unusedBG =
                utils::MakeBindGroup(device, testBGL, {{0, unusedTexture.CreateView()}});

            wgpu::Texture usedTexture = device.CreateTexture(&texDesc);
            wgpu::BindGroup usedBG =
                utils::MakeBindGroup(device, testBGL, {{0, unusedTexture.CreateView()}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, emptyBG);
            pass.SetBindGroup(1, emptyBG);
            pass.SetBindGroup(2, unusedBG);
            pass.SetBindGroup(2, usedBG);
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (destroy) {
                unusedTexture.Destroy();
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }
    }

}  // anonymous namespace
