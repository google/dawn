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

#ifndef DAWNNATIVE_CREATEREADYPIPELINETRACKER_H_
#define DAWNNATIVE_CREATEREADYPIPELINETRACKER_H_

#include "common/SerialQueue.h"
#include "dawn/webgpu.h"
#include "dawn_native/IntegerTypes.h"

#include <memory>

namespace dawn_native {

    class ComputePipelineBase;
    class DeviceBase;
    class PipelineBase;
    class RenderPipelineBase;

    struct CreateReadyPipelineTaskBase {
        CreateReadyPipelineTaskBase(void* userData);
        virtual ~CreateReadyPipelineTaskBase();

        virtual void Finish() = 0;

      protected:
        void* mUserData;
    };

    struct CreateReadyComputePipelineTask final : public CreateReadyPipelineTaskBase {
        CreateReadyComputePipelineTask(ComputePipelineBase* pipeline,
                                       WGPUCreateReadyComputePipelineCallback callback,
                                       void* userdata);

        void Finish() final;

      private:
        ComputePipelineBase* mPipeline;
        WGPUCreateReadyComputePipelineCallback mCreateReadyComputePipelineCallback;
    };

    struct CreateReadyRenderPipelineTask final : public CreateReadyPipelineTaskBase {
        CreateReadyRenderPipelineTask(RenderPipelineBase* pipeline,
                                      WGPUCreateReadyRenderPipelineCallback callback,
                                      void* userdata);

        void Finish() final;

      private:
        RenderPipelineBase* mPipeline;
        WGPUCreateReadyRenderPipelineCallback mCreateReadyRenderPipelineCallback;
    };

    class CreateReadyPipelineTracker {
      public:
        CreateReadyPipelineTracker(DeviceBase* device);
        ~CreateReadyPipelineTracker();

        void TrackTask(std::unique_ptr<CreateReadyPipelineTaskBase> task, ExecutionSerial serial);
        void Tick(ExecutionSerial finishedSerial);

      private:
        DeviceBase* mDevice;
        SerialQueue<ExecutionSerial, std::unique_ptr<CreateReadyPipelineTaskBase>>
            mCreateReadyPipelineTasksInFlight;
    };

}  // namespace dawn_native

#endif
