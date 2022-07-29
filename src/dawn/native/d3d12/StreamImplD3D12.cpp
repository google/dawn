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
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

template <>
void stream::Stream<D3D12_RENDER_TARGET_BLEND_DESC>::Write(
    stream::Sink* sink,
    const D3D12_RENDER_TARGET_BLEND_DESC& t) {
    StreamIn(sink, t.BlendEnable, t.LogicOpEnable, t.SrcBlend, t.DestBlend, t.BlendOp,
             t.SrcBlendAlpha, t.DestBlendAlpha, t.BlendOpAlpha, t.LogicOp, t.RenderTargetWriteMask);
}

template <>
void stream::Stream<D3D12_BLEND_DESC>::Write(stream::Sink* sink, const D3D12_BLEND_DESC& t) {
    StreamIn(sink, t.AlphaToCoverageEnable, t.IndependentBlendEnable, t.RenderTarget);
}

template <>
void stream::Stream<D3D12_DEPTH_STENCILOP_DESC>::Write(stream::Sink* sink,
                                                       const D3D12_DEPTH_STENCILOP_DESC& t) {
    StreamIn(sink, t.StencilFailOp, t.StencilDepthFailOp, t.StencilPassOp, t.StencilFunc);
}

template <>
void stream::Stream<D3D12_DEPTH_STENCIL_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_DEPTH_STENCIL_DESC& t) {
    StreamIn(sink, t.DepthEnable, t.DepthWriteMask, t.DepthFunc, t.StencilEnable, t.StencilReadMask,
             t.StencilWriteMask, t.FrontFace, t.BackFace);
}

template <>
void stream::Stream<D3D12_RASTERIZER_DESC>::Write(stream::Sink* sink,
                                                  const D3D12_RASTERIZER_DESC& t) {
    StreamIn(sink, t.FillMode, t.CullMode, t.FrontCounterClockwise, t.DepthBias, t.DepthBiasClamp,
             t.SlopeScaledDepthBias, t.DepthClipEnable, t.MultisampleEnable,
             t.AntialiasedLineEnable, t.ForcedSampleCount, t.ConservativeRaster);
}

template <>
void stream::Stream<D3D12_INPUT_ELEMENT_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_INPUT_ELEMENT_DESC& t) {
    StreamIn(sink, std::string_view(t.SemanticName), t.SemanticIndex, t.Format, t.InputSlot,
             t.AlignedByteOffset, t.InputSlotClass, t.InstanceDataStepRate);
}

template <>
void stream::Stream<D3D12_INPUT_LAYOUT_DESC>::Write(stream::Sink* sink,
                                                    const D3D12_INPUT_LAYOUT_DESC& t) {
    StreamIn(sink, Iterable(t.pInputElementDescs, t.NumElements));
}

template <>
void stream::Stream<D3D12_SO_DECLARATION_ENTRY>::Write(stream::Sink* sink,
                                                       const D3D12_SO_DECLARATION_ENTRY& t) {
    StreamIn(sink, t.Stream, std::string_view(t.SemanticName), t.SemanticIndex, t.StartComponent,
             t.ComponentCount, t.OutputSlot);
}

template <>
void stream::Stream<D3D12_STREAM_OUTPUT_DESC>::Write(stream::Sink* sink,
                                                     const D3D12_STREAM_OUTPUT_DESC& t) {
    StreamIn(sink, Iterable(t.pSODeclaration, t.NumEntries),
             Iterable(t.pBufferStrides, t.NumStrides), t.RasterizedStream);
}

template <>
void stream::Stream<DXGI_SAMPLE_DESC>::Write(stream::Sink* sink, const DXGI_SAMPLE_DESC& t) {
    StreamIn(sink, t.Count, t.Quality);
}

template <>
void stream::Stream<D3D12_SHADER_BYTECODE>::Write(stream::Sink* sink,
                                                  const D3D12_SHADER_BYTECODE& t) {
    StreamIn(sink, Iterable(reinterpret_cast<const uint8_t*>(t.pShaderBytecode), t.BytecodeLength));
}

template <>
void stream::Stream<D3D12_GRAPHICS_PIPELINE_STATE_DESC>::Write(
    stream::Sink* sink,
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& t) {
    // Don't Serialize pRootSignature as we already serialize the signature blob in pipline layout.
    // Don't Serialize CachedPSO as it is in the cached blob.
    StreamIn(sink, t.VS, t.PS, t.DS, t.HS, t.GS, t.StreamOutput, t.BlendState, t.SampleMask,
             t.RasterizerState, t.DepthStencilState, t.InputLayout, t.IBStripCutValue,
             t.PrimitiveTopologyType, Iterable(t.RTVFormats, t.NumRenderTargets), t.DSVFormat,
             t.SampleDesc, t.NodeMask, t.Flags);
}

template <>
void stream::Stream<D3D12_COMPUTE_PIPELINE_STATE_DESC>::Write(
    stream::Sink* sink,
    const D3D12_COMPUTE_PIPELINE_STATE_DESC& t) {
    // Don't Serialize pRootSignature as we already serialize the signature blob in pipline layout.
    StreamIn(sink, t.CS, t.NodeMask, t.Flags);
}

template <>
void stream::Stream<ID3DBlob>::Write(stream::Sink* sink, const ID3DBlob& t) {
    // Workaround: GetBufferPointer and GetbufferSize are not marked as const
    ID3DBlob* pBlob = const_cast<ID3DBlob*>(&t);
    StreamIn(sink, Iterable(reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
                            pBlob->GetBufferSize()));
}

}  // namespace dawn::native
