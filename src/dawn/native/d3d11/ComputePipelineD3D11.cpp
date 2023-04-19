// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d11/ComputePipelineD3D11.h"

#include <memory>
#include <utility>

#include "dawn/native/CreatePipelineAsyncTask.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/ShaderModuleD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const ComputePipelineDescriptor* descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::~ComputePipeline() = default;

MaybeError ComputePipeline::Initialize() {
    Device* device = ToBackend(GetDevice());
    uint32_t compileFlags = 0;

    if (!device->IsToggleEnabled(Toggle::UseDXC) &&
        !device->IsToggleEnabled(Toggle::FxcOptimizations)) {
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    }

    if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    // Tint does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    // FXC can miscompile code that depends on special float values (NaN, INF, etc) when IEEE
    // strictness is not enabled. See crbug.com/tint/976.
    compileFlags |= D3DCOMPILE_IEEE_STRICTNESS;

    const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Compute);

    d3d::CompiledShader compiledShader;
    DAWN_TRY_ASSIGN(compiledShader, ToBackend(programmableStage.module)
                                        ->Compile(programmableStage, SingleShaderStage::Compute,
                                                  ToBackend(GetLayout()), compileFlags));
    DAWN_TRY(CheckHRESULT(device->GetD3D11Device()->CreateComputeShader(
                              compiledShader.shaderBlob.Data(), compiledShader.shaderBlob.Size(),
                              nullptr, &mComputeShader),
                          "D3D11 create compute shader"));

    SetLabelImpl();

    return {};
}

void ComputePipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mComputeShader.Get(), "Dawn_ComputePipeline", GetLabel());
}

void ComputePipeline::ApplyNow(CommandRecordingContext* commandContext) {
    ID3D11DeviceContext1* d3dDeviceContext1 = commandContext->GetD3D11DeviceContext1();
    d3dDeviceContext1->CSSetShader(mComputeShader.Get(), nullptr, 0);
}

void ComputePipeline::InitializeAsync(Ref<ComputePipelineBase> computePipeline,
                                      WGPUCreateComputePipelineAsyncCallback callback,
                                      void* userdata) {
    std::unique_ptr<CreateComputePipelineAsyncTask> asyncTask =
        std::make_unique<CreateComputePipelineAsyncTask>(std::move(computePipeline), callback,
                                                         userdata);
    CreateComputePipelineAsyncTask::RunAsync(std::move(asyncTask));
}

}  // namespace dawn::native::d3d11
