// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/RenderPipeline.h"

#include <algorithm>
#include <cmath>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

static constexpr std::array<VertexFormatInfo, 32> sVertexFormatTable = {{
    //
    {wgpu::VertexFormat::Undefined, 0, 0, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Uint8x2, 2, 2, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint8x4, 4, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint8x2, 2, 2, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint8x4, 4, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Unorm8x2, 2, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Unorm8x4, 4, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm8x2, 2, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm8x4, 4, 4, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Uint16x2, 4, 2, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint16x4, 8, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint16x2, 4, 2, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint16x4, 8, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Unorm16x2, 4, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Unorm16x4, 8, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm16x2, 4, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm16x4, 8, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float16x2, 4, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float16x4, 8, 4, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Float32, 4, 1, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x2, 8, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x3, 12, 3, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x4, 16, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Uint32, 4, 1, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x2, 8, 2, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x3, 12, 3, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x4, 16, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint32, 4, 1, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x2, 8, 2, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x3, 12, 3, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x4, 16, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Unorm10_10_10_2, 4, 4, VertexFormatBaseType::Float},
    //
}};

const VertexFormatInfo& GetVertexFormatInfo(wgpu::VertexFormat format) {
    DAWN_ASSERT(format != wgpu::VertexFormat::Undefined);
    DAWN_ASSERT(static_cast<uint32_t>(format) < sVertexFormatTable.size());
    DAWN_ASSERT(sVertexFormatTable[static_cast<uint32_t>(format)].format == format);
    return sVertexFormatTable[static_cast<uint32_t>(format)];
}

// Helper functions
namespace {
MaybeError ValidateVertexAttribute(
    DeviceBase* device,
    const VertexAttribute* attribute,
    const EntryPointMetadata& metadata,
    uint64_t vertexBufferStride,
    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>* attributesSetMask) {
    DAWN_TRY(ValidateVertexFormat(attribute->format));
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(attribute->format);

    DAWN_INVALID_IF(
        attribute->shaderLocation >= kMaxVertexAttributes,
        "Attribute shader location (%u) exceeds the maximum number of vertex attributes "
        "(%u).",
        attribute->shaderLocation, kMaxVertexAttributes);

    VertexAttributeLocation location(static_cast<uint8_t>(attribute->shaderLocation));

    // No underflow is possible because the max vertex format size is smaller than
    // kMaxVertexBufferArrayStride.
    DAWN_ASSERT(kMaxVertexBufferArrayStride >= formatInfo.byteSize);
    DAWN_INVALID_IF(
        attribute->offset > kMaxVertexBufferArrayStride - formatInfo.byteSize,
        "Attribute offset (%u) with format %s (size: %u) doesn't fit in the maximum vertex "
        "buffer stride (%u).",
        attribute->offset, attribute->format, formatInfo.byteSize, kMaxVertexBufferArrayStride);

    // No overflow is possible because the offset is already validated to be less
    // than kMaxVertexBufferArrayStride.
    DAWN_ASSERT(attribute->offset < kMaxVertexBufferArrayStride);
    DAWN_INVALID_IF(
        vertexBufferStride > 0 && attribute->offset + formatInfo.byteSize > vertexBufferStride,
        "Attribute offset (%u) with format %s (size: %u) doesn't fit in the vertex buffer "
        "stride (%u).",
        attribute->offset, attribute->format, formatInfo.byteSize, vertexBufferStride);

    DAWN_INVALID_IF(attribute->offset % std::min(4u, formatInfo.byteSize) != 0,
                    "Attribute offset (%u) in not a multiple of %u.", attribute->offset,
                    std::min(4u, formatInfo.byteSize));

    DAWN_INVALID_IF(metadata.usedVertexInputs[location] &&
                        formatInfo.baseType != metadata.vertexInputBaseTypes[location],
                    "Attribute base type (%s) does not match the "
                    "shader's base type (%s) in location (%u).",
                    formatInfo.baseType, metadata.vertexInputBaseTypes[location],
                    attribute->shaderLocation);

    DAWN_INVALID_IF((*attributesSetMask)[location],
                    "Attribute shader location (%u) is used more than once.",
                    attribute->shaderLocation);

    attributesSetMask->set(location);
    return {};
}

MaybeError ValidateVertexBufferLayout(
    DeviceBase* device,
    const VertexBufferLayout* buffer,
    const EntryPointMetadata& metadata,
    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>* attributesSetMask) {
    DAWN_TRY(ValidateVertexStepMode(buffer->stepMode));
    DAWN_INVALID_IF(buffer->arrayStride > kMaxVertexBufferArrayStride,
                    "Vertex buffer arrayStride (%u) is larger than the maximum array stride (%u).",
                    buffer->arrayStride, kMaxVertexBufferArrayStride);

    DAWN_INVALID_IF(buffer->arrayStride % 4 != 0,
                    "Vertex buffer arrayStride (%u) is not a multiple of 4.", buffer->arrayStride);

    DAWN_INVALID_IF(
        buffer->stepMode == wgpu::VertexStepMode::VertexBufferNotUsed && buffer->attributeCount > 0,
        "attributeCount (%u) is not zero although vertex buffer stepMode is %s.",
        buffer->attributeCount, wgpu::VertexStepMode::VertexBufferNotUsed);

    for (uint32_t i = 0; i < buffer->attributeCount; ++i) {
        DAWN_TRY_CONTEXT(ValidateVertexAttribute(device, &buffer->attributes[i], metadata,
                                                 buffer->arrayStride, attributesSetMask),
                         "validating attributes[%u].", i);
    }

    return {};
}

MaybeError ValidateVertexState(DeviceBase* device,
                               const VertexState* descriptor,
                               const PipelineLayoutBase* layout,
                               wgpu::PrimitiveTopology primitiveTopology) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    const CombinedLimits& limits = device->GetLimits();

    DAWN_INVALID_IF(descriptor->bufferCount > limits.v1.maxVertexBuffers,
                    "Vertex buffer count (%u) exceeds the maximum number of vertex buffers (%u).",
                    descriptor->bufferCount, limits.v1.maxVertexBuffers);

