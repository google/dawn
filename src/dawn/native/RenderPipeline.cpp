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

#include "dawn/native/RenderPipeline.h"

#include <algorithm>
#include <cmath>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/VertexFormat.h"

namespace dawn::native {

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
    ASSERT(kMaxVertexBufferArrayStride >= formatInfo.byteSize);
    DAWN_INVALID_IF(
        attribute->offset > kMaxVertexBufferArrayStride - formatInfo.byteSize,
        "Attribute offset (%u) with format %s (size: %u) doesn't fit in the maximum vertex "
        "buffer stride (%u).",
        attribute->offset, attribute->format, formatInfo.byteSize, kMaxVertexBufferArrayStride);

    // No overflow is possible because the offset is already validated to be less
    // than kMaxVertexBufferArrayStride.
    ASSERT(attribute->offset < kMaxVertexBufferArrayStride);
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

    // Every vertex attribute has a member called shaderLocation, and there are some
    // requirements for shaderLocation: 1) >=0, 2) values are different across different
    // attributes, 3) can't exceed kMaxVertexAttributes. So it can ensure that total
    // attribute number never exceed kMaxVertexAttributes.
    ASSERT(totalAttributesNum <= kMaxVertexAttributes);

    // TODO(dawn:563): Specify which inputs were not used in error message.
    DAWN_INVALID_IF(!IsSubset(vertexMetadata.usedVertexInputs, attributesSetMask),
                    "Pipeline vertex stage uses vertex buffers not in the vertex state");

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
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain is not nullptr.");

    DAWN_TRY(ValidateCompareFunction(descriptor->depthCompare));
    DAWN_TRY(ValidateCompareFunction(descriptor->stencilFront.compare));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilFront.failOp));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilFront.depthFailOp));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilFront.passOp));
    DAWN_TRY(ValidateCompareFunction(descriptor->stencilBack.compare));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilBack.failOp));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilBack.depthFailOp));
    DAWN_TRY(ValidateStencilOperation(descriptor->stencilBack.passOp));

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
        !format->HasDepth() && (descriptor->depthCompare != wgpu::CompareFunction::Always ||
                                descriptor->depthWriteEnabled),
        "Depth stencil format (%s) doesn't have depth aspect while depthCompare (%s) is "
        "not %s or depthWriteEnabled (%u) is true.",
        descriptor->format, descriptor->depthCompare, wgpu::CompareFunction::Always,
        descriptor->depthWriteEnabled);

    DAWN_INVALID_IF(!format->HasStencil() && StencilTestEnabled(descriptor),
                    "Depth stencil format (%s) doesn't have stencil aspect while stencil "
                    "test or stencil write is enabled.",
                    descriptor->format);

    return {};
}

MaybeError ValidateMultisampleState(const MultisampleState* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->count),
                    "Multisample count (%u) is not supported.", descriptor->count);

    DAWN_INVALID_IF(descriptor->alphaToCoverageEnabled && descriptor->count <= 1,
                    "Multisample count (%u) must be > 1 when alphaToCoverage is enabled.",
                    descriptor->count);

    return {};
}

MaybeError ValidateBlendComponent(BlendComponent blendComponent) {
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
    DAWN_TRY(ValidateBlendComponent(descriptor->alpha));
    DAWN_TRY(ValidateBlendComponent(descriptor->color));

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

    ColorAttachmentFormats colorAttachmentFormats;
    for (ColorAttachmentIndex i(uint8_t(0));
         i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->targetCount)); ++i) {
        const ColorTargetState* target = &descriptor->targets[static_cast<uint8_t>(i)];
        if (target->format != wgpu::TextureFormat::Undefined) {
            DAWN_TRY_CONTEXT(
                ValidateColorTargetState(device, target, fragmentMetadata.fragmentOutputsWritten[i],
                                         fragmentMetadata.fragmentOutputVariables[i]),
                "validating targets[%u].", static_cast<uint8_t>(i));
            colorAttachmentFormats->push_back(&device->GetValidInternalFormat(target->format));
        } else {
            DAWN_INVALID_IF(target->blend,
                            "Color target[%u] blend state is set when the format is undefined.",
                            static_cast<uint8_t>(i));
        }
    }
    DAWN_TRY(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats));

    DAWN_INVALID_IF(fragmentMetadata.usesSampleMaskOutput && alphaToCoverageEnabled,
                    "alphaToCoverageEnabled is true when the sample_mask builtin is a "
                    "pipeline output of fragment stage of %s.",
                    descriptor->module);

    return {};
}

