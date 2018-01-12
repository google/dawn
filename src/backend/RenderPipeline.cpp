// Copyright 2017 The NXT Authors
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

#include "backend/RenderPipeline.h"

#include "backend/BlendState.h"
#include "backend/DepthStencilState.h"
#include "backend/Device.h"
#include "backend/InputState.h"
#include "backend/RenderPass.h"
#include "common/BitSetIterator.h"

namespace backend {

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(RenderPipelineBuilder* builder)
        : PipelineBase(builder),
          mDepthStencilState(std::move(builder->mDepthStencilState)),
          mIndexFormat(builder->mIndexFormat),
          mInputState(std::move(builder->mInputState)),
          mPrimitiveTopology(builder->mPrimitiveTopology),
          mBlendStates(builder->mBlendStates),
          mRenderPass(std::move(builder->mRenderPass)),
          mSubpass(builder->mSubpass) {
        if (GetStageMask() != (nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment)) {
            builder->HandleError("Render pipeline should have exactly a vertex and fragment stage");
            return;
        }

        // TODO(kainino@chromium.org): Need to verify the pipeline against its render subpass.

        if ((builder->GetStageInfo(nxt::ShaderStage::Vertex).module->GetUsedVertexAttributes() &
             ~mInputState->GetAttributesSetMask())
                .any()) {
            builder->HandleError("Pipeline vertex stage uses inputs not in the input state");
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

    nxt::IndexFormat RenderPipelineBase::GetIndexFormat() const {
        return mIndexFormat;
    }

    InputStateBase* RenderPipelineBase::GetInputState() {
        return mInputState.Get();
    }

    nxt::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
        return mPrimitiveTopology;
    }

    RenderPassBase* RenderPipelineBase::GetRenderPass() {
        return mRenderPass.Get();
    }

    uint32_t RenderPipelineBase::GetSubPass() {
        return mSubpass;
    }

    // RenderPipelineBuilder

    RenderPipelineBuilder::RenderPipelineBuilder(DeviceBase* device)
        : Builder(device), PipelineBuilder(this) {
    }

    RenderPipelineBase* RenderPipelineBuilder::GetResultImpl() {
        // TODO(cwallez@chromium.org): the layout should be required, and put the default objects in
        // the device
        if (!mInputState) {
            mInputState = mDevice->CreateInputStateBuilder()->GetResult();
            // Remove the external ref objects are created with
            mInputState->Release();
        }
        if (!mDepthStencilState) {
            mDepthStencilState = mDevice->CreateDepthStencilStateBuilder()->GetResult();
            // Remove the external ref objects are created with
            mDepthStencilState->Release();
        }
        if (!mRenderPass) {
            HandleError("Pipeline render pass not set");
            return nullptr;
        }
        const auto& subpassInfo = mRenderPass->GetSubpassInfo(mSubpass);
        if ((mBlendStatesSet | subpassInfo.colorAttachmentsSet) !=
            subpassInfo.colorAttachmentsSet) {
            HandleError("Blend state set on unset color attachment");
            return nullptr;
        }

        // Assign all color attachments without a blend state the default state
        // TODO(enga@google.com): Put the default objects in the device
        for (uint32_t attachmentSlot :
             IterateBitSet(subpassInfo.colorAttachmentsSet & ~mBlendStatesSet)) {
            mBlendStates[attachmentSlot] = mDevice->CreateBlendStateBuilder()->GetResult();
            // Remove the external ref objects are created with
            mBlendStates[attachmentSlot]->Release();
        }

        return mDevice->CreateRenderPipeline(this);
    }

    void RenderPipelineBuilder::SetColorAttachmentBlendState(uint32_t attachmentSlot,
                                                             BlendStateBase* blendState) {
        if (attachmentSlot > mBlendStates.size()) {
            HandleError("Attachment index out of bounds");
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
        mDepthStencilState = depthStencilState;
    }

    void RenderPipelineBuilder::SetIndexFormat(nxt::IndexFormat format) {
        mIndexFormat = format;
    }

    void RenderPipelineBuilder::SetInputState(InputStateBase* inputState) {
        mInputState = inputState;
    }

    void RenderPipelineBuilder::SetPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
        mPrimitiveTopology = primitiveTopology;
    }

    void RenderPipelineBuilder::SetSubpass(RenderPassBase* renderPass, uint32_t subpass) {
        mRenderPass = renderPass;
        mSubpass = subpass;
    }

}  // namespace backend