    DAWN_TRY_CONTEXT(ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                               descriptor->constantCount, descriptor->constants,
                                               layout, SingleShaderStage::Vertex),
                     "validating vertex stage (%s, entryPoint: %s).", descriptor->module,
                     descriptor->entryPoint);
    const EntryPointMetadata& vertexMetadata =
        descriptor->module->GetEntryPoint(descriptor->entryPoint);
    if (primitiveTopology == wgpu::PrimitiveTopology::PointList) {
        DAWN_INVALID_IF(
            vertexMetadata.totalInterStageShaderComponents + 1 >
                limits.v1.maxInterStageShaderComponents,
            "Total vertex output components count (%u) exceeds the maximum (%u) when primitive "
            "topology is %s as another component is implicitly used for the point size.",
            vertexMetadata.totalInterStageShaderComponents,
            limits.v1.maxInterStageShaderComponents - 1, primitiveTopology);
    }

    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> attributesSetMask;
    uint32_t totalAttributesNum = 0;
    for (uint32_t i = 0; i < descriptor->bufferCount; ++i) {
        DAWN_TRY_CONTEXT(ValidateVertexBufferLayout(device, &descriptor->buffers[i], vertexMetadata,
                                                    &attributesSetMask),
                         "validating buffers[%u].", i);
        totalAttributesNum += descriptor->buffers[i].attributeCount;
    }

    if (device->IsCompatibilityMode() &&
        (vertexMetadata.usesVertexIndex || vertexMetadata.usesInstanceIndex)) {
        uint32_t totalEffectiveAttributesNum = totalAttributesNum +
                                               (vertexMetadata.usesVertexIndex ? 1 : 0) +
                                               (vertexMetadata.usesInstanceIndex ? 1 : 0);
        DAWN_INVALID_IF(totalEffectiveAttributesNum > limits.v1.maxVertexAttributes,
                        "Attribute count (%u) exceeds the maximum number of attributes (%u) as "
                        "@builtin(vertex_index) and @builtin(instance_index) each use an attribute "
                        "in compatibility mode.",
                        totalEffectiveAttributesNum, limits.v1.maxVertexAttributes);
    }

    // Every vertex attribute has a member called shaderLocation, and there are some
    // requirements for shaderLocation: 1) >=0, 2) values are different across different
    // attributes, 3) can't exceed kMaxVertexAttributes. So it can ensure that total
    // attribute number never exceed kMaxVertexAttributes.
    DAWN_ASSERT(totalAttributesNum <= kMaxVertexAttributes);

    // Validate that attributes used by the VertexState are in the shader using bitmask operations
    // but try to be helpful by finding one missing attribute to surface in the error message
    if (!IsSubset(vertexMetadata.usedVertexInputs, attributesSetMask)) {
        const ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> missingAttributes =
            vertexMetadata.usedVertexInputs & ~attributesSetMask;
        DAWN_ASSERT(missingAttributes.any());

        VertexAttributeLocation firstMissing = ityp::Sub(
            GetHighestBitIndexPlusOne(missingAttributes), VertexAttributeLocation(uint8_t(1)));
        return DAWN_VALIDATION_ERROR(
            "Vertex attribute slot %u used in (%s, entryPoint: %s) is not present in the "
            "VertexState.",
            uint8_t(firstMissing), descriptor->module, descriptor->entryPoint);
    }

    return {};
}

MaybeError ValidatePrimitiveState(const DeviceBase* device, const PrimitiveState* descriptor) {
    DAWN_TRY(ValidateSingleSType(descriptor->nextInChain, wgpu::SType::PrimitiveDepthClipControl));
    const PrimitiveDepthClipControl* depthClipControl = nullptr;
    FindInChain(descriptor->nextInChain, &depthClipControl);
    DAWN_INVALID_IF(depthClipControl && !device->HasFeature(Feature::DepthClipControl),
                    "%s is not supported", wgpu::FeatureName::DepthClipControl);
    DAWN_TRY(ValidatePrimitiveTopology(descriptor->topology));
    DAWN_TRY(ValidateIndexFormat(descriptor->stripIndexFormat));
    DAWN_TRY(ValidateFrontFace(descriptor->frontFace));
    DAWN_TRY(ValidateCullMode(descriptor->cullMode));

    // Pipeline descriptors must have stripIndexFormat == undefined if they are using
    // non-strip topologies.
    if (!IsStripPrimitiveTopology(descriptor->topology)) {
        DAWN_INVALID_IF(descriptor->stripIndexFormat != wgpu::IndexFormat::Undefined,
                        "StripIndexFormat (%s) is not undefined when using a non-strip primitive "
                        "topology (%s).",
                        descriptor->stripIndexFormat, descriptor->topology);
    }

    return {};
}

MaybeError ValidateDepthStencilState(const DeviceBase* device,
                                     const DepthStencilState* descriptor) {
    if (descriptor->depthCompare != wgpu::CompareFunction::Undefined) {
        DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->depthCompare),
                         "validating depth compare function");
    }
    DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->stencilFront.compare),
                     "validating stencil front compare function");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.failOp),
                     "validating stencil front fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.depthFailOp),
                     "validating stencil front depth fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilFront.passOp),
                     "validating stencil front pass operation");
    DAWN_TRY_CONTEXT(ValidateCompareFunction(descriptor->stencilBack.compare),
                     "validating stencil back compare function");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.failOp),
                     "validating stencil back fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.depthFailOp),
                     "validating stencil back depth fail operation");
    DAWN_TRY_CONTEXT(ValidateStencilOperation(descriptor->stencilBack.passOp),
                     "validating stencil back pass operation");

    const Format* format;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));
    DAWN_INVALID_IF(!format->HasDepthOrStencil() || !format->isRenderable,
                    "Depth stencil format (%s) is not depth-stencil renderable.",
                    descriptor->format);

    DAWN_INVALID_IF(
        std::isnan(descriptor->depthBiasSlopeScale) || std::isnan(descriptor->depthBiasClamp),
        "Either depthBiasSlopeScale (%f) or depthBiasClamp (%f) is NaN.",
        descriptor->depthBiasSlopeScale, descriptor->depthBiasClamp);

    DAWN_INVALID_IF(
        format->HasDepth() && descriptor->depthCompare == wgpu::CompareFunction::Undefined &&
            (descriptor->depthWriteEnabled ||
             descriptor->stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
             descriptor->stencilBack.depthFailOp != wgpu::StencilOperation::Keep),
        "Depth stencil format (%s) has a depth aspect and depthCompare is %s while it's actually "
        "used by depthWriteEnabled (%u), or stencil front depth fail operation (%s), or "
        "stencil back depth fail operation (%s).",
        descriptor->format, wgpu::CompareFunction::Undefined, descriptor->depthWriteEnabled,
        descriptor->stencilFront.depthFailOp, descriptor->stencilBack.depthFailOp);

    UnpackedDepthStencilStateChain unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpackChain(descriptor));
    if (const auto* depthWriteDefined =
            std::get<const DepthStencilStateDepthWriteDefinedDawn*>(unpacked)) {
        DAWN_INVALID_IF(
            format->HasDepth() && !depthWriteDefined->depthWriteDefined,
            "Depth stencil format (%s) has a depth aspect and depthWriteEnabled is undefined.",
            descriptor->format);
    }

    DAWN_INVALID_IF(
        !format->HasDepth() && descriptor->depthCompare != wgpu::CompareFunction::Always &&
            descriptor->depthCompare != wgpu::CompareFunction::Undefined,
        "Depth stencil format (%s) doesn't have depth aspect while depthCompare (%s) is "
        "neither %s nor %s.",
        descriptor->format, descriptor->depthCompare, wgpu::CompareFunction::Always,
        wgpu::CompareFunction::Undefined);

    DAWN_INVALID_IF(
        !format->HasDepth() && descriptor->depthWriteEnabled,
        "Depth stencil format (%s) doesn't have depth aspect while depthWriteEnabled (%u) is true.",
        descriptor->format, descriptor->depthWriteEnabled);

    DAWN_INVALID_IF(!format->HasStencil() && StencilTestEnabled(descriptor),
                    "Depth stencil format (%s) doesn't have stencil aspect while stencil "
                    "test or stencil write is enabled.",
                    descriptor->format);

    return {};
}

