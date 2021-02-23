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

#ifndef DAWNNATIVE_CREATEPIPELINEASYNCTRACKER_H_
#define DAWNNATIVE_CREATEPIPELINEASYNCTRACKER_H_

#include "common/SerialQueue.h"
#include "dawn/webgpu.h"
#include "dawn_native/IntegerTypes.h"

#include <memory>

namespace dawn_native {

    class ComputePipelineBase;
    class DeviceBase;
    class RenderPipelineBase;

    struct CreatePipelineAsyncTaskBase {
        CreatePipelineAsyncTaskBase(void* userData);
        virtual ~CreatePipelineAsyncTaskBase();

        virtual void Finish(WGPUCreatePipelineAsyncStatus status) = 0;

      protected:
        void* mUserData;
    };

    struct CreateComputePipelineAsyncTask final : public CreatePipelineAsyncTaskBase {
        CreateComputePipelineAsyncTask(ComputePipelineBase* pipeline,
                                       WGPUCreateComputePipelineAsyncCallback callback,
                                       void* userdata);

        void Finish(WGPUCreatePipelineAsyncStatus status) final;

      private:
        ComputePipelineBase* mPipeline;
        WGPUCreateComputePipelineAsyncCallback mCreateComputePipelineAsyncCallback;
    };

    struct CreateRenderPipelineAsyncTask final : public CreatePipelineAsyncTaskBase {
        CreateRenderPipelineAsyncTask(RenderPipelineBase* pipeline,
                                      WGPUCreateRenderPipelineAsyncCallback callback,
                                      void* userdata);

        void Finish(WGPUCreatePipelineAsyncStatus status) final;

      private:
        RenderPipelineBase* mPipeline;
        WGPUCreateRenderPipelineAsyncCallback mCreateRenderPipelineAsyncCallback;
    };

    class CreatePipelineAsyncTracker {
      public:
        CreatePipelineAsyncTracker(DeviceBase* device);
        ~CreatePipelineAsyncTracker();

        void TrackTask(std::unique_ptr<CreatePipelineAsyncTaskBase> task, ExecutionSerial serial);
        void Tick(ExecutionSerial finishedSerial);
        void ClearForShutDown();

      private:
        DeviceBase* mDevice;
        SerialQueue<ExecutionSerial, std::unique_ptr<CreatePipelineAsyncTaskBase>>
            mCreatePipelineAsyncTasksInFlight;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_CREATEPIPELINEASYNCTRACKER_H_
