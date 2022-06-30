// Copyright 2019 The Dawn Authors
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

#include "dawn/native/RenderEncoderBase.h"

#include <math.h>
#include <cstring>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                     const char* label,
                                     EncodingContext* encodingContext,
                                     Ref<AttachmentState> attachmentState,
                                     bool depthReadOnly,
                                     bool stencilReadOnly)
    : ProgrammableEncoder(device, label, encodingContext),
      mIndirectDrawMetadata(device->GetLimits()),
      mAttachmentState(std::move(attachmentState)),
      mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
      mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {
    mDepthReadOnly = depthReadOnly;
    mStencilReadOnly = stencilReadOnly;
}

RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                     EncodingContext* encodingContext,
                                     ErrorTag errorTag)
    : ProgrammableEncoder(device, encodingContext, errorTag),
      mIndirectDrawMetadata(device->GetLimits()),
      mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
      mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {}

void RenderEncoderBase::DestroyImpl() {
    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

const AttachmentState* RenderEncoderBase::GetAttachmentState() const {
    ASSERT(!IsError());
    ASSERT(mAttachmentState != nullptr);
    return mAttachmentState.Get();
}

bool RenderEncoderBase::IsDepthReadOnly() const {
    ASSERT(!IsError());
    return mDepthReadOnly;
}

bool RenderEncoderBase::IsStencilReadOnly() const {
    ASSERT(!IsError());
    return mStencilReadOnly;
}

uint64_t RenderEncoderBase::GetDrawCount() const {
    ASSERT(!IsError());
    return mDrawCount;
}

Ref<AttachmentState> RenderEncoderBase::AcquireAttachmentState() {
    return std::move(mAttachmentState);
}

void RenderEncoderBase::APIDraw(uint32_t vertexCount,
                                uint32_t instanceCount,
                                uint32_t firstVertex,
                                uint32_t firstInstance) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(mCommandBufferState.ValidateCanDraw());

                DAWN_INVALID_IF(mDisableBaseInstance && firstInstance != 0,
                                "First instance (%u) must be zero.", firstInstance);

                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForVertexBuffer(vertexCount,
                                                                                  firstVertex));
                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(instanceCount,
                                                                                    firstInstance));
            }

            DrawCmd* draw = allocator->Allocate<DrawCmd>(Command::Draw);
            draw->vertexCount = vertexCount;
            draw->instanceCount = instanceCount;
            draw->firstVertex = firstVertex;
            draw->firstInstance = firstInstance;

            mDrawCount++;

            return {};
        },
        "encoding %s.Draw(%u, %u, %u, %u).", this, vertexCount, instanceCount, firstVertex,
        firstInstance);
}

void RenderEncoderBase::APIDrawIndexed(uint32_t indexCount,
                                       uint32_t instanceCount,
                                       uint32_t firstIndex,
                                       int32_t baseVertex,
                                       uint32_t firstInstance) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(mCommandBufferState.ValidateCanDrawIndexed());

                DAWN_INVALID_IF(mDisableBaseInstance && firstInstance != 0,
                                "First instance (%u) must be zero.", firstInstance);

                DAWN_INVALID_IF(mDisableBaseVertex && baseVertex != 0,
                                "Base vertex (%u) must be zero.", baseVertex);

                DAWN_TRY(mCommandBufferState.ValidateIndexBufferInRange(indexCount, firstIndex));

                // DrawIndexed only validate instance step mode vertex buffer
                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(instanceCount,
                                                                                    firstInstance));
            }

            DrawIndexedCmd* draw = allocator->Allocate<DrawIndexedCmd>(Command::DrawIndexed);
            draw->indexCount = indexCount;
            draw->instanceCount = instanceCount;
            draw->firstIndex = firstIndex;
            draw->baseVertex = baseVertex;
            draw->firstInstance = firstInstance;

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndexed(%u, %u, %u, %i, %u).", this, indexCount, instanceCount, firstIndex,
        baseVertex, firstInstance);
}

