// Copyright 2022 The Dawn Authors
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

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>
#include <utility>

#include "dawn/native/CreatePipelineAsyncTask.h"
#include "dawn/utils/WGPUHelpers.h"
#include "mocks/ComputePipelineMock.h"
#include "mocks/DawnMockTest.h"
#include "mocks/RenderPipelineMock.h"

namespace dawn::native {
namespace {

using ::testing::Test;

static constexpr std::string_view kComputeShader = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kVertexShader = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )";

class CreatePipelineAsyncTaskTests : public DawnMockTest {};

// A regression test for a null pointer issue in CreateRenderPipelineAsyncTask::Run().
// See crbug.com/dawn/1310 for more details.
TEST_F(CreatePipelineAsyncTaskTests, InitializationValidationErrorInCreateRenderPipelineAsync) {
    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.vertex.entryPoint = "main";
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*renderPipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(InternalErrorType::Validation, "Initialization Error"))));

    CreateRenderPipelineAsyncTask asyncTask(
        renderPipelineMock,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_ValidationError,
                      status);
        },
        nullptr);

    asyncTask.Run();
    device.Tick();

    EXPECT_CALL(*renderPipelineMock.Get(), DestroyImpl).Times(1);
}

// Test that Internal error are converted to the InternalError status in async pipeline creation
// callbacks.
TEST_F(CreatePipelineAsyncTaskTests, InitializationInternalErrorInCreateRenderPipelineAsync) {
    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.vertex.entryPoint = "main";
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*renderPipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(dawn::native::InternalErrorType::Internal, "Initialization Error"))));

    dawn::native::CreateRenderPipelineAsyncTask asyncTask(
        renderPipelineMock,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_InternalError,
                      status);
        },
        nullptr);

    asyncTask.Run();
    device.Tick();

    EXPECT_CALL(*renderPipelineMock.Get(), DestroyImpl).Times(1);
}

// A regression test for a null pointer issue in CreateComputePipelineAsyncTask::Run().
// See crbug.com/dawn/1310 for more details.
TEST_F(CreatePipelineAsyncTaskTests, InitializationValidationErrorInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    desc.compute.entryPoint = "main";
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*computePipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(InternalErrorType::Validation, "Initialization Error"))));

    CreateComputePipelineAsyncTask asyncTask(
        computePipelineMock,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_ValidationError,
                      status);
        },
        nullptr);

    asyncTask.Run();
    device.Tick();

    EXPECT_CALL(*computePipelineMock.Get(), DestroyImpl).Times(1);
}

// Test that Internal error are converted to the InternalError status in async pipeline creation
// callbacks.
TEST_F(CreatePipelineAsyncTaskTests, InitializationInternalErrorInCreateComputePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc = {};
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    desc.compute.entryPoint = "main";
    Ref<ComputePipelineMock> computePipelineMock =
        ComputePipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    ON_CALL(*computePipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(dawn::native::InternalErrorType::Internal, "Initialization Error"))));

    dawn::native::CreateComputePipelineAsyncTask asyncTask(
        computePipelineMock,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline returnPipeline,
           const char* message, void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_InternalError,
                      status);
        },
        nullptr);

    asyncTask.Run();
    device.Tick();

    EXPECT_CALL(*computePipelineMock.Get(), DestroyImpl).Times(1);
}

// Test that a long async task's execution won't extend to after the device is dropped.
// Device dropping should wait for that task to finish.
TEST_F(CreatePipelineAsyncTaskTests, LongAsyncTaskFinishesBeforeDeviceIsDropped) {
    wgpu::RenderPipelineDescriptor desc = {};
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.vertex.entryPoint = "main";
    Ref<RenderPipelineMock> renderPipelineMock =
        RenderPipelineMock::Create(mDeviceMock, FromCppAPI(&desc));

    // Simulate that Initialize() would take a long time to finish.
    ON_CALL(*renderPipelineMock.Get(), Initialize).WillByDefault([]() -> MaybeError {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return {};
    });

    bool done = false;
    auto asyncTask = std::make_unique<CreateRenderPipelineAsyncTask>(
        renderPipelineMock,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline returnPipeline,
           const char* message, void* userdata) {
            wgpu::RenderPipeline::Acquire(returnPipeline);

            *static_cast<bool*>(userdata) = true;
        },
        &done);

    CreateRenderPipelineAsyncTask::RunAsync(std::move(asyncTask));

    device = nullptr;
    // Dropping the device should force the async task to finish.
    EXPECT_TRUE(done);
}

}  // anonymous namespace
}  // namespace dawn::native
