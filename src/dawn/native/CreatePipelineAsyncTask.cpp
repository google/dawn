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

#include "dawn/native/CreatePipelineAsyncTask.h"

#include <utility>

#include "dawn/native/AsyncTask.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

CreatePipelineAsyncCallbackTaskBase::CreatePipelineAsyncCallbackTaskBase(void* userdata)
    : mStatus(WGPUCreatePipelineAsyncStatus_Success), mUserData(userdata) {}

CreatePipelineAsyncCallbackTaskBase::CreatePipelineAsyncCallbackTaskBase(
    WGPUCreatePipelineAsyncStatus status,
    std::string errorMessage,
    void* userdata)
    : mErrorMessage(errorMessage), mStatus(status), mUserData(userdata) {}

CreatePipelineAsyncCallbackTaskBase::~CreatePipelineAsyncCallbackTaskBase() = default;

CreateComputePipelineAsyncCallbackTask::CreateComputePipelineAsyncCallbackTask(
    Ref<ComputePipelineBase> pipeline,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata)
    : CreatePipelineAsyncCallbackTaskBase(userdata),
      mPipeline(std::move(pipeline)),
      mCreateComputePipelineAsyncCallback(callback) {}

CreateComputePipelineAsyncCallbackTask::CreateComputePipelineAsyncCallbackTask(
    WGPUCreatePipelineAsyncStatus status,
    std::string errorMessage,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata)
    : CreatePipelineAsyncCallbackTaskBase(status, errorMessage, userdata),
      mCreateComputePipelineAsyncCallback(callback) {}

CreateComputePipelineAsyncCallbackTask::~CreateComputePipelineAsyncCallbackTask() = default;

void CreateComputePipelineAsyncCallbackTask::Finish() {
    ASSERT(mCreateComputePipelineAsyncCallback != nullptr);
    mCreateComputePipelineAsyncCallback(mStatus, ToAPI(mPipeline.Detach()), mErrorMessage.c_str(),
                                        mUserData);
}

void CreateComputePipelineAsyncCallbackTask::HandleShutDown() {
    ASSERT(mCreateComputePipelineAsyncCallback != nullptr);
    mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                                        "Device destroyed before callback", mUserData);
}

void CreateComputePipelineAsyncCallbackTask::HandleDeviceLoss() {
    ASSERT(mCreateComputePipelineAsyncCallback != nullptr);
    mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                                        "Device lost before callback", mUserData);
}

CreateRenderPipelineAsyncCallbackTask::CreateRenderPipelineAsyncCallbackTask(
    Ref<RenderPipelineBase> pipeline,
    WGPUCreateRenderPipelineAsyncCallback callback,
    void* userdata)
    : CreatePipelineAsyncCallbackTaskBase(userdata),
      mPipeline(std::move(pipeline)),
      mCreateRenderPipelineAsyncCallback(callback) {}

CreateRenderPipelineAsyncCallbackTask::CreateRenderPipelineAsyncCallbackTask(
    WGPUCreatePipelineAsyncStatus status,
    std::string errorMessage,
    WGPUCreateRenderPipelineAsyncCallback callback,
    void* userdata)
    : CreatePipelineAsyncCallbackTaskBase(status, errorMessage, userdata),
      mCreateRenderPipelineAsyncCallback(callback) {}

CreateRenderPipelineAsyncCallbackTask::~CreateRenderPipelineAsyncCallbackTask() = default;

void CreateRenderPipelineAsyncCallbackTask::Finish() {
    ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);
    mCreateRenderPipelineAsyncCallback(mStatus, ToAPI(mPipeline.Detach()), mErrorMessage.c_str(),
                                       mUserData);
}

void CreateRenderPipelineAsyncCallbackTask::HandleShutDown() {
    ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);
    mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                                       "Device destroyed before callback", mUserData);
}

void CreateRenderPipelineAsyncCallbackTask::HandleDeviceLoss() {
    ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);
    mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                                       "Device lost before callback", mUserData);
}

CreateComputePipelineAsyncTask::CreateComputePipelineAsyncTask(
    Ref<ComputePipelineBase> nonInitializedComputePipeline,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata)
    : mComputePipeline(std::move(nonInitializedComputePipeline)),
      mCallback(callback),
      mUserdata(userdata) {
    ASSERT(mComputePipeline != nullptr);
}

CreateComputePipelineAsyncTask::~CreateComputePipelineAsyncTask() = default;

