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

#include "CommandBuffer.h"

#include "BindGroup.h"
#include "Buffer.h"
#include "Commands.h"
#include "CommandBufferStateTracker.h"
#include "Device.h"
#include "InputState.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "Texture.h"

#include <cstring>
#include <map>

namespace backend {

    namespace {

        bool ValidateCopyLocationFitsInTexture(CommandBufferBuilder* builder, const TextureCopyLocation& location) {
            const TextureBase* texture = location.texture.Get();
            if (location.level >= texture->GetNumMipLevels()) {
                builder->HandleError("Copy mip-level out of range");
                return false;
            }

            // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid overflows.
            uint64_t level = location.level;
            if (uint64_t(location.x) + uint64_t(location.width) > (static_cast<uint64_t>(texture->GetWidth()) >> level) ||
                uint64_t(location.y) + uint64_t(location.height) > (static_cast<uint64_t>(texture->GetHeight()) >> level)) {
                builder->HandleError("Copy would touch outside of the texture");
                return false;
            }

            // TODO(cwallez@chromium.org): Check the depth bound differently for 2D arrays and 3D textures
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

        bool ValidateCopySizeFitsInBuffer(CommandBufferBuilder* builder, const BufferCopyLocation& location, uint32_t dataSize) {
            if (!FitsInBuffer(location.buffer.Get(), location.offset, dataSize)) {
                builder->HandleError("Copy would overflow the buffer");
                return false;
            }

            return true;
        }

        bool ComputeTextureCopyBufferSize(CommandBufferBuilder* builder, const TextureCopyLocation& location, uint32_t* bufferSize) {
            // TODO(cwallez@chromium.org): check for overflows
            uint32_t pixelSize = TextureFormatPixelSize(location.texture->GetFormat());
            *bufferSize = location.width * location.height * location.depth * pixelSize;

            return true;
        }

    }

    CommandBufferBase::CommandBufferBase(CommandBufferBuilder* builder)
        : device(builder->device),
          buffersTransitioned(std::move(builder->state->buffersTransitioned)),
          texturesTransitioned(std::move(builder->state->texturesTransitioned)) {
    }

    bool CommandBufferBase::ValidateResourceUsagesImmediate() {
        for (auto buffer : buffersTransitioned) {
            if (buffer->IsFrozen()) {
                device->HandleError("Command buffer: cannot transition buffer with frozen usage");
                return false;
            }
        }
        for (auto texture : texturesTransitioned) {
            if (texture->IsFrozen()) {
                device->HandleError("Command buffer: cannot transition texture with frozen usage");
                return false;
            }
        }
        return true;
    }

