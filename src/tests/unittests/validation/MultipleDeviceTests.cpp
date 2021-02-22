// Copyright 2021 The Dawn Authors
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

#include "tests/MockCallback.h"

using namespace testing;

class MultipleDeviceTest : public ValidationTest {};

// Test that it is invalid to submit a command buffer created on a different device.
TEST_F(MultipleDeviceTest, ValidatesSameDevice) {
    wgpu::Device device2 = RegisterDevice(CreateTestDevice());
    wgpu::CommandBuffer commandBuffer = device2.CreateCommandEncoder().Finish();

    ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commandBuffer));
}

// Test that CreatePipelineAsync fails creation with an Error status if it uses
// objects from a different device.
TEST_F(MultipleDeviceTest, ValidatesSameDeviceCreatePipelineAsync) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.source = R"(
        [[stage(compute)]] fn main() -> void {
        }
    )";

    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    // Base case: CreateComputePipelineAsync succeeds.
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.computeStage.module = shaderModule;
        pipelineDesc.computeStage.entryPoint = "main";

        StrictMock<MockCallback<WGPUCreateComputePipelineAsyncCallback>> creationCallback;
        EXPECT_CALL(creationCallback,
                    Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), _, this))
            .WillOnce(WithArg<1>(Invoke(
                [](WGPUComputePipeline pipeline) { wgpu::ComputePipeline::Acquire(pipeline); })));
        device.CreateComputePipelineAsync(&pipelineDesc, creationCallback.Callback(),
                                          creationCallback.MakeUserdata(this));

        WaitForAllOperations(device);
    }

    // CreateComputePipelineAsync errors if the shader module is created on a different device.
    {
        wgpu::Device device2 = RegisterDevice(CreateTestDevice());
        wgpu::ShaderModule shaderModule = device2.CreateShaderModule(&shaderModuleDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.computeStage.module = shaderModule;
        pipelineDesc.computeStage.entryPoint = "main";

        StrictMock<MockCallback<WGPUCreateComputePipelineAsyncCallback>> creationCallback;
        EXPECT_CALL(creationCallback,
                    Call(WGPUCreatePipelineAsyncStatus_Error, nullptr, _, this + 1))
            .Times(1);
        device.CreateComputePipelineAsync(&pipelineDesc, creationCallback.Callback(),
                                          creationCallback.MakeUserdata(this + 1));

        WaitForAllOperations(device);
    }
}
