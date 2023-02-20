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

#include "dawn/native/d3d12/RenderPipelineD3D12.h"

#include <d3dcompiler.h>

#include <memory>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/native/CreatePipelineAsyncTask.h"
#include "dawn/native/d3d12/BlobD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctions.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {

namespace {
DXGI_FORMAT VertexFormatType(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8x2:
            return DXGI_FORMAT_R8G8_UINT;
        case wgpu::VertexFormat::Uint8x4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case wgpu::VertexFormat::Sint8x2:
            return DXGI_FORMAT_R8G8_SINT;
        case wgpu::VertexFormat::Sint8x4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case wgpu::VertexFormat::Unorm8x2:
            return DXGI_FORMAT_R8G8_UNORM;
        case wgpu::VertexFormat::Unorm8x4:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case wgpu::VertexFormat::Snorm8x2:
            return DXGI_FORMAT_R8G8_SNORM;
        case wgpu::VertexFormat::Snorm8x4:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case wgpu::VertexFormat::Uint16x2:
            return DXGI_FORMAT_R16G16_UINT;
        case wgpu::VertexFormat::Uint16x4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case wgpu::VertexFormat::Sint16x2:
            return DXGI_FORMAT_R16G16_SINT;
        case wgpu::VertexFormat::Sint16x4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case wgpu::VertexFormat::Unorm16x2:
            return DXGI_FORMAT_R16G16_UNORM;
        case wgpu::VertexFormat::Unorm16x4:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case wgpu::VertexFormat::Snorm16x2:
            return DXGI_FORMAT_R16G16_SNORM;
        case wgpu::VertexFormat::Snorm16x4:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case wgpu::VertexFormat::Float16x2:
            return DXGI_FORMAT_R16G16_FLOAT;
        case wgpu::VertexFormat::Float16x4:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case wgpu::VertexFormat::Float32:
            return DXGI_FORMAT_R32_FLOAT;
        case wgpu::VertexFormat::Float32x2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case wgpu::VertexFormat::Float32x3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case wgpu::VertexFormat::Float32x4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case wgpu::VertexFormat::Uint32:
            return DXGI_FORMAT_R32_UINT;
        case wgpu::VertexFormat::Uint32x2:
            return DXGI_FORMAT_R32G32_UINT;
        case wgpu::VertexFormat::Uint32x3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case wgpu::VertexFormat::Uint32x4:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case wgpu::VertexFormat::Sint32:
            return DXGI_FORMAT_R32_SINT;
        case wgpu::VertexFormat::Sint32x2:
            return DXGI_FORMAT_R32G32_SINT;
        case wgpu::VertexFormat::Sint32x3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case wgpu::VertexFormat::Sint32x4:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        default:
            UNREACHABLE();
    }
}

D3D12_INPUT_CLASSIFICATION VertexStepModeFunction(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        case wgpu::VertexStepMode::Instance:
            return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        case wgpu::VertexStepMode::VertexBufferNotUsed:
            UNREACHABLE();
    }
}

D3D12_PRIMITIVE_TOPOLOGY D3D12PrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case wgpu::PrimitiveTopology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case wgpu::PrimitiveTopology::LineStrip:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case wgpu::PrimitiveTopology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12PrimitiveTopologyType(
    wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case wgpu::PrimitiveTopology::LineList:
        case wgpu::PrimitiveTopology::LineStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case wgpu::PrimitiveTopology::TriangleList:
        case wgpu::PrimitiveTopology::TriangleStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}

D3D12_CULL_MODE D3D12CullMode(wgpu::CullMode mode) {
    switch (mode) {
        case wgpu::CullMode::None:
            return D3D12_CULL_MODE_NONE;
        case wgpu::CullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case wgpu::CullMode::Back:
            return D3D12_CULL_MODE_BACK;
    }
}

D3D12_BLEND D3D12Blend(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return D3D12_BLEND_ZERO;
        case wgpu::BlendFactor::One:
            return D3D12_BLEND_ONE;
        case wgpu::BlendFactor::Src:
            return D3D12_BLEND_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D12_BLEND_INV_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return D3D12_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D12_BLEND_DEST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D12_BLEND_INV_DEST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return D3D12_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case wgpu::BlendFactor::Constant:
            return D3D12_BLEND_BLEND_FACTOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return D3D12_BLEND_INV_BLEND_FACTOR;
    }
}

