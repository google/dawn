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

    MaybeError CommandBufferStateTracker::ValidateCanDispatch() {
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return {};
        }

        if (!mAspects[VALIDATION_ASPECT_PIPELINE]) {
            NXT_RETURN_ERROR("No active compute pipeline");
        }
        // Compute the lazily computed mAspects
        if (!RecomputeHaveAspectBindGroups()) {
            NXT_RETURN_ERROR("Bind group state not valid");
        }
        return {};
    }

    MaybeError CommandBufferStateTracker::ValidateCanDrawArrays() {
        constexpr ValidationAspects requiredAspects = 1 << VALIDATION_ASPECT_PIPELINE |
                                                      1 << VALIDATION_ASPECT_BIND_GROUPS |
                                                      1 << VALIDATION_ASPECT_VERTEX_BUFFERS;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return {};
        }

        return RevalidateCanDraw();
    }

    MaybeError CommandBufferStateTracker::ValidateCanDrawElements() {
        constexpr ValidationAspects requiredAspects =
            1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
            1 << VALIDATION_ASPECT_VERTEX_BUFFERS | 1 << VALIDATION_ASPECT_INDEX_BUFFER;
        if ((requiredAspects & ~mAspects).none()) {
            // Fast return-true path if everything is good
            return {};
        }

        if (!mAspects[VALIDATION_ASPECT_INDEX_BUFFER]) {
            NXT_RETURN_ERROR("Cannot DrawElements without index buffer set");
        }
        return RevalidateCanDraw();
    }

    void CommandBufferStateTracker::EndPass() {
        mInputsSet.reset();
        mAspects = 0;
        mBindgroups.fill(nullptr);
    }

    void CommandBufferStateTracker::SetComputePipeline(ComputePipelineBase* pipeline) {
        SetPipelineCommon(pipeline);
    }

    void CommandBufferStateTracker::SetRenderPipeline(RenderPipelineBase* pipeline) {
        mLastRenderPipeline = pipeline;
        SetPipelineCommon(pipeline);
    }

    void CommandBufferStateTracker::SetBindGroup(uint32_t index, BindGroupBase* bindgroup) {
        mBindgroupsSet.set(index);
        mBindgroups[index] = bindgroup;
    }

    MaybeError CommandBufferStateTracker::SetIndexBuffer() {
        if (!HavePipeline()) {
            NXT_RETURN_ERROR("Can't set the index buffer without a pipeline");
        }

        mAspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        return {};
    }

    MaybeError CommandBufferStateTracker::SetVertexBuffer(uint32_t index) {
        if (!HavePipeline()) {
            NXT_RETURN_ERROR("Can't set vertex buffers without a pipeline");
        }

        mInputsSet.set(index);
        return {};
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

    MaybeError CommandBufferStateTracker::RevalidateCanDraw() {
        if (!mAspects[VALIDATION_ASPECT_PIPELINE]) {
            NXT_RETURN_ERROR("No active render pipeline");
        }
        // Compute the lazily computed mAspects
        if (!RecomputeHaveAspectBindGroups()) {
            NXT_RETURN_ERROR("Bind group state not valid");
        }
        if (!RecomputeHaveAspectVertexBuffers()) {
            NXT_RETURN_ERROR("Some vertex buffers are not set");
        }
        return {};
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
