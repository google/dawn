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

#include "backend/CommandBufferStateTracker.h"

#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/Buffer.h"
#include "backend/ComputePipeline.h"
#include "backend/Forward.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/RenderPassInfo.h"
#include "backend/RenderPipeline.h"
#include "backend/Texture.h"
#include "common/Assert.h"
#include "common/BitSetIterator.h"

namespace backend {
    CommandBufferStateTracker::CommandBufferStateTracker(CommandBufferBuilder* mBuilder)
        : mBuilder(mBuilder) {
    }

    bool CommandBufferStateTracker::HaveRenderPass() const {
        return mCurrentRenderPass != nullptr;
    }

    bool CommandBufferStateTracker::ValidateCanCopy() const {
        if (mCurrentRenderPass) {
            mBuilder->HandleError("Copy cannot occur during a render pass");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanUseBufferAs(BufferBase* buffer,
                                                           nxt::BufferUsageBit usage) const {
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            mBuilder->HandleError("Buffer is not in the necessary usage");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanUseTextureAs(TextureBase* texture,
                                                            nxt::TextureUsageBit usage) const {
        if (!TextureHasGuaranteedUsageBit(texture, usage)) {
            mBuilder->HandleError("Texture is not in the necessary usage");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanDispatch() {
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE |  // implicitly requires COMPUTE_PASS
            1 << VALIDATION_ASPECT_BIND_GROUPS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        if (!mAspects[VALIDATION_ASPECT_COMPUTE_PIPELINE]) {
            mBuilder->HandleError("No active compute pipeline");
            return false;
        }
        // Compute the lazily computed mAspects
        if (!RecomputeHaveAspectBindGroups()) {
            mBuilder->HandleError("Bind group state not valid");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanDrawArrays() {
        // TODO(kainino@chromium.org): Check for a current render pass
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE |  // implicitly requires RENDER_PASS
            1 << VALIDATION_ASPECT_BIND_GROUPS | 1 << VALIDATION_ASPECT_VERTEX_BUFFERS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        return RevalidateCanDraw();
    }

    bool CommandBufferStateTracker::ValidateCanDrawElements() {
        // TODO(kainino@chromium.org): Check for a current render pass
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
            1 << VALIDATION_ASPECT_VERTEX_BUFFERS | 1 << VALIDATION_ASPECT_INDEX_BUFFER;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        if (!mAspects[VALIDATION_ASPECT_INDEX_BUFFER]) {
            mBuilder->HandleError("Cannot DrawElements without index buffer set");
            return false;
        }
        return RevalidateCanDraw();
    }

    bool CommandBufferStateTracker::ValidateEndCommandBuffer() const {
        if (mCurrentRenderPass != nullptr) {
            mBuilder->HandleError("Can't end command buffer with an active render pass");
            return false;
        }
        if (mAspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            mBuilder->HandleError("Can't end command buffer with an active compute pass");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateSetPushConstants(nxt::ShaderStageBit stages) {
        if (mAspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            if (stages & ~nxt::ShaderStageBit::Compute) {
                mBuilder->HandleError(
                    "SetPushConstants stage must be compute or 0 in compute passes");
                return false;
            }
        } else if (mAspects[VALIDATION_ASPECT_RENDER_PASS]) {
            if (stages & ~(nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment)) {
                mBuilder->HandleError(
                    "SetPushConstants stage must be a subset if (vertex|fragment) in render "
                    "passes");
                return false;
            }
        } else {
            mBuilder->HandleError(
                "PushConstants must be set in either compute passes or render passes");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::BeginComputePass() {
        if (mCurrentRenderPass != nullptr) {
            mBuilder->HandleError("Cannot begin a compute pass while a render pass is active");
            return false;
        }
        mAspects.set(VALIDATION_ASPECT_COMPUTE_PASS);
        return true;
    }

    bool CommandBufferStateTracker::EndComputePass() {
        if (!mAspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            mBuilder->HandleError("Can't end a compute pass without beginning one");
            return false;
        }
        mAspects.reset(VALIDATION_ASPECT_COMPUTE_PASS);
        UnsetPipeline();
        return true;
    }

    bool CommandBufferStateTracker::BeginRenderPass(RenderPassInfoBase* info) {
        if (mAspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            mBuilder->HandleError("Cannot begin a render pass while a compute pass is active");
            return false;
        }
        if (mCurrentRenderPass != nullptr) {
            mBuilder->HandleError("A render pass is already active");
            return false;
        }

        mCurrentRenderPass = info;
        mAspects.set(VALIDATION_ASPECT_RENDER_PASS);

        for (uint32_t i : IterateBitSet(info->GetColorAttachmentMask())) {
            TextureBase* texture = info->GetColorAttachment(i).view->GetTexture();
            if (!EnsureTextureUsage(texture, nxt::TextureUsageBit::OutputAttachment)) {
                mBuilder->HandleError("Unable to ensure texture has OutputAttachment usage");
                return false;
            }
            mTexturesAttached.insert(texture);
        }

        if (info->HasDepthStencilAttachment()) {
            TextureBase* texture = info->GetDepthStencilAttachment().view->GetTexture();
            if (!EnsureTextureUsage(texture, nxt::TextureUsageBit::OutputAttachment)) {
                mBuilder->HandleError("Unable to ensure texture has OutputAttachment usage");
                return false;
            }
            mTexturesAttached.insert(texture);
        }

        return true;
    }

    bool CommandBufferStateTracker::EndRenderPass() {
        if (mCurrentRenderPass == nullptr) {
            mBuilder->HandleError("No render pass is currently active");
            return false;
        }

        // Everything in mTexturesAttached should be for the current render pass.
        mTexturesAttached.clear();

        mInputsSet.reset();
        UnsetPipeline();

        mAspects.reset(VALIDATION_ASPECT_RENDER_PASS);
        mCurrentRenderPass = nullptr;

        return true;
    }

    bool CommandBufferStateTracker::SetComputePipeline(ComputePipelineBase* pipeline) {
        if (!mAspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            mBuilder->HandleError("A compute pass must be active when a compute pipeline is set");
            return false;
        }
        if (mCurrentRenderPass) {
            mBuilder->HandleError("Can't use a compute pipeline while a render pass is active");
            return false;
        }

        mAspects.set(VALIDATION_ASPECT_COMPUTE_PIPELINE);
        SetPipelineCommon(pipeline);
        return true;
    }

    bool CommandBufferStateTracker::SetRenderPipeline(RenderPipelineBase* pipeline) {
        if (!mAspects[VALIDATION_ASPECT_RENDER_PASS]) {
            mBuilder->HandleError("A render pass must be active when a render pipeline is set");
            return false;
        }
        if (!pipeline->IsCompatibleWith(mCurrentRenderPass)) {
            mBuilder->HandleError("Pipeline is incompatible with this render pass");
            return false;
        }

        mAspects.set(VALIDATION_ASPECT_RENDER_PIPELINE);
        mLastRenderPipeline = pipeline;
        SetPipelineCommon(pipeline);
        return true;
    }

    bool CommandBufferStateTracker::SetBindGroup(uint32_t index, BindGroupBase* bindgroup) {
        if (!ValidateBindGroupUsages(bindgroup)) {
            return false;
        }
        mBindgroupsSet.set(index);
        mBindgroups[index] = bindgroup;

        return true;
    }

    bool CommandBufferStateTracker::SetIndexBuffer(BufferBase* buffer) {
        if (!HavePipeline()) {
            mBuilder->HandleError("Can't set the index buffer without a pipeline");
            return false;
        }

        auto usage = nxt::BufferUsageBit::Index;
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            mBuilder->HandleError("Buffer needs the index usage bit to be guaranteed");
            return false;
        }

        mAspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        return true;
    }

    bool CommandBufferStateTracker::SetVertexBuffer(uint32_t index, BufferBase* buffer) {
        if (!HavePipeline()) {
            mBuilder->HandleError("Can't set vertex buffers without a pipeline");
            return false;
        }

        auto usage = nxt::BufferUsageBit::Vertex;
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            mBuilder->HandleError("Buffer needs vertex usage bit to be guaranteed");
            return false;
        }

        mInputsSet.set(index);
        return true;
    }

    bool CommandBufferStateTracker::TransitionBufferUsage(BufferBase* buffer,
                                                          nxt::BufferUsageBit usage) {
        if (!buffer->IsTransitionPossible(usage)) {
            if (buffer->IsFrozen()) {
                mBuilder->HandleError("Buffer transition not possible (usage is frozen)");
            } else if (!BufferBase::IsUsagePossible(buffer->GetAllowedUsage(), usage)) {
                mBuilder->HandleError("Buffer transition not possible (usage not allowed)");
            } else {
                mBuilder->HandleError("Buffer transition not possible");
            }
            return false;
        }

        mMostRecentBufferUsages[buffer] = usage;
        mBuffersTransitioned.insert(buffer);
        return true;
    }

    bool CommandBufferStateTracker::TransitionTextureUsage(TextureBase* texture,
                                                           nxt::TextureUsageBit usage) {
        if (!IsExplicitTextureTransitionPossible(texture, usage)) {
            if (texture->IsFrozen()) {
                mBuilder->HandleError("Texture transition not possible (usage is frozen)");
            } else if (!TextureBase::IsUsagePossible(texture->GetAllowedUsage(), usage)) {
                mBuilder->HandleError("Texture transition not possible (usage not allowed)");
            } else if (mTexturesAttached.find(texture) != mTexturesAttached.end()) {
                mBuilder->HandleError(
                    "Texture transition not possible (texture is in use as a framebuffer "
                    "attachment)");
            } else {
                mBuilder->HandleError("Texture transition not possible");
            }
            return false;
        }

        mMostRecentTextureUsages[texture] = usage;
        mTexturesTransitioned.insert(texture);
        return true;
    }

    bool CommandBufferStateTracker::EnsureTextureUsage(TextureBase* texture,
                                                       nxt::TextureUsageBit usage) {
        if (texture->HasFrozenUsage(usage)) {
            return true;
        }
        if (!IsInternalTextureTransitionPossible(texture, usage)) {
            return false;
        }
        mMostRecentTextureUsages[texture] = usage;
        mTexturesTransitioned.insert(texture);
        return true;
    }

    bool CommandBufferStateTracker::BufferHasGuaranteedUsageBit(BufferBase* buffer,
                                                                nxt::BufferUsageBit usage) const {
        ASSERT(usage != nxt::BufferUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (buffer->HasFrozenUsage(usage)) {
            return true;
        }
        auto it = mMostRecentBufferUsages.find(buffer);
        return it != mMostRecentBufferUsages.end() && (it->second & usage);
    }

    bool CommandBufferStateTracker::TextureHasGuaranteedUsageBit(TextureBase* texture,
                                                                 nxt::TextureUsageBit usage) const {
        ASSERT(usage != nxt::TextureUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (texture->HasFrozenUsage(usage)) {
            return true;
        }
        auto it = mMostRecentTextureUsages.find(texture);
        return it != mMostRecentTextureUsages.end() && (it->second & usage);
    }

    bool CommandBufferStateTracker::IsInternalTextureTransitionPossible(
        TextureBase* texture,
        nxt::TextureUsageBit usage) const {
        ASSERT(usage != nxt::TextureUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (mTexturesAttached.find(texture) != mTexturesAttached.end()) {
            return false;
        }
        return texture->IsTransitionPossible(usage);
    }

    bool CommandBufferStateTracker::IsExplicitTextureTransitionPossible(
        TextureBase* texture,
        nxt::TextureUsageBit usage) const {
        const nxt::TextureUsageBit attachmentUsages = nxt::TextureUsageBit::OutputAttachment;
        if (usage & attachmentUsages) {
            return false;
        }
        return IsInternalTextureTransitionPossible(texture, usage);
    }

    bool CommandBufferStateTracker::RecomputeHaveAspectBindGroups() {
        if (mAspects[VALIDATION_ASPECT_BIND_GROUPS]) {
            return true;
        }
        // Assumes we have a pipeline already
        if (!mBindgroupsSet.all()) {
            return false;
        }
        for (size_t i = 0; i < mBindgroups.size(); ++i) {
            if (auto* bindgroup = mBindgroups[i]) {
                // TODO(kainino@chromium.org): bind group compatibility
                if (bindgroup->GetLayout() != mLastPipeline->GetLayout()->GetBindGroupLayout(i)) {
                    return false;
                }
            }
        }
        mAspects.set(VALIDATION_ASPECT_BIND_GROUPS);
        return true;
    }

    bool CommandBufferStateTracker::RecomputeHaveAspectVertexBuffers() {
        if (mAspects[VALIDATION_ASPECT_VERTEX_BUFFERS]) {
            return true;
        }
        // Assumes we have a pipeline already
        auto requiredInputs = mLastRenderPipeline->GetInputState()->GetInputsSetMask();
        if ((mInputsSet & requiredInputs) == requiredInputs) {
            mAspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
            return true;
        }
        return false;
    }

    bool CommandBufferStateTracker::HavePipeline() const {
        constexpr ValidationAspects pipelineAspects =
            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE | 1 << VALIDATION_ASPECT_RENDER_PIPELINE;
        return (mAspects & pipelineAspects).any();
    }

    bool CommandBufferStateTracker::ValidateBindGroupUsages(BindGroupBase* group) const {
        const auto& layoutInfo = group->GetLayout()->GetBindingInfo();
        for (size_t i = 0; i < kMaxBindingsPerGroup; ++i) {
            if (!layoutInfo.mask[i]) {
                continue;
            }

            nxt::BindingType type = layoutInfo.types[i];
            switch (type) {
                case nxt::BindingType::UniformBuffer:
                case nxt::BindingType::StorageBuffer: {
                    nxt::BufferUsageBit requiredUsage = nxt::BufferUsageBit::None;
                    switch (type) {
                        case nxt::BindingType::UniformBuffer:
                            requiredUsage = nxt::BufferUsageBit::Uniform;
                            break;

                        case nxt::BindingType::StorageBuffer:
                            requiredUsage = nxt::BufferUsageBit::Storage;
                            break;

                        default:
                            UNREACHABLE();
                    }

                    auto buffer = group->GetBindingAsBufferView(i)->GetBuffer();
                    if (!BufferHasGuaranteedUsageBit(buffer, requiredUsage)) {
                        mBuilder->HandleError("Can't guarantee buffer usage needed by bind group");
                        return false;
                    }
                } break;
                case nxt::BindingType::SampledTexture: {
                    auto requiredUsage = nxt::TextureUsageBit::Sampled;

                    auto texture = group->GetBindingAsTextureView(i)->GetTexture();
                    if (!TextureHasGuaranteedUsageBit(texture, requiredUsage)) {
                        mBuilder->HandleError("Can't guarantee texture usage needed by bind group");
                        return false;
                    }
                } break;
                case nxt::BindingType::Sampler:
                    continue;
            }
        }
        return true;
    }

    bool CommandBufferStateTracker::RevalidateCanDraw() {
        if (!mAspects[VALIDATION_ASPECT_RENDER_PIPELINE]) {
            mBuilder->HandleError("No active render pipeline");
            return false;
        }
        // Compute the lazily computed mAspects
        if (!RecomputeHaveAspectBindGroups()) {
            mBuilder->HandleError("Bind group state not valid");
            return false;
        }
        if (!RecomputeHaveAspectVertexBuffers()) {
            mBuilder->HandleError("Some vertex buffers are not set");
            return false;
        }
        return true;
    }

    void CommandBufferStateTracker::SetPipelineCommon(PipelineBase* pipeline) {
        PipelineLayoutBase* layout = pipeline->GetLayout();

        mAspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
        mAspects.reset(VALIDATION_ASPECT_VERTEX_BUFFERS);
        // Reset bindgroups but mark unused bindgroups as valid
        mBindgroupsSet = ~layout->GetBindGroupsLayoutMask();

        // Only bindgroups that were not the same layout in the last pipeline need to be set again.
        if (mLastPipeline) {
            mBindgroupsSet |= layout->InheritedGroupsMask(mLastPipeline->GetLayout());
        }

        mLastPipeline = pipeline;
    }

    void CommandBufferStateTracker::UnsetPipeline() {
        constexpr ValidationAspects pipelineDependentAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE | 1 << VALIDATION_ASPECT_COMPUTE_PIPELINE |
            1 << VALIDATION_ASPECT_BIND_GROUPS | 1 << VALIDATION_ASPECT_VERTEX_BUFFERS |
            1 << VALIDATION_ASPECT_INDEX_BUFFER;
        mAspects &= ~pipelineDependentAspects;
        mBindgroups.fill(nullptr);
    }
}  // namespace backend
