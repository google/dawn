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

            // State-modifying methods
            bool BeginSubpass();
            bool EndSubpass();
            bool BeginRenderPass(RenderPassBase* renderPass, FramebufferBase* framebuffer);
            bool EndRenderPass();
            bool SetPipeline(PipelineBase* pipeline);
            bool SetBindGroup(uint32_t index, BindGroupBase* bindgroup);
            bool SetIndexBuffer(BufferBase* buffer);
            bool SetVertexBuffer(uint32_t index, BufferBase* buffer);
            bool TransitionBufferUsage(BufferBase* buffer, nxt::BufferUsageBit usage);
            bool TransitionTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage);

            // These collections are copied to the CommandBuffer at build time.
            // These pointers will remain valid since they are referenced by
            // the bind groups which are referenced by this command buffer.
            std::set<BufferBase*> buffersTransitioned;
            std::set<TextureBase*> texturesTransitioned;

        private:
            enum ValidationAspect {
                VALIDATION_ASPECT_RENDER_PIPELINE,
                VALIDATION_ASPECT_COMPUTE_PIPELINE,
                VALIDATION_ASPECT_BIND_GROUPS,
                VALIDATION_ASPECT_VERTEX_BUFFERS,
                VALIDATION_ASPECT_INDEX_BUFFER,
                VALIDATION_ASPECT_RENDER_SUBPASS,

                VALIDATION_ASPECT_COUNT
            };
            using ValidationAspects = std::bitset<VALIDATION_ASPECT_COUNT>;

            // Usage helper functions
            bool BufferHasGuaranteedUsageBit(BufferBase* buffer, nxt::BufferUsageBit usage) const;
            bool TextureHasGuaranteedUsageBit(TextureBase* texture, nxt::TextureUsageBit usage) const;
            bool IsTextureTransitionPossible(TextureBase* texture, nxt::TextureUsageBit usage) const;

            // Queries for lazily evaluated aspects
            bool RecomputeHaveAspectBindGroups();
            bool RecomputeHaveAspectVertexBuffers();

            bool HavePipeline() const;
            bool ValidateBindGroupUsages(BindGroupBase* group) const;
            bool RevalidateCanDraw();

            void UnsetPipeline();

            CommandBufferBuilder* builder;

            ValidationAspects aspects;

            std::bitset<kMaxBindGroups> bindgroupsSet;
            std::bitset<kMaxVertexInputs> inputsSet;
            PipelineBase* lastPipeline = nullptr;

            std::map<BufferBase*, nxt::BufferUsageBit> mostRecentBufferUsages;
            std::map<TextureBase*, nxt::TextureUsageBit> mostRecentTextureUsages;

            RenderPassBase* currentRenderPass = nullptr;
            FramebufferBase* currentFramebuffer = nullptr;
            uint32_t currentSubpass = 0;
    };
}

#endif // BACKEND_COMMANDBUFFERSTATETRACKER_H
