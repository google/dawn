// Copyright 2017 The Dawn Authors
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

#include "dawn_native/d3d12/ComputePipelineD3D12.h"

#include "dawn_native/CreatePipelineAsyncTask.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/UtilsD3D12.h"

namespace dawn_native { namespace d3d12 {

    ResultOrError<Ref<ComputePipeline>> ComputePipeline::Create(
        Device* device,
        const ComputePipelineDescriptor* descriptor) {
        Ref<ComputePipeline> pipeline = AcquireRef(new ComputePipeline(device, descriptor));
        DAWN_TRY(pipeline->Initialize());
        return pipeline;
    }

    MaybeError ComputePipeline::Initialize() {
        Device* device = ToBackend(GetDevice());
        uint32_t compileFlags = 0;

        if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
            compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        }

        // SPRIV-cross does matrix multiplication expecting row major matrices
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

        const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
        ShaderModule* module = ToBackend(computeStage.module.Get());

        D3D12_COMPUTE_PIPELINE_STATE_DESC d3dDesc = {};
        d3dDesc.pRootSignature = ToBackend(GetLayout())->GetRootSignature();

        CompiledShader compiledShader;
        DAWN_TRY_ASSIGN(compiledShader,
                        module->Compile(computeStage.entryPoint.c_str(), SingleShaderStage::Compute,
                                        ToBackend(GetLayout()), compileFlags));
        d3dDesc.CS = compiledShader.GetD3D12ShaderBytecode();
        auto* d3d12Device = device->GetD3D12Device();
        DAWN_TRY(CheckHRESULT(
            d3d12Device->CreateComputePipelineState(&d3dDesc, IID_PPV_ARGS(&mPipelineState)),
            "D3D12 creating pipeline state"));

        SetLabelImpl();

        return {};
    }

    ComputePipeline::~ComputePipeline() {
        ToBackend(GetDevice())->ReferenceUntilUnused(mPipelineState);
    }

    ID3D12PipelineState* ComputePipeline::GetPipelineState() const {
        return mPipelineState.Get();
    }

    void ComputePipeline::SetLabelImpl() {
        SetDebugName(ToBackend(GetDevice()), GetPipelineState(), "Dawn_ComputePipeline",
                     GetLabel());
    }

    void ComputePipeline::CreateAsync(Device* device,
                                      const ComputePipelineDescriptor* descriptor,
                                      size_t blueprintHash,
                                      WGPUCreateComputePipelineAsyncCallback callback,
                                      void* userdata) {
        Ref<ComputePipeline> pipeline = AcquireRef(new ComputePipeline(device, descriptor));
        std::unique_ptr<CreateComputePipelineAsyncTask> asyncTask =
            std::make_unique<CreateComputePipelineAsyncTask>(pipeline, blueprintHash, callback,
                                                             userdata);
        CreateComputePipelineAsyncTask::RunAsync(std::move(asyncTask));
    }

}}  // namespace dawn_native::d3d12
