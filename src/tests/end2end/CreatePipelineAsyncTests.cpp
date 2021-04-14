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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

namespace {
    struct CreatePipelineAsyncTask {
        wgpu::ComputePipeline computePipeline = nullptr;
        wgpu::RenderPipeline renderPipeline = nullptr;
        bool isCompleted = false;
        std::string message;
    };
}  // anonymous namespace

class CreatePipelineAsyncTest : public DawnTest {
  protected:
    void ValidateCreateComputePipelineAsync(CreatePipelineAsyncTask* currentTask) {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(uint32_t);
        bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer ssbo = device.CreateBuffer(&bufferDesc);

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

            while (!currentTask->isCompleted) {
                WaitABit();
            }
            ASSERT_TRUE(currentTask->message.empty());
            ASSERT_NE(nullptr, currentTask->computePipeline.Get());
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, currentTask->computePipeline.GetBindGroupLayout(0),
                                     {
                                         {0, ssbo, 0, sizeof(uint32_t)},
                                     });
            pass.SetBindGroup(0, bindGroup);
            pass.SetPipeline(currentTask->computePipeline);

            pass.Dispatch(1);
            pass.EndPass();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        constexpr uint32_t kExpected = 1u;
        EXPECT_BUFFER_U32_EQ(kExpected, ssbo, 0);
    }

    void ValidateCreateComputePipelineAsync() {
        ValidateCreateComputePipelineAsync(&task);
    }

    CreatePipelineAsyncTask task;
};

// Verify the basic use of CreateComputePipelineAsync works on all backends.
TEST_P(CreatePipelineAsyncTest, BasicUseOfCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[block]] struct SSBO {
            value : u32;
        };
        [[group(0), binding(0)]] var<storage> ssbo : [[access(read_write)]] SSBO;

        [[stage(compute)]] fn main() {
            ssbo.value = 1u;
        })");
    csDesc.computeStage.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success, status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    ValidateCreateComputePipelineAsync();
}

// Verify CreateComputePipelineAsync() works as expected when there is any error that happens during
// the creation of the compute pipeline. The SPEC requires that during the call of
// CreateComputePipelineAsync() any error won't be forwarded to the error scope / unhandled error
// callback.
TEST_P(CreatePipelineAsyncTest, CreateComputePipelineFailed) {
    DAWN_SKIP_TEST_IF(HasToggleEnabled("skip_validation"));

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[block]] struct SSBO {
            value : u32;
        };
        [[group(0), binding(0)]] var<storage> ssbo : [[access(read_write)]] SSBO;

        [[stage(compute)]] fn main() {
            ssbo.value = 1u;
        })");
    csDesc.computeStage.entryPoint = "main0";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Error, status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    while (!task.isCompleted) {
        WaitABit();
    }

    ASSERT_FALSE(task.message.empty());
    ASSERT_EQ(nullptr, task.computePipeline.Get());
}

// Verify the basic use of CreateRenderPipelineAsync() works on all backends.
TEST_P(CreatePipelineAsyncTest, BasicUseOfCreateRenderPipelineAsync) {
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    utils::ComboRenderPipelineDescriptor2 renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
            return vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success, status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = kRenderAttachmentFormat;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture outputTexture = device.CreateTexture(&textureDescriptor);

    utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPassDescriptor.cColorAttachments[0].clearColor = {1.f, 0.f, 0.f, 1.f};

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.renderPipeline.Get());

        renderPassEncoder.SetPipeline(task.renderPipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.EndPass();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), outputTexture, 0, 0);
}

// Verify CreateRenderPipelineAsync() works as expected when there is any error that happens during
// the creation of the render pipeline. The SPEC requires that during the call of
// CreateRenderPipelineAsync() any error won't be forwarded to the error scope / unhandled error
// callback.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineFailed) {
    DAWN_SKIP_TEST_IF(HasToggleEnabled("skip_validation"));

    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::Depth32Float;

    utils::ComboRenderPipelineDescriptor2 renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
            return vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Error, status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    while (!task.isCompleted) {
        WaitABit();
    }

    ASSERT_FALSE(task.message.empty());
    ASSERT_EQ(nullptr, task.computePipeline.Get());
}

