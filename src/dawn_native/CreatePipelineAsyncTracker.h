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

#include "common/RefCounted.h"
#include "common/SerialQueue.h"
#include "dawn/webgpu.h"
#include "dawn_native/IntegerTypes.h"

#include <memory>
#include <string>

namespace dawn_native {

    class ComputePipelineBase;
    class DeviceBase;
    class RenderPipelineBase;

    struct CreatePipelineAsyncTaskBase {
        CreatePipelineAsyncTaskBase(std::string errorMessage, void* userData);
        virtual ~CreatePipelineAsyncTaskBase();

        virtual void Finish() = 0;
        virtual void HandleShutDown() = 0;
        virtual void HandleDeviceLoss() = 0;

      protected:
        std::string mErrorMessage;
        void* mUserData;
    };

    struct CreateComputePipelineAsyncTask final : public CreatePipelineAsyncTaskBase {
        CreateComputePipelineAsyncTask(Ref<ComputePipelineBase> pipeline,
                                       std::string errorMessage,
                                       WGPUCreateComputePipelineAsyncCallback callback,
                                       void* userdata);

        void Finish() final;
        void HandleShutDown() final;
        void HandleDeviceLoss() final;

      private:
        Ref<ComputePipelineBase> mPipeline;
        WGPUCreateComputePipelineAsyncCallback mCreateComputePipelineAsyncCallback;
    };

    struct CreateRenderPipelineAsyncTask final : public CreatePipelineAsyncTaskBase {
        CreateRenderPipelineAsyncTask(Ref<RenderPipelineBase> pipeline,
                                      std::string errorMessage,
                                      WGPUCreateRenderPipelineAsyncCallback callback,
                                      void* userdata);

        void Finish() final;
        void HandleShutDown() final;
        void HandleDeviceLoss() final;

      private:
        Ref<RenderPipelineBase> mPipeline;
        WGPUCreateRenderPipelineAsyncCallback mCreateRenderPipelineAsyncCallback;
    };

    class CreatePipelineAsyncTracker {
      public:
        explicit CreatePipelineAsyncTracker(DeviceBase* device);
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
