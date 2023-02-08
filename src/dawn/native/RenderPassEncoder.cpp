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

#include "dawn/native/RenderPassEncoder.h"

#include <math.h>
#include <cstring>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/RenderPipeline.h"

namespace dawn::native {
namespace {

// Check the query at queryIndex is unavailable, otherwise it cannot be written.
MaybeError ValidateQueryIndexOverwrite(QuerySetBase* querySet,
                                       uint32_t queryIndex,
                                       const QueryAvailabilityMap& queryAvailabilityMap) {
    auto it = queryAvailabilityMap.find(querySet);
    DAWN_INVALID_IF(it != queryAvailabilityMap.end() && it->second[queryIndex],
                    "Query index %u of %s is written to twice in a render pass.", queryIndex,
                    querySet);

    return {};
}

}  // namespace

// The usage tracker is passed in here, because it is prepopulated with usages from the
// BeginRenderPassCmd. If we had RenderPassEncoder responsible for recording the
// command, then this wouldn't be necessary.
RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                     const RenderPassDescriptor* descriptor,
                                     CommandEncoder* commandEncoder,
                                     EncodingContext* encodingContext,
                                     RenderPassResourceUsageTracker usageTracker,
                                     Ref<AttachmentState> attachmentState,
                                     uint32_t renderTargetWidth,
                                     uint32_t renderTargetHeight,
                                     bool depthReadOnly,
                                     bool stencilReadOnly,
                                     std::function<void()> endCallback)
    : RenderEncoderBase(device,
                        descriptor->label,
                        encodingContext,
                        std::move(attachmentState),
                        depthReadOnly,
                        stencilReadOnly),
      mCommandEncoder(commandEncoder),
      mRenderTargetWidth(renderTargetWidth),
      mRenderTargetHeight(renderTargetHeight),
      mOcclusionQuerySet(descriptor->occlusionQuerySet),
      mEndCallback(std::move(endCallback)) {
    mUsageTracker = std::move(usageTracker);
    const RenderPassDescriptorMaxDrawCount* maxDrawCountInfo = nullptr;
    FindInChain(descriptor->nextInChain, &maxDrawCountInfo);
    if (maxDrawCountInfo) {
        mMaxDrawCount = maxDrawCountInfo->maxDrawCount;
    }
    GetObjectTrackingList()->Track(this);
}

// static
Ref<RenderPassEncoder> RenderPassEncoder::Create(DeviceBase* device,
                                                 const RenderPassDescriptor* descriptor,
                                                 CommandEncoder* commandEncoder,
                                                 EncodingContext* encodingContext,
                                                 RenderPassResourceUsageTracker usageTracker,
                                                 Ref<AttachmentState> attachmentState,
                                                 uint32_t renderTargetWidth,
                                                 uint32_t renderTargetHeight,
                                                 bool depthReadOnly,
                                                 bool stencilReadOnly,
                                                 std::function<void()> endCallback) {
    return AcquireRef(new RenderPassEncoder(device, descriptor, commandEncoder, encodingContext,
                                            std::move(usageTracker), std::move(attachmentState),
                                            renderTargetWidth, renderTargetHeight, depthReadOnly,
                                            stencilReadOnly, std::move(endCallback)));
}

RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                     CommandEncoder* commandEncoder,
                                     EncodingContext* encodingContext,
                                     ErrorTag errorTag)
    : RenderEncoderBase(device, encodingContext, errorTag), mCommandEncoder(commandEncoder) {}

// static
Ref<RenderPassEncoder> RenderPassEncoder::MakeError(DeviceBase* device,
                                                    CommandEncoder* commandEncoder,
                                                    EncodingContext* encodingContext) {
    return AcquireRef(
        new RenderPassEncoder(device, commandEncoder, encodingContext, ObjectBase::kError));
}

void RenderPassEncoder::DestroyImpl() {
    RenderEncoderBase::DestroyImpl();
    // Ensure that the pass has exited. This is done for passes only since validation requires
    // they exit before destruction while bundles do not.
    mEncodingContext->EnsurePassExited(this);
}

ObjectType RenderPassEncoder::GetType() const {
    return ObjectType::RenderPassEncoder;
}

void RenderPassEncoder::TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex) {
    DAWN_ASSERT(querySet != nullptr);

    // Track the query availability with true on render pass for rewrite validation and query
    // reset on render pass on Vulkan
    mUsageTracker.TrackQueryAvailability(querySet, queryIndex);

    // Track it again on command encoder for zero-initializing when resolving unused queries.
    mCommandEncoder->TrackQueryAvailability(querySet, queryIndex);
}