// When a blend factor is defined for the alpha channel, any of the factors that don't
// explicitly state that they apply to alpha should be treated as their explicitly-alpha
// equivalents. See: https://github.com/gpuweb/gpuweb/issues/65
D3D12_BLEND D3D12AlphaBlend(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Src:
            return D3D12_BLEND_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return D3D12_BLEND_DEST_ALPHA;
        case wgpu::BlendFactor::OneMinusDst:
            return D3D12_BLEND_INV_DEST_ALPHA;

        // Other blend factors translate to the same D3D12 enum as the color blend factors.
        default:
            return D3D12Blend(factor);
    }
}

D3D12_BLEND_OP D3D12BlendOperation(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return D3D12_BLEND_OP_ADD;
        case wgpu::BlendOperation::Subtract:
            return D3D12_BLEND_OP_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return D3D12_BLEND_OP_MIN;
        case wgpu::BlendOperation::Max:
            return D3D12_BLEND_OP_MAX;
    }
}

uint8_t D3D12RenderTargetWriteMask(wgpu::ColorWriteMask writeMask) {
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Red) ==
                      D3D12_COLOR_WRITE_ENABLE_RED,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Green) ==
                      D3D12_COLOR_WRITE_ENABLE_GREEN,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Blue) ==
                      D3D12_COLOR_WRITE_ENABLE_BLUE,
                  "ColorWriteMask values must match");
    static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(wgpu::ColorWriteMask::Alpha) ==
                      D3D12_COLOR_WRITE_ENABLE_ALPHA,
                  "ColorWriteMask values must match");
    return static_cast<uint8_t>(writeMask);
}

D3D12_RENDER_TARGET_BLEND_DESC ComputeColorDesc(const DeviceBase* device,
                                                const ColorTargetState* state) {
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
    blendDesc.BlendEnable = state->blend != nullptr;
    if (blendDesc.BlendEnable) {
        blendDesc.SrcBlend = D3D12Blend(state->blend->color.srcFactor);
        if (device->GetValidInternalFormat(state->format).componentCount < 4 &&
            blendDesc.SrcBlend == D3D12_BLEND_DEST_ALPHA) {
            // According to the D3D SPEC, the default value for missing components in an element
            // format is "0" for any component except A, which gets "1". So here
            // D3D12_BLEND_DEST_ALPHA should have same effect as D3D12_BLEND_ONE.
            // Note that this replacement can be an optimization as using D3D12_BLEND_ONE means the
            // GPU hardware no longer needs to get pixels from the destination texture. It can also
            // be served as a workaround against an Intel driver issue about alpha blending (see
            // http://crbug.com/dawn/1579 for more details).
            blendDesc.SrcBlend = D3D12_BLEND_ONE;
        }

        blendDesc.DestBlend = D3D12Blend(state->blend->color.dstFactor);
        blendDesc.BlendOp = D3D12BlendOperation(state->blend->color.operation);
        blendDesc.SrcBlendAlpha = D3D12AlphaBlend(state->blend->alpha.srcFactor);
        blendDesc.DestBlendAlpha = D3D12AlphaBlend(state->blend->alpha.dstFactor);
        blendDesc.BlendOpAlpha = D3D12BlendOperation(state->blend->alpha.operation);
    }
    blendDesc.RenderTargetWriteMask = D3D12RenderTargetWriteMask(state->writeMask);
    blendDesc.LogicOpEnable = false;
    blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    return blendDesc;
}

D3D12_STENCIL_OP StencilOp(wgpu::StencilOperation op) {
    switch (op) {
        case wgpu::StencilOperation::Keep:
            return D3D12_STENCIL_OP_KEEP;
        case wgpu::StencilOperation::Zero:
            return D3D12_STENCIL_OP_ZERO;
        case wgpu::StencilOperation::Replace:
            return D3D12_STENCIL_OP_REPLACE;
        case wgpu::StencilOperation::IncrementClamp:
            return D3D12_STENCIL_OP_INCR_SAT;
        case wgpu::StencilOperation::DecrementClamp:
            return D3D12_STENCIL_OP_DECR_SAT;
        case wgpu::StencilOperation::Invert:
            return D3D12_STENCIL_OP_INVERT;
        case wgpu::StencilOperation::IncrementWrap:
            return D3D12_STENCIL_OP_INCR;
        case wgpu::StencilOperation::DecrementWrap:
            return D3D12_STENCIL_OP_DECR;
    }
}

