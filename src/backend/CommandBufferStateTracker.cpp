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

#include "backend/Forward.h"
#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/Buffer.h"
#include "backend/Framebuffer.h"
#include "backend/InputState.h"
#include "backend/Pipeline.h"
#include "backend/PipelineLayout.h"
#include "backend/RenderPass.h"
#include "backend/Texture.h"
#include "common/Assert.h"
#include "common/BitSetIterator.h"

namespace backend {
    CommandBufferStateTracker::CommandBufferStateTracker(CommandBufferBuilder* builder)
        : builder(builder) {
    }

    bool CommandBufferStateTracker::HaveRenderPass() const {
        return currentRenderPass != nullptr;
    }

    bool CommandBufferStateTracker::ValidateCanCopy() const {
        if (currentRenderPass) {
            builder->HandleError("Copy cannot occur during a render pass");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanUseBufferAs(BufferBase* buffer, nxt::BufferUsageBit usage) const {
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            builder->HandleError("Buffer is not in the necessary usage");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanUseTextureAs(TextureBase* texture, nxt::TextureUsageBit usage) const {
        if (!TextureHasGuaranteedUsageBit(texture, usage)) {
            builder->HandleError("Texture is not in the necessary usage");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanDispatch() {
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE | // implicitly requires COMPUTE_PASS
            1 << VALIDATION_ASPECT_BIND_GROUPS;
        if ((requiredAspects & ~aspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        if (!aspects[VALIDATION_ASPECT_COMPUTE_PIPELINE]) {
            builder->HandleError("No active compute pipeline");
            return false;
        }
        // Compute the lazily computed aspects
        if (!RecomputeHaveAspectBindGroups()) {
            builder->HandleError("Bind group state not valid");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::ValidateCanDrawArrays() {
        // TODO(kainino@chromium.org): Check for a current render pass
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE | // implicitly requires RENDER_SUBPASS
            1 << VALIDATION_ASPECT_BIND_GROUPS |
            1 << VALIDATION_ASPECT_VERTEX_BUFFERS;
        if ((requiredAspects & ~aspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        return RevalidateCanDraw();
    }

    bool CommandBufferStateTracker::ValidateCanDrawElements() {
        // TODO(kainino@chromium.org): Check for a current render pass
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE |
            1 << VALIDATION_ASPECT_BIND_GROUPS |
            1 << VALIDATION_ASPECT_VERTEX_BUFFERS |
            1 << VALIDATION_ASPECT_INDEX_BUFFER;
        if ((requiredAspects & ~aspects).none()) {
            // Fast return-true path if everything is good
            return true;
        }

        if (!aspects[VALIDATION_ASPECT_INDEX_BUFFER]) {
            builder->HandleError("Cannot DrawElements without index buffer set");
            return false;
        }
        return RevalidateCanDraw();
    }

    bool CommandBufferStateTracker::ValidateEndCommandBuffer() const {
        if (currentRenderPass != nullptr) {
            builder->HandleError("Can't end command buffer with an active render pass");
            return false;
        }
        if (aspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            builder->HandleError("Can't end command buffer with an active compute pass");
            return false;
        }
        return true;
    }

    bool CommandBufferStateTracker::BeginComputePass() {
        if (currentRenderPass != nullptr) {
            builder->HandleError("Cannot begin a compute pass while a render pass is active");
            return false;
        }
        aspects.set(VALIDATION_ASPECT_COMPUTE_PASS);
        return true;
    }

    bool CommandBufferStateTracker::EndComputePass() {
        if (!aspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            builder->HandleError("Can't end a compute pass without beginning one");
            return false;
        }
        aspects.reset(VALIDATION_ASPECT_COMPUTE_PASS);
        UnsetPipeline();
        return true;
    }

    bool CommandBufferStateTracker::BeginSubpass() {
        if (currentRenderPass == nullptr) {
            builder->HandleError("Can't begin a subpass without an active render pass");
            return false;
        }
        if (aspects[VALIDATION_ASPECT_RENDER_SUBPASS]) {
            builder->HandleError("Can't begin a subpass without ending the previous subpass");
            return false;
        }
        if (currentSubpass >= currentRenderPass->GetSubpassCount()) {
            builder->HandleError("Can't begin a subpass beyond the last subpass");
            return false;
        }

        auto& subpassInfo = currentRenderPass->GetSubpassInfo(currentSubpass);
        for (auto location : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            auto attachmentSlot = subpassInfo.colorAttachments[location];
            auto* tv = currentFramebuffer->GetTextureView(attachmentSlot);
            // TODO(kainino@chromium.org): the TextureView can only be null
            // because of the null=backbuffer hack (null representing the
            // backbuffer). Once that hack is removed (once we have WSI)
            // this check isn't needed.
            if (tv == nullptr) {
                continue;
            }

            auto* texture = tv->GetTexture();
            if (!EnsureTextureUsage(texture, nxt::TextureUsageBit::OutputAttachment)) {
                builder->HandleError("Unable to ensure texture has OutputAttachment usage");
                return false;
            }
            texturesAttached.insert(texture);
        }

        aspects.set(VALIDATION_ASPECT_RENDER_SUBPASS);
        return true;
    };

    bool CommandBufferStateTracker::EndSubpass() {
        if (!aspects[VALIDATION_ASPECT_RENDER_SUBPASS]) {
            builder->HandleError("Can't end a subpass without beginning one");
            return false;
        }
        ASSERT(currentRenderPass != nullptr);

        auto& subpassInfo = currentRenderPass->GetSubpassInfo(currentSubpass);
        for (auto location : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            auto attachmentSlot = subpassInfo.colorAttachments[location];
            auto* tv = currentFramebuffer->GetTextureView(attachmentSlot);
            // TODO(kainino@chromium.org): the TextureView can only be null
            // because of the null=backbuffer hack (null representing the
            // backbuffer). Once that hack is removed (once we have WSI)
            // this check isn't needed.
            if (tv == nullptr) {
                continue;
            }

            auto* texture = tv->GetTexture();
            if (texture->IsFrozen()) {
                continue;
            }
        }
        // Everything in texturesAttached should be for the current render subpass.
        texturesAttached.clear();

        currentSubpass += 1;
        aspects.reset(VALIDATION_ASPECT_RENDER_SUBPASS);
        UnsetPipeline();
        return true;
    };

    bool CommandBufferStateTracker::BeginRenderPass(RenderPassBase* renderPass, FramebufferBase* framebuffer) {
        if (aspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
            builder->HandleError("Cannot begin a render pass while a compute pass is active");
            return false;
        }
        if (currentRenderPass != nullptr) {
            builder->HandleError("A render pass is already active");
            return false;
        }
        ASSERT(!aspects[VALIDATION_ASPECT_RENDER_SUBPASS]);
        if (!framebuffer->GetRenderPass()->IsCompatibleWith(renderPass)) {
            builder->HandleError("Framebuffer is incompatible with this render pass");
            return false;
        }

        currentRenderPass = renderPass;
        currentFramebuffer = framebuffer;
        currentSubpass = 0;

        return true;
    }

    bool CommandBufferStateTracker::EndRenderPass() {
        if (currentRenderPass == nullptr) {
            builder->HandleError("No render pass is currently active");
            return false;
        }
        if (aspects[VALIDATION_ASPECT_RENDER_SUBPASS]) {
            builder->HandleError("Can't end a render pass while a subpass is active");
            return false;
        }
        if (currentSubpass < currentRenderPass->GetSubpassCount() - 1) {
            builder->HandleError("Can't end a render pass before the last subpass");
            return false;
        }
        currentRenderPass = nullptr;
        currentFramebuffer = nullptr;

        return true;
    }

    bool CommandBufferStateTracker::SetPipeline(PipelineBase* pipeline) {
        PipelineLayoutBase* layout = pipeline->GetLayout();

        if (pipeline->IsCompute()) {
            if (!aspects[VALIDATION_ASPECT_COMPUTE_PASS]) {
                builder->HandleError("A compute pass must be active when a compute pipeline is set");
                return false;
            }
            if (currentRenderPass) {
                builder->HandleError("Can't use a compute pipeline while a render pass is active");
                return false;
            }
            aspects.set(VALIDATION_ASPECT_COMPUTE_PIPELINE);
        } else {
            if (!aspects[VALIDATION_ASPECT_RENDER_SUBPASS]) {
                builder->HandleError("A render subpass must be active when a render pipeline is set");
                return false;
            }
            if (!pipeline->GetRenderPass()->IsCompatibleWith(currentRenderPass)) {
                builder->HandleError("Pipeline is incompatible with this render pass");
                return false;
            }
            aspects.set(VALIDATION_ASPECT_RENDER_PIPELINE);
        }
        aspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
        bindgroupsSet = ~layout->GetBindGroupsLayoutMask();

        // Only bindgroups that were not the same layout in the last pipeline need to be set again.
        if (lastPipeline) {
            PipelineLayoutBase* lastLayout = lastPipeline->GetLayout();
            for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                if (lastLayout->GetBindGroupLayout(i) == layout->GetBindGroupLayout(i)) {
                    bindgroupsSet |= uint64_t(1) << i;
                } else {
                    break;
                }
            }
        }

        lastPipeline = pipeline;
        return true;
    }

    bool CommandBufferStateTracker::SetBindGroup(uint32_t index, BindGroupBase* bindgroup) {
        if (!ValidateBindGroupUsages(bindgroup)) {
            return false;
        }
        bindgroupsSet.set(index);
        bindgroups[index] = bindgroup;

        return true;
    }

    bool CommandBufferStateTracker::SetIndexBuffer(BufferBase* buffer) {
        if (!HavePipeline()) {
            builder->HandleError("Can't set the index buffer without a pipeline");
            return false;
        }

        auto usage = nxt::BufferUsageBit::Index;
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            builder->HandleError("Buffer needs the index usage bit to be guaranteed");
            return false;
        }

        aspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        return true;
    }

    bool CommandBufferStateTracker::SetVertexBuffer(uint32_t index, BufferBase* buffer) {
        if (!HavePipeline()) {
            builder->HandleError("Can't set vertex buffers without a pipeline");
            return false;
        }

        auto usage = nxt::BufferUsageBit::Vertex;
        if (!BufferHasGuaranteedUsageBit(buffer, usage)) {
            builder->HandleError("Buffer needs vertex usage bit to be guaranteed");
            return false;
        }

        inputsSet.set(index);
        return true;
    }

    bool CommandBufferStateTracker::TransitionBufferUsage(BufferBase* buffer, nxt::BufferUsageBit usage) {
        if (!buffer->IsTransitionPossible(usage)) {
            if (buffer->IsFrozen()) {
                builder->HandleError("Buffer transition not possible (usage is frozen)");
            } else if (!BufferBase::IsUsagePossible(buffer->GetAllowedUsage(), usage)) {
                builder->HandleError("Buffer transition not possible (usage not allowed)");
            } else {
                builder->HandleError("Buffer transition not possible");
            }
            return false;
        }

        mostRecentBufferUsages[buffer] = usage;
        buffersTransitioned.insert(buffer);
        return true;
    }

    bool CommandBufferStateTracker::TransitionTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage) {
        if (!IsExplicitTextureTransitionPossible(texture, usage)) {
            if (texture->IsFrozen()) {
                builder->HandleError("Texture transition not possible (usage is frozen)");
            } else if (!TextureBase::IsUsagePossible(texture->GetAllowedUsage(), usage)) {
                builder->HandleError("Texture transition not possible (usage not allowed)");
            } else if (texturesAttached.find(texture) != texturesAttached.end()) {
                builder->HandleError("Texture transition not possible (texture is in use as a framebuffer attachment)");
            } else {
                builder->HandleError("Texture transition not possible");
            }
            return false;
        }

        mostRecentTextureUsages[texture] = usage;
        texturesTransitioned.insert(texture);
        return true;
    }

    bool CommandBufferStateTracker::EnsureTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage) {
        if (texture->HasFrozenUsage(nxt::TextureUsageBit::OutputAttachment)) {
            return true;
        }
        if (!IsInternalTextureTransitionPossible(texture, usage)) {
            return false;
        }
        mostRecentTextureUsages[texture] = usage;
        texturesTransitioned.insert(texture);
        return true;
    }

    bool CommandBufferStateTracker::BufferHasGuaranteedUsageBit(BufferBase* buffer, nxt::BufferUsageBit usage) const {
        ASSERT(usage != nxt::BufferUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (buffer->HasFrozenUsage(usage)) {
            return true;
        }
        auto it = mostRecentBufferUsages.find(buffer);
        return it != mostRecentBufferUsages.end() && (it->second & usage);
    };

    bool CommandBufferStateTracker::TextureHasGuaranteedUsageBit(TextureBase* texture, nxt::TextureUsageBit usage) const {
        ASSERT(usage != nxt::TextureUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (texture->HasFrozenUsage(usage)) {
            return true;
        }
        auto it = mostRecentTextureUsages.find(texture);
        return it != mostRecentTextureUsages.end() && (it->second & usage);
    };

    bool CommandBufferStateTracker::IsInternalTextureTransitionPossible(TextureBase* texture, nxt::TextureUsageBit usage) const {
        ASSERT(usage != nxt::TextureUsageBit::None && nxt::HasZeroOrOneBits(usage));
        if (texturesAttached.find(texture) != texturesAttached.end()) {
            return false;
        }
        return texture->IsTransitionPossible(usage);
    };

    bool CommandBufferStateTracker::IsExplicitTextureTransitionPossible(TextureBase* texture, nxt::TextureUsageBit usage) const {
        const nxt::TextureUsageBit attachmentUsages =
            nxt::TextureUsageBit::OutputAttachment;
        if (usage & attachmentUsages) {
            return false;
        }
        return IsInternalTextureTransitionPossible(texture, usage);
    }

    bool CommandBufferStateTracker::RecomputeHaveAspectBindGroups() {
        if (aspects[VALIDATION_ASPECT_BIND_GROUPS]) {
            return true;
        }
        // Assumes we have a pipeline already
        if (!bindgroupsSet.all()) {
            return false;
        }
        for (size_t i = 0; i < bindgroups.size(); ++i) {
            if (auto* bindgroup = bindgroups[i]) {
                // TODO(kainino@chromium.org): bind group compatibility
                if (bindgroup->GetLayout() != lastPipeline->GetLayout()->GetBindGroupLayout(i)) {
                    return false;
                }
            }
        }
        aspects.set(VALIDATION_ASPECT_BIND_GROUPS);
        return true;
    }

    bool CommandBufferStateTracker::RecomputeHaveAspectVertexBuffers() {
        if (aspects[VALIDATION_ASPECT_VERTEX_BUFFERS]) {
            return true;
        }
        // Assumes we have a pipeline already
        auto requiredInputs = lastPipeline->GetInputState()->GetInputsSetMask();
        if ((inputsSet & ~requiredInputs).none()) {
            aspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
            return true;
        }
        return false;
    }

    bool CommandBufferStateTracker::HavePipeline() const {
        constexpr ValidationAspects pipelineAspects =
            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE |
            1 << VALIDATION_ASPECT_RENDER_PIPELINE;
        return (aspects & pipelineAspects).any();
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
                case nxt::BindingType::StorageBuffer:
                    {
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
                            builder->HandleError("Can't guarantee buffer usage needed by bind group");
                            return false;
                        }
                    }
                    break;
                case nxt::BindingType::SampledTexture:
                    {
                        auto requiredUsage = nxt::TextureUsageBit::Sampled;

                        auto texture = group->GetBindingAsTextureView(i)->GetTexture();
                        if (!TextureHasGuaranteedUsageBit(texture, requiredUsage)) {
                            builder->HandleError("Can't guarantee texture usage needed by bind group");
                            return false;
                        }
                    }
                    break;
                case nxt::BindingType::Sampler:
                    continue;
            }
        }
        return true;
    };

    bool CommandBufferStateTracker::RevalidateCanDraw() {
        if (!aspects[VALIDATION_ASPECT_RENDER_PIPELINE]) {
            builder->HandleError("No active render pipeline");
            return false;
        }
        // Compute the lazily computed aspects
        if (!RecomputeHaveAspectBindGroups()) {
            builder->HandleError("Bind group state not valid");
            return false;
        }
        if (!RecomputeHaveAspectVertexBuffers()) {
            builder->HandleError("Some vertex buffers are not set");
            return false;
        }
        return true;
    }

    void CommandBufferStateTracker::UnsetPipeline() {
        constexpr ValidationAspects pipelineDependentAspects =
            1 << VALIDATION_ASPECT_RENDER_PIPELINE |
            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE |
            1 << VALIDATION_ASPECT_BIND_GROUPS |
            1 << VALIDATION_ASPECT_VERTEX_BUFFERS |
            1 << VALIDATION_ASPECT_INDEX_BUFFER;
        aspects &= ~pipelineDependentAspects;
        bindgroups.fill(nullptr);
    }
}
