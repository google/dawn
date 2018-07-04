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

#include "backend/CommandBuffer.h"

#include "backend/BindGroup.h"
#include "backend/Buffer.h"
#include "backend/CommandBufferStateTracker.h"
#include "backend/Commands.h"
#include "backend/ComputePipeline.h"
#include "backend/Device.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/RenderPipeline.h"
#include "backend/Texture.h"

#include <cstring>
#include <map>

namespace backend {

    namespace {

        bool ValidateCopyLocationFitsInTexture(CommandBufferBuilder* builder,
                                               const TextureCopyLocation& location) {
            const TextureBase* texture = location.texture.Get();
            if (location.level >= texture->GetNumMipLevels()) {
                builder->HandleError("Copy mip-level out of range");
                return false;
            }

            // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
            // overflows.
            uint64_t level = location.level;
            if (uint64_t(location.x) + uint64_t(location.width) >
                    (static_cast<uint64_t>(texture->GetWidth()) >> level) ||
                uint64_t(location.y) + uint64_t(location.height) >
                    (static_cast<uint64_t>(texture->GetHeight()) >> level)) {
                builder->HandleError("Copy would touch outside of the texture");
                return false;
            }

            // TODO(cwallez@chromium.org): Check the depth bound differently for 2D arrays and 3D
            // textures
            if (location.z != 0 || location.depth != 1) {
                builder->HandleError("No support for z != 0 and depth != 1 for now");
                return false;
            }

            return true;
        }

        bool FitsInBuffer(const BufferBase* buffer, uint32_t offset, uint32_t size) {
            uint32_t bufferSize = buffer->GetSize();
            return offset <= bufferSize && (size <= (bufferSize - offset));
        }

        bool ValidateCopySizeFitsInBuffer(CommandBufferBuilder* builder,
                                          const BufferCopyLocation& location,
                                          uint32_t dataSize) {
            if (!FitsInBuffer(location.buffer.Get(), location.offset, dataSize)) {
                builder->HandleError("Copy would overflow the buffer");
                return false;
            }

            return true;
        }

        bool ValidateTexelBufferOffset(CommandBufferBuilder* builder,
                                       TextureBase* texture,
                                       const BufferCopyLocation& location) {
            uint32_t texelSize =
                static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat()));
            if (location.offset % texelSize != 0) {
                builder->HandleError("Buffer offset must be a multiple of the texel size");
                return false;
            }

