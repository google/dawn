// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_CREATEPIPELINEASYNCTASK_H_
#define SRC_DAWN_NATIVE_CREATEPIPELINEASYNCTASK_H_

#include <memory>
#include <string>

#include "dawn/common/Ref.h"
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
