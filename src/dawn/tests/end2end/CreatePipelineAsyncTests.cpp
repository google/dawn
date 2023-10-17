// Copyright 2020 The Dawn & Tint Authors
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

#include <string>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

struct CreatePipelineAsyncTask {
    wgpu::ComputePipeline computePipeline = nullptr;
    wgpu::RenderPipeline renderPipeline = nullptr;
    bool isCompleted = false;
    std::string message;
};

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

            pass.DispatchWorkgroups(1);
            pass.End();

            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        constexpr uint32_t kExpected = 1u;
        EXPECT_BUFFER_U32_EQ(kExpected, ssbo, 0);
    }

    void ValidateCreateComputePipelineAsync() { ValidateCreateComputePipelineAsync(&task); }

    void ValidateCreateRenderPipelineAsync(CreatePipelineAsyncTask* currentTask) {
        constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.format = kRenderAttachmentFormat;
        textureDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture outputTexture = device.CreateTexture(&textureDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder =
                encoder.BeginRenderPass(&renderPassDescriptor);

            while (!currentTask->isCompleted) {
                WaitABit();
            }
            ASSERT_TRUE(currentTask->message.empty());
            ASSERT_NE(nullptr, currentTask->renderPipeline.Get());

            renderPassEncoder.SetPipeline(currentTask->renderPipeline);
            renderPassEncoder.Draw(1);
            renderPassEncoder.End();
            commands = encoder.Finish();
        }

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), outputTexture, 0, 0);
    }

    void ValidateCreateRenderPipelineAsync() { ValidateCreateRenderPipelineAsync(&task); }

    void DoCreateRenderPipelineAsync(
        const utils::ComboRenderPipelineDescriptor& renderPipelineDescriptor) {
        device.CreateRenderPipelineAsync(
            &renderPipelineDescriptor,
            [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
               const char* message, void* userdata) {
                EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success,
                          status);

                CreatePipelineAsyncTask* currentTask =
                    static_cast<CreatePipelineAsyncTask*>(userdata);
                currentTask->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
                currentTask->isCompleted = true;
                currentTask->message = message;
            },
            &task);
    }

    CreatePipelineAsyncTask task;
};

// Verify the basic use of CreateComputePipelineAsync works on all backends.
TEST_P(CreatePipelineAsyncTest, BasicUseOfCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value = 1u;
        })");
    csDesc.compute.entryPoint = "main";

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

// This is a regression test for a bug on the member "entryPoint" of FlatComputePipelineDescriptor.
TEST_P(CreatePipelineAsyncTest, ReleaseEntryPointAfterCreatComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value = 1u;
        })");

    std::string entryPoint = "main";

    csDesc.compute.entryPoint = entryPoint.c_str();

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

    entryPoint = "";
    ValidateCreateComputePipelineAsync();
}

// Verify CreateComputePipelineAsync() works as expected when there is any error that happens during
// the creation of the compute pipeline. The SPEC requires that during the call of
// CreateComputePipelineAsync() any error won't be forwarded to the error scope / unhandled error
// callback.
TEST_P(CreatePipelineAsyncTest, CreateComputePipelineFailed) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value = 1u;
        })");
    csDesc.compute.entryPoint = "main0";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_ValidationError,
                      status);

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

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    DoCreateRenderPipelineAsync(renderPipelineDescriptor);

    ValidateCreateRenderPipelineAsync();
}

// Verify the render pipeline created with CreateRenderPipelineAsync() still works when the entry
// points are released after the creation of the render pipeline.
TEST_P(CreatePipelineAsyncTest, ReleaseEntryPointsAfterCreateRenderPipelineAsync) {
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    std::string vertexEntryPoint = "main";
    std::string fragmentEntryPoint = "main";
    renderPipelineDescriptor.vertex.entryPoint = vertexEntryPoint.c_str();
    renderPipelineDescriptor.cFragment.entryPoint = fragmentEntryPoint.c_str();

    DoCreateRenderPipelineAsync(renderPipelineDescriptor);

    vertexEntryPoint = "";
    fragmentEntryPoint = "";

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = kRenderAttachmentFormat;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture outputTexture = device.CreateTexture(&textureDescriptor);

    utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};

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
        renderPassEncoder.End();
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), outputTexture, 0, 0);
}