D3D12_DEPTH_STENCILOP_DESC StencilOpDesc(const StencilFaceState& descriptor) {
    D3D12_DEPTH_STENCILOP_DESC desc = {};

    desc.StencilFailOp = StencilOp(descriptor.failOp);
    desc.StencilDepthFailOp = StencilOp(descriptor.depthFailOp);
    desc.StencilPassOp = StencilOp(descriptor.passOp);
    desc.StencilFunc = ToD3D12ComparisonFunc(descriptor.compare);

    return desc;
}

D3D12_DEPTH_STENCIL_DESC ComputeDepthStencilDesc(const DepthStencilState* descriptor) {
    D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
    depthStencilDescriptor.DepthEnable =
        (descriptor->depthCompare == wgpu::CompareFunction::Always &&
         !descriptor->depthWriteEnabled)
            ? FALSE
            : TRUE;
    depthStencilDescriptor.DepthWriteMask =
        descriptor->depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDescriptor.DepthFunc = ToD3D12ComparisonFunc(descriptor->depthCompare);

    depthStencilDescriptor.StencilEnable = StencilTestEnabled(descriptor) ? TRUE : FALSE;
    depthStencilDescriptor.StencilReadMask = static_cast<UINT8>(descriptor->stencilReadMask);
    depthStencilDescriptor.StencilWriteMask = static_cast<UINT8>(descriptor->stencilWriteMask);

    depthStencilDescriptor.FrontFace = StencilOpDesc(descriptor->stencilFront);
    depthStencilDescriptor.BackFace = StencilOpDesc(descriptor->stencilBack);
    return depthStencilDescriptor;
}

D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ComputeIndexBufferStripCutValue(
    wgpu::PrimitiveTopology primitiveTopology,
    wgpu::IndexFormat indexFormat) {
    if (primitiveTopology != wgpu::PrimitiveTopology::TriangleStrip &&
        primitiveTopology != wgpu::PrimitiveTopology::LineStrip) {
        return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    }

    switch (indexFormat) {
        case wgpu::IndexFormat::Uint16:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        case wgpu::IndexFormat::Uint32:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
        case wgpu::IndexFormat::Undefined:
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    }
}

}  // anonymous namespace

Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const RenderPipelineDescriptor* descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