void RenderEncoderBase::APIDrawIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));
                DAWN_TRY(mCommandBufferState.ValidateCanDraw());

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(
                    indirectOffset >= indirectBuffer->GetSize() ||
                        kDrawIndirectSize > indirectBuffer->GetSize() - indirectOffset,
                    "Indirect offset (%u) is out of bounds of indirect buffer %s size (%u).",
                    indirectOffset, indirectBuffer, indirectBuffer->GetSize());
            }

            DrawIndirectCmd* cmd = allocator->Allocate<DrawIndirectCmd>(Command::DrawIndirect);

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());
            if (IsValidationEnabled() || duplicateBaseVertexInstance) {
                // Later, EncodeIndirectDrawValidationCommands will allocate a scratch storage
                // buffer which will store the validated or duplicated indirect data. The buffer
                // and offset will be updated to point to it.
                // |EncodeIndirectDrawValidationCommands| is called at the end of encoding the
                // render pass, while the |cmd| pointer is still valid.
                cmd->indirectBuffer = nullptr;

                mIndirectDrawMetadata.AddIndirectDraw(indirectBuffer, indirectOffset,
                                                      duplicateBaseVertexInstance, cmd);
            } else {
                cmd->indirectBuffer = indirectBuffer;
                cmd->indirectOffset = indirectOffset;
            }

            // TODO(crbug.com/dawn/1166): Adding the indirectBuffer is needed for correct usage
            // validation, but it will unnecessarily transition to indirectBuffer usage in the
            // backend.
            mUsageTracker.BufferUsedAs(indirectBuffer, wgpu::BufferUsage::Indirect);

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndirect(%s, %u).", this, indirectBuffer, indirectOffset);
}

void RenderEncoderBase::APIDrawIndexedIndirect(BufferBase* indirectBuffer,
                                               uint64_t indirectOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));
                DAWN_TRY(mCommandBufferState.ValidateCanDrawIndexed());

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(
                    (indirectOffset >= indirectBuffer->GetSize() ||
                     kDrawIndexedIndirectSize > indirectBuffer->GetSize() - indirectOffset),
                    "Indirect offset (%u) is out of bounds of indirect buffer %s size (%u).",
                    indirectOffset, indirectBuffer, indirectBuffer->GetSize());
            }

            DrawIndexedIndirectCmd* cmd =
                allocator->Allocate<DrawIndexedIndirectCmd>(Command::DrawIndexedIndirect);

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());
            if (IsValidationEnabled() || duplicateBaseVertexInstance) {
                // Later, EncodeIndirectDrawValidationCommands will allocate a scratch storage
                // buffer which will store the validated or duplicated indirect data. The buffer
                // and offset will be updated to point to it.
                // |EncodeIndirectDrawValidationCommands| is called at the end of encoding the
                // render pass, while the |cmd| pointer is still valid.
                cmd->indirectBuffer = nullptr;

                mIndirectDrawMetadata.AddIndexedIndirectDraw(
                    mCommandBufferState.GetIndexFormat(), mCommandBufferState.GetIndexBufferSize(),
                    indirectBuffer, indirectOffset, duplicateBaseVertexInstance, cmd);
            } else {
                cmd->indirectBuffer = indirectBuffer;
                cmd->indirectOffset = indirectOffset;
            }

            // TODO(crbug.com/dawn/1166): Adding the indirectBuffer is needed for correct usage
            // validation, but it will unecessarily transition to indirectBuffer usage in the
            // backend.
            mUsageTracker.BufferUsedAs(indirectBuffer, wgpu::BufferUsage::Indirect);

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndexedIndirect(%s, %u).", this, indirectBuffer, indirectOffset);
}

void RenderEncoderBase::APISetPipeline(RenderPipelineBase* pipeline) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(pipeline));

                DAWN_INVALID_IF(pipeline->GetAttachmentState() != mAttachmentState.Get(),
                                "Attachment state of %s is not compatible with %s.\n"
                                "%s expects an attachment state of %s.\n"
                                "%s has an attachment state of %s.",
                                pipeline, this, this, mAttachmentState.Get(), pipeline,
                                pipeline->GetAttachmentState());

                DAWN_INVALID_IF(pipeline->WritesDepth() && mDepthReadOnly,
                                "%s writes depth while %s's depthReadOnly is true", pipeline, this);

                DAWN_INVALID_IF(pipeline->WritesStencil() && mStencilReadOnly,
                                "%s writes stencil while %s's stencilReadOnly is true", pipeline,
                                this);
            }

            mCommandBufferState.SetRenderPipeline(pipeline);

            SetRenderPipelineCmd* cmd =
                allocator->Allocate<SetRenderPipelineCmd>(Command::SetRenderPipeline);
            cmd->pipeline = pipeline;

            return {};
        },
        "encoding %s.SetPipeline(%s).", this, pipeline);
}

