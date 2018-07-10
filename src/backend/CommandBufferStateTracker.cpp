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
#include "backend/RenderPassDescriptor.h"
#include "backend/RenderPipeline.h"
#include "backend/Texture.h"
#include "common/Assert.h"
#include "common/BitSetIterator.h"

namespace backend {

    CommandBufferStateTracker::CommandBufferStateTracker(CommandBufferBuilder* mBuilder)
        : mBuilder(mBuilder) {
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
            1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        if (!mAspects[VALIDATION_ASPECT_PIPELINE]) {
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
        constexpr ValidationAspects requiredAspects = 1 << VALIDATION_ASPECT_PIPELINE |
                                                      1 << VALIDATION_ASPECT_BIND_GROUPS |
                                                      1 << VALIDATION_ASPECT_VERTEX_BUFFERS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        return RevalidateCanDraw();
    }

    bool CommandBufferStateTracker::ValidateCanDrawElements() {
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
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

    bool CommandBufferStateTracker::BeginRenderPass(RenderPassDescriptorBase* info) {
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

    void CommandBufferStateTracker::EndPass() {
        // Everything in mTexturesAttached should be for the current render pass.
        mTexturesAttached.clear();

        mInputsSet.reset();
        mAspects = 0;
        mBindgroups.fill(nullptr);
    }

    bool CommandBufferStateTracker::SetComputePipeline(ComputePipelineBase* pipeline) {
        SetPipelineCommon(pipeline);
        return true;
    }

    bool CommandBufferStateTracker::SetRenderPipeline(RenderPipelineBase* pipeline) {
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
                auto* pipelineBGL = mLastPipeline->GetLayout()->GetBindGroupLayout(i);
                if (pipelineBGL && bindgroup->GetLayout() != pipelineBGL) {
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
        return mAspects[VALIDATION_ASPECT_PIPELINE];
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
        if (!mAspects[VALIDATION_ASPECT_PIPELINE]) {
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

        mAspects.set(VALIDATION_ASPECT_PIPELINE);

        mAspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
        mAspects.reset(VALIDATION_ASPECT_VERTEX_BUFFERS);
        // Reset bindgroups but mark unused bindgroups as valid
        mBindgroupsSet = ~layout->GetBindGroupLayoutsMask();

        // Only bindgroups that were not the same layout in the last pipeline need to be set again.
        if (mLastPipeline) {
            mBindgroupsSet |= layout->InheritedGroupsMask(mLastPipeline->GetLayout());
        }

        mLastPipeline = pipeline;
    }

}  // namespace backend
