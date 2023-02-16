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

#include "dawn/tests/DawnNativeTest.h"

#include "dawn/native/CreatePipelineAsyncTask.h"
#include "mocks/ComputePipelineMock.h"
#include "mocks/RenderPipelineMock.h"

class CreatePipelineAsyncTaskTests : public DawnNativeTest {};

// A regression test for a null pointer issue in CreateRenderPipelineAsyncTask::Run().
// See crbug.com/dawn/1310 for more details.
TEST_F(CreatePipelineAsyncTaskTests, InitializationValidationErrorInCreateRenderPipelineAsync) {
    dawn::native::DeviceBase* deviceBase =
        reinterpret_cast<dawn::native::DeviceBase*>(device.Get());
    Ref<dawn::native::RenderPipelineMock> renderPipelineMock =
        AcquireRef(new dawn::native::RenderPipelineMock(deviceBase));

    ON_CALL(*renderPipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(dawn::native::InternalErrorType::Validation, "Initialization Error"))));

    dawn::native::CreateRenderPipelineAsyncTask asyncTask(
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
    dawn::native::DeviceBase* deviceBase =
        reinterpret_cast<dawn::native::DeviceBase*>(device.Get());
    Ref<dawn::native::RenderPipelineMock> renderPipelineMock =
        AcquireRef(new dawn::native::RenderPipelineMock(deviceBase));

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
    dawn::native::DeviceBase* deviceBase =
        reinterpret_cast<dawn::native::DeviceBase*>(device.Get());
    Ref<dawn::native::ComputePipelineMock> computePipelineMock =
        AcquireRef(new dawn::native::ComputePipelineMock(deviceBase));

    ON_CALL(*computePipelineMock.Get(), Initialize)
        .WillByDefault(testing::Return(testing::ByMove(
            DAWN_MAKE_ERROR(dawn::native::InternalErrorType::Validation, "Initialization Error"))));

    dawn::native::CreateComputePipelineAsyncTask asyncTask(
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
    dawn::native::DeviceBase* deviceBase =
        reinterpret_cast<dawn::native::DeviceBase*>(device.Get());
    Ref<dawn::native::ComputePipelineMock> computePipelineMock =
        AcquireRef(new dawn::native::ComputePipelineMock(deviceBase));

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