            return true;
        }

        bool ComputeTextureCopyBufferSize(CommandBufferBuilder*,
                                          const TextureCopyLocation& location,
                                          uint32_t rowPitch,
                                          uint32_t* bufferSize) {
            // TODO(cwallez@chromium.org): check for overflows
            *bufferSize = (rowPitch * (location.height - 1) + location.width) * location.depth;

            return true;
        }

        uint32_t ComputeDefaultRowPitch(TextureBase* texture, uint32_t width) {
            uint32_t texelSize = TextureFormatPixelSize(texture->GetFormat());
            return texelSize * width;
        }

        bool ValidateRowPitch(CommandBufferBuilder* builder,
                              const TextureCopyLocation& location,
                              uint32_t rowPitch) {
            if (rowPitch % kTextureRowPitchAlignment != 0) {
                builder->HandleError("Row pitch must be a multiple of 256");
                return false;
            }

            uint32_t texelSize = TextureFormatPixelSize(location.texture.Get()->GetFormat());
            if (rowPitch < location.width * texelSize) {
                builder->HandleError("Row pitch must not be less than the number of bytes per row");
                return false;
            }

            return true;
        }

    }  // namespace

    CommandBufferBase::CommandBufferBase(CommandBufferBuilder* builder)
        : mDevice(builder->mDevice),
          mBuffersTransitioned(std::move(builder->mState->mBuffersTransitioned)),
          mTexturesTransitioned(std::move(builder->mState->mTexturesTransitioned)) {
    }

    bool CommandBufferBase::ValidateResourceUsagesImmediate() {
        for (auto buffer : mBuffersTransitioned) {
            if (buffer->IsFrozen()) {
                mDevice->HandleError("Command buffer: cannot transition buffer with frozen usage");
                return false;
            }
        }
        for (auto texture : mTexturesTransitioned) {
            if (texture->IsFrozen()) {
                mDevice->HandleError("Command buffer: cannot transition texture with frozen usage");
                return false;
            }
        }
        return true;
    }

    DeviceBase* CommandBufferBase::GetDevice() {
        return mDevice;
    }

    CommandBufferBuilder::CommandBufferBuilder(DeviceBase* device)
        : Builder(device), mState(std::make_unique<CommandBufferStateTracker>(this)) {
    }

    CommandBufferBuilder::~CommandBufferBuilder() {
        if (!mWereCommandsAcquired) {
            MoveToIterator();
            FreeCommands(&mIterator);
        }
    }

    bool CommandBufferBuilder::ValidateGetResult() {
        MoveToIterator();

        Command type;
        while (mIterator.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mIterator.NextCommand<BeginComputePassCmd>();
                    if (!mState->BeginComputePass()) {
                        return false;
                    }
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mIterator.NextCommand<BeginRenderPassCmd>();
                    RenderPassDescriptorBase* info = cmd->info.Get();
                    if (!mState->BeginRenderPass(info)) {
                        return false;
                    }
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mIterator.NextCommand<CopyBufferToBufferCmd>();
                    if (!ValidateCopySizeFitsInBuffer(this, copy->source, copy->size) ||
                        !ValidateCopySizeFitsInBuffer(this, copy->destination, copy->size) ||
                        !mState->ValidateCanCopy() ||
                        !mState->ValidateCanUseBufferAs(copy->source.buffer.Get(),
                                                        nxt::BufferUsageBit::TransferSrc) ||
                        !mState->ValidateCanUseBufferAs(copy->destination.buffer.Get(),
                                                        nxt::BufferUsageBit::TransferDst)) {
                        return false;
                    }
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mIterator.NextCommand<CopyBufferToTextureCmd>();

                    uint32_t bufferCopySize = 0;
                    if (!ValidateRowPitch(this, copy->destination, copy->rowPitch) ||
                        !ComputeTextureCopyBufferSize(this, copy->destination, copy->rowPitch,
                                                      &bufferCopySize) ||
                        !ValidateCopyLocationFitsInTexture(this, copy->destination) ||
                        !ValidateCopySizeFitsInBuffer(this, copy->source, bufferCopySize) ||
                        !ValidateTexelBufferOffset(this, copy->destination.texture.Get(),
                                                   copy->source) ||
                        !mState->ValidateCanCopy() ||
                        !mState->ValidateCanUseBufferAs(copy->source.buffer.Get(),
                                                        nxt::BufferUsageBit::TransferSrc) ||
                        !mState->ValidateCanUseTextureAs(copy->destination.texture.Get(),
                                                         nxt::TextureUsageBit::TransferDst)) {
                        return false;
                    }
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mIterator.NextCommand<CopyTextureToBufferCmd>();

                    uint32_t bufferCopySize = 0;
                    if (!ValidateRowPitch(this, copy->source, copy->rowPitch) ||
                        !ComputeTextureCopyBufferSize(this, copy->source, copy->rowPitch,
                                                      &bufferCopySize) ||
                        !ValidateCopyLocationFitsInTexture(this, copy->source) ||
                        !ValidateCopySizeFitsInBuffer(this, copy->destination, bufferCopySize) ||
                        !ValidateTexelBufferOffset(this, copy->source.texture.Get(),
                                                   copy->destination) ||
                        !mState->ValidateCanCopy() ||
                        !mState->ValidateCanUseTextureAs(copy->source.texture.Get(),
                                                         nxt::TextureUsageBit::TransferSrc) ||
                        !mState->ValidateCanUseBufferAs(copy->destination.buffer.Get(),
                                                        nxt::BufferUsageBit::TransferDst)) {
                        return false;
                    }
                } break;

                case Command::Dispatch: {
                    mIterator.NextCommand<DispatchCmd>();
                    if (!mState->ValidateCanDispatch()) {
                        return false;
                    }
                } break;

                case Command::DrawArrays: {
                    mIterator.NextCommand<DrawArraysCmd>();
                    if (!mState->ValidateCanDrawArrays()) {
                        return false;
                    }
                } break;

                case Command::DrawElements: {
                    mIterator.NextCommand<DrawElementsCmd>();
                    if (!mState->ValidateCanDrawElements()) {
                        return false;
                    }
                } break;

                case Command::EndComputePass: {
                    mIterator.NextCommand<EndComputePassCmd>();
                    if (!mState->EndComputePass()) {
                        return false;
                    }
                } break;

                case Command::EndRenderPass: {
                    mIterator.NextCommand<EndRenderPassCmd>();
                    if (!mState->EndRenderPass()) {
                        return false;
                    }
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mIterator.NextCommand<SetComputePipelineCmd>();
                    ComputePipelineBase* pipeline = cmd->pipeline.Get();
                    if (!mState->SetComputePipeline(pipeline)) {
                        return false;
                    }
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mIterator.NextCommand<SetRenderPipelineCmd>();
                    RenderPipelineBase* pipeline = cmd->pipeline.Get();
                    if (!mState->SetRenderPipeline(pipeline)) {
                        return false;
                    }
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mIterator.NextCommand<SetPushConstantsCmd>();
                    mIterator.NextData<uint32_t>(cmd->count);
                    // Validation of count and offset has already been done when the command was
                    // recorded because it impacts the size of an allocation in the
                    // CommandAllocator.
                    if (!mState->ValidateSetPushConstants(cmd->stages)) {
                        return false;
                    }
                } break;

                case Command::SetStencilReference: {
                    mIterator.NextCommand<SetStencilReferenceCmd>();
                    if (!mState->HaveRenderPass()) {
                        HandleError("Can't set stencil reference without an active render pass");
                        return false;
                    }
                } break;

                case Command::SetBlendColor: {
                    mIterator.NextCommand<SetBlendColorCmd>();
                    if (!mState->HaveRenderPass()) {
                        HandleError("Can't set blend color without an active render pass");
                        return false;
                    }
                } break;

                case Command::SetScissorRect: {
                    mIterator.NextCommand<SetScissorRectCmd>();
                    if (!mState->HaveRenderPass()) {
                        HandleError("Can't set scissor rect without an active render pass");
                        return false;
                    }
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mIterator.NextCommand<SetBindGroupCmd>();
                    if (!mState->SetBindGroup(cmd->index, cmd->group.Get())) {
                        return false;
                    }
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mIterator.NextCommand<SetIndexBufferCmd>();
                    if (!mState->SetIndexBuffer(cmd->buffer.Get())) {
                        return false;
                    }
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mIterator.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mIterator.NextData<Ref<BufferBase>>(cmd->count);
                    mIterator.NextData<uint32_t>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        mState->SetVertexBuffer(cmd->startSlot + i, buffers[i].Get());
                    }
                } break;

                case Command::TransitionBufferUsage: {
                    TransitionBufferUsageCmd* cmd =
                        mIterator.NextCommand<TransitionBufferUsageCmd>();
                    if (!mState->TransitionBufferUsage(cmd->buffer.Get(), cmd->usage)) {
                        return false;
                    }
                } break;

                case Command::TransitionTextureUsage: {
                    TransitionTextureUsageCmd* cmd =
                        mIterator.NextCommand<TransitionTextureUsageCmd>();
                    if (!mState->TransitionTextureUsage(cmd->texture.Get(), cmd->usage)) {
                        return false;
                    }

                } break;
            }
        }

        if (!mState->ValidateEndCommandBuffer()) {
            return false;
        }

        return true;
    }

    CommandIterator CommandBufferBuilder::AcquireCommands() {
        ASSERT(!mWereCommandsAcquired);
        mWereCommandsAcquired = true;
        return std::move(mIterator);
    }

    CommandBufferBase* CommandBufferBuilder::GetResultImpl() {
        MoveToIterator();
        return mDevice->CreateCommandBuffer(this);
    }

    void CommandBufferBuilder::BeginComputePass() {
        mAllocator.Allocate<BeginComputePassCmd>(Command::BeginComputePass);
    }

    void CommandBufferBuilder::BeginRenderPass(RenderPassDescriptorBase* info) {
        BeginRenderPassCmd* cmd = mAllocator.Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);
        new (cmd) BeginRenderPassCmd;
        cmd->info = info;
    }

    void CommandBufferBuilder::CopyBufferToBuffer(BufferBase* source,
                                                  uint32_t sourceOffset,
                                                  BufferBase* destination,
                                                  uint32_t destinationOffset,
                                                  uint32_t size) {
        CopyBufferToBufferCmd* copy =
            mAllocator.Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
        new (copy) CopyBufferToBufferCmd;
        copy->source.buffer = source;
        copy->source.offset = sourceOffset;
        copy->destination.buffer = destination;
        copy->destination.offset = destinationOffset;
        copy->size = size;
    }

    void CommandBufferBuilder::CopyBufferToTexture(BufferBase* buffer,
                                                   uint32_t bufferOffset,
                                                   uint32_t rowPitch,
                                                   TextureBase* texture,
                                                   uint32_t x,
                                                   uint32_t y,
                                                   uint32_t z,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   uint32_t depth,
                                                   uint32_t level) {
        if (rowPitch == 0) {
            rowPitch = ComputeDefaultRowPitch(texture, width);
        }
        CopyBufferToTextureCmd* copy =
            mAllocator.Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
        new (copy) CopyBufferToTextureCmd;
        copy->source.buffer = buffer;
        copy->source.offset = bufferOffset;
        copy->destination.texture = texture;
        copy->destination.x = x;
        copy->destination.y = y;
        copy->destination.z = z;
        copy->destination.width = width;
        copy->destination.height = height;
        copy->destination.depth = depth;
        copy->destination.level = level;
        copy->rowPitch = rowPitch;
    }

    void CommandBufferBuilder::CopyTextureToBuffer(TextureBase* texture,
                                                   uint32_t x,
                                                   uint32_t y,
                                                   uint32_t z,
                                                   uint32_t width,
                                                   uint32_t height,
                                                   uint32_t depth,
                                                   uint32_t level,
                                                   BufferBase* buffer,
                                                   uint32_t bufferOffset,
                                                   uint32_t rowPitch) {
        if (rowPitch == 0) {
            rowPitch = ComputeDefaultRowPitch(texture, width);
        }
        CopyTextureToBufferCmd* copy =
            mAllocator.Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
        new (copy) CopyTextureToBufferCmd;
        copy->source.texture = texture;
        copy->source.x = x;
        copy->source.y = y;
        copy->source.z = z;
        copy->source.width = width;
        copy->source.height = height;
        copy->source.depth = depth;
        copy->source.level = level;
        copy->destination.buffer = buffer;
        copy->destination.offset = bufferOffset;
        copy->rowPitch = rowPitch;
    }

    void CommandBufferBuilder::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
        DispatchCmd* dispatch = mAllocator.Allocate<DispatchCmd>(Command::Dispatch);
        new (dispatch) DispatchCmd;
        dispatch->x = x;
        dispatch->y = y;
        dispatch->z = z;
    }

    void CommandBufferBuilder::DrawArrays(uint32_t vertexCount,
                                          uint32_t instanceCount,
                                          uint32_t firstVertex,
                                          uint32_t firstInstance) {
        DrawArraysCmd* draw = mAllocator.Allocate<DrawArraysCmd>(Command::DrawArrays);
        new (draw) DrawArraysCmd;
        draw->vertexCount = vertexCount;
        draw->instanceCount = instanceCount;
        draw->firstVertex = firstVertex;
        draw->firstInstance = firstInstance;
    }

    void CommandBufferBuilder::DrawElements(uint32_t indexCount,
                                            uint32_t instanceCount,
                                            uint32_t firstIndex,
                                            uint32_t firstInstance) {
        DrawElementsCmd* draw = mAllocator.Allocate<DrawElementsCmd>(Command::DrawElements);
        new (draw) DrawElementsCmd;
        draw->indexCount = indexCount;
        draw->instanceCount = instanceCount;
        draw->firstIndex = firstIndex;
        draw->firstInstance = firstInstance;
    }

    void CommandBufferBuilder::EndComputePass() {
        mAllocator.Allocate<EndComputePassCmd>(Command::EndComputePass);
    }

    void CommandBufferBuilder::EndRenderPass() {
        mAllocator.Allocate<EndRenderPassCmd>(Command::EndRenderPass);
    }

    void CommandBufferBuilder::SetComputePipeline(ComputePipelineBase* pipeline) {
        SetComputePipelineCmd* cmd =
            mAllocator.Allocate<SetComputePipelineCmd>(Command::SetComputePipeline);
        new (cmd) SetComputePipelineCmd;
        cmd->pipeline = pipeline;
    }

    void CommandBufferBuilder::SetRenderPipeline(RenderPipelineBase* pipeline) {
        SetRenderPipelineCmd* cmd =
            mAllocator.Allocate<SetRenderPipelineCmd>(Command::SetRenderPipeline);
        new (cmd) SetRenderPipelineCmd;
        cmd->pipeline = pipeline;
    }

    void CommandBufferBuilder::SetPushConstants(nxt::ShaderStageBit stages,
                                                uint32_t offset,
                                                uint32_t count,
                                                const void* data) {
        // TODO(cwallez@chromium.org): check for overflows
        if (offset + count > kMaxPushConstants) {
            HandleError("Setting too many push constants");
            return;
        }

        SetPushConstantsCmd* cmd =
            mAllocator.Allocate<SetPushConstantsCmd>(Command::SetPushConstants);
        new (cmd) SetPushConstantsCmd;
        cmd->stages = stages;
        cmd->offset = offset;
        cmd->count = count;

        uint32_t* values = mAllocator.AllocateData<uint32_t>(count);
        memcpy(values, data, count * sizeof(uint32_t));
    }

    void CommandBufferBuilder::SetStencilReference(uint32_t reference) {
        SetStencilReferenceCmd* cmd =
            mAllocator.Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
        new (cmd) SetStencilReferenceCmd;
        cmd->reference = reference;
    }

    void CommandBufferBuilder::SetBlendColor(float r, float g, float b, float a) {
        SetBlendColorCmd* cmd = mAllocator.Allocate<SetBlendColorCmd>(Command::SetBlendColor);
        new (cmd) SetBlendColorCmd;
        cmd->r = r;
        cmd->g = g;
        cmd->b = b;
        cmd->a = a;
    }

    void CommandBufferBuilder::SetScissorRect(uint32_t x,
                                              uint32_t y,
                                              uint32_t width,
                                              uint32_t height) {
        SetScissorRectCmd* cmd = mAllocator.Allocate<SetScissorRectCmd>(Command::SetScissorRect);
        new (cmd) SetScissorRectCmd;
        cmd->x = x;
        cmd->y = y;
        cmd->width = width;
        cmd->height = height;
    }

    void CommandBufferBuilder::SetBindGroup(uint32_t groupIndex, BindGroupBase* group) {
        if (groupIndex >= kMaxBindGroups) {
            HandleError("Setting bind group over the max");
            return;
        }

        SetBindGroupCmd* cmd = mAllocator.Allocate<SetBindGroupCmd>(Command::SetBindGroup);
        new (cmd) SetBindGroupCmd;
        cmd->index = groupIndex;
        cmd->group = group;
    }

    void CommandBufferBuilder::SetIndexBuffer(BufferBase* buffer, uint32_t offset) {
        // TODO(kainino@chromium.org): validation

        SetIndexBufferCmd* cmd = mAllocator.Allocate<SetIndexBufferCmd>(Command::SetIndexBuffer);
        new (cmd) SetIndexBufferCmd;
        cmd->buffer = buffer;
        cmd->offset = offset;
    }

    void CommandBufferBuilder::SetVertexBuffers(uint32_t startSlot,
                                                uint32_t count,
                                                BufferBase* const* buffers,
                                                uint32_t const* offsets) {
        // TODO(kainino@chromium.org): validation

        SetVertexBuffersCmd* cmd =
            mAllocator.Allocate<SetVertexBuffersCmd>(Command::SetVertexBuffers);
        new (cmd) SetVertexBuffersCmd;
        cmd->startSlot = startSlot;
        cmd->count = count;

        Ref<BufferBase>* cmdBuffers = mAllocator.AllocateData<Ref<BufferBase>>(count);
        for (size_t i = 0; i < count; ++i) {
            new (&cmdBuffers[i]) Ref<BufferBase>(buffers[i]);
        }

        uint32_t* cmdOffsets = mAllocator.AllocateData<uint32_t>(count);
        memcpy(cmdOffsets, offsets, count * sizeof(uint32_t));
    }

    void CommandBufferBuilder::TransitionBufferUsage(BufferBase* buffer,
                                                     nxt::BufferUsageBit usage) {
        TransitionBufferUsageCmd* cmd =
            mAllocator.Allocate<TransitionBufferUsageCmd>(Command::TransitionBufferUsage);
        new (cmd) TransitionBufferUsageCmd;
        cmd->buffer = buffer;
        cmd->usage = usage;
    }

    void CommandBufferBuilder::TransitionTextureUsage(TextureBase* texture,
                                                      nxt::TextureUsageBit usage) {
        TransitionTextureUsageCmd* cmd =
            mAllocator.Allocate<TransitionTextureUsageCmd>(Command::TransitionTextureUsage);
        new (cmd) TransitionTextureUsageCmd;
        cmd->texture = texture;
        cmd->usage = usage;
    }

    void CommandBufferBuilder::MoveToIterator() {
        if (!mWasMovedToIterator) {
            mIterator = std::move(mAllocator);
            mWasMovedToIterator = true;
        }
    }

}  // namespace backend
