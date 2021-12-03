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

#include "dawn_native/CreatePipelineAsyncTask.h"

#include "dawn_native/AsyncTask.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"
#include "utils/WGPUHelpers.h"

namespace dawn_native {

    CreatePipelineAsyncCallbackTaskBase::CreatePipelineAsyncCallbackTaskBase(
        std::string errorMessage,
        void* userdata)
        : mErrorMessage(errorMessage), mUserData(userdata) {
    }

    CreateComputePipelineAsyncCallbackTask::CreateComputePipelineAsyncCallbackTask(
        Ref<ComputePipelineBase> pipeline,
        std::string errorMessage,
        WGPUCreateComputePipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncCallbackTaskBase(errorMessage, userdata),
          mPipeline(std::move(pipeline)),
          mCreateComputePipelineAsyncCallback(callback) {
    }

    void CreateComputePipelineAsyncCallbackTask::Finish() {
        ASSERT(mCreateComputePipelineAsyncCallback != nullptr);

        if (mPipeline.Get() != nullptr) {
            mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_Success,
                                                ToAPI(mPipeline.Detach()), "", mUserData);
        } else {
            mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_Error, nullptr,
                                                mErrorMessage.c_str(), mUserData);
        }
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
        std::string errorMessage,
        WGPUCreateRenderPipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncCallbackTaskBase(errorMessage, userdata),
          mPipeline(std::move(pipeline)),
          mCreateRenderPipelineAsyncCallback(callback) {
    }

    void CreateRenderPipelineAsyncCallbackTask::Finish() {
        ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);

        if (mPipeline.Get() != nullptr) {
            mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_Success,
                                               ToAPI(mPipeline.Detach()), "", mUserData);
        } else {
            mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_Error, nullptr,
                                               mErrorMessage.c_str(), mUserData);
        }
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

    void CreateComputePipelineAsyncTask::Run() {
        const char* eventLabel = utils::GetLabelForTrace(mComputePipeline->GetLabel().c_str());
        TRACE_EVENT_FLOW_END1(mComputePipeline->GetDevice()->GetPlatform(), General,
                              "CreateComputePipelineAsyncTask::RunAsync", this, "label",
                              eventLabel);
        TRACE_EVENT1(mComputePipeline->GetDevice()->GetPlatform(), General,
                     "CreateComputePipelineAsyncTask::Run", "label", eventLabel);

        MaybeError maybeError = mComputePipeline->Initialize();
        std::string errorMessage;
        if (maybeError.IsError()) {
            mComputePipeline = nullptr;
            errorMessage = maybeError.AcquireError()->GetMessage();
        }

        mComputePipeline->GetDevice()->AddComputePipelineAsyncCallbackTask(
            mComputePipeline, errorMessage, mCallback, mUserdata);
    }

    void CreateComputePipelineAsyncTask::RunAsync(
        std::unique_ptr<CreateComputePipelineAsyncTask> task) {
        DeviceBase* device = task->mComputePipeline->GetDevice();

        const char* eventLabel =
            utils::GetLabelForTrace(task->mComputePipeline->GetLabel().c_str());

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

    void CreateRenderPipelineAsyncTask::Run() {
        const char* eventLabel = utils::GetLabelForTrace(mRenderPipeline->GetLabel().c_str());
        TRACE_EVENT_FLOW_END1(mRenderPipeline->GetDevice()->GetPlatform(), General,
                              "CreateRenderPipelineAsyncTask::RunAsync", this, "label", eventLabel);
        TRACE_EVENT1(mRenderPipeline->GetDevice()->GetPlatform(), General,
                     "CreateRenderPipelineAsyncTask::Run", "label", eventLabel);

        MaybeError maybeError = mRenderPipeline->Initialize();
        std::string errorMessage;
        if (maybeError.IsError()) {
            mRenderPipeline = nullptr;
            errorMessage = maybeError.AcquireError()->GetMessage();
        }

        mRenderPipeline->GetDevice()->AddRenderPipelineAsyncCallbackTask(
            mRenderPipeline, errorMessage, mCallback, mUserdata);
    }

    void CreateRenderPipelineAsyncTask::RunAsync(
        std::unique_ptr<CreateRenderPipelineAsyncTask> task) {
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
}  // namespace dawn_native
