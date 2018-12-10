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

#include "dawn_native/d3d12/RenderPipelineD3D12.h"

#include "common/Assert.h"
#include "dawn_native/d3d12/BlendStateD3D12.h"
#include "dawn_native/d3d12/DepthStencilStateD3D12.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/InputStateD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

#include <d3dcompiler.h>

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_PRIMITIVE_TOPOLOGY D3D12PrimitiveTopology(dawn::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case dawn::PrimitiveTopology::PointList:
                    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case dawn::PrimitiveTopology::LineList:
                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case dawn::PrimitiveTopology::LineStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case dawn::PrimitiveTopology::TriangleList:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case dawn::PrimitiveTopology::TriangleStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12PrimitiveTopologyType(
            dawn::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case dawn::PrimitiveTopology::PointList:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                case dawn::PrimitiveTopology::LineList:
                case dawn::PrimitiveTopology::LineStrip:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                case dawn::PrimitiveTopology::TriangleList:
                case dawn::PrimitiveTopology::TriangleStrip:
                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                default:
                    UNREACHABLE();
            }
        }
    }  // namespace

    RenderPipeline::RenderPipeline(Device* device, const RenderPipelineDescriptor* descriptor)
        : RenderPipelineBase(device, descriptor),
          mD3d12PrimitiveTopology(D3D12PrimitiveTopology(GetPrimitiveTopology())) {
        uint32_t compileFlags = 0;
#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        // SPRIV-cross does matrix multiplication expecting row major matrices
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC descriptorD3D12 = {};

        PerStage<ComPtr<ID3DBlob>> compiledShader;
        ComPtr<ID3DBlob> errors;

        dawn::ShaderStageBit renderStages =
            dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment;
        for (auto stage : IterateStages(renderStages)) {
            const ShaderModule* module = nullptr;
            const char* entryPoint = nullptr;
            const char* compileTarget = nullptr;
            D3D12_SHADER_BYTECODE* shader = nullptr;
            switch (stage) {
                case dawn::ShaderStage::Vertex:
                    module = ToBackend(descriptor->vertexStage->module);
                    entryPoint = descriptor->vertexStage->entryPoint;
                    shader = &descriptorD3D12.VS;
                    compileTarget = "vs_5_1";
                    break;
                case dawn::ShaderStage::Fragment:
                    module = ToBackend(descriptor->fragmentStage->module);
                    entryPoint = descriptor->fragmentStage->entryPoint;
                    shader = &descriptorD3D12.PS;
                    compileTarget = "ps_5_1";
                    break;
                default:
                    UNREACHABLE();
                    break;
            }

            const std::string hlslSource = module->GetHLSLSource(ToBackend(GetLayout()));

            const PlatformFunctions* functions = device->GetFunctions();
            if (FAILED(functions->d3dCompile(hlslSource.c_str(), hlslSource.length(), nullptr,
                                             nullptr, nullptr, entryPoint, compileTarget,
                                             compileFlags, 0, &compiledShader[stage], &errors))) {
                printf("%s\n", reinterpret_cast<char*>(errors->GetBufferPointer()));
                ASSERT(false);
            }

            if (shader != nullptr) {
                shader->pShaderBytecode = compiledShader[stage]->GetBufferPointer();
                shader->BytecodeLength = compiledShader[stage]->GetBufferSize();
            }
        }

        PipelineLayout* layout = ToBackend(GetLayout());

        descriptorD3D12.pRootSignature = layout->GetRootSignature().Get();

        // D3D12 logs warnings if any empty input state is used
        InputState* inputState = ToBackend(GetInputState());
        if (inputState->GetAttributesSetMask().any()) {
            descriptorD3D12.InputLayout = inputState->GetD3D12InputLayoutDescriptor();
        }

        descriptorD3D12.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        descriptorD3D12.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        descriptorD3D12.RasterizerState.FrontCounterClockwise = FALSE;
        descriptorD3D12.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        descriptorD3D12.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        descriptorD3D12.RasterizerState.SlopeScaledDepthBias =
            D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        descriptorD3D12.RasterizerState.DepthClipEnable = TRUE;
        descriptorD3D12.RasterizerState.MultisampleEnable = FALSE;
        descriptorD3D12.RasterizerState.AntialiasedLineEnable = FALSE;
        descriptorD3D12.RasterizerState.ForcedSampleCount = 0;
        descriptorD3D12.RasterizerState.ConservativeRaster =
            D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        if (HasDepthStencilAttachment()) {
            descriptorD3D12.DSVFormat = D3D12TextureFormat(GetDepthStencilFormat());
        }

        for (uint32_t i : IterateBitSet(GetColorAttachmentsMask())) {
            descriptorD3D12.RTVFormats[i] = D3D12TextureFormat(GetColorAttachmentFormat(i));
            descriptorD3D12.BlendState.RenderTarget[i] =
                ToBackend(GetBlendState(i))->GetD3D12BlendDesc();
        }
        descriptorD3D12.NumRenderTargets = static_cast<uint32_t>(GetColorAttachmentsMask().count());

        descriptorD3D12.BlendState.AlphaToCoverageEnable = FALSE;
        descriptorD3D12.BlendState.IndependentBlendEnable = TRUE;

        DepthStencilState* depthStencilState = ToBackend(GetDepthStencilState());
        descriptorD3D12.DepthStencilState = depthStencilState->GetD3D12DepthStencilDescriptor();

        descriptorD3D12.SampleMask = UINT_MAX;
        descriptorD3D12.PrimitiveTopologyType = D3D12PrimitiveTopologyType(GetPrimitiveTopology());
        descriptorD3D12.SampleDesc.Count = 1;

        ASSERT_SUCCESS(device->GetD3D12Device()->CreateGraphicsPipelineState(
            &descriptorD3D12, IID_PPV_ARGS(&mPipelineState)));
    }

    RenderPipeline::~RenderPipeline() {
        ToBackend(GetDevice())->ReferenceUntilUnused(mPipelineState);
    }

    D3D12_PRIMITIVE_TOPOLOGY RenderPipeline::GetD3D12PrimitiveTopology() const {
        return mD3d12PrimitiveTopology;
    }

    ComPtr<ID3D12PipelineState> RenderPipeline::GetPipelineState() {
        return mPipelineState;
    }

}}  // namespace dawn_native::d3d12