// Verify CreateRenderPipelineAsync() works as expected when there is any error that happens during
// the creation of the render pipeline. The SPEC requires that during the call of
// CreateRenderPipelineAsync() any error won't be forwarded to the error scope / unhandled error
// callback.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineFailed) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::Depth32Float;

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_ValidationError,
                      status);

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
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");
    csDesc.compute.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(returnPipeline, nullptr);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    while (!task.isCompleted) {
        WaitABit();
    }
}

// Verify there is no error when the device is released before the callback of
// CreateRenderPipelineAsync() is called.
TEST_P(CreatePipelineAsyncTest, ReleaseDeviceBeforeCallbackOfCreateRenderPipelineAsync) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(returnPipeline, nullptr);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    while (!task.isCompleted) {
        WaitABit();
    }
}

// Verify there is no error when the device is destroyed before the callback of
// CreateComputePipelineAsync() is called.
TEST_P(CreatePipelineAsyncTest, DestroyDeviceBeforeCallbackOfCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");
    csDesc.compute.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &csDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(returnPipeline, nullptr);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);
    DestroyDevice();
    while (!task.isCompleted) {
        WaitABit();
    }
}

// Verify there is no error when the device is destroyed before the callback of
// CreateRenderPipelineAsync() is called.
TEST_P(CreatePipelineAsyncTest, DestroyDeviceBeforeCallbackOfCreateRenderPipelineAsync) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    device.CreateRenderPipelineAsync(
        &renderPipelineDescriptor,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(returnPipeline, nullptr);

            CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
            task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);
    DestroyDevice();
    while (!task.isCompleted) {
        WaitABit();
    }
}

// Verify the code path of CreateComputePipelineAsync() to directly return the compute pipeline
// object from cache works correctly.
TEST_P(CreatePipelineAsyncTest, CreateSameComputePipelineTwice) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value = 1u;
        })");
    csDesc.compute.entryPoint = "main";

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
TEST_P(CreatePipelineAsyncTest, CreateSameComputePipelineTwiceAtSameTime) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Storage;
    binding.visibility = wgpu::ShaderStage::Compute;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&desc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct SSBO {
            value : u32
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

        @compute @workgroup_size(1) fn main() {
            ssbo.value = 1u;
        })");
    csDesc.compute.entryPoint = "main";

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

// Verify the basic use of CreateRenderPipelineAsync() works on all backends.
TEST_P(CreatePipelineAsyncTest, CreateSameRenderPipelineTwiceAtSameTime) {
    constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    auto callback = [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
                       const char* message, void* userdata) {
        EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success, status);

        CreatePipelineAsyncTask* task = static_cast<CreatePipelineAsyncTask*>(userdata);
        task->renderPipeline = wgpu::RenderPipeline::Acquire(returnPipeline);
        task->isCompleted = true;
        task->message = message;
    };

    // Create two render pipelines with same descriptor.
    CreatePipelineAsyncTask anotherTask;
    device.CreateRenderPipelineAsync(&renderPipelineDescriptor, callback, &task);
    device.CreateRenderPipelineAsync(&renderPipelineDescriptor, callback, &anotherTask);

    // Verify task.renderPipeline and anotherTask.renderPipeline are both created correctly.
    ValidateCreateRenderPipelineAsync(&task);
    ValidateCreateRenderPipelineAsync(&anotherTask);

    // Verify task.renderPipeline and anotherTask.renderPipeline are pointing to the same Dawn
    // object.
    if (!UsesWire()) {
        EXPECT_EQ(task.renderPipeline.Get(), anotherTask.renderPipeline.Get());
    }
}

