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
        // Non-state-modifying validation functions
        MaybeError ValidateCanCopy() const;
        MaybeError ValidateCanDispatch();
        MaybeError ValidateCanDrawArrays();
        MaybeError ValidateCanDrawElements();

        // State-modifying methods
        void EndPass();
        void SetComputePipeline(ComputePipelineBase* pipeline);
        void SetRenderPipeline(RenderPipelineBase* pipeline);
        void SetBindGroup(uint32_t index, BindGroupBase* bindgroup);
        MaybeError SetIndexBuffer();
        MaybeError SetVertexBuffer(uint32_t index);

      private:
        enum ValidationAspect {
            VALIDATION_ASPECT_PIPELINE,
            VALIDATION_ASPECT_BIND_GROUPS,
            VALIDATION_ASPECT_VERTEX_BUFFERS,
            VALIDATION_ASPECT_INDEX_BUFFER,

            VALIDATION_ASPECT_COUNT
        };
        using ValidationAspects = std::bitset<VALIDATION_ASPECT_COUNT>;

        // Queries for lazily evaluated aspects
        bool RecomputeHaveAspectBindGroups();
        bool RecomputeHaveAspectVertexBuffers();

        bool HavePipeline() const;
        MaybeError RevalidateCanDraw();

        void SetPipelineCommon(PipelineBase* pipeline);

        ValidationAspects mAspects;

        std::bitset<kMaxBindGroups> mBindgroupsSet;
        std::array<BindGroupBase*, kMaxBindGroups> mBindgroups = {};
        std::bitset<kMaxVertexInputs> mInputsSet;
        PipelineBase* mLastPipeline = nullptr;
        RenderPipelineBase* mLastRenderPipeline = nullptr;
    };

}  // namespace backend

#endif  // BACKEND_COMMANDBUFFERSTATETRACKER_H
