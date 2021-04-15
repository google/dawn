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

#include "dawn_native/RenderPipeline.h"

#include "common/BitSetIterator.h"
#include "common/VertexFormatUtils.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/ObjectContentHasher.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <cmath>

namespace dawn_native {
    // Helper functions
    namespace {

        MaybeError ValidateVertexAttribute(DeviceBase* device,
                                           const VertexAttribute* attribute,
                                           uint64_t vertexBufferStride,
                                           std::bitset<kMaxVertexAttributes>* attributesSetMask) {
            DAWN_TRY(ValidateVertexFormat(attribute->format));

            if (dawn::IsDeprecatedVertexFormat(attribute->format)) {
                device->EmitDeprecationWarning(
                    "Vertex formats have changed and the old types will be removed soon.");
            }

            if (attribute->shaderLocation >= kMaxVertexAttributes) {
                return DAWN_VALIDATION_ERROR("Setting attribute out of bounds");
            }

            // No underflow is possible because the max vertex format size is smaller than
            // kMaxVertexBufferStride.
            ASSERT(kMaxVertexBufferStride >= dawn::VertexFormatSize(attribute->format));
            if (attribute->offset >
                kMaxVertexBufferStride - dawn::VertexFormatSize(attribute->format)) {
                return DAWN_VALIDATION_ERROR("Setting attribute offset out of bounds");
            }

            // No overflow is possible because the offset is already validated to be less
            // than kMaxVertexBufferStride.
            ASSERT(attribute->offset < kMaxVertexBufferStride);
            if (vertexBufferStride > 0 &&
                attribute->offset + dawn::VertexFormatSize(attribute->format) >
                    vertexBufferStride) {
                return DAWN_VALIDATION_ERROR("Setting attribute offset out of bounds");
            }

            if (attribute->offset % dawn::VertexFormatComponentSize(attribute->format) != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Attribute offset needs to be a multiple of the size format's components");
            }

            if ((*attributesSetMask)[attribute->shaderLocation]) {
                return DAWN_VALIDATION_ERROR("Setting already set attribute");
            }

            attributesSetMask->set(attribute->shaderLocation);
            return {};
        }

        MaybeError ValidateVertexBufferLayout(
            DeviceBase* device,
            const VertexBufferLayout* buffer,
            std::bitset<kMaxVertexAttributes>* attributesSetMask) {
            DAWN_TRY(ValidateInputStepMode(buffer->stepMode));
            if (buffer->arrayStride > kMaxVertexBufferStride) {
                return DAWN_VALIDATION_ERROR("Setting arrayStride out of bounds");
            }

            if (buffer->arrayStride % 4 != 0) {
                return DAWN_VALIDATION_ERROR(
                    "arrayStride of Vertex buffer needs to be a multiple of 4 bytes");
            }

            for (uint32_t i = 0; i < buffer->attributeCount; ++i) {
                DAWN_TRY(ValidateVertexAttribute(device, &buffer->attributes[i],
                                                 buffer->arrayStride, attributesSetMask));
            }

            return {};
        }

        MaybeError ValidateVertexState(DeviceBase* device,
                                       const VertexState* descriptor,
                                       const PipelineLayoutBase* layout) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }

            if (descriptor->bufferCount > kMaxVertexBuffers) {
                return DAWN_VALIDATION_ERROR("Vertex buffer count exceeds maximum");
            }

            std::bitset<kMaxVertexAttributes> attributesSetMask;
            uint32_t totalAttributesNum = 0;
            for (uint32_t i = 0; i < descriptor->bufferCount; ++i) {
                DAWN_TRY(ValidateVertexBufferLayout(device, &descriptor->buffers[i],
                                                    &attributesSetMask));
                totalAttributesNum += descriptor->buffers[i].attributeCount;
            }

            // Every vertex attribute has a member called shaderLocation, and there are some
            // requirements for shaderLocation: 1) >=0, 2) values are different across different
            // attributes, 3) can't exceed kMaxVertexAttributes. So it can ensure that total
            // attribute number never exceed kMaxVertexAttributes.
            ASSERT(totalAttributesNum <= kMaxVertexAttributes);

