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

#include "dawn_native/CreatePipelineAsyncTracker.h"

#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/RenderPipeline.h"

namespace dawn_native {

    CreatePipelineAsyncTaskBase::CreatePipelineAsyncTaskBase(std::string errorMessage,
                                                             void* userdata)
        : mErrorMessage(errorMessage), mUserData(userdata) {
    }

    CreatePipelineAsyncTaskBase::~CreatePipelineAsyncTaskBase() {
    }

    CreateComputePipelineAsyncTask::CreateComputePipelineAsyncTask(
        Ref<ComputePipelineBase> pipeline,
        std::string errorMessage,
        WGPUCreateComputePipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncTaskBase(errorMessage, userdata),
          mPipeline(std::move(pipeline)),
          mCreateComputePipelineAsyncCallback(callback) {
    }

    void CreateComputePipelineAsyncTask::Finish() {
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

    void CreateComputePipelineAsyncTask::HandleShutDown() {
        ASSERT(mCreateComputePipelineAsyncCallback != nullptr);

        mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                                            "Device destroyed before callback", mUserData);
    }

    void CreateComputePipelineAsyncTask::HandleDeviceLoss() {
        ASSERT(mCreateComputePipelineAsyncCallback != nullptr);

        mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                                            "Device lost before callback", mUserData);
    }

    CreateRenderPipelineAsyncTask::CreateRenderPipelineAsyncTask(
        Ref<RenderPipelineBase> pipeline,
        std::string errorMessage,
        WGPUCreateRenderPipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncTaskBase(errorMessage, userdata),
          mPipeline(std::move(pipeline)),
          mCreateRenderPipelineAsyncCallback(callback) {
    }

    void CreateRenderPipelineAsyncTask::Finish() {
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

    void CreateRenderPipelineAsyncTask::HandleShutDown() {
        ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);

        mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed, nullptr,
                                           "Device destroyed before callback", mUserData);
    }

    void CreateRenderPipelineAsyncTask::HandleDeviceLoss() {
        ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);

        mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceLost, nullptr,
                                           "Device lost before callback", mUserData);
    }

    CreatePipelineAsyncTracker::CreatePipelineAsyncTracker(DeviceBase* device) : mDevice(device) {
    }

    CreatePipelineAsyncTracker::~CreatePipelineAsyncTracker() {
        ASSERT(mCreatePipelineAsyncTasksInFlight.Empty());
    }

    void CreatePipelineAsyncTracker::TrackTask(std::unique_ptr<CreatePipelineAsyncTaskBase> task,
                                               ExecutionSerial serial) {
        mCreatePipelineAsyncTasksInFlight.Enqueue(std::move(task), serial);
        mDevice->AddFutureSerial(serial);
    }

    void CreatePipelineAsyncTracker::Tick(ExecutionSerial finishedSerial) {
        // If a user calls Queue::Submit inside Create*PipelineAsync, then the device will be
        // ticked, which in turns ticks the tracker, causing reentrance here. To prevent the
        // reentrant call from invalidating mCreatePipelineAsyncTasksInFlight while in use by the
        // first call, we remove the tasks to finish from the queue, update
        // mCreatePipelineAsyncTasksInFlight, then run the callbacks.
        std::vector<std::unique_ptr<CreatePipelineAsyncTaskBase>> tasks;
        for (auto& task : mCreatePipelineAsyncTasksInFlight.IterateUpTo(finishedSerial)) {
            tasks.push_back(std::move(task));
        }
        mCreatePipelineAsyncTasksInFlight.ClearUpTo(finishedSerial);

        for (auto& task : tasks) {
            if (mDevice->IsLost()) {
                task->HandleDeviceLoss();
            } else {
                task->Finish();
            }
        }
    }

    void CreatePipelineAsyncTracker::ClearForShutDown() {
        for (auto& task : mCreatePipelineAsyncTasksInFlight.IterateAll()) {
            task->HandleShutDown();
        }
        mCreatePipelineAsyncTasksInFlight.Clear();
    }

}  // namespace dawn_native