void CreateComputePipelineAsyncTask::Run() {
    const char* eventLabel = utils::GetLabelForTrace(mComputePipeline->GetLabel().c_str());

    DeviceBase* device = mComputePipeline->GetDevice();
    TRACE_EVENT_FLOW_END1(device->GetPlatform(), General,
                          "CreateComputePipelineAsyncTask::RunAsync", this, "label", eventLabel);
    TRACE_EVENT1(device->GetPlatform(), General, "CreateComputePipelineAsyncTask::Run", "label",
                 eventLabel);

    MaybeError maybeError = mComputePipeline->Initialize();
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> error = maybeError.AcquireError();
        WGPUCreatePipelineAsyncStatus status =
            CreatePipelineAsyncStatusFromErrorType(error->GetType());
        device->GetCallbackTaskManager()->AddCallbackTask(
            std::make_unique<CreateComputePipelineAsyncCallbackTask>(status, error->GetMessage(),
                                                                     mCallback, mUserdata));
    } else {
        device->AddComputePipelineAsyncCallbackTask(mComputePipeline, mCallback, mUserdata);
    }
}

void CreateComputePipelineAsyncTask::RunAsync(
    std::unique_ptr<CreateComputePipelineAsyncTask> task) {
    DeviceBase* device = task->mComputePipeline->GetDevice();

    const char* eventLabel = utils::GetLabelForTrace(task->mComputePipeline->GetLabel().c_str());

    // Using "taskPtr = std::move(task)" causes compilation error while it should be supported
    // since C++14:
    // https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=msvc-160
    auto asyncTask = [taskPtr = task.release()] {
        std::unique_ptr<CreateComputePipelineAsyncTask> innnerTaskPtr(taskPtr);
        innnerTaskPtr->Run();
    };

    TRACE_EVENT_FLOW_BEGIN1(device->GetPlatform(), General,
                            "CreateComputePipelineAsyncTask::RunAsync", task.get(), "label",
                            eventLabel);
    device->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
}

CreateRenderPipelineAsyncTask::CreateRenderPipelineAsyncTask(
    Ref<RenderPipelineBase> nonInitializedRenderPipeline,
    WGPUCreateRenderPipelineAsyncCallback callback,
    void* userdata)
    : mRenderPipeline(std::move(nonInitializedRenderPipeline)),
      mCallback(callback),
      mUserdata(userdata) {
    ASSERT(mRenderPipeline != nullptr);
}

CreateRenderPipelineAsyncTask::~CreateRenderPipelineAsyncTask() = default;

void CreateRenderPipelineAsyncTask::Run() {
    const char* eventLabel = utils::GetLabelForTrace(mRenderPipeline->GetLabel().c_str());

    DeviceBase* device = mRenderPipeline->GetDevice();
    TRACE_EVENT_FLOW_END1(device->GetPlatform(), General, "CreateRenderPipelineAsyncTask::RunAsync",
                          this, "label", eventLabel);
    TRACE_EVENT1(device->GetPlatform(), General, "CreateRenderPipelineAsyncTask::Run", "label",
                 eventLabel);

    MaybeError maybeError = mRenderPipeline->Initialize();
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> error = maybeError.AcquireError();
        WGPUCreatePipelineAsyncStatus status =
            CreatePipelineAsyncStatusFromErrorType(error->GetType());
        device->GetCallbackTaskManager()->AddCallbackTask(
            std::make_unique<CreateRenderPipelineAsyncCallbackTask>(status, error->GetMessage(),
                                                                    mCallback, mUserdata));
    } else {
        device->AddRenderPipelineAsyncCallbackTask(mRenderPipeline, mCallback, mUserdata);
    }
}

void CreateRenderPipelineAsyncTask::RunAsync(std::unique_ptr<CreateRenderPipelineAsyncTask> task) {
    DeviceBase* device = task->mRenderPipeline->GetDevice();

    const char* eventLabel = utils::GetLabelForTrace(task->mRenderPipeline->GetLabel().c_str());

    // Using "taskPtr = std::move(task)" causes compilation error while it should be supported
    // since C++14:
    // https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=msvc-160
    auto asyncTask = [taskPtr = task.release()] {
        std::unique_ptr<CreateRenderPipelineAsyncTask> innerTaskPtr(taskPtr);
        innerTaskPtr->Run();
    };

    TRACE_EVENT_FLOW_BEGIN1(device->GetPlatform(), General,
                            "CreateRenderPipelineAsyncTask::RunAsync", task.get(), "label",
                            eventLabel);
    device->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
}
}  // namespace dawn::native
