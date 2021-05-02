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

#ifndef DAWNNATIVE_CREATEPIPELINEASYNCTASK_H_
#define DAWNNATIVE_CREATEPIPELINEASYNCTASK_H_

#include "common/RefCounted.h"
#include "dawn/webgpu.h"
#include "dawn_native/CallbackTaskManager.h"

namespace dawn_native {

    class ComputePipelineBase;
    class DeviceBase;
    class RenderPipelineBase;

    struct CreatePipelineAsyncCallbackTaskBase : CallbackTask {
        CreatePipelineAsyncCallbackTaskBase(std::string errorMessage, void* userData);

      protected:
        std::string mErrorMessage;
        void* mUserData;
    };

    struct CreateComputePipelineAsyncCallbackTask final : CreatePipelineAsyncCallbackTaskBase {
        CreateComputePipelineAsyncCallbackTask(Ref<ComputePipelineBase> pipeline,
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

    struct CreateRenderPipelineAsyncCallbackTask final : CreatePipelineAsyncCallbackTaskBase {
        CreateRenderPipelineAsyncCallbackTask(Ref<RenderPipelineBase> pipeline,
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

}  // namespace dawn_native

#endif  // DAWNNATIVE_CREATEPIPELINEASYNCTASK_H_
