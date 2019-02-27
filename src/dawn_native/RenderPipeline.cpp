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
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/InputState.h"
#include "dawn_native/Texture.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {
    // Helper functions
    namespace {

        MaybeError ValidatePipelineStageDescriptor(DeviceBase* device,
                                                   const PipelineStageDescriptor* descriptor,
                                                   const PipelineLayoutBase* layout,
                                                   dawn::ShaderStage stage) {
            DAWN_TRY(device->ValidateObject(descriptor->module));

            if (descriptor->entryPoint != std::string("main")) {
                return DAWN_VALIDATION_ERROR("Entry point must be \"main\"");
            }
            if (descriptor->module->GetExecutionModel() != stage) {
                return DAWN_VALIDATION_ERROR("Setting module with wrong stages");
            }
            if (!descriptor->module->IsCompatibleWithPipelineLayout(layout)) {
                return DAWN_VALIDATION_ERROR("Stage not compatible with layout");
            }
            return {};
        }

        MaybeError ValidateColorStateDescriptor(const ColorStateDescriptor* descriptor) {
            if (descriptor->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
            }
            DAWN_TRY(ValidateBlendOperation(descriptor->alphaBlend.operation));
            DAWN_TRY(ValidateBlendFactor(descriptor->alphaBlend.srcFactor));
            DAWN_TRY(ValidateBlendFactor(descriptor->alphaBlend.dstFactor));
            DAWN_TRY(ValidateBlendOperation(descriptor->colorBlend.operation));
            DAWN_TRY(ValidateBlendFactor(descriptor->colorBlend.srcFactor));
            DAWN_TRY(ValidateBlendFactor(descriptor->colorBlend.dstFactor));
            DAWN_TRY(ValidateColorWriteMask(descriptor->colorWriteMask));

            dawn::TextureFormat format = descriptor->format;
            DAWN_TRY(ValidateTextureFormat(format));
            if (!IsColorRenderableTextureFormat(format)) {
                return DAWN_VALIDATION_ERROR("Color format must be color renderable");
            }

            return {};
        }

        MaybeError ValidateDepthStencilStateDescriptor(
            const DepthStencilStateDescriptor* descriptor) {
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

            dawn::TextureFormat format = descriptor->format;
            DAWN_TRY(ValidateTextureFormat(format));
            if (!IsDepthStencilRenderableTextureFormat(format)) {
                return DAWN_VALIDATION_ERROR(
                    "Depth stencil format must be depth-stencil renderable");
            }

            return {};
        }

    }  // anonymous namespace

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        DAWN_TRY(device->ValidateObject(descriptor->layout));

        if (descriptor->inputState == nullptr) {
            return DAWN_VALIDATION_ERROR("Input state must not be null");
        }

        DAWN_TRY(ValidateIndexFormat(descriptor->indexFormat));
        DAWN_TRY(ValidatePrimitiveTopology(descriptor->primitiveTopology));
        DAWN_TRY(ValidatePipelineStageDescriptor(device, descriptor->vertexStage,
                                                 descriptor->layout, dawn::ShaderStage::Vertex));
        DAWN_TRY(ValidatePipelineStageDescriptor(device, descriptor->fragmentStage,
                                                 descriptor->layout, dawn::ShaderStage::Fragment));

        if ((descriptor->vertexStage->module->GetUsedVertexAttributes() &
             ~descriptor->inputState->GetAttributesSetMask())
                .any()) {
            return DAWN_VALIDATION_ERROR(
                "Pipeline vertex stage uses inputs not in the input state");
        }

        if (descriptor->sampleCount != 1) {
            return DAWN_VALIDATION_ERROR("Sample count must be one");
        }

        if (descriptor->colorStateCount > kMaxColorAttachments) {
            return DAWN_VALIDATION_ERROR("Color States number exceeds maximum");
        }

        if (descriptor->colorStateCount == 0 && !descriptor->depthStencilState) {
            return DAWN_VALIDATION_ERROR("Should have at least one attachment");
        }

        for (uint32_t i = 0; i < descriptor->colorStateCount; ++i) {
            DAWN_TRY(ValidateColorStateDescriptor(descriptor->colorStates[i]));
        }

        if (descriptor->depthStencilState) {
            DAWN_TRY(ValidateDepthStencilStateDescriptor(descriptor->depthStencilState));
        }

        return {};
    }

    bool StencilTestEnabled(const DepthStencilStateDescriptor* mDepthStencilState) {
        return mDepthStencilState->stencilBack.compare != dawn::CompareFunction::Always ||
               mDepthStencilState->stencilBack.failOp != dawn::StencilOperation::Keep ||
               mDepthStencilState->stencilBack.depthFailOp != dawn::StencilOperation::Keep ||
               mDepthStencilState->stencilBack.passOp != dawn::StencilOperation::Keep ||
               mDepthStencilState->stencilFront.compare != dawn::CompareFunction::Always ||
               mDepthStencilState->stencilFront.failOp != dawn::StencilOperation::Keep ||
               mDepthStencilState->stencilFront.depthFailOp != dawn::StencilOperation::Keep ||
               mDepthStencilState->stencilFront.passOp != dawn::StencilOperation::Keep;
    }

    bool BlendEnabled(const ColorStateDescriptor* mColorState) {
        return mColorState->alphaBlend.operation != dawn::BlendOperation::Add ||
               mColorState->alphaBlend.srcFactor != dawn::BlendFactor::One ||
               mColorState->alphaBlend.dstFactor != dawn::BlendFactor::Zero ||
               mColorState->colorBlend.operation != dawn::BlendOperation::Add ||
               mColorState->colorBlend.srcFactor != dawn::BlendFactor::One ||
               mColorState->colorBlend.dstFactor != dawn::BlendFactor::Zero;
    }

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                           const RenderPipelineDescriptor* descriptor)
        : PipelineBase(device,
                       descriptor->layout,
                       dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment),
          mIndexFormat(descriptor->indexFormat),
          mInputState(descriptor->inputState),
          mPrimitiveTopology(descriptor->primitiveTopology),
          mHasDepthStencilAttachment(descriptor->depthStencilState != nullptr) {
        if (mHasDepthStencilAttachment) {
            mDepthStencilState = *descriptor->depthStencilState;
        } else {
            // These default values below are useful for backends to fill information.
            // The values indicate that depth and stencil test are disabled when backends
            // set their own depth stencil states/descriptors according to the values in
            // mDepthStencilState.
            mDepthStencilState.depthCompare = dawn::CompareFunction::Always;
            mDepthStencilState.depthWriteEnabled = false;
            mDepthStencilState.stencilBack.compare = dawn::CompareFunction::Always;
            mDepthStencilState.stencilBack.failOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilBack.depthFailOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilBack.passOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilFront.compare = dawn::CompareFunction::Always;
            mDepthStencilState.stencilFront.failOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilFront.depthFailOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilFront.passOp = dawn::StencilOperation::Keep;
            mDepthStencilState.stencilReadMask = 0xff;
            mDepthStencilState.stencilWriteMask = 0xff;
        }
        ExtractModuleData(dawn::ShaderStage::Vertex, descriptor->vertexStage->module);
        ExtractModuleData(dawn::ShaderStage::Fragment, descriptor->fragmentStage->module);

        for (uint32_t i = 0; i < descriptor->colorStateCount; ++i) {
            mColorAttachmentsSet.set(i);
            mColorStates[i] = *descriptor->colorStates[i];
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

    const ColorStateDescriptor* RenderPipelineBase::GetColorStateDescriptor(
        uint32_t attachmentSlot) {
        ASSERT(!IsError());
        ASSERT(attachmentSlot < mColorStates.size());
        return &mColorStates[attachmentSlot];
    }

    const DepthStencilStateDescriptor* RenderPipelineBase::GetDepthStencilStateDescriptor() {
        ASSERT(!IsError());
        return &mDepthStencilState;
    }

    dawn::IndexFormat RenderPipelineBase::GetIndexFormat() const {
        ASSERT(!IsError());
        return mIndexFormat;
    }

    InputStateBase* RenderPipelineBase::GetInputState() {
        ASSERT(!IsError());
        return mInputState.Get();
    }

    dawn::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
        ASSERT(!IsError());
        return mPrimitiveTopology;
    }

    std::bitset<kMaxColorAttachments> RenderPipelineBase::GetColorAttachmentsMask() const {
        ASSERT(!IsError());
        return mColorAttachmentsSet;
    }

    bool RenderPipelineBase::HasDepthStencilAttachment() const {
        ASSERT(!IsError());
        return mHasDepthStencilAttachment;
    }

    dawn::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(uint32_t attachment) const {
        ASSERT(!IsError());
        return mColorStates[attachment].format;
    }

    dawn::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
        ASSERT(!IsError());
        ASSERT(mHasDepthStencilAttachment);
        return mDepthStencilState.format;
    }

    bool RenderPipelineBase::IsCompatibleWith(const BeginRenderPassCmd* renderPass) const {
        ASSERT(!IsError());
        // TODO(cwallez@chromium.org): This is called on every SetPipeline command. Optimize it for
        // example by caching some "attachment compatibility" object that would make the
        // compatibility check a single pointer comparison.

        if (renderPass->colorAttachmentsSet != mColorAttachmentsSet) {
            return false;
        }

        for (uint32_t i : IterateBitSet(mColorAttachmentsSet)) {
            if (renderPass->colorAttachments[i].view->GetFormat() != mColorStates[i].format) {
                return false;
            }
        }

        if (renderPass->hasDepthStencilAttachment != mHasDepthStencilAttachment) {
            return false;
        }

        if (mHasDepthStencilAttachment &&
            (renderPass->depthStencilAttachment.view->GetFormat() != mDepthStencilState.format)) {
            return false;
        }

        return true;
    }

}  // namespace dawn_native
