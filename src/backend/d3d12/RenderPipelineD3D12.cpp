// Copyright 2017 The NXT Authors
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

#include "backend/d3d12/RenderPipelineD3D12.h"

#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/InputStateD3D12.h"
#include "backend/d3d12/DepthStencilStateD3D12.h"
#include "backend/d3d12/ShaderModuleD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "common/Assert.h"

#include <d3dcompiler.h>

namespace backend {
namespace d3d12 {

    namespace {
        D3D12_PRIMITIVE_TOPOLOGY D3D12PrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::PointList:
                    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case nxt::PrimitiveTopology::LineList:
                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case nxt::PrimitiveTopology::LineStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case nxt::PrimitiveTopology::TriangleList:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case nxt::PrimitiveTopology::TriangleStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12PrimitiveTopologyType(nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::PointList:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                case nxt::PrimitiveTopology::LineList:
                case nxt::PrimitiveTopology::LineStrip:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                case nxt::PrimitiveTopology::TriangleList:
                case nxt::PrimitiveTopology::TriangleStrip:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                default:
                    UNREACHABLE();
            }
        }
    }

    RenderPipeline::RenderPipeline(RenderPipelineBuilder* builder)
        : RenderPipelineBase(builder), d3d12PrimitiveTopology(D3D12PrimitiveTopology(GetPrimitiveTopology())) {
        uint32_t compileFlags = 0;
#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        // SPRIV-cross does matrix multiplication expecting row major matrices
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC descriptor = {};

        PerStage<ComPtr<ID3DBlob>> compiledShader;
        ComPtr<ID3DBlob> errors;

        for (auto stage : IterateStages(GetStageMask())) {
            const auto& module = ToBackend(builder->GetStageInfo(stage).module);
            const auto& entryPoint = builder->GetStageInfo(stage).entryPoint;
            const auto& hlslSource = module->GetHLSLSource();

            const char* compileTarget = nullptr;

            D3D12_SHADER_BYTECODE* shader = nullptr;
            switch (stage) {
            case nxt::ShaderStage::Vertex:
                shader = &descriptor.VS;
                compileTarget = "vs_5_1";
                break;
            case nxt::ShaderStage::Fragment:
                shader = &descriptor.PS;
                compileTarget = "ps_5_1";
                break;
            case nxt::ShaderStage::Compute:
                UNREACHABLE();
                break;
            }

            if(FAILED(D3DCompile(
                hlslSource.c_str(),
                hlslSource.length(),
                nullptr,
                nullptr,
                nullptr,
                entryPoint.c_str(),
                compileTarget,
                compileFlags,
                0,
                &compiledShader[stage],
                &errors
            ))) {
                printf("%s\n", reinterpret_cast<char*>(errors->GetBufferPointer()));
                ASSERT(false);
            }

            if (shader != nullptr) {
                shader->pShaderBytecode = compiledShader[stage]->GetBufferPointer();
                shader->BytecodeLength = compiledShader[stage]->GetBufferSize();
            }
        }

        PipelineLayout* layout = ToBackend(GetLayout());

        descriptor.pRootSignature = layout->GetRootSignature().Get();

        // D3D12 logs warnings if any empty input state is used
        InputState* inputState = ToBackend(GetInputState());
        if (inputState->GetAttributesSetMask().any()) {
            descriptor.InputLayout = inputState->GetD3D12InputLayoutDescriptor();
        }

        descriptor.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        descriptor.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        descriptor.RasterizerState.FrontCounterClockwise = FALSE;
        descriptor.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        descriptor.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        descriptor.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        descriptor.RasterizerState.DepthClipEnable = TRUE;
        descriptor.RasterizerState.MultisampleEnable = FALSE;
        descriptor.RasterizerState.AntialiasedLineEnable = FALSE;
        descriptor.RasterizerState.ForcedSampleCount = 0;
        descriptor.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        descriptor.BlendState.AlphaToCoverageEnable = FALSE;
        descriptor.BlendState.IndependentBlendEnable = FALSE;
        descriptor.BlendState.RenderTarget[0].BlendEnable = FALSE;
        descriptor.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
        descriptor.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        descriptor.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        descriptor.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        descriptor.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        descriptor.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        descriptor.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        descriptor.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        descriptor.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		DepthStencilState* depthStencilState = ToBackend(GetDepthStencilState());
		descriptor.DepthStencilState = depthStencilState->GetD3D12DepthStencilDescriptor();

        descriptor.SampleMask = UINT_MAX;
        descriptor.PrimitiveTopologyType = D3D12PrimitiveTopologyType(GetPrimitiveTopology());
        descriptor.NumRenderTargets = 1;
        descriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        descriptor.SampleDesc.Count = 1;

        Device* device = ToBackend(builder->GetDevice());
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateGraphicsPipelineState(&descriptor, IID_PPV_ARGS(&pipelineState)));
    }

    D3D12_PRIMITIVE_TOPOLOGY RenderPipeline::GetD3D12PrimitiveTopology() const {
        return d3d12PrimitiveTopology;
    }

    ComPtr<ID3D12PipelineState> RenderPipeline::GetPipelineState() {
        return pipelineState;
    }

}
}
