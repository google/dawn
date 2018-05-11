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

#ifndef BACKEND_COMMANDBUFFERSTATETRACKER_H
#define BACKEND_COMMANDBUFFERSTATETRACKER_H

#include "backend/CommandBuffer.h"
#include "common/Constants.h"

#include <array>
#include <bitset>
#include <map>
#include <set>

namespace backend {
    class CommandBufferStateTracker {
      public:
        explicit CommandBufferStateTracker(CommandBufferBuilder* builder);

        // Non-state-modifying validation functions
        bool HaveRenderPass() const;
        bool ValidateCanCopy() const;
        bool ValidateCanUseBufferAs(BufferBase* buffer, nxt::BufferUsageBit usage) const;
        bool ValidateCanUseTextureAs(TextureBase* texture, nxt::TextureUsageBit usage) const;
        bool ValidateCanDispatch();
        bool ValidateCanDrawArrays();
        bool ValidateCanDrawElements();
        bool ValidateEndCommandBuffer() const;
        bool ValidateSetPushConstants(nxt::ShaderStageBit stages);

        // State-modifying methods
        bool BeginComputePass();
        bool EndComputePass();
        bool BeginRenderPass(RenderPassDescriptorBase* info);
        bool EndRenderPass();
        bool SetComputePipeline(ComputePipelineBase* pipeline);
        bool SetRenderPipeline(RenderPipelineBase* pipeline);
        bool SetBindGroup(uint32_t index, BindGroupBase* bindgroup);
        bool SetIndexBuffer(BufferBase* buffer);
        bool SetVertexBuffer(uint32_t index, BufferBase* buffer);
        bool TransitionBufferUsage(BufferBase* buffer, nxt::BufferUsageBit usage);
        bool TransitionTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage);
        bool EnsureTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage);

        // These collections are copied to the CommandBuffer at build time. These pointers will
        // remain valid since they are referenced by the bind groups which are referenced by this
        // command buffer.
        std::set<BufferBase*> mBuffersTransitioned;
        std::set<TextureBase*> mTexturesTransitioned;
        std::set<TextureBase*> mTexturesAttached;

      private:
        enum ValidationAspect {
            VALIDATION_ASPECT_RENDER_PIPELINE,
            VALIDATION_ASPECT_COMPUTE_PIPELINE,
            VALIDATION_ASPECT_BIND_GROUPS,
            VALIDATION_ASPECT_VERTEX_BUFFERS,
            VALIDATION_ASPECT_INDEX_BUFFER,
            VALIDATION_ASPECT_RENDER_PASS,
            VALIDATION_ASPECT_COMPUTE_PASS,

            VALIDATION_ASPECT_COUNT
        };
        using ValidationAspects = std::bitset<VALIDATION_ASPECT_COUNT>;

        // Usage helper functions
        bool BufferHasGuaranteedUsageBit(BufferBase* buffer, nxt::BufferUsageBit usage) const;
        bool TextureHasGuaranteedUsageBit(TextureBase* texture, nxt::TextureUsageBit usage) const;
        bool IsInternalTextureTransitionPossible(TextureBase* texture,
                                                 nxt::TextureUsageBit usage) const;
        bool IsExplicitTextureTransitionPossible(TextureBase* texture,
                                                 nxt::TextureUsageBit usage) const;

        // Queries for lazily evaluated aspects
        bool RecomputeHaveAspectBindGroups();
        bool RecomputeHaveAspectVertexBuffers();

        bool HavePipeline() const;
        bool ValidateBindGroupUsages(BindGroupBase* group) const;
        bool RevalidateCanDraw();

        void SetPipelineCommon(PipelineBase* pipeline);
        void UnsetPipeline();

        CommandBufferBuilder* mBuilder;

        ValidationAspects mAspects;

        std::bitset<kMaxBindGroups> mBindgroupsSet;
        std::array<BindGroupBase*, kMaxBindGroups> mBindgroups = {};
        std::bitset<kMaxVertexInputs> mInputsSet;
        PipelineBase* mLastPipeline = nullptr;
        RenderPipelineBase* mLastRenderPipeline = nullptr;

        std::map<BufferBase*, nxt::BufferUsageBit> mMostRecentBufferUsages;
        std::map<TextureBase*, nxt::TextureUsageBit> mMostRecentTextureUsages;

        RenderPassDescriptorBase* mCurrentRenderPass = nullptr;
    };
}  // namespace backend

#endif  // BACKEND_COMMANDBUFFERSTATETRACKER_H