void RenderEncoderBase::APISetIndexBuffer(BufferBase* buffer,
                                          wgpu::IndexFormat format,
                                          uint64_t offset,
                                          uint64_t size) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(buffer));
                DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::Index));

                DAWN_TRY(ValidateIndexFormat(format));

                DAWN_INVALID_IF(format == wgpu::IndexFormat::Undefined,
                                "Index format must be specified");

                DAWN_INVALID_IF(offset % uint64_t(IndexFormatSize(format)) != 0,
                                "Index buffer offset (%u) is not a multiple of the size (%u) "
                                "of %s.",
                                offset, IndexFormatSize(format), format);

                uint64_t bufferSize = buffer->GetSize();
                DAWN_INVALID_IF(offset > bufferSize,
                                "Index buffer offset (%u) is larger than the size (%u) of %s.",
                                offset, bufferSize, buffer);

                uint64_t remainingSize = bufferSize - offset;

                if (size == wgpu::kWholeSize) {
                    size = remainingSize;
                } else {
                    DAWN_INVALID_IF(size > remainingSize,
                                    "Index buffer range (offset: %u, size: %u) doesn't fit in "
                                    "the size (%u) of "
                                    "%s.",
                                    offset, size, bufferSize, buffer);
                }
            } else {
                if (size == wgpu::kWholeSize) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            mCommandBufferState.SetIndexBuffer(format, size);

            SetIndexBufferCmd* cmd =
                allocator->Allocate<SetIndexBufferCmd>(Command::SetIndexBuffer);
            cmd->buffer = buffer;
            cmd->format = format;
            cmd->offset = offset;
            cmd->size = size;

            mUsageTracker.BufferUsedAs(buffer, wgpu::BufferUsage::Index);

            return {};
        },
        "encoding %s.SetIndexBuffer(%s, %s, %u, %u).", this, buffer, format, offset, size);
}

void RenderEncoderBase::APISetVertexBuffer(uint32_t slot,
                                           BufferBase* buffer,
                                           uint64_t offset,
                                           uint64_t size) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(buffer));
                DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::Vertex));

                DAWN_INVALID_IF(slot >= kMaxVertexBuffers,
                                "Vertex buffer slot (%u) is larger the maximum (%u)", slot,
                                kMaxVertexBuffers - 1);

                DAWN_INVALID_IF(offset % 4 != 0, "Vertex buffer offset (%u) is not a multiple of 4",
                                offset);

                uint64_t bufferSize = buffer->GetSize();
                DAWN_INVALID_IF(offset > bufferSize,
                                "Vertex buffer offset (%u) is larger than the size (%u) of %s.",
                                offset, bufferSize, buffer);

                uint64_t remainingSize = bufferSize - offset;

                if (size == wgpu::kWholeSize) {
                    size = remainingSize;
                } else {
                    DAWN_INVALID_IF(size > remainingSize,
                                    "Vertex buffer range (offset: %u, size: %u) doesn't fit in "
                                    "the size (%u) "
                                    "of %s.",
                                    offset, size, bufferSize, buffer);
                }
            } else {
                if (size == wgpu::kWholeSize) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            mCommandBufferState.SetVertexBuffer(VertexBufferSlot(uint8_t(slot)), size);

            SetVertexBufferCmd* cmd =
                allocator->Allocate<SetVertexBufferCmd>(Command::SetVertexBuffer);
            cmd->slot = VertexBufferSlot(static_cast<uint8_t>(slot));
            cmd->buffer = buffer;
            cmd->offset = offset;
            cmd->size = size;

            mUsageTracker.BufferUsedAs(buffer, wgpu::BufferUsage::Vertex);

            return {};
        },
        "encoding %s.SetVertexBuffer(%u, %s, %u, %u).", this, slot, buffer, offset, size);
}

void RenderEncoderBase::APISetBindGroup(uint32_t groupIndexIn,
                                        BindGroupBase* group,
                                        uint32_t dynamicOffsetCount,
                                        const uint32_t* dynamicOffsets) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            BindGroupIndex groupIndex(groupIndexIn);

            if (IsValidationEnabled()) {
                DAWN_TRY(
                    ValidateSetBindGroup(groupIndex, group, dynamicOffsetCount, dynamicOffsets));
            }

            RecordSetBindGroup(allocator, groupIndex, group, dynamicOffsetCount, dynamicOffsets);
            mCommandBufferState.SetBindGroup(groupIndex, group, dynamicOffsetCount, dynamicOffsets);
            mUsageTracker.AddBindGroup(group);

            return {};
        },
        // TODO(dawn:1190): For unknown reasons formatting this message fails if `group` is used
        // as a string value in the message. This despite the exact same code working as
        // intended in ComputePassEncoder::APISetBindGroup. Replacing with a static [BindGroup]
        // until the reason for the failure can be determined.
        "encoding %s.SetBindGroup(%u, [BindGroup], %u, ...).", this, groupIndexIn,
        dynamicOffsetCount);
}

}  // namespace dawn::native
