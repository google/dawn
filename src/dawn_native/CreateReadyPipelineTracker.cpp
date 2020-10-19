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

#include "dawn_native/Device.h"

namespace dawn_native {

    CreateReadyComputePipelineTask::CreateReadyComputePipelineTask(
        ComputePipelineBase* pipeline,
        WGPUCreateReadyComputePipelineCallback callback,
        void* userdata)
        : mPipeline(pipeline), mCallback(callback), mUserData(userdata) {
    }

    CreateReadyComputePipelineTask::~CreateReadyComputePipelineTask() {
    }

    void CreateReadyComputePipelineTask::Finish() {
        mCallback(WGPUCreateReadyPipelineStatus_Success,
                  reinterpret_cast<WGPUComputePipeline>(mPipeline), "", mUserData);
    }

    CreateReadyPipelineTracker::CreateReadyPipelineTracker(DeviceBase* device) : mDevice(device) {
    }

    CreateReadyPipelineTracker::~CreateReadyPipelineTracker() {
        ASSERT(mCreateReadyComputePipelineTasksInFlight.Empty());
    }

    void CreateReadyPipelineTracker::TrackTask(std::unique_ptr<CreateReadyComputePipelineTask> task,
                                               ExecutionSerial serial) {
        mCreateReadyComputePipelineTasksInFlight.Enqueue(std::move(task), serial);
        mDevice->AddFutureSerial(serial);
    }

    void CreateReadyPipelineTracker::Tick(ExecutionSerial finishedSerial) {
        for (auto& task : mCreateReadyComputePipelineTasksInFlight.IterateUpTo(finishedSerial)) {
            task->Finish();
        }
        mCreateReadyComputePipelineTasksInFlight.ClearUpTo(finishedSerial);
    }

}  // namespace dawn_native