            DAWN_TRY(ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                               layout, SingleShaderStage::Vertex));
            const EntryPointMetadata& vertexMetadata =
                descriptor->module->GetEntryPoint(descriptor->entryPoint);
            if (!IsSubset(vertexMetadata.usedVertexAttributes, attributesSetMask)) {
                return DAWN_VALIDATION_ERROR(
                    "Pipeline vertex stage uses vertex buffers not in the vertex state");
            }

            return {};
        }

        MaybeError ValidatePrimitiveState(const DeviceBase* device,
                                          const PrimitiveState* descriptor) {
            const ChainedStruct* chained = descriptor->nextInChain;
            if (chained != nullptr) {
                if (chained->sType != wgpu::SType::PrimitiveDepthClampingState) {
                    return DAWN_VALIDATION_ERROR("Unsupported sType");
                }
                if (!device->IsExtensionEnabled(Extension::DepthClamping)) {
                    return DAWN_VALIDATION_ERROR("The depth clamping feature is not supported");
                }
            }

            DAWN_TRY(ValidatePrimitiveTopology(descriptor->topology));
            DAWN_TRY(ValidateIndexFormat(descriptor->stripIndexFormat));
            DAWN_TRY(ValidateFrontFace(descriptor->frontFace));
            DAWN_TRY(ValidateCullMode(descriptor->cullMode));

            // Pipeline descriptors must have stripIndexFormat != undefined IFF they are using strip
            // topologies.
            if (IsStripPrimitiveTopology(descriptor->topology)) {
                if (descriptor->stripIndexFormat == wgpu::IndexFormat::Undefined) {
                    return DAWN_VALIDATION_ERROR(
                        "stripIndexFormat must not be undefined when using strip primitive "
                        "topologies");
                }
            } else if (descriptor->stripIndexFormat != wgpu::IndexFormat::Undefined) {
                return DAWN_VALIDATION_ERROR(
                    "stripIndexFormat must be undefined when using non-strip primitive topologies");
            }

            return {};
        }

        MaybeError ValidateDepthStencilState(const DeviceBase* device,
                                             const DepthStencilState* descriptor) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }

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
            if (!format->HasDepthOrStencil() || !format->isRenderable) {
                return DAWN_VALIDATION_ERROR(
                    "Depth stencil format must be depth-stencil renderable");
            }

            if (std::isnan(descriptor->depthBiasSlopeScale) ||
                std::isnan(descriptor->depthBiasClamp)) {
                return DAWN_VALIDATION_ERROR("Depth bias parameters must not be NaN.");
            }

            return {};
        }

        MaybeError ValidateMultisampleState(const MultisampleState* descriptor) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }

            if (!IsValidSampleCount(descriptor->count)) {
                return DAWN_VALIDATION_ERROR("Multisample count is not supported");
            }

            if (descriptor->alphaToCoverageEnabled && descriptor->count <= 1) {
                return DAWN_VALIDATION_ERROR("Enabling alphaToCoverage requires sample count > 1");
            }

            return {};
        }

        static constexpr wgpu::BlendFactor kFirstDeprecatedBlendFactor =
            wgpu::BlendFactor::SrcColor;
        static constexpr uint32_t kDeprecatedBlendFactorOffset = 100;

        bool IsDeprecatedBlendFactor(wgpu::BlendFactor blendFactor) {
            return blendFactor >= kFirstDeprecatedBlendFactor;
        }

        wgpu::BlendFactor NormalizeBlendFactor(wgpu::BlendFactor blendFactor) {
            // If the specified format is from the deprecated range return the corresponding
            // non-deprecated format.
            if (blendFactor >= kFirstDeprecatedBlendFactor) {
                uint32_t blendFactorValue = static_cast<uint32_t>(blendFactor);
                return static_cast<wgpu::BlendFactor>(blendFactorValue -
                                                      kDeprecatedBlendFactorOffset);
            }
            return blendFactor;
        }

        MaybeError ValidateBlendState(DeviceBase* device, const BlendState* descriptor) {
            DAWN_TRY(ValidateBlendOperation(descriptor->alpha.operation));
            DAWN_TRY(ValidateBlendFactor(descriptor->alpha.srcFactor));
            DAWN_TRY(ValidateBlendFactor(descriptor->alpha.dstFactor));
            DAWN_TRY(ValidateBlendOperation(descriptor->color.operation));
            DAWN_TRY(ValidateBlendFactor(descriptor->color.srcFactor));
            DAWN_TRY(ValidateBlendFactor(descriptor->color.dstFactor));

            if (IsDeprecatedBlendFactor(descriptor->alpha.srcFactor) ||
                IsDeprecatedBlendFactor(descriptor->alpha.dstFactor) ||
                IsDeprecatedBlendFactor(descriptor->color.srcFactor) ||
                IsDeprecatedBlendFactor(descriptor->color.dstFactor)) {
                device->EmitDeprecationWarning(
                    "Blend factor enums have changed and the old enums will be removed soon.");
            }

            return {};
        }

        MaybeError ValidateColorTargetState(DeviceBase* device,
                                            const ColorTargetState* descriptor,
                                            bool fragmentWritten,
                                            wgpu::TextureComponentType fragmentOutputBaseType) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }

            if (descriptor->blend) {
                DAWN_TRY(ValidateBlendState(device, descriptor->blend));
            }

            DAWN_TRY(ValidateColorWriteMask(descriptor->writeMask));

            const Format* format;
            DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));
            if (!format->IsColor() || !format->isRenderable) {
                return DAWN_VALIDATION_ERROR("Color format must be color renderable");
            }
            if (fragmentWritten &&
                fragmentOutputBaseType != format->GetAspectInfo(Aspect::Color).baseType) {
                return DAWN_VALIDATION_ERROR(
                    "Color format must match the fragment stage output type");
            }

            return {};
        }

        MaybeError ValidateFragmentState(DeviceBase* device,
                                         const FragmentState* descriptor,
                                         const PipelineLayoutBase* layout) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }

            DAWN_TRY(ValidateProgrammableStage(device, descriptor->module, descriptor->entryPoint,
                                               layout, SingleShaderStage::Fragment));

            if (descriptor->targetCount > kMaxColorAttachments) {
                return DAWN_VALIDATION_ERROR("Number of color targets exceeds maximum");
            }

            const EntryPointMetadata& fragmentMetadata =
                descriptor->module->GetEntryPoint(descriptor->entryPoint);
            for (ColorAttachmentIndex i(uint8_t(0));
                 i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->targetCount)); ++i) {
                DAWN_TRY(
                    ValidateColorTargetState(device, &descriptor->targets[static_cast<uint8_t>(i)],
                                             fragmentMetadata.fragmentOutputsWritten[i],
                                             fragmentMetadata.fragmentOutputFormatBaseTypes[i]));
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
                UNREACHABLE();
        }
    }

    bool IsStripPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
        return primitiveTopology == wgpu::PrimitiveTopology::LineStrip ||
               primitiveTopology == wgpu::PrimitiveTopology::TriangleStrip;
    }

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor2* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        if (descriptor->layout != nullptr) {
            DAWN_TRY(device->ValidateObject(descriptor->layout));
        }

        // TODO(crbug.com/dawn/136): Support vertex-only pipelines.
        if (descriptor->fragment == nullptr) {
            return DAWN_VALIDATION_ERROR("Null fragment stage is not supported (yet)");
        }

        DAWN_TRY(ValidateVertexState(device, &descriptor->vertex, descriptor->layout));

        DAWN_TRY(ValidatePrimitiveState(device, &descriptor->primitive));

        if (descriptor->depthStencil) {
            DAWN_TRY(ValidateDepthStencilState(device, descriptor->depthStencil));
        }

        DAWN_TRY(ValidateMultisampleState(&descriptor->multisample));

        ASSERT(descriptor->fragment != nullptr);
        DAWN_TRY(ValidateFragmentState(device, descriptor->fragment, descriptor->layout));

        if (descriptor->fragment->targetCount == 0 && !descriptor->depthStencil) {
            return DAWN_VALIDATION_ERROR("Should have at least one color target or a depthStencil");
        }

        return {};
    }

    std::vector<StageAndDescriptor> GetStages(const RenderPipelineDescriptor2* descriptor) {
        std::vector<StageAndDescriptor> stages;
        stages.push_back(
            {SingleShaderStage::Vertex, descriptor->vertex.module, descriptor->vertex.entryPoint});
        if (descriptor->fragment != nullptr) {
            stages.push_back({SingleShaderStage::Fragment, descriptor->fragment->module,
                              descriptor->fragment->entryPoint});
        }
        return stages;
    }

    bool StencilTestEnabled(const DepthStencilState* mDepthStencil) {
        return mDepthStencil->stencilBack.compare != wgpu::CompareFunction::Always ||
               mDepthStencil->stencilBack.failOp != wgpu::StencilOperation::Keep ||
               mDepthStencil->stencilBack.depthFailOp != wgpu::StencilOperation::Keep ||
               mDepthStencil->stencilBack.passOp != wgpu::StencilOperation::Keep ||
               mDepthStencil->stencilFront.compare != wgpu::CompareFunction::Always ||
               mDepthStencil->stencilFront.failOp != wgpu::StencilOperation::Keep ||
               mDepthStencil->stencilFront.depthFailOp != wgpu::StencilOperation::Keep ||
               mDepthStencil->stencilFront.passOp != wgpu::StencilOperation::Keep;
    }

    bool BlendEnabled(const ColorStateDescriptor* mColorState) {
        return mColorState->alphaBlend.operation != wgpu::BlendOperation::Add ||
               mColorState->alphaBlend.srcFactor != wgpu::BlendFactor::One ||
               mColorState->alphaBlend.dstFactor != wgpu::BlendFactor::Zero ||
               mColorState->colorBlend.operation != wgpu::BlendOperation::Add ||
               mColorState->colorBlend.srcFactor != wgpu::BlendFactor::One ||
               mColorState->colorBlend.dstFactor != wgpu::BlendFactor::Zero;
    }

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                           const RenderPipelineDescriptor2* descriptor)
        : PipelineBase(device,
                       descriptor->layout,
                       {{SingleShaderStage::Vertex, descriptor->vertex.module,
                         descriptor->vertex.entryPoint},
                        {SingleShaderStage::Fragment, descriptor->fragment->module,
                         descriptor->fragment->entryPoint}}),
          mAttachmentState(device->GetOrCreateAttachmentState(descriptor)) {
        mVertexBufferCount = descriptor->vertex.bufferCount;
        const VertexBufferLayout* buffers = descriptor->vertex.buffers;
        for (uint8_t slot = 0; slot < mVertexBufferCount; ++slot) {
            if (buffers[slot].attributeCount == 0) {
                continue;
            }

            VertexBufferSlot typedSlot(slot);

            mVertexBufferSlotsUsed.set(typedSlot);
            mVertexBufferInfos[typedSlot].arrayStride = buffers[slot].arrayStride;
            mVertexBufferInfos[typedSlot].stepMode = buffers[slot].stepMode;

            for (uint32_t i = 0; i < buffers[slot].attributeCount; ++i) {
                VertexAttributeLocation location = VertexAttributeLocation(
                    static_cast<uint8_t>(buffers[slot].attributes[i].shaderLocation));
                mAttributeLocationsUsed.set(location);
                mAttributeInfos[location].shaderLocation = location;
                mAttributeInfos[location].vertexBufferSlot = typedSlot;
                mAttributeInfos[location].offset = buffers[slot].attributes[i].offset;

                mAttributeInfos[location].format =
                    dawn::NormalizeVertexFormat(buffers[slot].attributes[i].format);
            }
        }

        mPrimitive = descriptor->primitive;
        const ChainedStruct* chained = mPrimitive.nextInChain;
        if (chained != nullptr) {
            ASSERT(chained->sType == wgpu::SType::PrimitiveDepthClampingState);
            const auto* clampState = static_cast<const PrimitiveDepthClampingState*>(chained);
            mClampDepth = clampState->clampDepth;
        }
        mMultisample = descriptor->multisample;

        if (mAttachmentState->HasDepthStencilAttachment()) {
            mDepthStencil = *descriptor->depthStencil;
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
            const ColorTargetState* target =
                &descriptor->fragment->targets[static_cast<uint8_t>(i)];
            mTargets[i] = *target;

            if (target->blend != nullptr) {
                mTargetBlend[i] = *target->blend;
                mTargets[i].blend = &mTargetBlend[i];
                mTargetBlend[i].alpha.srcFactor =
                    NormalizeBlendFactor(mTargetBlend[i].alpha.srcFactor);
                mTargetBlend[i].alpha.dstFactor =
                    NormalizeBlendFactor(mTargetBlend[i].alpha.dstFactor);
                mTargetBlend[i].color.srcFactor =
                    NormalizeBlendFactor(mTargetBlend[i].color.srcFactor);
                mTargetBlend[i].color.dstFactor =
                    NormalizeBlendFactor(mTargetBlend[i].color.dstFactor);
            }
        }

        // TODO(cwallez@chromium.org): Check against the shader module that the correct color
        // attachment are set?
    }

    RenderPipelineBase::RenderPipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : PipelineBase(device, tag) {
    }

    // static
    RenderPipelineBase* RenderPipelineBase::MakeError(DeviceBase* device) {
        return new RenderPipelineBase(device, ObjectBase::kError);
    }

    RenderPipelineBase::~RenderPipelineBase() {
        if (IsCachedReference()) {
            GetDevice()->UncacheRenderPipeline(this);
        }
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

    bool RenderPipelineBase::ShouldClampDepth() const {
        ASSERT(!IsError());
        return mClampDepth;
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
                        mPrimitive.cullMode, mClampDepth);

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
                a->mClampDepth != b->mClampDepth) {
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

}  // namespace dawn_native