void RenderPassEncoder::APIEnd() {
    if (mEnded && IsValidationEnabled()) {
        GetDevice()->ConsumedError(DAWN_VALIDATION_ERROR("%s was already ended.", this));
        return;
    }

    mEnded = true;

    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateProgrammableEncoderEnd());

                DAWN_INVALID_IF(
                    mOcclusionQueryActive,
                    "Render pass %s ended with incomplete occlusion query index %u of %s.", this,
                    mCurrentOcclusionQueryIndex, mOcclusionQuerySet.Get());

                DAWN_INVALID_IF(mDrawCount > mMaxDrawCount,
                                "The drawCount (%u) of %s is greater than the maxDrawCount (%u).",
                                mDrawCount, this, mMaxDrawCount);
            }

            allocator->Allocate<EndRenderPassCmd>(Command::EndRenderPass);

            DAWN_TRY(mEncodingContext->ExitRenderPass(this, std::move(mUsageTracker),
                                                      mCommandEncoder.Get(),
                                                      std::move(mIndirectDrawMetadata)));
            return {};
        },
        "encoding %s.End().", this);

    if (mEndCallback) {
        mEndCallback();
    }
}

void RenderPassEncoder::APIEndPass() {
    if (GetDevice()->ConsumedError(DAWN_MAKE_DEPRECATION_ERROR(
            GetDevice(), "endPass() has been deprecated. Use end() instead."))) {
        return;
    }
    APIEnd();
}

void RenderPassEncoder::APISetStencilReference(uint32_t reference) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            SetStencilReferenceCmd* cmd =
                allocator->Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
            cmd->reference = reference & 255;

            return {};
        },
        "encoding %s.SetStencilReference(%u).", this, reference);
}

void RenderPassEncoder::APISetBlendConstant(const Color* color) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            SetBlendConstantCmd* cmd =
                allocator->Allocate<SetBlendConstantCmd>(Command::SetBlendConstant);
            cmd->color = *color;

            return {};
        },
        "encoding %s.SetBlendConstant(%s).", this, color);
}

void RenderPassEncoder::APISetViewport(float x,
                                       float y,
                                       float width,
                                       float height,
                                       float minDepth,
                                       float maxDepth) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF((isnan(x) || isnan(y) || isnan(width) || isnan(height) ||
                                 isnan(minDepth) || isnan(maxDepth)),
                                "A parameter of the viewport (x: %f, y: %f, width: %f, height: %f, "
                                "minDepth: %f, maxDepth: %f) is NaN.",
                                x, y, width, height, minDepth, maxDepth);

                DAWN_INVALID_IF(
                    x < 0 || y < 0 || width < 0 || height < 0,
                    "Viewport bounds (x: %f, y: %f, width: %f, height: %f) contains a negative "
                    "value.",
                    x, y, width, height);

                DAWN_INVALID_IF(
                    x + width > mRenderTargetWidth || y + height > mRenderTargetHeight,
                    "Viewport bounds (x: %f, y: %f, width: %f, height: %f) are not contained "
                    "in "
                    "the render target dimensions (%u x %u).",
                    x, y, width, height, mRenderTargetWidth, mRenderTargetHeight);

                // Check for depths being in [0, 1] and min <= max in 3 checks instead of 5.
                DAWN_INVALID_IF(minDepth < 0 || minDepth > maxDepth || maxDepth > 1,
                                "Viewport minDepth (%f) and maxDepth (%f) are not in [0, 1] or "
                                "minDepth was "
                                "greater than maxDepth.",
                                minDepth, maxDepth);
            }

            SetViewportCmd* cmd = allocator->Allocate<SetViewportCmd>(Command::SetViewport);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;
            cmd->minDepth = minDepth;
            cmd->maxDepth = maxDepth;

            return {};
        },
        "encoding %s.SetViewport(%f, %f, %f, %f, %f, %f).", this, x, y, width, height, minDepth,
        maxDepth);
}

void RenderPassEncoder::APISetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(
                    width > mRenderTargetWidth || height > mRenderTargetHeight ||
                        x > mRenderTargetWidth - width || y > mRenderTargetHeight - height,
                    "Scissor rect (x: %u, y: %u, width: %u, height: %u) is not contained in "
                    "the render target dimensions (%u x %u).",
                    x, y, width, height, mRenderTargetWidth, mRenderTargetHeight);
            }

            SetScissorRectCmd* cmd =
                allocator->Allocate<SetScissorRectCmd>(Command::SetScissorRect);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;

            return {};
        },
        "encoding %s.SetScissorRect(%u, %u, %u, %u).", this, x, y, width, height);
}