MaybeError ValidateMultisampleState(const DeviceBase* device, const MultisampleState* descriptor) {
    const DawnMultisampleStateRenderToSingleSampled* msaaRenderToSingleSampledDesc = nullptr;
    FindInChain(descriptor->nextInChain, &msaaRenderToSingleSampledDesc);
    if (msaaRenderToSingleSampledDesc != nullptr) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::MSAARenderToSingleSampled),
                        "The msaaRenderToSingleSampledDesc is not empty while the "
                        "msaa-render-to-single-sampled feature is not enabled.");

        DAWN_INVALID_IF(descriptor->count <= 1,
                        "The msaaRenderToSingleSampledDesc is not empty while multisample count "
                        "(%u) is not > 1.",
                        descriptor->count);
    }

    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->count),
                    "Multisample count (%u) is not supported.", descriptor->count);

    DAWN_INVALID_IF(descriptor->alphaToCoverageEnabled && descriptor->count <= 1,
                    "Multisample count (%u) must be > 1 when alphaToCoverage is enabled.",
                    descriptor->count);

    return {};
}

MaybeError ValidateBlendComponent(BlendComponent blendComponent, bool dualSourceBlendingEnabled) {
    if (!dualSourceBlendingEnabled) {
        DAWN_INVALID_IF(blendComponent.srcFactor == wgpu::BlendFactor::Src1 ||
                            blendComponent.srcFactor == wgpu::BlendFactor::OneMinusSrc1 ||
                            blendComponent.srcFactor == wgpu::BlendFactor::Src1Alpha ||
                            blendComponent.srcFactor == wgpu::BlendFactor::OneMinusSrc1Alpha,
                        "Source blend factor is %s while dualSourceBlending is not enabled.",
                        blendComponent.srcFactor);

        DAWN_INVALID_IF(blendComponent.dstFactor == wgpu::BlendFactor::Src1 ||
                            blendComponent.dstFactor == wgpu::BlendFactor::OneMinusSrc1 ||
                            blendComponent.dstFactor == wgpu::BlendFactor::Src1Alpha ||
                            blendComponent.dstFactor == wgpu::BlendFactor::OneMinusSrc1Alpha,
                        "Destination blend factor is %s while dualSourceBlending is not enabled.",
                        blendComponent.dstFactor);
    }

    if (blendComponent.operation == wgpu::BlendOperation::Min ||
        blendComponent.operation == wgpu::BlendOperation::Max) {
        DAWN_INVALID_IF(blendComponent.srcFactor != wgpu::BlendFactor::One ||
                            blendComponent.dstFactor != wgpu::BlendFactor::One,
                        "Blend factor is not %s when blend operation is %s.",
                        wgpu::BlendFactor::One, blendComponent.operation);
    }

    return {};
}

MaybeError ValidateBlendState(DeviceBase* device, const BlendState* descriptor) {
    DAWN_TRY(ValidateBlendOperation(descriptor->alpha.operation));
    DAWN_TRY(ValidateBlendFactor(descriptor->alpha.srcFactor));
    DAWN_TRY(ValidateBlendFactor(descriptor->alpha.dstFactor));
    DAWN_TRY(ValidateBlendOperation(descriptor->color.operation));
    DAWN_TRY(ValidateBlendFactor(descriptor->color.srcFactor));
    DAWN_TRY(ValidateBlendFactor(descriptor->color.dstFactor));

    bool dualSourceBlendingEnabled = device->HasFeature(Feature::DualSourceBlending);
    DAWN_TRY(ValidateBlendComponent(descriptor->alpha, dualSourceBlendingEnabled));
    DAWN_TRY(ValidateBlendComponent(descriptor->color, dualSourceBlendingEnabled));

    return {};
}

bool BlendFactorContainsSrcAlpha(const wgpu::BlendFactor& blendFactor) {
    return blendFactor == wgpu::BlendFactor::SrcAlpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrcAlpha ||
           blendFactor == wgpu::BlendFactor::SrcAlphaSaturated;
}

MaybeError ValidateColorTargetState(
    DeviceBase* device,
    const ColorTargetState* descriptor,
    bool fragmentWritten,
    const EntryPointMetadata::FragmentOutputVariableInfo& fragmentOutputVariable) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    if (descriptor->blend) {
        DAWN_TRY_CONTEXT(ValidateBlendState(device, descriptor->blend), "validating blend state.");
    }

    DAWN_TRY(ValidateColorWriteMask(descriptor->writeMask));

    const Format* format;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));
    DAWN_INVALID_IF(!format->IsColor() || !format->isRenderable,
                    "Color format (%s) is not color renderable.", descriptor->format);

    DAWN_INVALID_IF(
        descriptor->blend &&
            !(format->GetAspectInfo(Aspect::Color).supportedSampleTypes & SampleTypeBit::Float),
        "Blending is enabled but color format (%s) is not blendable.", descriptor->format);

    if (fragmentWritten) {
        DAWN_INVALID_IF(
            fragmentOutputVariable.baseType != format->GetAspectInfo(Aspect::Color).baseType,
            "Color format (%s) base type (%s) doesn't match the fragment "
            "module output type (%s).",
            descriptor->format, format->GetAspectInfo(Aspect::Color).baseType,
            fragmentOutputVariable.baseType);

        DAWN_INVALID_IF(fragmentOutputVariable.componentCount < format->componentCount,
                        "The fragment stage has fewer output components (%u) than the color format "
                        "(%s) component count (%u).",
                        fragmentOutputVariable.componentCount, descriptor->format,
                        format->componentCount);

        if (descriptor->blend) {
            if (fragmentOutputVariable.componentCount < 4u) {
                // No alpha channel output
                // Make sure there's no alpha involved in the blending operation
                DAWN_INVALID_IF(BlendFactorContainsSrcAlpha(descriptor->blend->color.srcFactor) ||
                                    BlendFactorContainsSrcAlpha(descriptor->blend->color.dstFactor),
                                "Color blending srcfactor (%s) or dstFactor (%s) is reading alpha "
                                "but it is missing from fragment output.",
                                descriptor->blend->color.srcFactor,
                                descriptor->blend->color.dstFactor);
            }
        }
    } else {
        DAWN_INVALID_IF(
            descriptor->writeMask != wgpu::ColorWriteMask::None,
            "Color target has no corresponding fragment stage output but writeMask (%s) is "
            "not zero.",
            descriptor->writeMask);
    }

    return {};
}

