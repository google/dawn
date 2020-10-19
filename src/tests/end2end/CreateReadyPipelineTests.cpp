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

#include "utils/WGPUHelpers.h"

namespace {
    struct CreateReadyPipelineTask {
        wgpu::ComputePipeline computePipeline;
        bool isCompleted = false;
        std::string message;
    };
}  // anonymous namespace

class CreateReadyPipelineTest : public DawnTest {};

// Verify the basic use of CreateReadyComputePipeline works on all backends.
TEST_P(CreateReadyPipelineTest, BasicUseOfCreateReadyComputePipeline) {
    const char* computeShader = R"(
        #version 450
        layout(std140, set = 0, binding = 0) buffer SSBO { uint value; } ssbo;
        void main() {
            ssbo.value = 1u;
        })";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, computeShader);
    csDesc.computeStage.entryPoint = "main";

    CreateReadyPipelineTask task;
    device.CreateReadyComputePipeline(
        &csDesc,
        [](WGPUCreateReadyPipelineStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            ASSERT_EQ(WGPUCreateReadyPipelineStatus::WGPUCreateReadyPipelineStatus_Success, status);

            CreateReadyPipelineTask* task = static_cast<CreateReadyPipelineTask*>(userdata);
            task->computePipeline = wgpu::ComputePipeline::Acquire(returnPipeline);
            task->isCompleted = true;
            task->message = message;
        },
        &task);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer ssbo = device.CreateBuffer(&bufferDesc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        while (!task.isCompleted) {
            WaitABit();
        }
        ASSERT_TRUE(task.message.empty());
        ASSERT_NE(nullptr, task.computePipeline.Get());
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, task.computePipeline.GetBindGroupLayout(0),
                                 {
                                     {0, ssbo, 0, sizeof(uint32_t)},
                                 });
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(task.computePipeline);

        pass.Dispatch(1);
        pass.EndPass();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    constexpr uint32_t kExpected = 1u;
    EXPECT_BUFFER_U32_EQ(kExpected, ssbo, 0);
}

// Verify CreateReadyComputePipeline() works as expected when there is any error that happens during
// the creation of the compute pipeline. The SPEC requires that during the call of
// CreateReadyComputePipeline() any error won't be forwarded to the error scope / unhandled error
// callback.
TEST_P(CreateReadyPipelineTest, CreateComputePipelineFailed) {
    DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());

    const char* computeShader = R"(
        #version 450
        layout(std140, set = 0, binding = 0) buffer SSBO { uint value; } ssbo;
        void main() {
            ssbo.value = 1u;
        })";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.computeStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, computeShader);
    csDesc.computeStage.entryPoint = "main0";

    CreateReadyPipelineTask task;
    device.CreateReadyComputePipeline(
        &csDesc,
        [](WGPUCreateReadyPipelineStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            ASSERT_EQ(WGPUCreateReadyPipelineStatus::WGPUCreateReadyPipelineStatus_Error, status);

            CreateReadyPipelineTask* task = static_cast<CreateReadyPipelineTask*>(userdata);
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

DAWN_INSTANTIATE_TEST(CreateReadyPipelineTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