void RenderPassEncoder::APIExecuteBundles(uint32_t count, RenderBundleBase* const* renderBundles) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                const AttachmentState* attachmentState = GetAttachmentState();
                bool depthReadOnlyInPass = IsDepthReadOnly();
                bool stencilReadOnlyInPass = IsStencilReadOnly();
                for (uint32_t i = 0; i < count; ++i) {
                    DAWN_TRY(GetDevice()->ValidateObject(renderBundles[i]));

                    DAWN_INVALID_IF(attachmentState != renderBundles[i]->GetAttachmentState(),
                                    "Attachment state of renderBundles[%i] (%s) is not "
                                    "compatible with %s.\n"
                                    "%s expects an attachment state of %s.\n"
                                    "renderBundles[%i] (%s) has an attachment state of %s.",
                                    i, renderBundles[i], this, this, attachmentState, i,
                                    renderBundles[i], renderBundles[i]->GetAttachmentState());

                    bool depthReadOnlyInBundle = renderBundles[i]->IsDepthReadOnly();
                    DAWN_INVALID_IF(depthReadOnlyInPass && !depthReadOnlyInBundle,
                                    "DepthReadOnly (%u) of renderBundle[%i] (%s) is not compatible "
                                    "with DepthReadOnly (%u) of %s.",
                                    depthReadOnlyInBundle, i, renderBundles[i], depthReadOnlyInPass,
                                    this);

                    bool stencilReadOnlyInBundle = renderBundles[i]->IsStencilReadOnly();
                    DAWN_INVALID_IF(stencilReadOnlyInPass && !stencilReadOnlyInBundle,
                                    "StencilReadOnly (%u) of renderBundle[%i] (%s) is not "
                                    "compatible with StencilReadOnly (%u) of %s.",
                                    stencilReadOnlyInBundle, i, renderBundles[i],
                                    stencilReadOnlyInPass, this);
                }
            }

            mCommandBufferState = CommandBufferStateTracker{};

            ExecuteBundlesCmd* cmd =
                allocator->Allocate<ExecuteBundlesCmd>(Command::ExecuteBundles);
            cmd->count = count;

            Ref<RenderBundleBase>* bundles = allocator->AllocateData<Ref<RenderBundleBase>>(count);
            for (uint32_t i = 0; i < count; ++i) {
                bundles[i] = renderBundles[i];

                const RenderPassResourceUsage& usages = bundles[i]->GetResourceUsage();
                for (uint32_t i = 0; i < usages.buffers.size(); ++i) {
                    mUsageTracker.BufferUsedAs(usages.buffers[i], usages.bufferUsages[i]);
                }

                for (uint32_t i = 0; i < usages.textures.size(); ++i) {
                    mUsageTracker.AddRenderBundleTextureUsage(usages.textures[i],
                                                              usages.textureUsages[i]);
                }

                if (IsValidationEnabled()) {
                    mIndirectDrawMetadata.AddBundle(renderBundles[i]);
                }

                mDrawCount += bundles[i]->GetDrawCount();
            }

            return {};
        },
        "encoding %s.ExecuteBundles(%u, ...).", this, count);
}

void RenderPassEncoder::APIBeginOcclusionQuery(uint32_t queryIndex) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(mOcclusionQuerySet.Get() == nullptr,
                                "The occlusionQuerySet in RenderPassDescriptor is not set.");

                // The type of querySet has been validated by ValidateRenderPassDescriptor

                DAWN_INVALID_IF(queryIndex >= mOcclusionQuerySet->GetQueryCount(),
                                "Query index (%u) exceeds the number of queries (%u) in %s.",
                                queryIndex, mOcclusionQuerySet->GetQueryCount(),
                                mOcclusionQuerySet.Get());

                DAWN_INVALID_IF(mOcclusionQueryActive,
                                "An occlusion query (%u) in %s is already active.",
                                mCurrentOcclusionQueryIndex, mOcclusionQuerySet.Get());

                DAWN_TRY_CONTEXT(
                    ValidateQueryIndexOverwrite(mOcclusionQuerySet.Get(), queryIndex,
                                                mUsageTracker.GetQueryAvailabilityMap()),
                    "validating the occlusion query index (%u) in %s", queryIndex,
                    mOcclusionQuerySet.Get());
            }

            // Record the current query index for endOcclusionQuery.
            mCurrentOcclusionQueryIndex = queryIndex;
            mOcclusionQueryActive = true;

            BeginOcclusionQueryCmd* cmd =
                allocator->Allocate<BeginOcclusionQueryCmd>(Command::BeginOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.BeginOcclusionQuery(%u).", this, queryIndex);
}

void RenderPassEncoder::APIEndOcclusionQuery() {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(!mOcclusionQueryActive, "No occlusion queries are active.");
            }

            TrackQueryAvailability(mOcclusionQuerySet.Get(), mCurrentOcclusionQueryIndex);

            mOcclusionQueryActive = false;

            EndOcclusionQueryCmd* cmd =
                allocator->Allocate<EndOcclusionQueryCmd>(Command::EndOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = mCurrentOcclusionQueryIndex;

            return {};
        },
        "encoding %s.EndOcclusionQuery().", this);
}

void RenderPassEncoder::APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateTimestampQuery(GetDevice(), querySet, queryIndex,
                                                Feature::TimestampQueryInsidePasses));
                DAWN_TRY_CONTEXT(ValidateQueryIndexOverwrite(
                                     querySet, queryIndex, mUsageTracker.GetQueryAvailabilityMap()),
                                 "validating the timestamp query index (%u) of %s", queryIndex,
                                 querySet);
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.WriteTimestamp(%s, %u).", this, querySet, queryIndex);
}

}  // namespace dawn::native
