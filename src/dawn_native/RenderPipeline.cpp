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

namespace dawn_native {

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(RenderPipelineBuilder* builder)
        : PipelineBase(builder->GetDevice(), builder),
          mDepthStencilState(std::move(builder->mDepthStencilState)),
          mIndexFormat(builder->mIndexFormat),
          mInputState(std::move(builder->mInputState)),
          mPrimitiveTopology(builder->mPrimitiveTopology),
          mBlendStates(builder->mBlendStates),
          mColorAttachmentsSet(builder->mColorAttachmentsSet),
          mColorAttachmentFormats(builder->mColorAttachmentFormats),
          mDepthStencilFormatSet(builder->mDepthStencilFormatSet),
          mDepthStencilFormat(builder->mDepthStencilFormat) {
        if (GetStageMask() != (dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment)) {
            builder->HandleError("Render pipeline should have exactly a vertex and fragment stage");
            return;
        }

        // TODO(kainino@chromium.org): Need to verify the pipeline against its render subpass.

        if ((builder->GetStageInfo(dawn::ShaderStage::Vertex).module->GetUsedVertexAttributes() &
             ~mInputState->GetAttributesSetMask())
                .any()) {
            builder->HandleError("Pipeline vertex stage uses inputs not in the input state");
            return;
        }

        // TODO(cwallez@chromium.org): Check against the shader module that the correct color
        // attachment are set?

        size_t attachmentCount = mColorAttachmentsSet.count();
        if (mDepthStencilFormatSet) {
            attachmentCount++;
        }

        if (attachmentCount == 0) {
            builder->HandleError("Should have at least one attachment");
            return;
        }
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
        return mDepthStencilFormatSet;
    }

    dawn::TextureFormat RenderPipelineBase::GetColorAttachmentFormat(uint32_t attachment) const {
        return mColorAttachmentFormats[attachment];
    }

    dawn::TextureFormat RenderPipelineBase::GetDepthStencilFormat() const {
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

        if (renderPass->HasDepthStencilAttachment() != mDepthStencilFormatSet) {
            return false;
        }

        if (mDepthStencilFormatSet &&
            (renderPass->GetDepthStencilAttachment().view->GetTexture()->GetFormat() !=
             mDepthStencilFormat)) {
            return false;
        }

        return true;
    }

    // RenderPipelineBuilder

    RenderPipelineBuilder::RenderPipelineBuilder(DeviceBase* device)
        : Builder(device), PipelineBuilder(this) {
    }

    RenderPipelineBase* RenderPipelineBuilder::GetResultImpl() {
        DeviceBase* device = GetDevice();
        // TODO(cwallez@chromium.org): the layout should be required, and put the default objects in
        // the device
        if (!mInputState) {
            auto builder = device->CreateInputStateBuilder();
            mInputState = builder->GetResult();
            // Remove the external ref objects are created with
            mInputState->Release();
            builder->Release();
        }
        if (!mDepthStencilState) {
            auto builder = device->CreateDepthStencilStateBuilder();
            mDepthStencilState = builder->GetResult();
            // Remove the external ref objects are created with
            mDepthStencilState->Release();
            builder->Release();
        }

        if ((mBlendStatesSet | mColorAttachmentsSet) != mColorAttachmentsSet) {
            HandleError("Blend state set on unset color attachment");
            return nullptr;
        }

        // Assign all color attachments without a blend state the default state
        // TODO(enga@google.com): Put the default objects in the device
        for (uint32_t attachmentSlot : IterateBitSet(mColorAttachmentsSet & ~mBlendStatesSet)) {
            mBlendStates[attachmentSlot] = device->CreateBlendStateBuilder()->GetResult();
            // Remove the external ref objects are created with
            mBlendStates[attachmentSlot]->Release();
        }

        return device->CreateRenderPipeline(this);
    }

    void RenderPipelineBuilder::SetColorAttachmentFormat(uint32_t attachmentSlot,
                                                         dawn::TextureFormat format) {
        if (attachmentSlot >= kMaxColorAttachments) {
            HandleError("Attachment index out of bounds");
            return;
        }

        mColorAttachmentsSet.set(attachmentSlot);
        mColorAttachmentFormats[attachmentSlot] = format;
    }

    void RenderPipelineBuilder::SetColorAttachmentBlendState(uint32_t attachmentSlot,
                                                             BlendStateBase* blendState) {
        if (attachmentSlot >= kMaxColorAttachments) {
            HandleError("Attachment index out of bounds");
            return;
        }
        if (blendState == nullptr) {
            HandleError("Blend state must not be null");
            return;
        }
        if (mBlendStatesSet[attachmentSlot]) {
            HandleError("Attachment blend state already set");
            return;
        }

        mBlendStatesSet.set(attachmentSlot);
        mBlendStates[attachmentSlot] = blendState;
    }

    void RenderPipelineBuilder::SetDepthStencilState(DepthStencilStateBase* depthStencilState) {
        if (depthStencilState == nullptr) {
            HandleError("Depth stencil state must not be null");
            return;
        }

        mDepthStencilState = depthStencilState;
    }

    void RenderPipelineBuilder::SetDepthStencilAttachmentFormat(dawn::TextureFormat format) {
        mDepthStencilFormatSet = true;
        mDepthStencilFormat = format;
    }

    void RenderPipelineBuilder::SetIndexFormat(dawn::IndexFormat format) {
        mIndexFormat = format;
    }

    void RenderPipelineBuilder::SetInputState(InputStateBase* inputState) {
        if (inputState == nullptr) {
            HandleError("Input state must not be null");
            return;
        }

        mInputState = inputState;
    }

    void RenderPipelineBuilder::SetPrimitiveTopology(dawn::PrimitiveTopology primitiveTopology) {
        mPrimitiveTopology = primitiveTopology;
    }

}  // namespace dawn_native