MaybeError ValidateCompatibilityColorTargetState(
    const uint8_t firstColorTargetIndex,
    const ColorTargetState* const firstColorTargetState,
    const uint8_t targetIndex,
    const ColorTargetState* target) {
    DAWN_INVALID_IF(firstColorTargetState->writeMask != target->writeMask,
                    "targets[%u].writeMask (%s) does not match targets[%u].writeMask (%s).",
                    targetIndex, target->writeMask, firstColorTargetIndex,
                    firstColorTargetState->writeMask);
    if (!firstColorTargetState->blend) {
        DAWN_INVALID_IF(target->blend,
                        "targets[%u].blend has a blend state but targets[%u].blend does not.",
                        targetIndex, firstColorTargetIndex);
    } else {
        DAWN_INVALID_IF(!target->blend,
                        "targets[%u].blend has a blend state but targets[%u].blend does not.",
                        firstColorTargetIndex, targetIndex);

        const BlendState& currBlendState = *target->blend;
        const BlendState& firstBlendState = *firstColorTargetState->blend;

        DAWN_INVALID_IF(
            firstBlendState.color.operation != currBlendState.color.operation,
            "targets[%u].color.operation (%s) does not match targets[%u].color.operation (%s).",
            firstColorTargetIndex, firstBlendState.color.operation, targetIndex,
            currBlendState.color.operation);
        DAWN_INVALID_IF(
            firstBlendState.color.srcFactor != currBlendState.color.srcFactor,
            "targets[%u].color.srcFactor (%s) does not match targets[%u].color.srcFactor (%s).",
            firstColorTargetIndex, firstBlendState.color.srcFactor, targetIndex,
            currBlendState.color.srcFactor);
        DAWN_INVALID_IF(
            firstBlendState.color.dstFactor != currBlendState.color.dstFactor,
            "targets[%u].color.dstFactor (%s) does not match targets[%u].color.dstFactor (%s).",
            firstColorTargetIndex, firstBlendState.color.dstFactor, targetIndex,
            currBlendState.color.dstFactor);
        DAWN_INVALID_IF(
            firstBlendState.alpha.operation != currBlendState.alpha.operation,
            "targets[%u].alpha.operation (%s) does not match targets[%u].alpha.operation (%s).",
            firstColorTargetIndex, firstBlendState.alpha.operation, targetIndex,
            currBlendState.alpha.operation);
        DAWN_INVALID_IF(
            firstBlendState.alpha.srcFactor != currBlendState.alpha.srcFactor,
            "targets[%u].alpha.srcFactor (%s) does not match targets[%u].alpha.srcFactor (%s).",
            firstColorTargetIndex, firstBlendState.alpha.srcFactor, targetIndex,
            currBlendState.alpha.srcFactor);
        DAWN_INVALID_IF(
            firstBlendState.alpha.dstFactor != currBlendState.alpha.dstFactor,
            "targets[%u].alpha.dstFactor (%s) does not match targets[%u].alpha.dstFactor (%s).",
            firstColorTargetIndex, firstBlendState.alpha.dstFactor, targetIndex,
            currBlendState.alpha.dstFactor);
    }
    return {};
}

MaybeError ValidateFragmentState(DeviceBase* device,
                                 const FragmentState* descriptor,
                                 const PipelineLayoutBase* layout,
                                 const DepthStencilState* depthStencil,
                                 bool alphaToCoverageEnabled) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY_CONTEXT(ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                               descriptor->constantCount, descriptor->constants,
                                               layout, SingleShaderStage::Fragment),
                     "validating fragment stage (%s, entryPoint: %s).", descriptor->module,
                     descriptor->entryPoint);

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(descriptor->targetCount > maxColorAttachments,
                    "Number of targets (%u) exceeds the maximum (%u).", descriptor->targetCount,
                    maxColorAttachments);

    const EntryPointMetadata& fragmentMetadata =
        descriptor->module->GetEntryPoint(descriptor->entryPoint);

    if (fragmentMetadata.usesFragDepth) {
        DAWN_INVALID_IF(
            depthStencil == nullptr,
            "Depth stencil state is not present when fragment stage (%s, entryPoint: %s) is "
            "writing to frag_depth.",
            descriptor->module, descriptor->entryPoint);
        const Format* depthStencilFormat;
        DAWN_TRY_ASSIGN(depthStencilFormat, device->GetInternalFormat(depthStencil->format));
        DAWN_INVALID_IF(!depthStencilFormat->HasDepth(),
                        "Depth stencil state format (%s) has no depth aspect when fragment stage "
                        "(%s, entryPoint: %s) is "
                        "writing to frag_depth.",
                        depthStencil->format, descriptor->module, descriptor->entryPoint);
    }

    uint8_t firstColorTargetIndex = 0;
    const ColorTargetState* firstColorTargetState = nullptr;
    ColorAttachmentFormats colorAttachmentFormats;

    for (ColorAttachmentIndex attachmentIndex(uint8_t(0));
         attachmentIndex < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->targetCount));
         ++attachmentIndex) {
        const uint8_t i = static_cast<uint8_t>(attachmentIndex);
        const ColorTargetState* target = &descriptor->targets[i];

        if (target->format != wgpu::TextureFormat::Undefined) {
            DAWN_TRY_CONTEXT(
                ValidateColorTargetState(device, target,
                                         fragmentMetadata.fragmentOutputsWritten[attachmentIndex],
                                         fragmentMetadata.fragmentOutputVariables[attachmentIndex]),
                "validating targets[%u].", i);
            colorAttachmentFormats->push_back(&device->GetValidInternalFormat(target->format));
            if (device->IsCompatibilityMode()) {
                if (!firstColorTargetState) {
                    firstColorTargetState = target;
                    firstColorTargetIndex = i;
                } else {
                    DAWN_TRY_CONTEXT(ValidateCompatibilityColorTargetState(
                                         firstColorTargetIndex, firstColorTargetState, i, target),
                                     "validating targets[%u] in compatibility mode.", i);
                }
            }
        } else {
            DAWN_INVALID_IF(target->blend,
                            "Color target[%u] blend state is set when the format is undefined.", i);
        }
    }
    DAWN_TRY(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats));

    if (alphaToCoverageEnabled) {
        DAWN_INVALID_IF(fragmentMetadata.usesSampleMaskOutput,
                        "alphaToCoverageEnabled is true when the sample_mask builtin is a "
                        "pipeline output of fragment stage of %s.",
                        descriptor->module);

        DAWN_INVALID_IF(descriptor->targetCount == 0 ||
                            descriptor->targets[0].format == wgpu::TextureFormat::Undefined,
                        "alphaToCoverageEnabled is true when color target[0] is not present.");

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->targets[0].format));
        DAWN_INVALID_IF(
            !format->HasAlphaChannel(),
            "alphaToCoverageEnabled is true when target[0].format (%s) has no alpha channel.",
            format->format);
    }

    if (device->IsCompatibilityMode()) {
        DAWN_INVALID_IF(
            fragmentMetadata.usesSampleMaskOutput,
            "sample_mask is not supported in compatibility mode in the fragment stage (%s, "
            "entryPoint: %s)",
            descriptor->module, descriptor->entryPoint);
    }

    return {};
}