    void FreeCommands(CommandIterator* commands) {
        Command type;
        while(commands->NextCommandId(&type)) {
            switch (type) {
                case Command::AdvanceSubpass:
                    {
                        AdvanceSubpassCmd* cmd = commands->NextCommand<AdvanceSubpassCmd>();
                        cmd->~AdvanceSubpassCmd();
                    }
                    break;
                case Command::BeginRenderPass:
                    {
                        BeginRenderPassCmd* begin = commands->NextCommand<BeginRenderPassCmd>();
                        begin->~BeginRenderPassCmd();
                    }
                    break;
                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = commands->NextCommand<CopyBufferToBufferCmd>();
                        copy->~CopyBufferToBufferCmd();
                    }
                    break;
                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands->NextCommand<CopyBufferToTextureCmd>();
                        copy->~CopyBufferToTextureCmd();
                    }
                    break;
                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = commands->NextCommand<CopyTextureToBufferCmd>();
                        copy->~CopyTextureToBufferCmd();
                    }
                    break;
                case Command::Dispatch:
                    {
                        DispatchCmd* dispatch = commands->NextCommand<DispatchCmd>();
                        dispatch->~DispatchCmd();
                    }
                    break;
                case Command::DrawArrays:
                    {
                        DrawArraysCmd* draw = commands->NextCommand<DrawArraysCmd>();
                        draw->~DrawArraysCmd();
                    }
                    break;
                case Command::DrawElements:
                    {
                        DrawElementsCmd* draw = commands->NextCommand<DrawElementsCmd>();
                        draw->~DrawElementsCmd();
                    }
                    break;
                case Command::EndRenderPass:
                    {
                        EndRenderPassCmd* cmd = commands->NextCommand<EndRenderPassCmd>();
                        cmd->~EndRenderPassCmd();
                    }
                    break;
                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = commands->NextCommand<SetPipelineCmd>();
                        cmd->~SetPipelineCmd();
                    }
                    break;
                case Command::SetPushConstants:
                    {
                        SetPushConstantsCmd* cmd = commands->NextCommand<SetPushConstantsCmd>();
                        commands->NextData<uint32_t>(cmd->count);
                        cmd->~SetPushConstantsCmd();
                    }
                    break;
                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd* cmd = commands->NextCommand<SetStencilReferenceCmd>();
                        cmd->~SetStencilReferenceCmd();
                    }
                    break;
                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                        cmd->~SetBindGroupCmd();
                    }
                    break;
                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = commands->NextCommand<SetIndexBufferCmd>();
                        cmd->~SetIndexBufferCmd();
                    }
                    break;
                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = commands->NextCommand<SetVertexBuffersCmd>();
                        auto buffers = commands->NextData<Ref<BufferBase>>(cmd->count);
                        for (size_t i = 0; i < cmd->count; ++i) {
                            (&buffers[i])->~Ref<BufferBase>();
                        }
                        commands->NextData<uint32_t>(cmd->count);
                        cmd->~SetVertexBuffersCmd();
                    }
                    break;
                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = commands->NextCommand<TransitionBufferUsageCmd>();
                        cmd->~TransitionBufferUsageCmd();
                    }
                    break;
                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = commands->NextCommand<TransitionTextureUsageCmd>();
                        cmd->~TransitionTextureUsageCmd();
                    }
                    break;
            }
        }
        commands->DataWasDestroyed();
    }

    CommandBufferBuilder::CommandBufferBuilder(DeviceBase* device) : Builder(device), state(std::make_unique<CommandBufferStateTracker>(this)) {
    }

    CommandBufferBuilder::~CommandBufferBuilder() {
        if (!commandsAcquired) {
            MoveToIterator();
            FreeCommands(&iterator);
        }
    }

    bool CommandBufferBuilder::ValidateGetResult() {
        MoveToIterator();

        Command type;
        while (iterator.NextCommandId(&type)) {
            switch (type) {
                case Command::AdvanceSubpass:
                    {
                        iterator.NextCommand<AdvanceSubpassCmd>();
                        if (!state->AdvanceSubpass()) {
                            return false;
                        }
                    }
                    break;

                case Command::BeginRenderPass:
                    {
                        BeginRenderPassCmd* cmd = iterator.NextCommand<BeginRenderPassCmd>();
                        auto* renderPass = cmd->renderPass.Get();
                        auto* framebuffer = cmd->framebuffer.Get();
                        // TODO(kainino@chromium.org): null checks should not be necessary
                        if (renderPass == nullptr) {
                            HandleError("Render pass is invalid");
                            return false;
                        }
                        if (framebuffer == nullptr) {
                            HandleError("Framebuffer is invalid");
                            return false;
                        }
                        if (!state->BeginRenderPass(renderPass, framebuffer)) {
                            return false;
                        }
                    }
                    break;

                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = iterator.NextCommand<CopyBufferToBufferCmd>();
                        if (!ValidateCopySizeFitsInBuffer(this, copy->source, copy->size) ||
                            !ValidateCopySizeFitsInBuffer(this, copy->destination, copy->size) ||
                            !state->ValidateCanCopy() ||
                            !state->ValidateCanUseBufferAs(copy->source.buffer.Get(), nxt::BufferUsageBit::TransferSrc) ||
                            !state->ValidateCanUseBufferAs(copy->destination.buffer.Get(), nxt::BufferUsageBit::TransferDst)) {
                            return false;
                        }
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = iterator.NextCommand<CopyBufferToTextureCmd>();

                        uint32_t bufferCopySize = 0;
                        if (!ComputeTextureCopyBufferSize(this, copy->destination, &bufferCopySize) ||
                            !ValidateCopyLocationFitsInTexture(this, copy->destination) ||
                            !ValidateCopySizeFitsInBuffer(this, copy->source, bufferCopySize) ||
                            !state->ValidateCanCopy() ||
                            !state->ValidateCanUseBufferAs(copy->source.buffer.Get(), nxt::BufferUsageBit::TransferSrc) ||
                            !state->ValidateCanUseTextureAs(copy->destination.texture.Get(), nxt::TextureUsageBit::TransferDst)) {
                            return false;
                        }
                    }
                    break;

                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = iterator.NextCommand<CopyTextureToBufferCmd>();

                        uint32_t bufferCopySize = 0;
                        if (!ComputeTextureCopyBufferSize(this, copy->source, &bufferCopySize) ||
                            !ValidateCopyLocationFitsInTexture(this, copy->source) ||
                            !ValidateCopySizeFitsInBuffer(this, copy->destination, bufferCopySize) ||
                            !state->ValidateCanCopy() ||
                            !state->ValidateCanUseTextureAs(copy->source.texture.Get(), nxt::TextureUsageBit::TransferSrc) ||
                            !state->ValidateCanUseBufferAs(copy->destination.buffer.Get(), nxt::BufferUsageBit::TransferDst)) {
                            return false;
                        }
                    }
                    break;

                case Command::Dispatch:
                    {
                        iterator.NextCommand<DispatchCmd>();
                        if (!state->ValidateCanDispatch()) {
                            return false;
                        }
                    }
                    break;

                case Command::DrawArrays:
                    {
                        iterator.NextCommand<DrawArraysCmd>();
                        if (!state->ValidateCanDrawArrays()) {
                            return false;
                        }
                    }
                    break;

                case Command::DrawElements:
                    {
                        iterator.NextCommand<DrawElementsCmd>();
                        if (!state->ValidateCanDrawElements()) {
                            return false;
                        }
                    }
                    break;

                case Command::EndRenderPass:
                    {
                        iterator.NextCommand<EndRenderPassCmd>();
                        if (!state->EndRenderPass()) {
                            return false;
                        }
                    }
                    break;

                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = iterator.NextCommand<SetPipelineCmd>();
                        PipelineBase* pipeline = cmd->pipeline.Get();
                        if (!state->SetPipeline(pipeline)) {
                            return false;
                        }
                    }
                    break;

                case Command::SetPushConstants:
                    {
                        SetPushConstantsCmd* cmd = iterator.NextCommand<SetPushConstantsCmd>();
                        iterator.NextData<uint32_t>(cmd->count);
                        if (cmd->count + cmd->offset > kMaxPushConstants) {
                            HandleError("Setting pushconstants past the limit");
                            return false;
                        }
                    }
                    break;

                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd* cmd = iterator.NextCommand<SetStencilReferenceCmd>();
                        if (!state->HaveRenderPass()) {
                            HandleError("Can't set stencil reference without an active render pass");
                            return false;
                        }
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = iterator.NextCommand<SetBindGroupCmd>();
                        if (!state->SetBindGroup(cmd->index, cmd->group.Get())) {
                            return false;
                        }
                    }
                    break;

                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = iterator.NextCommand<SetIndexBufferCmd>();
                        if (!state->SetIndexBuffer(cmd->buffer.Get())) {
                            return false;
                        }
                    }
                    break;

                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = iterator.NextCommand<SetVertexBuffersCmd>();
                        auto buffers = iterator.NextData<Ref<BufferBase>>(cmd->count);
                        iterator.NextData<uint32_t>(cmd->count);

                        for (uint32_t i = 0; i < cmd->count; ++i) {
                            state->SetVertexBuffer(cmd->startSlot + i, buffers[i].Get());
                        }
                    }
                    break;

                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = iterator.NextCommand<TransitionBufferUsageCmd>();
                        if (!state->TransitionBufferUsage(cmd->buffer.Get(), cmd->usage)) {
                            return false;
                        }
                    }
                    break;

                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = iterator.NextCommand<TransitionTextureUsageCmd>();
                        if (!state->TransitionTextureUsage(cmd->texture.Get(), cmd->usage)) {
                            return false;
                        }

                    }
                    break;
            }
        }

        if (!state->ValidateEndCommandBuffer()) {
            return false;
        }

        return true;
    }

    CommandIterator CommandBufferBuilder::AcquireCommands() {
        ASSERT(!commandsAcquired);
        commandsAcquired = true;
        return std::move(iterator);
    }

    CommandBufferBase* CommandBufferBuilder::GetResultImpl() {
        MoveToIterator();
        return device->CreateCommandBuffer(this);
    }

    void CommandBufferBuilder::AdvanceSubpass() {
        allocator.Allocate<AdvanceSubpassCmd>(Command::AdvanceSubpass);
    }

    void CommandBufferBuilder::BeginRenderPass(RenderPassBase* renderPass, FramebufferBase* framebuffer) {
        BeginRenderPassCmd* cmd = allocator.Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);
        new(cmd) BeginRenderPassCmd;
        cmd->renderPass = renderPass;
        cmd->framebuffer = framebuffer;
    }

    void CommandBufferBuilder::CopyBufferToBuffer(BufferBase* source, uint32_t sourceOffset, BufferBase* destination, uint32_t destinationOffset, uint32_t size) {
        CopyBufferToBufferCmd* copy = allocator.Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
        new(copy) CopyBufferToBufferCmd;
        copy->source.buffer = source;
        copy->source.offset = sourceOffset;
        copy->destination.buffer = destination;
        copy->destination.offset = destinationOffset;
        copy->size = size;
    }

    void CommandBufferBuilder::CopyBufferToTexture(BufferBase* buffer, uint32_t bufferOffset,
                                                   TextureBase* texture, uint32_t x, uint32_t y, uint32_t z,
                                                   uint32_t width, uint32_t height, uint32_t depth, uint32_t level) {
        CopyBufferToTextureCmd* copy = allocator.Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
        new(copy) CopyBufferToTextureCmd;
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
    }

    void CommandBufferBuilder::CopyTextureToBuffer(TextureBase* texture, uint32_t x, uint32_t y, uint32_t z,
                                                  uint32_t width, uint32_t height, uint32_t depth, uint32_t level,
                                                  BufferBase* buffer, uint32_t bufferOffset) {
        CopyTextureToBufferCmd* copy = allocator.Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
        new(copy) CopyTextureToBufferCmd;
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
    }

    void CommandBufferBuilder::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
        DispatchCmd* dispatch = allocator.Allocate<DispatchCmd>(Command::Dispatch);
        new(dispatch) DispatchCmd;
        dispatch->x = x;
        dispatch->y = y;
        dispatch->z = z;
    }

    void CommandBufferBuilder::DrawArrays(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
        DrawArraysCmd* draw = allocator.Allocate<DrawArraysCmd>(Command::DrawArrays);
        new(draw) DrawArraysCmd;
        draw->vertexCount = vertexCount;
        draw->instanceCount = instanceCount;
        draw->firstVertex = firstVertex;
        draw->firstInstance = firstInstance;
    }

    void CommandBufferBuilder::DrawElements(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance) {
        DrawElementsCmd* draw = allocator.Allocate<DrawElementsCmd>(Command::DrawElements);
        new(draw) DrawElementsCmd;
        draw->indexCount = indexCount;
        draw->instanceCount = instanceCount;
        draw->firstIndex = firstIndex;
        draw->firstInstance = firstInstance;
    }

    void CommandBufferBuilder::EndRenderPass() {
        allocator.Allocate<EndRenderPassCmd>(Command::EndRenderPass);
    }

    void CommandBufferBuilder::SetPipeline(PipelineBase* pipeline) {
        SetPipelineCmd* cmd = allocator.Allocate<SetPipelineCmd>(Command::SetPipeline);
        new(cmd) SetPipelineCmd;
        cmd->pipeline = pipeline;
    }

    void CommandBufferBuilder::SetPushConstants(nxt::ShaderStageBit stage, uint32_t offset, uint32_t count, const void* data) {
        if (offset + count > kMaxPushConstants) {
            HandleError("Setting too many push constants");
            return;
        }

        SetPushConstantsCmd* cmd = allocator.Allocate<SetPushConstantsCmd>(Command::SetPushConstants);
        new(cmd) SetPushConstantsCmd;
        cmd->stage = stage;
        cmd->offset = offset;
        cmd->count = count;

        uint32_t* values = allocator.AllocateData<uint32_t>(count);
        memcpy(values, data, count * sizeof(uint32_t));
    }

    void CommandBufferBuilder::SetStencilReference(uint32_t reference) {
        SetStencilReferenceCmd* cmd = allocator.Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
        new(cmd) SetStencilReferenceCmd;
        cmd->reference = reference;
    }

    void CommandBufferBuilder::SetBindGroup(uint32_t groupIndex, BindGroupBase* group) {
        if (groupIndex >= kMaxBindGroups) {
            HandleError("Setting bind group over the max");
            return;
        }

        SetBindGroupCmd* cmd = allocator.Allocate<SetBindGroupCmd>(Command::SetBindGroup);
        new(cmd) SetBindGroupCmd;
        cmd->index = groupIndex;
        cmd->group = group;
    }

    void CommandBufferBuilder::SetIndexBuffer(BufferBase* buffer, uint32_t offset, nxt::IndexFormat format) {
        // TODO(kainino@chromium.org): validation

        SetIndexBufferCmd* cmd = allocator.Allocate<SetIndexBufferCmd>(Command::SetIndexBuffer);
        new(cmd) SetIndexBufferCmd;
        cmd->buffer = buffer;
        cmd->offset = offset;
        cmd->format = format;
    }

    void CommandBufferBuilder::SetVertexBuffers(uint32_t startSlot, uint32_t count, BufferBase* const* buffers, uint32_t const* offsets){
        // TODO(kainino@chromium.org): validation

        SetVertexBuffersCmd* cmd = allocator.Allocate<SetVertexBuffersCmd>(Command::SetVertexBuffers);
        new(cmd) SetVertexBuffersCmd;
        cmd->startSlot = startSlot;
        cmd->count = count;

        Ref<BufferBase>* cmdBuffers = allocator.AllocateData<Ref<BufferBase>>(count);
        for (size_t i = 0; i < count; ++i) {
            new(&cmdBuffers[i]) Ref<BufferBase>(buffers[i]);
        }

        uint32_t* cmdOffsets = allocator.AllocateData<uint32_t>(count);
        memcpy(cmdOffsets, offsets, count * sizeof(uint32_t));
    }

    void CommandBufferBuilder::TransitionBufferUsage(BufferBase* buffer, nxt::BufferUsageBit usage) {
        TransitionBufferUsageCmd* cmd = allocator.Allocate<TransitionBufferUsageCmd>(Command::TransitionBufferUsage);
        new(cmd) TransitionBufferUsageCmd;
        cmd->buffer = buffer;
        cmd->usage = usage;
    }

    void CommandBufferBuilder::TransitionTextureUsage(TextureBase* texture, nxt::TextureUsageBit usage) {
        TransitionTextureUsageCmd* cmd = allocator.Allocate<TransitionTextureUsageCmd>(Command::TransitionTextureUsage);
        new(cmd) TransitionTextureUsageCmd;
        cmd->texture = texture;
        cmd->usage = usage;
    }

    void CommandBufferBuilder::MoveToIterator() {
        if (!movedToIterator) {
            iterator = std::move(allocator);
            movedToIterator = true;
        }
    }

}
