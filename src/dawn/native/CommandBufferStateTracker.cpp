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

#include "dawn/native/CommandBufferStateTracker.h"

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"

// TODO(dawn:563): None of the error messages in this file include the buffer objects they are
// validating against. It would be nice to improve that, but difficult to do without incurring
// additional tracking costs.

namespace dawn::native {

namespace {
bool BufferSizesAtLeastAsBig(const ityp::span<uint32_t, uint64_t> unverifiedBufferSizes,
                             const std::vector<uint64_t>& pipelineMinBufferSizes) {
    ASSERT(unverifiedBufferSizes.size() == pipelineMinBufferSizes.size());

    for (uint32_t i = 0; i < unverifiedBufferSizes.size(); ++i) {
        if (unverifiedBufferSizes[i] < pipelineMinBufferSizes[i]) {
            return false;
        }
    }

    return true;
}
}  // namespace

enum ValidationAspect {
    VALIDATION_ASPECT_PIPELINE,
    VALIDATION_ASPECT_BIND_GROUPS,
    VALIDATION_ASPECT_VERTEX_BUFFERS,
    VALIDATION_ASPECT_INDEX_BUFFER,

    VALIDATION_ASPECT_COUNT
};
static_assert(VALIDATION_ASPECT_COUNT == CommandBufferStateTracker::kNumAspects);

static constexpr CommandBufferStateTracker::ValidationAspects kDispatchAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS;

static constexpr CommandBufferStateTracker::ValidationAspects kDrawAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
    1 << VALIDATION_ASPECT_VERTEX_BUFFERS;

static constexpr CommandBufferStateTracker::ValidationAspects kDrawIndexedAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
    1 << VALIDATION_ASPECT_VERTEX_BUFFERS | 1 << VALIDATION_ASPECT_INDEX_BUFFER;

static constexpr CommandBufferStateTracker::ValidationAspects kLazyAspects =
    1 << VALIDATION_ASPECT_BIND_GROUPS | 1 << VALIDATION_ASPECT_VERTEX_BUFFERS |
    1 << VALIDATION_ASPECT_INDEX_BUFFER;

MaybeError CommandBufferStateTracker::ValidateCanDispatch() {
    return ValidateOperation(kDispatchAspects);
}

MaybeError CommandBufferStateTracker::ValidateCanDraw() {
    return ValidateOperation(kDrawAspects);
}

MaybeError CommandBufferStateTracker::ValidateCanDrawIndexed() {
    return ValidateOperation(kDrawIndexedAspects);
}

MaybeError CommandBufferStateTracker::ValidateBufferInRangeForVertexBuffer(uint32_t vertexCount,
                                                                           uint32_t firstVertex) {
    RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

    const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>& vertexBufferSlotsUsedAsVertexBuffer =
        lastRenderPipeline->GetVertexBufferSlotsUsedAsVertexBuffer();

    for (auto usedSlotVertex : IterateBitSet(vertexBufferSlotsUsedAsVertexBuffer)) {
        const VertexBufferInfo& vertexBuffer = lastRenderPipeline->GetVertexBuffer(usedSlotVertex);
        uint64_t arrayStride = vertexBuffer.arrayStride;
        uint64_t bufferSize = mVertexBufferSizes[usedSlotVertex];

        if (arrayStride == 0) {
            DAWN_INVALID_IF(vertexBuffer.usedBytesInStride > bufferSize,
                            "Bound vertex buffer size (%u) at slot %u with an arrayStride of 0 "
                            "is smaller than the required size for all attributes (%u)",
                            bufferSize, static_cast<uint8_t>(usedSlotVertex),
                            vertexBuffer.usedBytesInStride);
        } else {
            uint64_t strideCount = static_cast<uint64_t>(firstVertex) + vertexCount;
            if (strideCount != 0u) {
                uint64_t requiredSize = (strideCount - 1u) * arrayStride + vertexBuffer.lastStride;
                // firstVertex and vertexCount are in uint32_t,
                // arrayStride must not be larger than kMaxVertexBufferArrayStride, which is
                // currently 2048, and vertexBuffer.lastStride = max(attribute.offset +
                // sizeof(attribute.format)) with attribute.offset being no larger than
                // kMaxVertexBufferArrayStride, so by doing checks in uint64_t we avoid
                // overflows.
                DAWN_INVALID_IF(
                    requiredSize > bufferSize,
                    "Vertex range (first: %u, count: %u) requires a larger buffer (%u) than "
                    "the "
                    "bound buffer size (%u) of the vertex buffer at slot %u with stride %u.",
                    firstVertex, vertexCount, requiredSize, bufferSize,
                    static_cast<uint8_t>(usedSlotVertex), arrayStride);
            }
        }
    }

    return {};
}

MaybeError CommandBufferStateTracker::ValidateBufferInRangeForInstanceBuffer(
    uint32_t instanceCount,
    uint32_t firstInstance) {
    RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

    const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>& vertexBufferSlotsUsedAsInstanceBuffer =
        lastRenderPipeline->GetVertexBufferSlotsUsedAsInstanceBuffer();

    for (auto usedSlotInstance : IterateBitSet(vertexBufferSlotsUsedAsInstanceBuffer)) {
        const VertexBufferInfo& vertexBuffer =
            lastRenderPipeline->GetVertexBuffer(usedSlotInstance);
        uint64_t arrayStride = vertexBuffer.arrayStride;
        uint64_t bufferSize = mVertexBufferSizes[usedSlotInstance];
        if (arrayStride == 0) {
            DAWN_INVALID_IF(vertexBuffer.usedBytesInStride > bufferSize,
                            "Bound vertex buffer size (%u) at slot %u with an arrayStride of 0 "
                            "is smaller than the required size for all attributes (%u)",
                            bufferSize, static_cast<uint8_t>(usedSlotInstance),
                            vertexBuffer.usedBytesInStride);
        } else {
            uint64_t strideCount = static_cast<uint64_t>(firstInstance) + instanceCount;
            if (strideCount != 0u) {
                uint64_t requiredSize = (strideCount - 1u) * arrayStride + vertexBuffer.lastStride;
                // firstInstance and instanceCount are in uint32_t,
                // arrayStride must not be larger than kMaxVertexBufferArrayStride, which is
                // currently 2048, and vertexBuffer.lastStride = max(attribute.offset +
                // sizeof(attribute.format)) with attribute.offset being no larger than
                // kMaxVertexBufferArrayStride, so by doing checks in uint64_t we avoid
                // overflows.
                DAWN_INVALID_IF(
                    requiredSize > bufferSize,
                    "Instance range (first: %u, count: %u) requires a larger buffer (%u) than "
                    "the "
                    "bound buffer size (%u) of the vertex buffer at slot %u with stride %u.",
                    firstInstance, instanceCount, requiredSize, bufferSize,
                    static_cast<uint8_t>(usedSlotInstance), arrayStride);
            }
        }
    }

    return {};
}

MaybeError CommandBufferStateTracker::ValidateIndexBufferInRange(uint32_t indexCount,
                                                                 uint32_t firstIndex) {
    // Validate the range of index buffer
    // firstIndex and indexCount are in uint32_t, while IndexFormatSize is 2 (for
    // wgpu::IndexFormat::Uint16) or 4 (for wgpu::IndexFormat::Uint32), so by doing checks in
    // uint64_t we avoid overflows.
    DAWN_INVALID_IF(
        (static_cast<uint64_t>(firstIndex) + indexCount) * IndexFormatSize(mIndexFormat) >
            mIndexBufferSize,
        "Index range (first: %u, count: %u, format: %s) does not fit in index buffer size "
        "(%u).",
        firstIndex, indexCount, mIndexFormat, mIndexBufferSize);
    return {};
}

MaybeError CommandBufferStateTracker::ValidateOperation(ValidationAspects requiredAspects) {
    // Fast return-true path if everything is good
    ValidationAspects missingAspects = requiredAspects & ~mAspects;
    if (missingAspects.none()) {
        return {};
    }

    // Generate an error immediately if a non-lazy aspect is missing as computing lazy aspects
    // requires the pipeline to be set.
    DAWN_TRY(CheckMissingAspects(missingAspects & ~kLazyAspects));

    RecomputeLazyAspects(missingAspects);

    DAWN_TRY(CheckMissingAspects(requiredAspects & ~mAspects));

    return {};
}

void CommandBufferStateTracker::RecomputeLazyAspects(ValidationAspects aspects) {
    ASSERT(mAspects[VALIDATION_ASPECT_PIPELINE]);
    ASSERT((aspects & ~kLazyAspects).none());

    if (aspects[VALIDATION_ASPECT_BIND_GROUPS]) {
        bool matches = true;

        for (BindGroupIndex i : IterateBitSet(mLastPipelineLayout->GetBindGroupLayoutsMask())) {
            if (mBindgroups[i] == nullptr ||
                mLastPipelineLayout->GetBindGroupLayout(i) != mBindgroups[i]->GetLayout() ||
                !BufferSizesAtLeastAsBig(mBindgroups[i]->GetUnverifiedBufferSizes(),
                                         (*mMinBufferSizes)[i])) {
                matches = false;
                break;
            }
        }

        if (matches) {
            mAspects.set(VALIDATION_ASPECT_BIND_GROUPS);
        }
    }

    if (aspects[VALIDATION_ASPECT_VERTEX_BUFFERS]) {
        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

        const ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>& requiredVertexBuffers =
            lastRenderPipeline->GetVertexBufferSlotsUsed();
        if (IsSubset(requiredVertexBuffers, mVertexBufferSlotsUsed)) {
            mAspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
        }
    }

    if (aspects[VALIDATION_ASPECT_INDEX_BUFFER] && mIndexBufferSet) {
        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();
        if (!IsStripPrimitiveTopology(lastRenderPipeline->GetPrimitiveTopology()) ||
            mIndexFormat == lastRenderPipeline->GetStripIndexFormat()) {
            mAspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        }
    }
}

MaybeError CommandBufferStateTracker::CheckMissingAspects(ValidationAspects aspects) {
    if (!aspects.any()) {
        return {};
    }

    DAWN_INVALID_IF(aspects[VALIDATION_ASPECT_PIPELINE], "No pipeline set.");

    if (DAWN_UNLIKELY(aspects[VALIDATION_ASPECT_INDEX_BUFFER])) {
        DAWN_INVALID_IF(!mIndexBufferSet, "Index buffer was not set.");

        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();
        wgpu::IndexFormat pipelineIndexFormat = lastRenderPipeline->GetStripIndexFormat();

        if (IsStripPrimitiveTopology(lastRenderPipeline->GetPrimitiveTopology())) {
            DAWN_INVALID_IF(
                pipelineIndexFormat == wgpu::IndexFormat::Undefined,
                "%s has a strip primitive topology (%s) but a strip index format of %s, which "
                "prevents it for being used for indexed draw calls.",
                lastRenderPipeline, lastRenderPipeline->GetPrimitiveTopology(),
                pipelineIndexFormat);

            DAWN_INVALID_IF(
                mIndexFormat != pipelineIndexFormat,
                "Strip index format (%s) of %s does not match index buffer format (%s).",
                pipelineIndexFormat, lastRenderPipeline, mIndexFormat);
        }

        // The chunk of code above should be similar to the one in |RecomputeLazyAspects|.
        // It returns the first invalid state found. We shouldn't be able to reach this line
        // because to have invalid aspects one of the above conditions must have failed earlier.
        // If this is reached, make sure lazy aspects and the error checks above are consistent.
        UNREACHABLE();
        return DAWN_FORMAT_VALIDATION_ERROR("Index buffer is invalid.");
    }

    // TODO(dawn:563): Indicate which slots were not set.
    DAWN_INVALID_IF(aspects[VALIDATION_ASPECT_VERTEX_BUFFERS],
                    "Vertex buffer slots required by %s were not set.", GetRenderPipeline());

    if (DAWN_UNLIKELY(aspects[VALIDATION_ASPECT_BIND_GROUPS])) {
        for (BindGroupIndex i : IterateBitSet(mLastPipelineLayout->GetBindGroupLayoutsMask())) {
            ASSERT(HasPipeline());

            DAWN_INVALID_IF(mBindgroups[i] == nullptr, "No bind group set at index %u.",
                            static_cast<uint32_t>(i));

            BindGroupLayoutBase* requiredBGL = mLastPipelineLayout->GetBindGroupLayout(i);
            BindGroupLayoutBase* currentBGL = mBindgroups[i]->GetLayout();

            DAWN_INVALID_IF(
                requiredBGL->GetPipelineCompatibilityToken() != PipelineCompatibilityToken(0) &&
                    currentBGL->GetPipelineCompatibilityToken() !=
                        requiredBGL->GetPipelineCompatibilityToken(),
                "The current pipeline (%s) was created with a default layout, and is not "
                "compatible with the %s at index %u which uses a %s that was not created by "
                "the pipeline. Either use the bind group layout returned by calling "
                "getBindGroupLayout(%u) on the pipeline when creating the bind group, or "
                "provide an explicit pipeline layout when creating the pipeline.",
                mLastPipeline, mBindgroups[i], static_cast<uint32_t>(i), currentBGL,
                static_cast<uint32_t>(i));

            DAWN_INVALID_IF(
                requiredBGL->GetPipelineCompatibilityToken() == PipelineCompatibilityToken(0) &&
                    currentBGL->GetPipelineCompatibilityToken() != PipelineCompatibilityToken(0),
                "%s at index %u uses a %s which was created as part of the default layout for "
                "a different pipeline than the current one (%s), and as a result is not "
                "compatible. Use an explicit bind group layout when creating bind groups and "
                "an explicit pipeline layout when creating pipelines to share bind groups "
                "between pipelines.",
                mBindgroups[i], static_cast<uint32_t>(i), currentBGL, mLastPipeline);

            DAWN_INVALID_IF(
                mLastPipelineLayout->GetBindGroupLayout(i) != mBindgroups[i]->GetLayout(),
                "Bind group layout %s of pipeline layout %s does not match layout %s of bind "
                "group %s at index %u.",
                requiredBGL, mLastPipelineLayout, currentBGL, mBindgroups[i],
                static_cast<uint32_t>(i));

            // TODO(dawn:563): Report the binding sizes and which ones are failing.
            DAWN_INVALID_IF(!BufferSizesAtLeastAsBig(mBindgroups[i]->GetUnverifiedBufferSizes(),
                                                     (*mMinBufferSizes)[i]),
                            "Binding sizes are too small for bind group %s at index %u",
                            mBindgroups[i], static_cast<uint32_t>(i));
        }

        // The chunk of code above should be similar to the one in |RecomputeLazyAspects|.
        // It returns the first invalid state found. We shouldn't be able to reach this line
        // because to have invalid aspects one of the above conditions must have failed earlier.
        // If this is reached, make sure lazy aspects and the error checks above are consistent.
        UNREACHABLE();
        return DAWN_FORMAT_VALIDATION_ERROR("Bind groups are invalid.");
    }

    UNREACHABLE();
}

void CommandBufferStateTracker::SetComputePipeline(ComputePipelineBase* pipeline) {
    SetPipelineCommon(pipeline);
}

void CommandBufferStateTracker::SetRenderPipeline(RenderPipelineBase* pipeline) {
    SetPipelineCommon(pipeline);
}

void CommandBufferStateTracker::SetBindGroup(BindGroupIndex index,
                                             BindGroupBase* bindgroup,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets) {
    mBindgroups[index] = bindgroup;
    mDynamicOffsets[index].assign(dynamicOffsets, dynamicOffsets + dynamicOffsetCount);
    mAspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
}

void CommandBufferStateTracker::SetIndexBuffer(wgpu::IndexFormat format, uint64_t size) {
    mIndexBufferSet = true;
    mIndexFormat = format;
    mIndexBufferSize = size;
}

void CommandBufferStateTracker::SetVertexBuffer(VertexBufferSlot slot, uint64_t size) {
    mVertexBufferSlotsUsed.set(slot);
    mVertexBufferSizes[slot] = size;
}

void CommandBufferStateTracker::SetPipelineCommon(PipelineBase* pipeline) {
    mLastPipeline = pipeline;
    mLastPipelineLayout = pipeline != nullptr ? pipeline->GetLayout() : nullptr;
    mMinBufferSizes = pipeline != nullptr ? &pipeline->GetMinBufferSizes() : nullptr;

    mAspects.set(VALIDATION_ASPECT_PIPELINE);

    // Reset lazy aspects so they get recomputed on the next operation.
    mAspects &= ~kLazyAspects;
}

BindGroupBase* CommandBufferStateTracker::GetBindGroup(BindGroupIndex index) const {
    return mBindgroups[index];
}

const std::vector<uint32_t>& CommandBufferStateTracker::GetDynamicOffsets(
    BindGroupIndex index) const {
    return mDynamicOffsets[index];
}

bool CommandBufferStateTracker::HasPipeline() const {
    return mLastPipeline != nullptr;
}

RenderPipelineBase* CommandBufferStateTracker::GetRenderPipeline() const {
    ASSERT(HasPipeline() && mLastPipeline->GetType() == ObjectType::RenderPipeline);
    return static_cast<RenderPipelineBase*>(mLastPipeline);
}

ComputePipelineBase* CommandBufferStateTracker::GetComputePipeline() const {
    ASSERT(HasPipeline() && mLastPipeline->GetType() == ObjectType::ComputePipeline);
    return static_cast<ComputePipelineBase*>(mLastPipeline);
}

PipelineLayoutBase* CommandBufferStateTracker::GetPipelineLayout() const {
    return mLastPipelineLayout;
}

wgpu::IndexFormat CommandBufferStateTracker::GetIndexFormat() const {
    return mIndexFormat;
}

uint64_t CommandBufferStateTracker::GetIndexBufferSize() const {
    return mIndexBufferSize;
}

}  // namespace dawn::native
