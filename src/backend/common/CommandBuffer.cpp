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
                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands->NextCommand<CopyBufferToTextureCmd>();
                        copy->~CopyBufferToTextureCmd();
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
                        if (!state->BeginRenderPass(renderPass, framebuffer)) {
                            return false;
                        }
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = iterator.NextCommand<CopyBufferToTextureCmd>();
                        BufferBase* buffer = copy->buffer.Get();
                        uint32_t bufferOffset = copy->bufferOffset;
                        TextureBase* texture = copy->texture.Get();
                        uint64_t width = copy->width;
                        uint64_t height = copy->height;
                        uint64_t depth = copy->depth;
                        uint64_t x = copy->x;
                        uint64_t y = copy->y;
                        uint64_t z = copy->z;
                        uint32_t level = copy->level;

                        if (width == 0 || height == 0 || depth == 0) {
                            HandleError("Empty copy");
                            return false;
                        }

                        // TODO(cwallez@chromium.org): check for overflows
                        uint64_t pixelSize = TextureFormatPixelSize(texture->GetFormat());
                        uint64_t dataSize = width * height * depth * pixelSize;
                        if (dataSize + static_cast<uint64_t>(bufferOffset) > static_cast<uint64_t>(buffer->GetSize())) {
                            HandleError("Copy would read after end of the buffer");
                            return false;
                        }

                        if (x + width > static_cast<uint64_t>(texture->GetWidth()) ||
                            y + height > static_cast<uint64_t>(texture->GetHeight()) ||
                            z + depth > static_cast<uint64_t>(texture->GetDepth()) ||
                            level > texture->GetNumMipLevels()) {
                            HandleError("Copy would write outside of the texture");
                            return false;
                        }

                        if (!state->ValidateCanCopy() ||
                            !state->ValidateCanUseBufferAs(buffer, nxt::BufferUsageBit::TransferSrc) ||
                            !state->ValidateCanUseTextureAs(texture, nxt::TextureUsageBit::TransferDst)) {
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

    void CommandBufferBuilder::CopyBufferToTexture(BufferBase* buffer, uint32_t bufferOffset,
                                                   TextureBase* texture, uint32_t x, uint32_t y, uint32_t z,
                                                   uint32_t width, uint32_t height, uint32_t depth, uint32_t level) {
        CopyBufferToTextureCmd* copy = allocator.Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
        new(copy) CopyBufferToTextureCmd;
        copy->buffer = buffer;
        copy->bufferOffset = bufferOffset;
        copy->texture = texture;
        copy->x = x;
        copy->y = y;
        copy->z = z;
        copy->width = width;
        copy->height = height;
        copy->depth = depth;
        copy->level = level;
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