// Verify there is no error when the device is released before the callback of
// CreateComputePipelineAsync() is called.
TEST_P(CreatePipelineAsyncTest, ReleaseDeviceBeforeCallbackOfCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[stage(compute)]] fn main() {
        })");
    csDesc.computeStage.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_DeviceDestroyed,
                      status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);
}

// Verify there is no error when the device is released before the callback of
// CreateRenderPipelineAsync() is called.
TEST_P(CreatePipelineAsyncTest, ReleaseDeviceBeforeCallbackOfCreateRenderPipelineAsync) {
    utils::ComboRenderPipelineDescriptor2 renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
            return vec4<f32>(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_DeviceDestroyed,
                      status);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);
}

// Verify the code path of CreateComputePipelineAsync() to directly return the compute pipeline
// object from cache works correctly.
TEST_P(CreatePipelineAsyncTest, CreateSameComputePipelineTwice) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[block]] struct SSBO {
            value : u32;
        };
        [[group(0), binding(0)]] var<storage> ssbo : [[access(read_write)]] SSBO;

        [[stage(compute)]] fn main() {
            ssbo.value = 1u;
        })");
    csDesc.computeStage.entryPoint = "main";

    auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
                       const char* message, void* userdata) {
        EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success, status);

        CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
        task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
        task->isCompleted = true;
        task->message = message;
    };

    // Create a pipeline object and save it into anotherTask.computePipeline.
    CreatePipelineAsyncTask anotherTask;
    device.CreateComputePipelineAsync(&csDesc, callback, &anotherTask);
    while (!anotherTask.isCompleted) {
        WaitABit();
    }
    ASSERT_TRUE(anotherTask.message.empty());
    ASSERT_NE(nullptr, anotherTask.computePipeline.Get());

    // Create another pipeline object task.comnputepipeline with the same compute pipeline
    // descriptor used in the creation of anotherTask.computePipeline. This time the pipeline
    // object should be directly got from the pipeline object cache.
    device.CreateComputePipelineAsync(&csDesc, callback, &task);
    ValidateCreateComputePipelineAsync();
}

// Verify creating compute pipeline with same descriptor and CreateComputePipelineAsync() at the
// same time works correctly.
TEST_P(CreatePipelineAsyncTest, CreateSamePipelineTwiceAtSameTime) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module = utils::CreateShaderModule(device, R"(
        [[block]] struct SSBO {
            value : u32;
        };
        [[group(0), binding(0)]] var<storage> ssbo : [[access(read_write)]] SSBO;

        [[stage(compute)]] fn main() {
            ssbo.value = 1u;
        })");
    csDesc.computeStage.entryPoint = "main";

    auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
                       const char* message, void* userdata) {
        EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success, status);

        CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
        task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
        task->isCompleted = true;
        task->message = message;
    };

    // Create two pipeline objects with same descriptor.
    CreatePipelineAsyncTask anotherTask;
    device.CreateComputePipelineAsync(&csDesc, callback, &task);
    device.CreateComputePipelineAsync(&csDesc, callback, &anotherTask);

    // Verify both task.computePipeline and anotherTask.computePipeline are created correctly.
    ValidateCreateComputePipelineAsync(&anotherTask);
    ValidateCreateComputePipelineAsync(&task);

    // Verify task.computePipeline and anotherTask.computePipeline are pointing to the same Dawn
    // object.
    if (!UsesWire()) {
        EXPECT_EQ(task.computePipeline.Get(), anotherTask.computePipeline.Get());
    }
}

DAWN_INSTANTIATE_TEST(CreatePipelineAsyncTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
