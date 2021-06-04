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
#include "dawn_native/Error.h"

namespace dawn_native {

    class ComputePipelineBase;
    class DeviceBase;
    class PipelineLayoutBase;
    class RenderPipelineBase;
    class ShaderModuleBase;
    struct ComputePipelineDescriptor;

    struct CreatePipelineAsyncCallbackTaskBase : CallbackTask {
        CreatePipelineAsyncCallbackTaskBase(std::string errorMessage, void* userData);

      protected:
        std::string mErrorMessage;
        void* mUserData;
    };

    struct CreateComputePipelineAsyncCallbackTask : CreatePipelineAsyncCallbackTaskBase {
        CreateComputePipelineAsyncCallbackTask(Ref<ComputePipelineBase> pipeline,
                                               std::string errorMessage,
                                               WGPUCreateComputePipelineAsyncCallback callback,
                                               void* userdata);

        void Finish() override;
        void HandleShutDown() final;
        void HandleDeviceLoss() final;

      protected:
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

    // CreateComputePipelineAsyncTask defines all the inputs and outputs of
    // CreateComputePipelineAsync() tasks, which are the same among all the backends.
    // TODO(crbug.com/dawn/529): Define a "flat descriptor"
    // (like utils::ComboRenderPipelineDescriptor) in ComputePipeline.h that's reused here and for
    // caching, etc. ValidateComputePipelineDescriptor() could produce that flat descriptor so that
    // it is reused in other places.
    class CreateComputePipelineAsyncTask {
      public:
        CreateComputePipelineAsyncTask(Ref<ComputePipelineBase> nonInitializedComputePipeline,
                                       const ComputePipelineDescriptor* descriptor,
                                       size_t blueprintHash,
                                       WGPUCreateComputePipelineAsyncCallback callback,
                                       void* userdata);

        virtual ~CreateComputePipelineAsyncTask() = default;
        void Run();

        static void RunAsync(std::unique_ptr<CreateComputePipelineAsyncTask> task);

      protected:
        Ref<ComputePipelineBase> mComputePipeline;
        size_t mBlueprintHash;
        WGPUCreateComputePipelineAsyncCallback mCallback;
        void* mUserdata;

        std::string mLabel;
        Ref<PipelineLayoutBase> mLayout;
        std::string mEntryPoint;
        Ref<ShaderModuleBase> mComputeShaderModule;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_CREATEPIPELINEASYNCTASK_H_