MaybeError RenderPipeline::Initialize() {
    Device* device = ToBackend(GetDevice());
    uint32_t compileFlags = 0;

    if (!device->IsToggleEnabled(Toggle::UseDXC) &&
        !device->IsToggleEnabled(Toggle::FxcOptimizations)) {
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    }

    if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    // SPRIV-cross does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    // FXC can miscompile code that depends on special float values (NaN, INF, etc) when IEEE
    // strictness is not enabled. See crbug.com/tint/976.
    compileFlags |= D3DCOMPILE_IEEE_STRICTNESS;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC descriptorD3D12 = {};

    PerStage<D3D12_SHADER_BYTECODE*> shaders;
    shaders[SingleShaderStage::Vertex] = &descriptorD3D12.VS;
    shaders[SingleShaderStage::Fragment] = &descriptorD3D12.PS;

    PerStage<CompiledShader> compiledShader;

    std::bitset<kMaxInterStageShaderVariables>* usedInterstageVariables = nullptr;
    dawn::native::EntryPointMetadata fragmentEntryPoint;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        // Now that only fragment shader can have interstage inputs.
        const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Fragment);
        fragmentEntryPoint = programmableStage.module->GetEntryPoint(programmableStage.entryPoint);
        usedInterstageVariables = &fragmentEntryPoint.usedInterStageVariables;
    }

    for (auto stage : IterateStages(GetStageMask())) {
        const ProgrammableStage& programmableStage = GetStage(stage);
        DAWN_TRY_ASSIGN(compiledShader[stage],
                        ToBackend(programmableStage.module)
                            ->Compile(programmableStage, stage, ToBackend(GetLayout()),
                                      compileFlags, usedInterstageVariables));
        *shaders[stage] = compiledShader[stage].GetD3D12ShaderBytecode();
    }

    mUsesVertexOrInstanceIndex =
        compiledShader[SingleShaderStage::Vertex].usesVertexOrInstanceIndex;

    PipelineLayout* layout = ToBackend(GetLayout());

    descriptorD3D12.pRootSignature = layout->GetRootSignature();

    // D3D12 logs warnings if any empty input state is used
    std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes> inputElementDescriptors;
    if (GetAttributeLocationsUsed().any()) {
        descriptorD3D12.InputLayout = ComputeInputLayout(&inputElementDescriptors);
    }

    descriptorD3D12.IBStripCutValue =
        ComputeIndexBufferStripCutValue(GetPrimitiveTopology(), GetStripIndexFormat());

    descriptorD3D12.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    descriptorD3D12.RasterizerState.CullMode = D3D12CullMode(GetCullMode());
    descriptorD3D12.RasterizerState.FrontCounterClockwise =
        (GetFrontFace() == wgpu::FrontFace::CCW) ? TRUE : FALSE;
    descriptorD3D12.RasterizerState.DepthBias = GetDepthBias();
    descriptorD3D12.RasterizerState.DepthBiasClamp = GetDepthBiasClamp();
    descriptorD3D12.RasterizerState.SlopeScaledDepthBias = GetDepthBiasSlopeScale();
    descriptorD3D12.RasterizerState.DepthClipEnable = !HasUnclippedDepth();
    descriptorD3D12.RasterizerState.MultisampleEnable = (GetSampleCount() > 1) ? TRUE : FALSE;
    descriptorD3D12.RasterizerState.AntialiasedLineEnable = FALSE;
    descriptorD3D12.RasterizerState.ForcedSampleCount = 0;
    descriptorD3D12.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    if (HasDepthStencilAttachment()) {
        descriptorD3D12.DSVFormat = D3D12TextureFormat(GetDepthStencilFormat());
    }

    static_assert(kMaxColorAttachments == 8);
    for (uint8_t i = 0; i < kMaxColorAttachments; i++) {
        descriptorD3D12.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        descriptorD3D12.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
    }
    ColorAttachmentIndex highestColorAttachmentIndexPlusOne =
        GetHighestBitIndexPlusOne(GetColorAttachmentsMask());
    for (ColorAttachmentIndex i : IterateBitSet(GetColorAttachmentsMask())) {
        descriptorD3D12.RTVFormats[static_cast<uint8_t>(i)] =
            D3D12TextureFormat(GetColorAttachmentFormat(i));
        descriptorD3D12.BlendState.RenderTarget[static_cast<uint8_t>(i)] =
            ComputeColorDesc(device, GetColorTargetState(i));
    }
    ASSERT(highestColorAttachmentIndexPlusOne <= kMaxColorAttachmentsTyped);
    descriptorD3D12.NumRenderTargets = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);

    descriptorD3D12.BlendState.AlphaToCoverageEnable = IsAlphaToCoverageEnabled();
    descriptorD3D12.BlendState.IndependentBlendEnable = TRUE;

    descriptorD3D12.DepthStencilState = ComputeDepthStencilDesc(GetDepthStencilState());

    descriptorD3D12.SampleMask = GetSampleMask();
    descriptorD3D12.PrimitiveTopologyType = D3D12PrimitiveTopologyType(GetPrimitiveTopology());
    descriptorD3D12.SampleDesc.Count = GetSampleCount();
    descriptorD3D12.SampleDesc.Quality = 0;

    mD3d12PrimitiveTopology = D3D12PrimitiveTopology(GetPrimitiveTopology());

    StreamIn(&mCacheKey, descriptorD3D12, *layout->GetRootSignatureBlob());

    // Try to see if we have anything in the blob cache.
    Blob blob = device->LoadCachedBlob(GetCacheKey());
    const bool cacheHit = !blob.Empty();
    if (cacheHit) {
        // Cache hits, attach cached blob to descriptor.
        descriptorD3D12.CachedPSO.pCachedBlob = blob.Data();
        descriptorD3D12.CachedPSO.CachedBlobSizeInBytes = blob.Size();
    }

    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->CreateGraphicsPipelineState(
                              &descriptorD3D12, IID_PPV_ARGS(&mPipelineState)),
                          "D3D12 create graphics pipeline state"));

    if (!cacheHit) {
        // Cache misses, need to get pipeline cached blob and store.
        ComPtr<ID3DBlob> d3dBlob;
        DAWN_TRY(CheckHRESULT(GetPipelineState()->GetCachedBlob(&d3dBlob),
                              "D3D12 render pipeline state get cached blob"));
        device->StoreCachedBlob(GetCacheKey(), CreateBlob(std::move(d3dBlob)));
    }

    SetLabelImpl();

    return {};
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::DestroyImpl() {
    RenderPipelineBase::DestroyImpl();
    ToBackend(GetDevice())->ReferenceUntilUnused(mPipelineState);
}

