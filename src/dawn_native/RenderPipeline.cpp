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
#include "dawn_native/BlendState.h"
#include "dawn_native/DepthStencilState.h"
#include "dawn_native/Device.h"
#include "dawn_native/InputState.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/Texture.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {
    // Helper functions
    namespace {

        MaybeError ValidatePipelineStageDescriptor(const PipelineStageDescriptor* descriptor,
                                                   const PipelineLayoutBase* layout,
                                                   dawn::ShaderStage stage) {
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

        MaybeError ValidateAttachmentsStateDescriptor(
            const AttachmentsStateDescriptor* descriptor) {
            if (descriptor->numColorAttachments > kMaxColorAttachments) {
                return DAWN_VALIDATION_ERROR("Color attachments number exceeds maximum");
            }

            if (descriptor->numColorAttachments == 0 && !descriptor->hasDepthStencilAttachment) {
                return DAWN_VALIDATION_ERROR("Should have at least one attachment");
            }

            if (descriptor->hasDepthStencilAttachment) {
                dawn::TextureFormat format = descriptor->depthStencilAttachment->format;
                DAWN_TRY(ValidateTextureFormat(format));

                if (!IsDepthStencilRenderableTextureFormat(format)) {
                    return DAWN_VALIDATION_ERROR(
                        "Depth stencil format must be depth-stencil renderable");
                }
            }

            for (uint32_t i = 0; i < descriptor->numColorAttachments; ++i) {
                dawn::TextureFormat format = descriptor->colorAttachments[i].format;
                DAWN_TRY(ValidateTextureFormat(format));

                if (!IsColorRenderableTextureFormat(format)) {
                    return DAWN_VALIDATION_ERROR("Color format must be color renderable");
                }
            }
            return {};
        }

    }  // namespace

    MaybeError ValidateRenderPipelineDescriptor(DeviceBase* device,
                                                const RenderPipelineDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        if (descriptor->layout == nullptr) {
            return DAWN_VALIDATION_ERROR("Layout must not be null");
        }

        if (descriptor->inputState == nullptr) {
            return DAWN_VALIDATION_ERROR("Input state must not be null");
        }

        if (descriptor->depthStencilState == nullptr) {
            return DAWN_VALIDATION_ERROR("Depth stencil state must not be null");
        }

        for (uint32_t i = 0; i < descriptor->numBlendStates; ++i) {
            if (descriptor->blendStates[i] == nullptr) {
                return DAWN_VALIDATION_ERROR("Blend state must not be null");
            }
        }

        DAWN_TRY(ValidateIndexFormat(descriptor->indexFormat));
        DAWN_TRY(ValidatePrimitiveTopology(descriptor->primitiveTopology));
        DAWN_TRY(ValidatePipelineStageDescriptor(descriptor->vertexStage, descriptor->layout,
                                                 dawn::ShaderStage::Vertex));
        DAWN_TRY(ValidatePipelineStageDescriptor(descriptor->fragmentStage, descriptor->layout,
                                                 dawn::ShaderStage::Fragment));
        DAWN_TRY(ValidateAttachmentsStateDescriptor(descriptor->attachmentsState));

        if ((descriptor->vertexStage->module->GetUsedVertexAttributes() &
             ~descriptor->inputState->GetAttributesSetMask())
                .any()) {
            return DAWN_VALIDATION_ERROR(
                "Pipeline vertex stage uses inputs not in the input state");
        }

        if (descriptor->sampleCount != 1) {
            return DAWN_VALIDATION_ERROR("Sample count must be one");
        }

        if (descriptor->numBlendStates > kMaxColorAttachments) {
            return DAWN_VALIDATION_ERROR("Blend states number exceeds maximum");
        }

        if (descriptor->attachmentsState->numColorAttachments != descriptor->numBlendStates) {
            return DAWN_VALIDATION_ERROR("Each color attachment should have blend state");
        }

        // TODO: validate depth stencil state
        return {};
    }

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(DeviceBase* device,
                                           const RenderPipelineDescriptor* descriptor)
        : PipelineBase(device,
                       descriptor->layout,
                       dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment),
          mDepthStencilState(descriptor->depthStencilState),
          mIndexFormat(descriptor->indexFormat),
          mInputState(descriptor->inputState),
          mPrimitiveTopology(descriptor->primitiveTopology),
          mHasDepthStencilAttachment(descriptor->attachmentsState->hasDepthStencilAttachment) {
        if (mHasDepthStencilAttachment) {
            mDepthStencilFormat = descriptor->attachmentsState->depthStencilAttachment->format;
        }
        ExtractModuleData(dawn::ShaderStage::Vertex, descriptor->vertexStage->module);
        ExtractModuleData(dawn::ShaderStage::Fragment, descriptor->fragmentStage->module);

        for (uint32_t i = 0; i < descriptor->attachmentsState->numColorAttachments; ++i) {
            mColorAttachmentsSet.set(i);
            mBlendStates[i] = descriptor->blendStates[i];
            mColorAttachmentFormats[i] = descriptor->attachmentsState->colorAttachments[i].format;
        }

        // TODO(cwallez@chromium.org): Check against the shader module that the correct color
        // attachment are set?
    }

    BlendStateBase* RenderPipelineBase::GetBlendState(uint32_t attachmentSlot) {
        ASSERT(attachmentSlot < mBlendStates.size());
        return mBlendStates[attachmentSlot].Get();
    }

    DepthStencilStateBase* RenderPipelineBase::GetDepthStencilState() {
        return mDepthStencilState.Get();
    }

    dawn::IndexFormat RenderPipelineBase::GetIndexFormat() const {
        return mIndexFormat;
    }

    InputStateBase* RenderPipelineBase::GetInputState() {
        return mInputState.Get();
    }

    dawn::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
        return mPrimitiveTopology;
    }

    std::bitset<kMaxColorAttachments> RenderPipelineBase::GetColorAttachmentsMask() const {
        return mColorAttachmentsSet;
    }

    bool RenderPipelineBase::HasDepthStencilAttachment() const {
        return mHasDepthStencilAttachment;
    }

    dawn::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(uint32_t attachment) const {
        return mColorAttachmentFormats[attachment];
    }

    dawn::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
        ASSERT(mHasDepthStencilAttachment);
        return mDepthStencilFormat;
    }

    bool RenderPipelineBase::IsCompatibleWith(const RenderPassDescriptorBase* renderPass) const {
        // TODO(cwallez@chromium.org): This is called on every SetPipeline command. Optimize it for
        // example by caching some "attachment compatibility" object that would make the
        // compatibility check a single pointer comparison.

        if (renderPass->GetColorAttachmentMask() != mColorAttachmentsSet) {
            return false;
        }

        for (uint32_t i : IterateBitSet(mColorAttachmentsSet)) {
            if (renderPass->GetColorAttachment(i).view->GetTexture()->GetFormat() !=
                mColorAttachmentFormats[i]) {
                return false;
            }
        }

        if (renderPass->HasDepthStencilAttachment() != mHasDepthStencilAttachment) {
            return false;
        }

        if (mHasDepthStencilAttachment &&
            (renderPass->GetDepthStencilAttachment().view->GetTexture()->GetFormat() !=
             mDepthStencilFormat)) {
            return false;
        }

        return true;
    }

}  // namespace dawn_native
