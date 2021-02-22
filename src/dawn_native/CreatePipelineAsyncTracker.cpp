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

    CreatePipelineAsyncTaskBase::CreatePipelineAsyncTaskBase(void* userdata) : mUserData(userdata) {
    }

    CreatePipelineAsyncTaskBase::~CreatePipelineAsyncTaskBase() {
    }

    CreateComputePipelineAsyncTask::CreateComputePipelineAsyncTask(
        ComputePipelineBase* pipeline,
        WGPUCreateComputePipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncTaskBase(userdata),
          mPipeline(pipeline),
          mCreateComputePipelineAsyncCallback(callback) {
    }

    void CreateComputePipelineAsyncTask::Finish(WGPUCreatePipelineAsyncStatus status) {
        ASSERT(mPipeline != nullptr);
        ASSERT(mCreateComputePipelineAsyncCallback != nullptr);

        if (status != WGPUCreatePipelineAsyncStatus_Success) {
            // TODO(jiawei.shao@intel.com): support handling device lost
            ASSERT(status == WGPUCreatePipelineAsyncStatus_DeviceDestroyed);
            mCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed,
                                                nullptr, "Device destroyed before callback",
                                                mUserData);
            mPipeline->Release();
        } else {
            mCreateComputePipelineAsyncCallback(
                status, reinterpret_cast<WGPUComputePipeline>(mPipeline), "", mUserData);
        }

        // Set mCreateComputePipelineAsyncCallback to nullptr in case it is called more than once.
        mCreateComputePipelineAsyncCallback = nullptr;
    }

    CreateRenderPipelineAsyncTask::CreateRenderPipelineAsyncTask(
        RenderPipelineBase* pipeline,
        WGPUCreateRenderPipelineAsyncCallback callback,
        void* userdata)
        : CreatePipelineAsyncTaskBase(userdata),
          mPipeline(pipeline),
          mCreateRenderPipelineAsyncCallback(callback) {
    }

    void CreateRenderPipelineAsyncTask::Finish(WGPUCreatePipelineAsyncStatus status) {
        ASSERT(mPipeline != nullptr);
        ASSERT(mCreateRenderPipelineAsyncCallback != nullptr);

        if (status != WGPUCreatePipelineAsyncStatus_Success) {
            // TODO(jiawei.shao@intel.com): support handling device lost
            ASSERT(status == WGPUCreatePipelineAsyncStatus_DeviceDestroyed);
            mCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus_DeviceDestroyed,
                                               nullptr, "Device destroyed before callback",
                                               mUserData);
            mPipeline->Release();
        } else {
            mCreateRenderPipelineAsyncCallback(
                status, reinterpret_cast<WGPURenderPipeline>(mPipeline), "", mUserData);
        }

        // Set mCreatePipelineAsyncCallback to nullptr in case it is called more than once.
        mCreateRenderPipelineAsyncCallback = nullptr;
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
        for (auto& task : mCreatePipelineAsyncTasksInFlight.IterateUpTo(finishedSerial)) {
            task->Finish(WGPUCreatePipelineAsyncStatus_Success);
        }
        mCreatePipelineAsyncTasksInFlight.ClearUpTo(finishedSerial);
    }

    void CreatePipelineAsyncTracker::ClearForShutDown() {
        for (auto& task : mCreatePipelineAsyncTasksInFlight.IterateAll()) {
            task->Finish(WGPUCreatePipelineAsyncStatus_DeviceDestroyed);
        }
        mCreatePipelineAsyncTasksInFlight.Clear();
    }

}  // namespace dawn_native