// Verify calling CreateRenderPipelineAsync() with valid VertexBufferLayouts works on all backends.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineAsyncWithVertexBufferLayouts) {
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture renderTarget = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderTargetView});
    {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        struct VertexInput {
            @location(0) input0: u32,
            @location(1) input1: u32,
        }

        struct VertexOutput {
            @location(0) vertexColorOut: vec4f,
            @builtin(position) position: vec4f,
        }

        @vertex
        fn main(vertexInput : VertexInput) -> VertexOutput {
            var vertexOutput : VertexOutput;
            vertexOutput.position = vec4f(0.0, 0.0, 0.0, 1.0);
            if (vertexInput.input0 == 1u && vertexInput.input1 == 2u) {
                vertexOutput.vertexColorOut = vec4f(0.0, 1.0, 0.0, 1.0);
            } else {
                vertexOutput.vertexColorOut = vec4f(1.0, 0.0, 0.0, 1.0);
            }
            return vertexOutput;
        })");
        renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment
        fn main(@location(0) fragColorIn : vec4f) -> @location(0) vec4f {
            return fragColorIn;
        })");

        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        renderPipelineDescriptor.cFragment.targetCount = 1;
        renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        // Create a render pipeline with two VertexBufferLayouts
        renderPipelineDescriptor.vertex.buffers = renderPipelineDescriptor.cBuffers.data();
        renderPipelineDescriptor.vertex.bufferCount = 2;
        renderPipelineDescriptor.cBuffers[0].attributeCount = 1;
        renderPipelineDescriptor.cBuffers[0].attributes = &renderPipelineDescriptor.cAttributes[0];
        renderPipelineDescriptor.cAttributes[0].format = wgpu::VertexFormat::Uint32;
        renderPipelineDescriptor.cAttributes[0].shaderLocation = 0;
        renderPipelineDescriptor.cBuffers[1].attributeCount = 1;
        renderPipelineDescriptor.cBuffers[1].attributes = &renderPipelineDescriptor.cAttributes[1];
        renderPipelineDescriptor.cAttributes[1].format = wgpu::VertexFormat::Uint32;
        renderPipelineDescriptor.cAttributes[1].shaderLocation = 1;

        DoCreateRenderPipelineAsync(renderPipelineDescriptor);
    }

    wgpu::Buffer vertexBuffer1 = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex, {1u});
    wgpu::Buffer vertexBuffer2 = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex, {2u});

    // Do the draw call with the render pipeline
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.renderPipeline.Get());
        pass.SetPipeline(task.renderPipeline);

        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.SetVertexBuffer(1, vertexBuffer2);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The color attachment will have the expected color when the vertex attribute values are
    // fetched correctly.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), renderTarget, 0, 0);
}

// Verify calling CreateRenderPipelineAsync() with valid depthStencilState works on all backends.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineAsyncWithDepthStencilState) {
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture renderTarget = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    textureDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    wgpu::Texture depthStencilTarget = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView depthStencilView = depthStencilTarget.CreateView();

    // Clear the color attachment to green and the stencil aspect of the depth stencil attachment
    // to 0.
    utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthStencilView);
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].clearValue = {0.0, 1.0, 0.0, 1.0};
    renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
    renderPass.cDepthStencilAttachmentInfo.stencilClearValue = 0u;

    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment
        fn main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        })");

        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        renderPipelineDescriptor.cFragment.targetCount = 1;
        renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        // Create a render pipeline with stencil compare function "Equal".
        renderPipelineDescriptor.depthStencil = &renderPipelineDescriptor.cDepthStencil;
        renderPipelineDescriptor.cDepthStencil.stencilFront.compare = wgpu::CompareFunction::Equal;

        DoCreateRenderPipelineAsync(renderPipelineDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.renderPipeline.Get());
        pass.SetPipeline(task.renderPipeline);

        // The stencil reference is set to 1, so there should be no pixel that can pass the stencil
        // test.
        pass.SetStencilReference(1);

        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The color in the color attachment should not be changed after the draw call as no pixel can
    // pass the stencil test.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), renderTarget, 0, 0);
}

