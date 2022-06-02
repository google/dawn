// Copyright 2022 The Dawn Authors
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

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native {

template <>
void CacheKeySerializer<D3D12_COMPUTE_PIPELINE_STATE_DESC>::Serialize(
    CacheKey* key,
    const D3D12_COMPUTE_PIPELINE_STATE_DESC& t) {
    // Don't record pRootSignature as we already record the signature blob in pipline layout.
    key->Record(t.CS).Record(t.NodeMask).Record(t.Flags);
}

template <>
void CacheKeySerializer<D3D12_RENDER_TARGET_BLEND_DESC>::Serialize(
    CacheKey* key,
    const D3D12_RENDER_TARGET_BLEND_DESC& t) {
    key->Record(t.BlendEnable, t.LogicOpEnable, t.SrcBlend, t.DestBlend, t.BlendOp, t.SrcBlendAlpha,
                t.DestBlendAlpha, t.BlendOpAlpha, t.LogicOp, t.RenderTargetWriteMask);
}

template <>
void CacheKeySerializer<D3D12_BLEND_DESC>::Serialize(CacheKey* key, const D3D12_BLEND_DESC& t) {
    key->Record(t.AlphaToCoverageEnable, t.IndependentBlendEnable).Record(t.RenderTarget);
}

template <>
void CacheKeySerializer<D3D12_DEPTH_STENCILOP_DESC>::Serialize(
    CacheKey* key,
    const D3D12_DEPTH_STENCILOP_DESC& t) {
    key->Record(t.StencilFailOp, t.StencilDepthFailOp, t.StencilPassOp, t.StencilFunc);
}

template <>
void CacheKeySerializer<D3D12_DEPTH_STENCIL_DESC>::Serialize(CacheKey* key,
                                                             const D3D12_DEPTH_STENCIL_DESC& t) {
    key->Record(t.DepthEnable, t.DepthWriteMask, t.DepthFunc, t.StencilEnable, t.StencilReadMask,
                t.StencilWriteMask, t.FrontFace, t.BackFace);
}

template <>
void CacheKeySerializer<D3D12_RASTERIZER_DESC>::Serialize(CacheKey* key,
                                                          const D3D12_RASTERIZER_DESC& t) {
    key->Record(t.FillMode, t.CullMode, t.FrontCounterClockwise, t.DepthBias, t.DepthBiasClamp,
                t.SlopeScaledDepthBias, t.DepthClipEnable, t.MultisampleEnable,
                t.AntialiasedLineEnable, t.ForcedSampleCount, t.ConservativeRaster);
}

template <>
void CacheKeySerializer<D3D12_INPUT_ELEMENT_DESC>::Serialize(CacheKey* key,
                                                             const D3D12_INPUT_ELEMENT_DESC& t) {
    key->Record(t.SemanticName, t.SemanticIndex, t.Format, t.InputSlot, t.AlignedByteOffset,
                t.InputSlotClass, t.InstanceDataStepRate);
}

template <>
void CacheKeySerializer<D3D12_INPUT_LAYOUT_DESC>::Serialize(CacheKey* key,
                                                            const D3D12_INPUT_LAYOUT_DESC& t) {
    key->RecordIterable(t.pInputElementDescs, t.NumElements);
}

template <>
void CacheKeySerializer<D3D12_SO_DECLARATION_ENTRY>::Serialize(
    CacheKey* key,
    const D3D12_SO_DECLARATION_ENTRY& t) {
    key->Record(t.Stream, t.SemanticName, t.SemanticIndex, t.StartComponent, t.ComponentCount,
                t.OutputSlot);
}

template <>
void CacheKeySerializer<D3D12_STREAM_OUTPUT_DESC>::Serialize(CacheKey* key,
                                                             const D3D12_STREAM_OUTPUT_DESC& t) {
    key->RecordIterable(t.pSODeclaration, t.NumEntries)
        .RecordIterable(t.pBufferStrides, t.NumStrides)
        .Record(t.RasterizedStream);
}

template <>
void CacheKeySerializer<DXGI_SAMPLE_DESC>::Serialize(CacheKey* key, const DXGI_SAMPLE_DESC& t) {
    key->Record(t.Count, t.Quality);
}

template <>
void CacheKeySerializer<D3D12_SHADER_BYTECODE>::Serialize(CacheKey* key,
                                                          const D3D12_SHADER_BYTECODE& t) {
    key->RecordIterable(reinterpret_cast<const uint8_t*>(t.pShaderBytecode), t.BytecodeLength);
}

template <>
void CacheKeySerializer<D3D12_GRAPHICS_PIPELINE_STATE_DESC>::Serialize(
    CacheKey* key,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& t) {
    // Don't record pRootSignature as we already record the signature blob in pipline layout.
    // Don't record CachedPSO as it is in the cached blob.
    key->Record(t.VS)
        .Record(t.PS)
        .Record(t.DS)
        .Record(t.HS)
        .Record(t.GS)
        .Record(t.StreamOutput)
        .Record(t.BlendState)
        .Record(t.SampleMask)
        .Record(t.RasterizerState)
        .Record(t.DepthStencilState)
        .Record(t.InputLayout)
        .Record(t.IBStripCutValue)
        .Record(t.PrimitiveTopologyType)
        .RecordIterable(t.RTVFormats, t.NumRenderTargets)
        .Record(t.DSVFormat)
        .Record(t.SampleDesc)
        .Record(t.NodeMask)
        .Record(t.Flags);
}

template <>
void CacheKeySerializer<ID3DBlob>::Serialize(CacheKey* key, const ID3DBlob& t) {
    // Workaround: GetBufferPointer and GetbufferSize are not marked as const
    ID3DBlob* pBlob = const_cast<ID3DBlob*>(&t);
    key->RecordIterable(reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
                        pBlob->GetBufferSize());
}

}  // namespace dawn::native