MaybeError ValidateInterStageMatching(DeviceBase* device,
                                      const VertexState& vertexState,
                                      const FragmentState& fragmentState) {
    const EntryPointMetadata& vertexMetadata =
        vertexState.module->GetEntryPoint(vertexState.entryPoint);
    const EntryPointMetadata& fragmentMetadata =
        fragmentState.module->GetEntryPoint(fragmentState.entryPoint);

    size_t maxInterStageShaderVariables = device->GetLimits().v1.maxInterStageShaderVariables;
    DAWN_ASSERT(vertexMetadata.usedInterStageVariables.size() == maxInterStageShaderVariables);
    DAWN_ASSERT(fragmentMetadata.usedInterStageVariables.size() == maxInterStageShaderVariables);
    for (size_t i = 0; i < maxInterStageShaderVariables; ++i) {
        if (!vertexMetadata.usedInterStageVariables[i]) {
            if (fragmentMetadata.usedInterStageVariables[i]) {
                return DAWN_VALIDATION_ERROR(
                    "The fragment input at location %u doesn't have a corresponding vertex output.",
                    i);
            }
            continue;
        }

        // It is valid that fragment output is a subset of vertex input
        if (!fragmentMetadata.usedInterStageVariables[i]) {
            continue;
        }
        const auto& vertexOutputInfo = vertexMetadata.interStageVariables[i];
        const auto& fragmentInputInfo = fragmentMetadata.interStageVariables[i];
        DAWN_INVALID_IF(
            vertexOutputInfo.baseType != fragmentInputInfo.baseType,
            "The base type (%s) of the vertex output at location %u is different from the "
            "base type (%s) of the fragment input at location %u.",
            vertexOutputInfo.baseType, i, fragmentInputInfo.baseType, i);

        DAWN_INVALID_IF(vertexOutputInfo.componentCount != fragmentInputInfo.componentCount,
                        "The component count (%u) of the vertex output at location %u is different "
                        "from the component count (%u) of the fragment input at location %u.",
                        vertexOutputInfo.componentCount, i, fragmentInputInfo.componentCount, i);

        DAWN_INVALID_IF(
            vertexOutputInfo.interpolationType != fragmentInputInfo.interpolationType,
            "The interpolation type (%s) of the vertex output at location %u is different "
            "from the interpolation type (%s) of the fragment input at location %u.",
            vertexOutputInfo.interpolationType, i, fragmentInputInfo.interpolationType, i);

        DAWN_INVALID_IF(
            vertexOutputInfo.interpolationSampling != fragmentInputInfo.interpolationSampling,
            "The interpolation sampling (%s) of the vertex output at location %u is "
            "different from the interpolation sampling (%s) of the fragment input at "
            "location %u.",
            vertexOutputInfo.interpolationSampling, i, fragmentInputInfo.interpolationSampling, i);
    }

    return {};
}
}  // anonymous namespace

// Helper functions
size_t IndexFormatSize(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return sizeof(uint16_t);
        case wgpu::IndexFormat::Uint32:
            return sizeof(uint32_t);
        case wgpu::IndexFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    return primitiveTopology == wgpu::PrimitiveTopology::LineStrip ||
           primitiveTopology == wgpu::PrimitiveTopology::TriangleStrip;
}

MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                            const RenderPipelineDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    if (descriptor->layout != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->layout));
    }

    DAWN_TRY_CONTEXT(ValidateVertexState(device, &descriptor->vertex, descriptor->layout,
                                         descriptor->primitive.topology),
                     "validating vertex state.");

    DAWN_TRY_CONTEXT(ValidatePrimitiveState(device, &descriptor->primitive),
                     "validating primitive state.");

    if (descriptor->depthStencil) {
        DAWN_TRY_CONTEXT(ValidateDepthStencilState(device, descriptor->depthStencil),
                         "validating depthStencil state.");
    }

    DAWN_TRY_CONTEXT(ValidateMultisampleState(device, &descriptor->multisample),
                     "validating multisample state.");

    DAWN_INVALID_IF(
        descriptor->multisample.alphaToCoverageEnabled && descriptor->fragment == nullptr,
        "alphaToCoverageEnabled is true when fragment state is not present.");

    if (descriptor->fragment != nullptr) {
        DAWN_TRY_CONTEXT(ValidateFragmentState(device, descriptor->fragment, descriptor->layout,
                                               descriptor->depthStencil,
                                               descriptor->multisample.alphaToCoverageEnabled),
                         "validating fragment state.");

        bool hasStorageAttachments =
            descriptor->layout != nullptr && descriptor->layout->HasAnyStorageAttachments();
        DAWN_INVALID_IF(descriptor->fragment->targetCount == 0 && !descriptor->depthStencil &&
                            !hasStorageAttachments,
                        "No attachment was specified (color, depth-stencil or other).");

        DAWN_TRY(ValidateInterStageMatching(device, descriptor->vertex, *(descriptor->fragment)));
    }

    return {};
}

std::vector<StageAndDescriptor> GetRenderStagesAndSetPlaceholderShader(
    DeviceBase* device,
    const RenderPipelineDescriptor* descriptor) {
    std::vector<StageAndDescriptor> stages;
    stages.push_back({SingleShaderStage::Vertex, descriptor->vertex.module,
                      descriptor->vertex.entryPoint, descriptor->vertex.constantCount,
                      descriptor->vertex.constants});
    if (descriptor->fragment != nullptr) {
        stages.push_back({SingleShaderStage::Fragment, descriptor->fragment->module,
                          descriptor->fragment->entryPoint, descriptor->fragment->constantCount,
                          descriptor->fragment->constants});
    } else if (device->IsToggleEnabled(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline)) {
        InternalPipelineStore* store = device->GetInternalPipelineStore();
        // The placeholder fragment shader module should already be initialized
        DAWN_ASSERT(store->placeholderFragmentShader != nullptr);
        ShaderModuleBase* placeholderFragmentShader = store->placeholderFragmentShader.Get();
        stages.push_back(
            {SingleShaderStage::Fragment, placeholderFragmentShader, "fs_empty_main", 0, nullptr});
    }
    return stages;
}