// Verify calling CreateRenderPipelineAsync() with multisample.Count > 1 works on all backends.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineWithMultisampleState) {
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture resolveTarget = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView();

    textureDescriptor.sampleCount = 4;
    wgpu::Texture renderTarget = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    // Set the multi-sampled render target, its resolve target to render pass and clear color to
    // (1, 0, 0, 1).
    utils::ComboRenderPassDescriptor renderPass({renderTargetView});
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].clearValue = {1.0, 0.0, 0.0, 1.0};
    renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;

    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment
        fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        renderPipelineDescriptor.cFragment.targetCount = 1;
        renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        // Create a render pipeline with multisample.count == 4.
        renderPipelineDescriptor.multisample.count = 4;

        DoCreateRenderPipelineAsync(renderPipelineDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.renderPipeline.Get());
        pass.SetPipeline(task.renderPipeline);

        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The color in resolveTarget should be the expected color (0, 1, 0, 1).
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 255, 0, 255), resolveTarget, 0, 0);
}

// Verify calling CreateRenderPipelineAsync() with valid BlendState works on all backends.
TEST_P(CreatePipelineAsyncTest, CreateRenderPipelineAsyncWithBlendState) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_indexed_draw_buffers"));

    std::array<wgpu::Texture, 2> renderTargets;
    std::array<wgpu::TextureView, 2> renderTargetViews;

    {
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

        for (uint32_t i = 0; i < renderTargets.size(); ++i) {
            renderTargets[i] = device.CreateTexture(&textureDescriptor);
            renderTargetViews[i] = renderTargets[i].CreateView();
        }
    }

    // Prepare two color attachments
    utils::ComboRenderPassDescriptor renderPass({renderTargetViews[0], renderTargetViews[1]});
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].clearValue = {0.2, 0.0, 0.0, 0.2};
    renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[1].clearValue = {0.0, 0.2, 0.0, 0.2};

    {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
        renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
         struct FragmentOut {
            @location(0) fragColor0 : vec4f,
            @location(1) fragColor1 : vec4f,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = vec4f(0.4, 0.0, 0.0, 0.4);
            output.fragColor1 = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");

        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        // Create a render pipeline with blending states
        renderPipelineDescriptor.cFragment.targetCount = renderTargets.size();

        // The blend operation for the first render target is "add".
        wgpu::BlendComponent blendComponent0;
        blendComponent0.operation = wgpu::BlendOperation::Add;
        blendComponent0.srcFactor = wgpu::BlendFactor::One;
        blendComponent0.dstFactor = wgpu::BlendFactor::One;

        wgpu::BlendState blend0;
        blend0.color = blendComponent0;
        blend0.alpha = blendComponent0;

        // The blend operation for the first render target is "subtract".
        wgpu::BlendComponent blendComponent1;
        blendComponent1.operation = wgpu::BlendOperation::Subtract;
        blendComponent1.srcFactor = wgpu::BlendFactor::One;
        blendComponent1.dstFactor = wgpu::BlendFactor::One;

        wgpu::BlendState blend1;
        blend1.color = blendComponent1;
        blend1.alpha = blendComponent1;

        renderPipelineDescriptor.cTargets[0].blend = &blend0;
        renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        renderPipelineDescriptor.cTargets[1].blend = &blend1;
        renderPipelineDescriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

        DoCreateRenderPipelineAsync(renderPipelineDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.renderPipeline.Get());
        pass.SetPipeline(task.renderPipeline);

        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // When the blend states are all set correctly, the color of renderTargets[0] should be
    // (0.6, 0, 0, 0.6) = colorAttachment0.clearValue + (0.4, 0.0, 0.0, 0.4), and the color of
    // renderTargets[1] should be (0.8, 0, 0, 0.8) = (1, 0, 0, 1) - colorAttachment1.clearValue.
    utils::RGBA8 expected0 = {153, 0, 0, 153};
    utils::RGBA8 expected1 = {0, 204, 0, 204};
    EXPECT_PIXEL_RGBA8_EQ(expected0, renderTargets[0], 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(expected1, renderTargets[1], 0, 0);
}

DAWN_INSTANTIATE_TEST(CreatePipelineAsyncTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
