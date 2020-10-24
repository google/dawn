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

#include "dawn_native/CreateReadyPipelineTracker.h"

#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/RenderPipeline.h"

namespace dawn_native {

    CreateReadyPipelineTaskBase::CreateReadyPipelineTaskBase(void* userdata) : mUserData(userdata) {
    }

    CreateReadyPipelineTaskBase::~CreateReadyPipelineTaskBase() {
    }

    CreateReadyComputePipelineTask::CreateReadyComputePipelineTask(
        ComputePipelineBase* pipeline,
        WGPUCreateReadyComputePipelineCallback callback,
        void* userdata)
        : CreateReadyPipelineTaskBase(userdata),
          mPipeline(pipeline),
          mCreateReadyComputePipelineCallback(callback) {
    }

    void CreateReadyComputePipelineTask::Finish(WGPUCreateReadyPipelineStatus status) {
        ASSERT(mPipeline != nullptr);
        ASSERT(mCreateReadyComputePipelineCallback != nullptr);

        if (status != WGPUCreateReadyPipelineStatus_Success) {
            // TODO(jiawei.shao@intel.com): support handling device lost
            ASSERT(status == WGPUCreateReadyPipelineStatus_DeviceDestroyed);
            mCreateReadyComputePipelineCallback(WGPUCreateReadyPipelineStatus_DeviceDestroyed,
                                                nullptr, "Device destroyed before callback",
                                                mUserData);
            mPipeline->Release();
        } else {
            mCreateReadyComputePipelineCallback(
                status, reinterpret_cast<WGPUComputePipeline>(mPipeline), "", mUserData);
        }

        // Set mCreateReadyComputePipelineCallback to nullptr in case it is called more than once.
        mCreateReadyComputePipelineCallback = nullptr;
    }

    CreateReadyRenderPipelineTask::CreateReadyRenderPipelineTask(
        RenderPipelineBase* pipeline,
        WGPUCreateReadyRenderPipelineCallback callback,
        void* userdata)
        : CreateReadyPipelineTaskBase(userdata),
          mPipeline(pipeline),
          mCreateReadyRenderPipelineCallback(callback) {
    }

    void CreateReadyRenderPipelineTask::Finish(WGPUCreateReadyPipelineStatus status) {
        ASSERT(mPipeline != nullptr);
        ASSERT(mCreateReadyRenderPipelineCallback != nullptr);

        if (status != WGPUCreateReadyPipelineStatus_Success) {
            // TODO(jiawei.shao@intel.com): support handling device lost
            ASSERT(status == WGPUCreateReadyPipelineStatus_DeviceDestroyed);
            mCreateReadyRenderPipelineCallback(WGPUCreateReadyPipelineStatus_DeviceDestroyed,
                                               nullptr, "Device destroyed before callback",
                                               mUserData);
            mPipeline->Release();
        } else {
            mCreateReadyRenderPipelineCallback(
                status, reinterpret_cast<WGPURenderPipeline>(mPipeline), "", mUserData);
        }

        // Set mCreateReadyPipelineCallback to nullptr in case it is called more than once.
        mCreateReadyRenderPipelineCallback = nullptr;
    }

    CreateReadyPipelineTracker::CreateReadyPipelineTracker(DeviceBase* device) : mDevice(device) {
    }

    CreateReadyPipelineTracker::~CreateReadyPipelineTracker() {
        ASSERT(mCreateReadyPipelineTasksInFlight.Empty());
    }

    void CreateReadyPipelineTracker::TrackTask(std::unique_ptr<CreateReadyPipelineTaskBase> task,
                                               ExecutionSerial serial) {
        mCreateReadyPipelineTasksInFlight.Enqueue(std::move(task), serial);
        mDevice->AddFutureSerial(serial);
    }

    void CreateReadyPipelineTracker::Tick(ExecutionSerial finishedSerial) {
        for (auto& task : mCreateReadyPipelineTasksInFlight.IterateUpTo(finishedSerial)) {
            task->Finish(WGPUCreateReadyPipelineStatus_Success);
        }
        mCreateReadyPipelineTasksInFlight.ClearUpTo(finishedSerial);
    }

    void CreateReadyPipelineTracker::ClearForShutDown() {
        for (auto& task : mCreateReadyPipelineTasksInFlight.IterateAll()) {
            task->Finish(WGPUCreateReadyPipelineStatus_DeviceDestroyed);
        }
        mCreateReadyPipelineTasksInFlight.Clear();
    }

}  // namespace dawn_native
