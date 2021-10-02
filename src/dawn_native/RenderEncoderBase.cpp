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

#include "dawn_native/RenderEncoderBase.h"

#include "common/Constants.h"
#include "common/Log.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/BufferLocation.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <math.h>
#include <cstring>

namespace dawn_native {

    RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                         EncodingContext* encodingContext,
                                         Ref<AttachmentState> attachmentState)
        : ProgrammablePassEncoder(device, encodingContext),
          mAttachmentState(std::move(attachmentState)),
          mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
          mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {
    }

    RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                         EncodingContext* encodingContext,
                                         ErrorTag errorTag)
        : ProgrammablePassEncoder(device, encodingContext, errorTag),
          mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
          mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {
    }

    const AttachmentState* RenderEncoderBase::GetAttachmentState() const {
        ASSERT(!IsError());
        ASSERT(mAttachmentState != nullptr);
        return mAttachmentState.Get();
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
                    DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(
                        instanceCount, firstInstance));
                }

                DrawCmd* draw = allocator->Allocate<DrawCmd>(Command::Draw);
                draw->vertexCount = vertexCount;
                draw->instanceCount = instanceCount;
                draw->firstVertex = firstVertex;
                draw->firstInstance = firstInstance;

                return {};
            },
            "encoding Draw(%u, %u, %u, %u).", vertexCount, instanceCount, firstVertex,
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

                    DAWN_TRY(
                        mCommandBufferState.ValidateIndexBufferInRange(indexCount, firstIndex));

                    // Although we don't know actual vertex access range in CPU, we still call the
                    // ValidateBufferInRangeForVertexBuffer in order to deal with those vertex step
                    // mode vertex buffer with an array stride of zero.
                    DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForVertexBuffer(0, 0));
                    DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(
                        instanceCount, firstInstance));
                }

                DrawIndexedCmd* draw = allocator->Allocate<DrawIndexedCmd>(Command::DrawIndexed);
                draw->indexCount = indexCount;
                draw->instanceCount = instanceCount;
                draw->firstIndex = firstIndex;
                draw->baseVertex = baseVertex;
                draw->firstInstance = firstInstance;

                return {};
            },
            "encoding DrawIndexed(%u, %u, %u, %i, %u).", indexCount, instanceCount, firstIndex,
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
                cmd->indirectBuffer = indirectBuffer;
                cmd->indirectOffset = indirectOffset;

                mUsageTracker.BufferUsedAs(indirectBuffer, wgpu::BufferUsage::Indirect);

                return {};
            },
            "encoding DrawIndirect(%s, %u).", indirectBuffer, indirectOffset);
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

                    // Disallow draw indexed indirect until the validation is correctly implemented.
                    if (GetDevice()->IsToggleEnabled(Toggle::DisallowUnsafeAPIs)) {
                        return DAWN_VALIDATION_ERROR(
                            "DrawIndexedIndirect is disallowed because it doesn't correctly "
                            "validate that "
                            "the index range is valid yet.");
                    }

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
                if (IsValidationEnabled()) {
                    cmd->indirectBufferLocation = BufferLocation::New();
                    mIndirectDrawMetadata.AddIndexedIndirectDraw(
                        mCommandBufferState.GetIndexFormat(),
                        mCommandBufferState.GetIndexBufferSize(), indirectBuffer, indirectOffset,
                        cmd->indirectBufferLocation.Get());
                } else {
                    cmd->indirectBufferLocation =
                        BufferLocation::New(indirectBuffer, indirectOffset);
                }

                mUsageTracker.BufferUsedAs(indirectBuffer, wgpu::BufferUsage::Indirect);

                return {};
            },
            "encoding DrawIndexedIndirect(%s, %u).", indirectBuffer, indirectOffset);
    }

    void RenderEncoderBase::APISetPipeline(RenderPipelineBase* pipeline) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(GetDevice()->ValidateObject(pipeline));

                    // TODO(dawn:563): More detail about why the states are incompatible would be
                    // nice.
                    DAWN_INVALID_IF(
                        pipeline->GetAttachmentState() != mAttachmentState.Get(),
                        "Attachment state of %s is not compatible with the attachment state of %s",
                        pipeline, this);
                }

                mCommandBufferState.SetRenderPipeline(pipeline);

                SetRenderPipelineCmd* cmd =
                    allocator->Allocate<SetRenderPipelineCmd>(Command::SetRenderPipeline);
                cmd->pipeline = pipeline;

                return {};
            },
            "encoding SetPipeline(%s).", pipeline);
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
                                    "Index buffer offset (%u) is not a multiple of the size (%u)"
                                    "of %s.",
                                    offset, IndexFormatSize(format), format);

                    uint64_t bufferSize = buffer->GetSize();
                    DAWN_INVALID_IF(offset > bufferSize,
                                    "Index buffer offset (%u) is larger than the size (%u) of %s.",
                                    offset, bufferSize, buffer);

                    uint64_t remainingSize = bufferSize - offset;

                    // Temporarily treat 0 as undefined for size, and give a warning
                    // TODO(dawn:1058): Remove this if block
                    if (size == 0) {
                        size = wgpu::kWholeSize;
                        GetDevice()->EmitDeprecationWarning(
                            "Using size=0 to indicate default binding size for setIndexBuffer "
                            "is deprecated. In the future it will result in a zero-size binding. "
                            "Use `undefined` (wgpu::kWholeSize) or just omit the parameter "
                            "instead.");
                    }

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
            "encoding SetIndexBuffer(%s, %s, %u, %u).", buffer, format, offset, size);
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

                    DAWN_INVALID_IF(offset % 4 != 0,
                                    "Vertex buffer offset (%u) is not a multiple of 4", offset);

                    uint64_t bufferSize = buffer->GetSize();
                    DAWN_INVALID_IF(offset > bufferSize,
                                    "Vertex buffer offset (%u) is larger than the size (%u) of %s.",
                                    offset, bufferSize, buffer);

                    uint64_t remainingSize = bufferSize - offset;

                    // Temporarily treat 0 as undefined for size, and give a warning
                    // TODO(dawn:1058): Remove this if block
                    if (size == 0) {
                        size = wgpu::kWholeSize;
                        GetDevice()->EmitDeprecationWarning(
                            "Using size=0 to indicate default binding size for setVertexBuffer "
                            "is deprecated. In the future it will result in a zero-size binding. "
                            "Use `undefined` (wgpu::kWholeSize) or just omit the parameter "
                            "instead.");
                    }

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
            "encoding SetVertexBuffer(%u, %s, %u, %u).", slot, buffer, offset, size);
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
                    DAWN_TRY(ValidateSetBindGroup(groupIndex, group, dynamicOffsetCount,
                                                  dynamicOffsets));
                }

                RecordSetBindGroup(allocator, groupIndex, group, dynamicOffsetCount,
                                   dynamicOffsets);
                mCommandBufferState.SetBindGroup(groupIndex, group);
                mUsageTracker.AddBindGroup(group);

                return {};
            },
            "encoding SetBindGroup(%u, %s, %u).", groupIndexIn, group, dynamicOffsetCount);
    }

}  // namespace dawn_native
