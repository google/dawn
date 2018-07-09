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

    void CommandBufferStateTracker::EndPass() {
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

    void CommandBufferStateTracker::SetBindGroup(uint32_t index, BindGroupBase* bindgroup) {
        mBindgroupsSet.set(index);
        mBindgroups[index] = bindgroup;
    }

    bool CommandBufferStateTracker::SetIndexBuffer() {
        if (!HavePipeline()) {
            mBuilder->HandleError("Can't set the index buffer without a pipeline");
            return false;
        }

        mAspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        return true;
    }

    bool CommandBufferStateTracker::SetVertexBuffer(uint32_t index) {
        if (!HavePipeline()) {
            mBuilder->HandleError("Can't set vertex buffers without a pipeline");
            return false;
        }

        mInputsSet.set(index);
        return true;
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