D3D12_PRIMITIVE_TOPOLOGY RenderPipeline::GetD3D12PrimitiveTopology() const {
    return mD3d12PrimitiveTopology;
}

ID3D12PipelineState* RenderPipeline::GetPipelineState() const {
    return mPipelineState.Get();
}

bool RenderPipeline::UsesVertexOrInstanceIndex() const {
    return mUsesVertexOrInstanceIndex;
}

void RenderPipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), GetPipelineState(), "Dawn_RenderPipeline", GetLabel());
}

ComPtr<ID3D12CommandSignature> RenderPipeline::GetDrawIndirectCommandSignature() {
    if (mUsesVertexOrInstanceIndex) {
        return ToBackend(GetLayout())->GetDrawIndirectCommandSignatureWithInstanceVertexOffsets();
    }

    return ToBackend(GetDevice())->GetDrawIndirectSignature();
}

ComPtr<ID3D12CommandSignature> RenderPipeline::GetDrawIndexedIndirectCommandSignature() {
    if (mUsesVertexOrInstanceIndex) {
        return ToBackend(GetLayout())
            ->GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets();
    }

    return ToBackend(GetDevice())->GetDrawIndexedIndirectSignature();
}

D3D12_INPUT_LAYOUT_DESC RenderPipeline::ComputeInputLayout(
    std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes>* inputElementDescriptors) {
    unsigned int count = 0;
    for (VertexAttributeLocation loc : IterateBitSet(GetAttributeLocationsUsed())) {
        D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = (*inputElementDescriptors)[count++];

        const VertexAttributeInfo& attribute = GetAttribute(loc);

        // If the HLSL semantic is TEXCOORDN the SemanticName should be "TEXCOORD" and the
        // SemanticIndex N
        inputElementDescriptor.SemanticName = "TEXCOORD";
        inputElementDescriptor.SemanticIndex = static_cast<uint8_t>(loc);
        inputElementDescriptor.Format = VertexFormatType(attribute.format);
        inputElementDescriptor.InputSlot = static_cast<uint8_t>(attribute.vertexBufferSlot);

        const VertexBufferInfo& input = GetVertexBuffer(attribute.vertexBufferSlot);

        inputElementDescriptor.AlignedByteOffset = attribute.offset;
        inputElementDescriptor.InputSlotClass = VertexStepModeFunction(input.stepMode);
        if (inputElementDescriptor.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA) {
            inputElementDescriptor.InstanceDataStepRate = 0;
        } else {
            inputElementDescriptor.InstanceDataStepRate = 1;
        }
    }

    D3D12_INPUT_LAYOUT_DESC inputLayoutDescriptor;
    inputLayoutDescriptor.pInputElementDescs = &(*inputElementDescriptors)[0];
    inputLayoutDescriptor.NumElements = count;
    return inputLayoutDescriptor;
}

void RenderPipeline::InitializeAsync(Ref<RenderPipelineBase> renderPipeline,
                                     WGPUCreateRenderPipelineAsyncCallback callback,
                                     void* userdata) {
    std::unique_ptr<CreateRenderPipelineAsyncTask> asyncTask =
        std::make_unique<CreateRenderPipelineAsyncTask>(std::move(renderPipeline), callback,
                                                        userdata);
    CreateRenderPipelineAsyncTask::RunAsync(std::move(asyncTask));
}

}  // namespace dawn::native::d3d12