bool StencilTestEnabled(const DepthStencilState* depthStencil) {
    return depthStencil->stencilBack.compare != wgpu::CompareFunction::Always ||
           depthStencil->stencilBack.failOp != wgpu::StencilOperation::Keep ||
           depthStencil->stencilBack.depthFailOp != wgpu::StencilOperation::Keep ||
           depthStencil->stencilBack.passOp != wgpu::StencilOperation::Keep ||
           depthStencil->stencilFront.compare != wgpu::CompareFunction::Always ||
           depthStencil->stencilFront.failOp != wgpu::StencilOperation::Keep ||
           depthStencil->stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
           depthStencil->stencilFront.passOp != wgpu::StencilOperation::Keep;
}

// RenderPipelineBase

RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                       const RenderPipelineDescriptor* descriptor)
    : PipelineBase(device,
                   descriptor->layout,
                   descriptor->label,
                   GetRenderStagesAndSetPlaceholderShader(device, descriptor)),
      mAttachmentState(device->GetOrCreateAttachmentState(descriptor, GetLayout())) {
    mVertexBufferCount = descriptor->vertex.bufferCount;
    const VertexBufferLayout* buffers = descriptor->vertex.buffers;
    for (uint8_t slot = 0; slot < mVertexBufferCount; ++slot) {
        // Skip unused slots
        if (buffers[slot].stepMode == wgpu::VertexStepMode::VertexBufferNotUsed) {
            continue;
        }

        VertexBufferSlot typedSlot(slot);

        mVertexBufferSlotsUsed.set(typedSlot);
        mVertexBufferInfos[typedSlot].arrayStride = buffers[slot].arrayStride;
        mVertexBufferInfos[typedSlot].stepMode = buffers[slot].stepMode;
        mVertexBufferInfos[typedSlot].usedBytesInStride = 0;
        mVertexBufferInfos[typedSlot].lastStride = 0;
        switch (buffers[slot].stepMode) {
            case wgpu::VertexStepMode::Vertex:
                mVertexBufferSlotsUsedAsVertexBuffer.set(typedSlot);
                break;
            case wgpu::VertexStepMode::Instance:
                mVertexBufferSlotsUsedAsInstanceBuffer.set(typedSlot);
                break;
            default:
                DAWN_UNREACHABLE();
        }

        for (uint32_t i = 0; i < buffers[slot].attributeCount; ++i) {
            VertexAttributeLocation location = VertexAttributeLocation(
                static_cast<uint8_t>(buffers[slot].attributes[i].shaderLocation));
            mAttributeLocationsUsed.set(location);
            mAttributeInfos[location].shaderLocation = location;
            mAttributeInfos[location].vertexBufferSlot = typedSlot;
            mAttributeInfos[location].offset = buffers[slot].attributes[i].offset;
            mAttributeInfos[location].format = buffers[slot].attributes[i].format;
            // Compute the access boundary of this attribute by adding attribute format size to
            // attribute offset. Although offset is in uint64_t, such sum must be no larger than
            // maxVertexBufferArrayStride (2048), which is promised by the GPUVertexBufferLayout
            // validation of creating render pipeline. Therefore, calculating in uint16_t will
            // cause no overflow.
            uint32_t formatByteSize =
                GetVertexFormatInfo(buffers[slot].attributes[i].format).byteSize;
            DAWN_ASSERT(buffers[slot].attributes[i].offset <= 2048);
            uint16_t accessBoundary =
                uint16_t(buffers[slot].attributes[i].offset) + uint16_t(formatByteSize);
            mVertexBufferInfos[typedSlot].usedBytesInStride =
                std::max(mVertexBufferInfos[typedSlot].usedBytesInStride, accessBoundary);
            mVertexBufferInfos[typedSlot].lastStride =
                std::max(mVertexBufferInfos[typedSlot].lastStride,
                         mAttributeInfos[location].offset + formatByteSize);
        }
    }

    mPrimitive = descriptor->primitive;
    const PrimitiveDepthClipControl* depthClipControl = nullptr;
    FindInChain(mPrimitive.nextInChain, &depthClipControl);
    if (depthClipControl) {
        mUnclippedDepth = depthClipControl->unclippedDepth;
    }

    mMultisample = descriptor->multisample;

    if (mAttachmentState->HasDepthStencilAttachment()) {
        mDepthStencil = *descriptor->depthStencil;
        // Reify depth option for stencil-only formats
        const Format& format = device->GetValidInternalFormat(mDepthStencil.format);
        if (!format.HasDepth()) {
            mDepthStencil.depthWriteEnabled = false;
            mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        }
        if (format.HasDepth() && mDepthStencil.depthCompare == wgpu::CompareFunction::Undefined &&
            !mDepthStencil.depthWriteEnabled &&
            mDepthStencil.stencilFront.depthFailOp == wgpu::StencilOperation::Keep &&
            mDepthStencil.stencilBack.depthFailOp == wgpu::StencilOperation::Keep) {
            mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        }
        mWritesDepth = mDepthStencil.depthWriteEnabled;
        if (mDepthStencil.stencilWriteMask) {
            if ((mPrimitive.cullMode != wgpu::CullMode::Front &&
                 (mDepthStencil.stencilFront.failOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilFront.passOp != wgpu::StencilOperation::Keep)) ||
                (mPrimitive.cullMode != wgpu::CullMode::Back &&
                 (mDepthStencil.stencilBack.failOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilBack.depthFailOp != wgpu::StencilOperation::Keep ||
                  mDepthStencil.stencilBack.passOp != wgpu::StencilOperation::Keep))) {
                mWritesStencil = true;
            }
        }
    } else {
        // These default values below are useful for backends to fill information.
        // The values indicate that depth and stencil test are disabled when backends
        // set their own depth stencil states/descriptors according to the values in
        // mDepthStencil.
        mDepthStencil.format = wgpu::TextureFormat::Undefined;
        mDepthStencil.depthWriteEnabled = false;
        mDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        mDepthStencil.stencilBack.compare = wgpu::CompareFunction::Always;
        mDepthStencil.stencilBack.failOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilBack.passOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilFront.compare = wgpu::CompareFunction::Always;
        mDepthStencil.stencilFront.failOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilFront.passOp = wgpu::StencilOperation::Keep;
        mDepthStencil.stencilReadMask = 0xff;
        mDepthStencil.stencilWriteMask = 0xff;
        mDepthStencil.depthBias = 0;
        mDepthStencil.depthBiasSlopeScale = 0.0f;
        mDepthStencil.depthBiasClamp = 0.0f;
    }

    for (ColorAttachmentIndex i : IterateBitSet(mAttachmentState->GetColorAttachmentsMask())) {
        // Vertex-only render pipeline have no color attachment. For a render pipeline with
        // color attachments, there must be a valid FragmentState.
        DAWN_ASSERT(descriptor->fragment != nullptr);
        const ColorTargetState* target = &descriptor->fragment->targets[static_cast<uint8_t>(i)];
        mTargets[i] = *target;

        if (target->blend != nullptr) {
            mTargetBlend[i] = *target->blend;
            mTargets[i].blend = &mTargetBlend[i];
        }
    }

    if (HasStage(SingleShaderStage::Fragment)) {
        mUsesFragDepth = GetStage(SingleShaderStage::Fragment).metadata->usesFragDepth;
    }

    SetContentHash(ComputeContentHash());
    GetObjectTrackingList()->Track(this);

    // Initialize the cache key to include the cache type and device information.
    StreamIn(&mCacheKey, CacheKey::Type::RenderPipeline, device->GetCacheKey());
}

RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                       ObjectBase::ErrorTag tag,
                                       const char* label)
    : PipelineBase(device, tag, label) {}

RenderPipelineBase::~RenderPipelineBase() = default;

void RenderPipelineBase::DestroyImpl() {
    Uncache();

    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

// static
RenderPipelineBase* RenderPipelineBase::MakeError(DeviceBase* device, const char* label) {
    class ErrorRenderPipeline final : public RenderPipelineBase {
      public:
        explicit ErrorRenderPipeline(DeviceBase* device, const char* label)
            : RenderPipelineBase(device, ObjectBase::kError, label) {}

        MaybeError Initialize() override {
            DAWN_UNREACHABLE();
            return {};
        }
    };

    return new ErrorRenderPipeline(device, label);
}

ObjectType RenderPipelineBase::GetType() const {
    return ObjectType::RenderPipeline;
}

const ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>&
RenderPipelineBase::GetAttributeLocationsUsed() const {
    DAWN_ASSERT(!IsError());
    return mAttributeLocationsUsed;
}

const VertexAttributeInfo& RenderPipelineBase::GetAttribute(
    VertexAttributeLocation location) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mAttributeLocationsUsed[location]);
    return mAttributeInfos[location];
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsed() const {
    DAWN_ASSERT(!IsError());
    return mVertexBufferSlotsUsed;
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsedAsVertexBuffer() const {
    DAWN_ASSERT(!IsError());
    return mVertexBufferSlotsUsedAsVertexBuffer;
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsedAsInstanceBuffer() const {
    DAWN_ASSERT(!IsError());
    return mVertexBufferSlotsUsedAsInstanceBuffer;
}

const VertexBufferInfo& RenderPipelineBase::GetVertexBuffer(VertexBufferSlot slot) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mVertexBufferSlotsUsed[slot]);
    return mVertexBufferInfos[slot];
}

uint32_t RenderPipelineBase::GetVertexBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mVertexBufferCount;
}

const ColorTargetState* RenderPipelineBase::GetColorTargetState(
    ColorAttachmentIndex attachmentSlot) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(attachmentSlot < mTargets.size());
    return &mTargets[attachmentSlot];
}

const DepthStencilState* RenderPipelineBase::GetDepthStencilState() const {
    DAWN_ASSERT(!IsError());
    return &mDepthStencil;
}

wgpu::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.topology;
}

wgpu::IndexFormat RenderPipelineBase::GetStripIndexFormat() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.stripIndexFormat;
}

wgpu::CullMode RenderPipelineBase::GetCullMode() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.cullMode;
}

wgpu::FrontFace RenderPipelineBase::GetFrontFace() const {
    DAWN_ASSERT(!IsError());
    return mPrimitive.frontFace;
}

bool RenderPipelineBase::IsDepthBiasEnabled() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBias != 0 || mDepthStencil.depthBiasSlopeScale != 0;
}

int32_t RenderPipelineBase::GetDepthBias() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBias;
}

float RenderPipelineBase::GetDepthBiasSlopeScale() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBiasSlopeScale;
}

float RenderPipelineBase::GetDepthBiasClamp() const {
    DAWN_ASSERT(!IsError());
    return mDepthStencil.depthBiasClamp;
}

bool RenderPipelineBase::HasUnclippedDepth() const {
    DAWN_ASSERT(!IsError());
    return mUnclippedDepth;
}

ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments>
RenderPipelineBase::GetColorAttachmentsMask() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->GetColorAttachmentsMask();
}

bool RenderPipelineBase::HasDepthStencilAttachment() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->HasDepthStencilAttachment();
}

wgpu::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(
    ColorAttachmentIndex attachment) const {
    DAWN_ASSERT(!IsError());
    return mTargets[attachment].format;
}

wgpu::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mAttachmentState->HasDepthStencilAttachment());
    return mDepthStencil.format;
}

uint32_t RenderPipelineBase::GetSampleCount() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState->GetSampleCount();
}

uint32_t RenderPipelineBase::GetSampleMask() const {
    DAWN_ASSERT(!IsError());
    return mMultisample.mask;
}

bool RenderPipelineBase::IsAlphaToCoverageEnabled() const {
    DAWN_ASSERT(!IsError());
    return mMultisample.alphaToCoverageEnabled;
}

const AttachmentState* RenderPipelineBase::GetAttachmentState() const {
    DAWN_ASSERT(!IsError());
    return mAttachmentState.Get();
}

bool RenderPipelineBase::WritesDepth() const {
    DAWN_ASSERT(!IsError());
    return mWritesDepth;
}

bool RenderPipelineBase::WritesStencil() const {
    DAWN_ASSERT(!IsError());
    return mWritesStencil;
}

bool RenderPipelineBase::UsesFragDepth() const {
    DAWN_ASSERT(!IsError());
    return mUsesFragDepth;
}

