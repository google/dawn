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

#ifndef SRC_DAWN_NATIVE_CREATEPIPELINEASYNCTASK_H_
#define SRC_DAWN_NATIVE_CREATEPIPELINEASYNCTASK_H_

#include <memory>
#include <string>

#include "dawn/common/RefCounted.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/Error.h"
#include "dawn/webgpu.h"

namespace dawn::native {

class ComputePipelineBase;
class DeviceBase;
class PipelineLayoutBase;
class RenderPipelineBase;
class ShaderModuleBase;
struct FlatComputePipelineDescriptor;

struct CreatePipelineAsyncCallbackTaskBase : CallbackTask {
    explicit CreatePipelineAsyncCallbackTaskBase(void* userData);
    CreatePipelineAsyncCallbackTaskBase(WGPUCreatePipelineAsyncStatus status,
                                        std::string errorMessage,
                                        void* userData);
    ~CreatePipelineAsyncCallbackTaskBase() override;

  protected:
    std::string mErrorMessage;
    WGPUCreatePipelineAsyncStatus mStatus;
    void* mUserData;
};

struct CreateComputePipelineAsyncCallbackTask : CreatePipelineAsyncCallbackTaskBase {
    CreateComputePipelineAsyncCallbackTask(Ref<ComputePipelineBase> pipeline,
                                           WGPUCreateComputePipelineAsyncCallback callback,
                                           void* userdata);
    CreateComputePipelineAsyncCallbackTask(WGPUCreatePipelineAsyncStatus status,
                                           std::string errorMessage,
                                           WGPUCreateComputePipelineAsyncCallback callback,
                                           void* userdata);
    ~CreateComputePipelineAsyncCallbackTask() override;

    void Finish() override;
    void HandleShutDown() final;
    void HandleDeviceLoss() final;

  protected:
    Ref<ComputePipelineBase> mPipeline;
    WGPUCreateComputePipelineAsyncCallback mCreateComputePipelineAsyncCallback;
};

struct CreateRenderPipelineAsyncCallbackTask : CreatePipelineAsyncCallbackTaskBase {
    CreateRenderPipelineAsyncCallbackTask(Ref<RenderPipelineBase> pipeline,
                                          WGPUCreateRenderPipelineAsyncCallback callback,
                                          void* userdata);
    CreateRenderPipelineAsyncCallbackTask(WGPUCreatePipelineAsyncStatus status,
                                          std::string errorMessage,
                                          WGPUCreateRenderPipelineAsyncCallback callback,
                                          void* userdata);
    ~CreateRenderPipelineAsyncCallbackTask() override;

    void Finish() override;
    void HandleShutDown() final;
    void HandleDeviceLoss() final;

  protected:
    Ref<RenderPipelineBase> mPipeline;
    WGPUCreateRenderPipelineAsyncCallback mCreateRenderPipelineAsyncCallback;
};

// CreateComputePipelineAsyncTask defines all the inputs and outputs of
// CreateComputePipelineAsync() tasks, which are the same among all the backends.
class CreateComputePipelineAsyncTask {
  public:
    CreateComputePipelineAsyncTask(Ref<ComputePipelineBase> nonInitializedComputePipeline,
                                   WGPUCreateComputePipelineAsyncCallback callback,
                                   void* userdata);
    ~CreateComputePipelineAsyncTask();

    void Run();

    static void RunAsync(std::unique_ptr<CreateComputePipelineAsyncTask> task);

  private:
    Ref<ComputePipelineBase> mComputePipeline;
    WGPUCreateComputePipelineAsyncCallback mCallback;
    void* mUserdata;
};

// CreateRenderPipelineAsyncTask defines all the inputs and outputs of
// CreateRenderPipelineAsync() tasks, which are the same among all the backends.
class CreateRenderPipelineAsyncTask {
  public:
    CreateRenderPipelineAsyncTask(Ref<RenderPipelineBase> nonInitializedRenderPipeline,
                                  WGPUCreateRenderPipelineAsyncCallback callback,
                                  void* userdata);
    ~CreateRenderPipelineAsyncTask();

    void Run();

    static void RunAsync(std::unique_ptr<CreateRenderPipelineAsyncTask> task);

  private:
    Ref<RenderPipelineBase> mRenderPipeline;
    WGPUCreateRenderPipelineAsyncCallback mCallback;
    void* mUserdata;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CREATEPIPELINEASYNCTASK_H_