MaybeError ValidateInterStageMatching(DeviceBase* device,
                                      const VertexState& vertexState,
                                      const FragmentState& fragmentState) {
    const EntryPointMetadata& vertexMetadata =
        vertexState.module->GetEntryPoint(vertexState.entryPoint);
    const EntryPointMetadata& fragmentMetadata =
        fragmentState.module->GetEntryPoint(fragmentState.entryPoint);

    if (DAWN_UNLIKELY(
            (vertexMetadata.usedInterStageVariables | fragmentMetadata.usedInterStageVariables) !=
            vertexMetadata.usedInterStageVariables)) {
        for (size_t i : IterateBitSet(fragmentMetadata.usedInterStageVariables)) {
            if (!vertexMetadata.usedInterStageVariables.test(i)) {
                return DAWN_VALIDATION_ERROR(
                    "The fragment input at location %u doesn't have a corresponding vertex output.",
                    i);
            }
        }
        UNREACHABLE();
    }

    for (size_t i : IterateBitSet(vertexMetadata.usedInterStageVariables)) {
        if (!fragmentMetadata.usedInterStageVariables.test(i)) {
            // It is valid that fragment output is a subset of vertex input
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
    UNREACHABLE();
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

    DAWN_TRY_CONTEXT(ValidateMultisampleState(&descriptor->multisample),
                     "validating multisample state.");

    if (descriptor->fragment != nullptr) {
        DAWN_TRY_CONTEXT(ValidateFragmentState(device, descriptor->fragment, descriptor->layout,
                                               descriptor->depthStencil,
                                               descriptor->multisample.alphaToCoverageEnabled),
                         "validating fragment state.");

        DAWN_INVALID_IF(descriptor->fragment->targetCount == 0 && !descriptor->depthStencil,
                        "Must have at least one color or depthStencil target.");

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
      mAttachmentState(device->GetOrCreateAttachmentState(descriptor)) {
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
        ASSERT(descriptor->fragment != nullptr);
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

RenderPipelineBase::RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : PipelineBase(device, tag) {}

RenderPipelineBase::~RenderPipelineBase() = default;

void RenderPipelineBase::DestroyImpl() {
    if (IsCachedReference()) {
        // Do not uncache the actual cached object if we are a blueprint.
        GetDevice()->UncacheRenderPipeline(this);
    }

    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

// static
RenderPipelineBase* RenderPipelineBase::MakeError(DeviceBase* device) {
    class ErrorRenderPipeline final : public RenderPipelineBase {
      public:
        explicit ErrorRenderPipeline(DeviceBase* device)
            : RenderPipelineBase(device, ObjectBase::kError) {}

        MaybeError Initialize() override {
            UNREACHABLE();
            return {};
        }
    };

    return new ErrorRenderPipeline(device);
}

ObjectType RenderPipelineBase::GetType() const {
    return ObjectType::RenderPipeline;
}

const ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>&
RenderPipelineBase::GetAttributeLocationsUsed() const {
    ASSERT(!IsError());
    return mAttributeLocationsUsed;
}

const VertexAttributeInfo& RenderPipelineBase::GetAttribute(
    VertexAttributeLocation location) const {
    ASSERT(!IsError());
    ASSERT(mAttributeLocationsUsed[location]);
    return mAttributeInfos[location];
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsed() const {
    ASSERT(!IsError());
    return mVertexBufferSlotsUsed;
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsedAsVertexBuffer() const {
    ASSERT(!IsError());
    return mVertexBufferSlotsUsedAsVertexBuffer;
}

const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>&
RenderPipelineBase::GetVertexBufferSlotsUsedAsInstanceBuffer() const {
    ASSERT(!IsError());
    return mVertexBufferSlotsUsedAsInstanceBuffer;
}

const VertexBufferInfo& RenderPipelineBase::GetVertexBuffer(VertexBufferSlot slot) const {
    ASSERT(!IsError());
    ASSERT(mVertexBufferSlotsUsed[slot]);
    return mVertexBufferInfos[slot];
}

uint32_t RenderPipelineBase::GetVertexBufferCount() const {
    ASSERT(!IsError());
    return mVertexBufferCount;
}

const ColorTargetState* RenderPipelineBase::GetColorTargetState(
    ColorAttachmentIndex attachmentSlot) const {
    ASSERT(!IsError());
    ASSERT(attachmentSlot < mTargets.size());
    return &mTargets[attachmentSlot];
}

const DepthStencilState* RenderPipelineBase::GetDepthStencilState() const {
    ASSERT(!IsError());
    return &mDepthStencil;
}

wgpu::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
    ASSERT(!IsError());
    return mPrimitive.topology;
}

wgpu::IndexFormat RenderPipelineBase::GetStripIndexFormat() const {
    ASSERT(!IsError());
    return mPrimitive.stripIndexFormat;
}

wgpu::CullMode RenderPipelineBase::GetCullMode() const {
    ASSERT(!IsError());
    return mPrimitive.cullMode;
}

wgpu::FrontFace RenderPipelineBase::GetFrontFace() const {
    ASSERT(!IsError());
    return mPrimitive.frontFace;
}

bool RenderPipelineBase::IsDepthBiasEnabled() const {
    ASSERT(!IsError());
    return mDepthStencil.depthBias != 0 || mDepthStencil.depthBiasSlopeScale != 0;
}

int32_t RenderPipelineBase::GetDepthBias() const {
    ASSERT(!IsError());
    return mDepthStencil.depthBias;
}

float RenderPipelineBase::GetDepthBiasSlopeScale() const {
    ASSERT(!IsError());
    return mDepthStencil.depthBiasSlopeScale;
}

float RenderPipelineBase::GetDepthBiasClamp() const {
    ASSERT(!IsError());
    return mDepthStencil.depthBiasClamp;
}

bool RenderPipelineBase::HasUnclippedDepth() const {
    ASSERT(!IsError());
    return mUnclippedDepth;
}

ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments>
RenderPipelineBase::GetColorAttachmentsMask() const {
    ASSERT(!IsError());
    return mAttachmentState->GetColorAttachmentsMask();
}

bool RenderPipelineBase::HasDepthStencilAttachment() const {
    ASSERT(!IsError());
    return mAttachmentState->HasDepthStencilAttachment();
}

wgpu::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(
    ColorAttachmentIndex attachment) const {
    ASSERT(!IsError());
    return mTargets[attachment].format;
}

wgpu::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
    ASSERT(!IsError());
    ASSERT(mAttachmentState->HasDepthStencilAttachment());
    return mDepthStencil.format;
}

uint32_t RenderPipelineBase::GetSampleCount() const {
    ASSERT(!IsError());
    return mAttachmentState->GetSampleCount();
}

uint32_t RenderPipelineBase::GetSampleMask() const {
    ASSERT(!IsError());
    return mMultisample.mask;
}

bool RenderPipelineBase::IsAlphaToCoverageEnabled() const {
    ASSERT(!IsError());
    return mMultisample.alphaToCoverageEnabled;
}

const AttachmentState* RenderPipelineBase::GetAttachmentState() const {
    ASSERT(!IsError());
    return mAttachmentState.Get();
}

bool RenderPipelineBase::WritesDepth() const {
    ASSERT(!IsError());
    return mWritesDepth;
}

bool RenderPipelineBase::WritesStencil() const {
    ASSERT(!IsError());
    return mWritesStencil;
}

bool RenderPipelineBase::UsesFragDepth() const {
    ASSERT(!IsError());
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

            ASSERT(!std::isnan(stateA.depthBiasSlopeScale));
            ASSERT(!std::isnan(stateB.depthBiasSlopeScale));
            ASSERT(!std::isnan(stateA.depthBiasClamp));
            ASSERT(!std::isnan(stateB.depthBiasClamp));

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
