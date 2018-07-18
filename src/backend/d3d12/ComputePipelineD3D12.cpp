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

#include "backend/d3d12/ComputePipelineD3D12.h"

#include "backend/d3d12/DeviceD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "backend/d3d12/ShaderModuleD3D12.h"
#include "common/Assert.h"

#include <d3dcompiler.h>

namespace backend { namespace d3d12 {

    ComputePipeline::ComputePipeline(ComputePipelineBuilder* builder)
        : ComputePipelineBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        uint32_t compileFlags = 0;
#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        // SPRIV-cross does matrix multiplication expecting row major matrices
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

        const auto& module = ToBackend(builder->GetStageInfo(nxt::ShaderStage::Compute).module);
        const auto& entryPoint = builder->GetStageInfo(nxt::ShaderStage::Compute).entryPoint;
        const auto& hlslSource = module->GetHLSLSource();

        ComPtr<ID3DBlob> compiledShader;
        ComPtr<ID3DBlob> errors;

        if (FAILED(D3DCompile(hlslSource.c_str(), hlslSource.length(), nullptr, {nullptr}, nullptr,
                              entryPoint.c_str(), "cs_5_1", compileFlags, 0, &compiledShader,
                              &errors))) {
            printf("%s\n", reinterpret_cast<char*>(errors->GetBufferPointer()));
            ASSERT(false);
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC descriptor = {};
        descriptor.pRootSignature = ToBackend(GetLayout())->GetRootSignature().Get();
        descriptor.CS.pShaderBytecode = compiledShader->GetBufferPointer();
        descriptor.CS.BytecodeLength = compiledShader->GetBufferSize();

        Device* device = ToBackend(builder->GetDevice());
        device->GetD3D12Device()->CreateComputePipelineState(&descriptor,
                                                             IID_PPV_ARGS(&mPipelineState));
    }

    ComputePipeline::~ComputePipeline() {
        mDevice->ReferenceUntilUnused(mPipelineState);
    }

    ComPtr<ID3D12PipelineState> ComputePipeline::GetPipelineState() {
        return mPipelineState;
    }

}}  // namespace backend::d3d12