size_t RenderPipelineBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // Record modules and layout
    recorder.Record(PipelineBase::ComputeContentHash());

    // Hierarchically record the attachment state.
    // It contains the attachments set, texture formats, and sample count.
    recorder.Record(mAttachmentState->GetContentHash());

    // Record attachments
    for (ColorAttachmentIndex i : IterateBitSet(mAttachmentState->GetColorAttachmentsMask())) {
        const ColorTargetState& desc = *GetColorTargetState(i);
        recorder.Record(desc.writeMask);
        if (desc.blend != nullptr) {
            recorder.Record(desc.blend->color.operation, desc.blend->color.srcFactor,
                            desc.blend->color.dstFactor);
            recorder.Record(desc.blend->alpha.operation, desc.blend->alpha.srcFactor,
                            desc.blend->alpha.dstFactor);
        }
    }

    if (mAttachmentState->HasDepthStencilAttachment()) {
        const DepthStencilState& desc = mDepthStencil;
        recorder.Record(desc.depthWriteEnabled, desc.depthCompare);
        recorder.Record(desc.stencilReadMask, desc.stencilWriteMask);
        recorder.Record(desc.stencilFront.compare, desc.stencilFront.failOp,
                        desc.stencilFront.depthFailOp, desc.stencilFront.passOp);
        recorder.Record(desc.stencilBack.compare, desc.stencilBack.failOp,
                        desc.stencilBack.depthFailOp, desc.stencilBack.passOp);
        recorder.Record(desc.depthBias, desc.depthBiasSlopeScale, desc.depthBiasClamp);
    }

    // Record vertex state
    recorder.Record(mAttributeLocationsUsed);
    for (VertexAttributeLocation location : IterateBitSet(mAttributeLocationsUsed)) {
        const VertexAttributeInfo& desc = GetAttribute(location);
        recorder.Record(desc.shaderLocation, desc.vertexBufferSlot, desc.offset, desc.format);
    }

    recorder.Record(mVertexBufferSlotsUsed);
    for (VertexBufferSlot slot : IterateBitSet(mVertexBufferSlotsUsed)) {
        const VertexBufferInfo& desc = GetVertexBuffer(slot);
        recorder.Record(desc.arrayStride, desc.stepMode);
    }

    // Record primitive state
    recorder.Record(mPrimitive.topology, mPrimitive.stripIndexFormat, mPrimitive.frontFace,
                    mPrimitive.cullMode, mUnclippedDepth);

    // Record multisample state
    // Sample count hashed as part of the attachment state
    recorder.Record(mMultisample.mask, mMultisample.alphaToCoverageEnabled);

    return recorder.GetContentHash();
}

bool RenderPipelineBase::EqualityFunc::operator()(const RenderPipelineBase* a,
                                                  const RenderPipelineBase* b) const {
    // Check the layout and shader stages.
    if (!PipelineBase::EqualForCache(a, b)) {
        return false;
    }

    // Check the attachment state.
    // It contains the attachments set, texture formats, and sample count.
    if (a->mAttachmentState.Get() != b->mAttachmentState.Get()) {
        return false;
    }

    if (a->mAttachmentState.Get() != nullptr) {
        for (ColorAttachmentIndex i :
             IterateBitSet(a->mAttachmentState->GetColorAttachmentsMask())) {
            const ColorTargetState& descA = *a->GetColorTargetState(i);
            const ColorTargetState& descB = *b->GetColorTargetState(i);
            if (descA.writeMask != descB.writeMask) {
                return false;
            }
            if ((descA.blend == nullptr) != (descB.blend == nullptr)) {
                return false;
            }
            if (descA.blend != nullptr) {
                if (descA.blend->color.operation != descB.blend->color.operation ||
                    descA.blend->color.srcFactor != descB.blend->color.srcFactor ||
                    descA.blend->color.dstFactor != descB.blend->color.dstFactor) {
                    return false;
                }
                if (descA.blend->alpha.operation != descB.blend->alpha.operation ||
                    descA.blend->alpha.srcFactor != descB.blend->alpha.srcFactor ||
                    descA.blend->alpha.dstFactor != descB.blend->alpha.dstFactor) {
                    return false;
                }
            }
        }

        // Check depth/stencil state
        if (a->mAttachmentState->HasDepthStencilAttachment()) {
            const DepthStencilState& stateA = a->mDepthStencil;
            const DepthStencilState& stateB = b->mDepthStencil;

            DAWN_ASSERT(!std::isnan(stateA.depthBiasSlopeScale));
            DAWN_ASSERT(!std::isnan(stateB.depthBiasSlopeScale));
            DAWN_ASSERT(!std::isnan(stateA.depthBiasClamp));
            DAWN_ASSERT(!std::isnan(stateB.depthBiasClamp));

            if (stateA.depthWriteEnabled != stateB.depthWriteEnabled ||
                stateA.depthCompare != stateB.depthCompare ||
                stateA.depthBias != stateB.depthBias ||
                stateA.depthBiasSlopeScale != stateB.depthBiasSlopeScale ||
                stateA.depthBiasClamp != stateB.depthBiasClamp) {
                return false;
            }
            if (stateA.stencilFront.compare != stateB.stencilFront.compare ||
                stateA.stencilFront.failOp != stateB.stencilFront.failOp ||
                stateA.stencilFront.depthFailOp != stateB.stencilFront.depthFailOp ||
                stateA.stencilFront.passOp != stateB.stencilFront.passOp) {
                return false;
            }
            if (stateA.stencilBack.compare != stateB.stencilBack.compare ||
                stateA.stencilBack.failOp != stateB.stencilBack.failOp ||
                stateA.stencilBack.depthFailOp != stateB.stencilBack.depthFailOp ||
                stateA.stencilBack.passOp != stateB.stencilBack.passOp) {
                return false;
            }
            if (stateA.stencilReadMask != stateB.stencilReadMask ||
                stateA.stencilWriteMask != stateB.stencilWriteMask) {
                return false;
            }
        }
    }

    // Check vertex state
    if (a->mAttributeLocationsUsed != b->mAttributeLocationsUsed) {
        return false;
    }

    for (VertexAttributeLocation loc : IterateBitSet(a->mAttributeLocationsUsed)) {
        const VertexAttributeInfo& descA = a->GetAttribute(loc);
        const VertexAttributeInfo& descB = b->GetAttribute(loc);
        if (descA.shaderLocation != descB.shaderLocation ||
            descA.vertexBufferSlot != descB.vertexBufferSlot || descA.offset != descB.offset ||
            descA.format != descB.format) {
            return false;
        }
    }

    if (a->mVertexBufferSlotsUsed != b->mVertexBufferSlotsUsed) {
        return false;
    }

    for (VertexBufferSlot slot : IterateBitSet(a->mVertexBufferSlotsUsed)) {
        const VertexBufferInfo& descA = a->GetVertexBuffer(slot);
        const VertexBufferInfo& descB = b->GetVertexBuffer(slot);
        if (descA.arrayStride != descB.arrayStride || descA.stepMode != descB.stepMode) {
            return false;
        }
    }

    // Check primitive state
    {
        const PrimitiveState& stateA = a->mPrimitive;
        const PrimitiveState& stateB = b->mPrimitive;
        if (stateA.topology != stateB.topology ||
            stateA.stripIndexFormat != stateB.stripIndexFormat ||
            stateA.frontFace != stateB.frontFace || stateA.cullMode != stateB.cullMode ||
            a->mUnclippedDepth != b->mUnclippedDepth) {
            return false;
        }
    }

    // Check multisample state
    {
        const MultisampleState& stateA = a->mMultisample;
        const MultisampleState& stateB = b->mMultisample;
        // Sample count already checked as part of the attachment state.
        if (stateA.mask != stateB.mask ||
            stateA.alphaToCoverageEnabled != stateB.alphaToCoverageEnabled) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native
