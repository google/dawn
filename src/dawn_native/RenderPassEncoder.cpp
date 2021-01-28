// Copyright 2018 The Dawn Authors
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

#include "dawn_native/RenderPassEncoder.h"

#include "common/Constants.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/QuerySet.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderPipeline.h"

#include <math.h>
#include <cstring>

namespace dawn_native {
    namespace {

        // Check the query at queryIndex is unavailable, otherwise it cannot be written.
        MaybeError ValidateQueryIndexOverwrite(QuerySetBase* querySet,
                                               uint32_t queryIndex,
                                               const QueryAvailabilityMap& queryAvailabilityMap) {
            auto it = queryAvailabilityMap.find(querySet);
            if (it != queryAvailabilityMap.end() && it->second[queryIndex]) {
                return DAWN_VALIDATION_ERROR(
                    "The same query cannot be written twice in same render pass.");
            }

            return {};
        }

    }  // namespace

    // The usage tracker is passed in here, because it is prepopulated with usages from the
    // BeginRenderPassCmd. If we had RenderPassEncoder responsible for recording the
    // command, then this wouldn't be necessary.
    RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                         CommandEncoder* commandEncoder,
                                         EncodingContext* encodingContext,
                                         PassResourceUsageTracker usageTracker,
                                         Ref<AttachmentState> attachmentState,
                                         QuerySetBase* occlusionQuerySet,
                                         uint32_t renderTargetWidth,
                                         uint32_t renderTargetHeight)
        : RenderEncoderBase(device, encodingContext, std::move(attachmentState)),
          mCommandEncoder(commandEncoder),
          mRenderTargetWidth(renderTargetWidth),
          mRenderTargetHeight(renderTargetHeight),
          mOcclusionQuerySet(occlusionQuerySet) {
        mUsageTracker = std::move(usageTracker);
    }

    RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                         CommandEncoder* commandEncoder,
                                         EncodingContext* encodingContext,
                                         ErrorTag errorTag)
        : RenderEncoderBase(device, encodingContext, errorTag), mCommandEncoder(commandEncoder) {
    }

    RenderPassEncoder* RenderPassEncoder::MakeError(DeviceBase* device,
                                                    CommandEncoder* commandEncoder,
                                                    EncodingContext* encodingContext) {
        return new RenderPassEncoder(device, commandEncoder, encodingContext, ObjectBase::kError);
    }

    void RenderPassEncoder::TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex) {
        DAWN_ASSERT(querySet != nullptr);

        // Gets the iterator for that querySet or create a new vector of bool set to false
        // if the querySet wasn't registered.
        auto it = mQueryAvailabilityMap.emplace(querySet, querySet->GetQueryCount()).first;
        it->second[queryIndex] = 1;

        // Track it again on command encoder for zero-initializing when resolving unused queries.
        mCommandEncoder->TrackQueryAvailability(querySet, queryIndex);
    }

    const QueryAvailabilityMap& RenderPassEncoder::GetQueryAvailabilityMap() const {
        return mQueryAvailabilityMap;
    }

    void RenderPassEncoder::EndPass() {
        if (mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(ValidateProgrammableEncoderEnd());
                    if (mOcclusionQueryActive) {
                        return DAWN_VALIDATION_ERROR(
                            "The occlusion query must be ended before endPass.");
                    }
                }

                allocator->Allocate<EndRenderPassCmd>(Command::EndRenderPass);
                return {};
            })) {
            mEncodingContext->ExitPass(this, mUsageTracker.AcquireResourceUsage());
        }
    }

    void RenderPassEncoder::SetStencilReference(uint32_t reference) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            SetStencilReferenceCmd* cmd =
                allocator->Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
            cmd->reference = reference;

            return {};
        });
    }

    void RenderPassEncoder::SetBlendColor(const Color* color) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            SetBlendColorCmd* cmd = allocator->Allocate<SetBlendColorCmd>(Command::SetBlendColor);
            cmd->color = *color;

            return {};
        });
    }

    void RenderPassEncoder::SetViewport(float x,
                                        float y,
                                        float width,
                                        float height,
                                        float minDepth,
                                        float maxDepth) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if ((isnan(x) || isnan(y) || isnan(width) || isnan(height) || isnan(minDepth) ||
                     isnan(maxDepth))) {
                    return DAWN_VALIDATION_ERROR("NaN is not allowed.");
                }

                if (x < 0 || y < 0 || width < 0 || height < 0) {
                    return DAWN_VALIDATION_ERROR("X, Y, width and height must be non-negative.");
                }

                if (x + width > mRenderTargetWidth || y + height > mRenderTargetHeight) {
                    return DAWN_VALIDATION_ERROR(
                        "The viewport must be contained in the render targets");
                }

                // Check for depths being in [0, 1] and min <= max in 3 checks instead of 5.
                if (minDepth < 0 || minDepth > maxDepth || maxDepth > 1) {
                    return DAWN_VALIDATION_ERROR(
                        "minDepth and maxDepth must be in [0, 1] and minDepth <= maxDepth.");
                }
            }

            SetViewportCmd* cmd = allocator->Allocate<SetViewportCmd>(Command::SetViewport);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;
            cmd->minDepth = minDepth;
            cmd->maxDepth = maxDepth;

            return {};
        });
    }

    void RenderPassEncoder::SetScissorRect(uint32_t x,
                                           uint32_t y,
                                           uint32_t width,
                                           uint32_t height) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if (width > mRenderTargetWidth || height > mRenderTargetHeight ||
                    x > mRenderTargetWidth - width || y > mRenderTargetHeight - height) {
                    return DAWN_VALIDATION_ERROR(
                        "The scissor rect must be contained in the render targets");
                }
            }

            SetScissorRectCmd* cmd =
                allocator->Allocate<SetScissorRectCmd>(Command::SetScissorRect);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;

            return {};
        });
    }

    void RenderPassEncoder::ExecuteBundles(uint32_t count, RenderBundleBase* const* renderBundles) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                for (uint32_t i = 0; i < count; ++i) {
                    DAWN_TRY(GetDevice()->ValidateObject(renderBundles[i]));

                    if (GetAttachmentState() != renderBundles[i]->GetAttachmentState()) {
                        return DAWN_VALIDATION_ERROR(
                            "Render bundle attachment state is not compatible with render pass "
                            "attachment state");
                    }
                }
            }

            mCommandBufferState = CommandBufferStateTracker{};

            ExecuteBundlesCmd* cmd =
                allocator->Allocate<ExecuteBundlesCmd>(Command::ExecuteBundles);
            cmd->count = count;

            Ref<RenderBundleBase>* bundles = allocator->AllocateData<Ref<RenderBundleBase>>(count);
            for (uint32_t i = 0; i < count; ++i) {
                bundles[i] = renderBundles[i];

                const PassResourceUsage& usages = bundles[i]->GetResourceUsage();
                for (uint32_t i = 0; i < usages.buffers.size(); ++i) {
                    mUsageTracker.BufferUsedAs(usages.buffers[i], usages.bufferUsages[i]);
                }

                for (uint32_t i = 0; i < usages.textures.size(); ++i) {
                    mUsageTracker.AddTextureUsage(usages.textures[i], usages.textureUsages[i]);
                }
            }

            return {};
        });
    }

    void RenderPassEncoder::BeginOcclusionQuery(uint32_t queryIndex) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if (mOcclusionQuerySet.Get() == nullptr) {
                    return DAWN_VALIDATION_ERROR(
                        "The occlusionQuerySet in RenderPassDescriptor must be set.");
                }

                // The type of querySet has been validated by ValidateRenderPassDescriptor

                if (queryIndex >= mOcclusionQuerySet->GetQueryCount()) {
                    return DAWN_VALIDATION_ERROR(
                        "Query index exceeds the number of queries in query set.");
                }

                if (mOcclusionQueryActive) {
                    return DAWN_VALIDATION_ERROR(
                        "Only a single occlusion query can be begun at a time.");
                }

                DAWN_TRY(ValidateQueryIndexOverwrite(mOcclusionQuerySet.Get(), queryIndex,
                                                     GetQueryAvailabilityMap()));

                mCommandEncoder->TrackUsedQuerySet(mOcclusionQuerySet.Get());
            }

            // Record the current query index for endOcclusionQuery.
            mCurrentOcclusionQueryIndex = queryIndex;
            mOcclusionQueryActive = true;

            BeginOcclusionQueryCmd* cmd =
                allocator->Allocate<BeginOcclusionQueryCmd>(Command::BeginOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = queryIndex;

            return {};
        });
    }

    void RenderPassEncoder::EndOcclusionQuery() {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if (!mOcclusionQueryActive) {
                    return DAWN_VALIDATION_ERROR(
                        "EndOcclusionQuery cannot be called without corresponding "
                        "BeginOcclusionQuery.");
                }
            }

            TrackQueryAvailability(mOcclusionQuerySet.Get(), mCurrentOcclusionQueryIndex);
            mOcclusionQueryActive = false;

            EndOcclusionQueryCmd* cmd =
                allocator->Allocate<EndOcclusionQueryCmd>(Command::EndOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = mCurrentOcclusionQueryIndex;

            return {};
        });
    }

    void RenderPassEncoder::WriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(querySet));
                DAWN_TRY(ValidateTimestampQuery(querySet, queryIndex));
                DAWN_TRY(
                    ValidateQueryIndexOverwrite(querySet, queryIndex, GetQueryAvailabilityMap()));
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

            return {};
        });
    }

}  // namespace dawn_native
