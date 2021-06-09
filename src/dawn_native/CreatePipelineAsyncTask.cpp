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
            mCreateComputePipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                reinterpret_cast<WGPUComputePipeline>(mPipeline.Detach()), "", mUserData);
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
            mCreateRenderPipelineAsyncCallback(
                WGPUCreatePipelineAsyncStatus_Success,
                reinterpret_cast<WGPURenderPipeline>(mPipeline.Detach()), "", mUserData);
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
        const ComputePipelineDescriptor* descriptor,
        size_t blueprintHash,
        WGPUCreateComputePipelineAsyncCallback callback,
        void* userdata)
        : mComputePipeline(nonInitializedComputePipeline),
          mBlueprintHash(blueprintHash),
          mCallback(callback),
          mUserdata(userdata),
          mLabel(descriptor->label != nullptr ? descriptor->label : ""),
          mLayout(descriptor->layout),
          mEntryPoint(descriptor->compute.entryPoint),
          mComputeShaderModule(descriptor->compute.module) {
        ASSERT(mComputePipeline != nullptr);

        // TODO(jiawei.shao@intel.com): save nextInChain when it is supported in Dawn.
        ASSERT(descriptor->nextInChain == nullptr);
    }

    void CreateComputePipelineAsyncTask::Run() {
        ComputePipelineDescriptor descriptor;
        if (!mLabel.empty()) {
            descriptor.label = mLabel.c_str();
        }
        descriptor.compute.entryPoint = mEntryPoint.c_str();
        descriptor.layout = mLayout.Get();
        descriptor.compute.module = mComputeShaderModule.Get();
        MaybeError maybeError = mComputePipeline->Initialize(&descriptor);
        std::string errorMessage;
        if (maybeError.IsError()) {
            mComputePipeline = nullptr;
            errorMessage = maybeError.AcquireError()->GetMessage();
        }

        mComputeShaderModule = nullptr;
        mComputePipeline->GetDevice()->AddComputePipelineAsyncCallbackTask(
            mComputePipeline, errorMessage, mCallback, mUserdata, mBlueprintHash);
    }

    void CreateComputePipelineAsyncTask::RunAsync(
        std::unique_ptr<CreateComputePipelineAsyncTask> task) {
        DeviceBase* device = task->mComputePipeline->GetDevice();

        // Using "taskPtr = std::move(task)" causes compilation error while it should be supported
        // since C++14:
        // https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=msvc-160
        auto asyncTask = [taskPtr = task.release()] {
            std::unique_ptr<CreateComputePipelineAsyncTask> innnerTaskPtr(taskPtr);
            innnerTaskPtr->Run();
        };
        device->GetAsyncTaskManager()->PostTask(std::move(asyncTask));
    }

}  // namespace dawn_native
