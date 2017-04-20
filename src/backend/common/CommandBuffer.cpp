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
#include "BindGroupLayout.h"
#include "Buffer.h"
#include "Commands.h"
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
          buffersTransitioned(std::move(builder->buffersTransitioned)),
          texturesTransitioned(std::move(builder->texturesTransitioned)) {
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

    CommandBufferBuilder::CommandBufferBuilder(DeviceBase* device) : device(device) {
    }

    CommandBufferBuilder::~CommandBufferBuilder() {
        if (!consumed) {
            MoveToIterator();
            FreeCommands(&iterator);
        }
    }

    bool CommandBufferBuilder::WasConsumed() const {
        return consumed;
    }

    enum ValidationAspect {
        VALIDATION_ASPECT_RENDER_PIPELINE,
        VALIDATION_ASPECT_COMPUTE_PIPELINE,
        VALIDATION_ASPECT_BINDGROUPS,
        VALIDATION_ASPECT_VERTEX_BUFFERS,
        VALIDATION_ASPECT_INDEX_BUFFER,

        VALIDATION_ASPECT_COUNT,
    };

    using ValidationAspects = std::bitset<VALIDATION_ASPECT_COUNT>;

    bool CommandBufferBuilder::ValidateGetResult() {
        MoveToIterator();

        ValidationAspects aspects;
        std::bitset<kMaxBindGroups> bindgroupsSet;
        std::bitset<kMaxVertexInputs> inputsSet;
        PipelineBase* lastPipeline = nullptr;

        std::map<BufferBase*, nxt::BufferUsageBit> mostRecentBufferUsages;
        auto bufferHasGuaranteedUsageBit = [&](BufferBase* buffer, nxt::BufferUsageBit usage) -> bool {
            assert(usage != nxt::BufferUsageBit::None && nxt::HasZeroOrOneBits(usage));
            if (buffer->HasFrozenUsage(usage)) {
                return true;
            }
            auto it = mostRecentBufferUsages.find(buffer);
            return it != mostRecentBufferUsages.end() && (it->second & usage);
        };

        std::map<TextureBase*, nxt::TextureUsageBit> mostRecentTextureUsages;
        auto textureHasGuaranteedUsageBit = [&](TextureBase* texture, nxt::TextureUsageBit usage) -> bool {
            assert(usage != nxt::TextureUsageBit::None && nxt::HasZeroOrOneBits(usage));
            if (texture->HasFrozenUsage(usage)) {
                return true;
            }
            auto it = mostRecentTextureUsages.find(texture);
            return it != mostRecentTextureUsages.end() && (it->second & usage);
        };

        auto validateBindGroupUsages = [&](BindGroupBase* group) -> bool {
            const auto& layoutInfo = group->GetLayout()->GetBindingInfo();
            for (size_t i = 0; i < kMaxBindingsPerGroup; ++i) {
                if (!layoutInfo.mask[i]) {
                    continue;
                }

                nxt::BindingType type = layoutInfo.types[i];
                switch (type) {
                    case nxt::BindingType::UniformBuffer:
                    case nxt::BindingType::StorageBuffer:
                        {
                            nxt::BufferUsageBit requiredUsage;
                            switch (type) {
                                case nxt::BindingType::UniformBuffer:
                                    requiredUsage = nxt::BufferUsageBit::Uniform;
                                    break;

                                case nxt::BindingType::StorageBuffer:
                                    requiredUsage = nxt::BufferUsageBit::Storage;
                                    break;

                                default:
                                    assert(false);
                                    return false;
                            }

                            auto buffer = group->GetBindingAsBufferView(i)->GetBuffer();
                            if (!bufferHasGuaranteedUsageBit(buffer, requiredUsage)) {
                                device->HandleError("Can't guarantee buffer usage needed by bind group");
                                return false;
                            }
                        }
                        break;
                    case nxt::BindingType::SampledTexture:
                        {
                            auto requiredUsage = nxt::TextureUsageBit::Sampled;

                            auto texture = group->GetBindingAsTextureView(i)->GetTexture();
                            if (!textureHasGuaranteedUsageBit(texture, requiredUsage)) {
                                device->HandleError("Can't guarantee texture usage needed by bind group");
                                return false;
                            }
                        }
                        break;
                    case nxt::BindingType::Sampler:
                        continue;
                }
            }
            return true;
        };

        Command type;
        while(iterator.NextCommandId(&type)) {
            switch (type) {
                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = iterator.NextCommand<CopyBufferToTextureCmd>();
                        BufferBase* buffer = copy->buffer.Get();
                        TextureBase* texture = copy->texture.Get();
                        uint64_t width = copy->width;
                        uint64_t height = copy->height;
                        uint64_t depth = copy->depth;
                        uint64_t x = copy->x;
                        uint64_t y = copy->y;
                        uint64_t z = copy->z;
                        uint32_t level = copy->level;

                        if (!bufferHasGuaranteedUsageBit(buffer, nxt::BufferUsageBit::TransferSrc)) {
                            device->HandleError("Buffer needs the transfer source usage bit");
                            return false;
                        }

                        if (!textureHasGuaranteedUsageBit(texture, nxt::TextureUsageBit::TransferDst)) {
                            device->HandleError("Texture needs the transfer destination usage bit");
                            return false;
                        }

                        if (width == 0 || height == 0 || depth == 0) {
                            device->HandleError("Empty copy");
                            return false;
                        }

                        // TODO(cwallez@chromium.org): check for overflows
                        uint64_t pixelSize = TextureFormatPixelSize(texture->GetFormat());
                        uint64_t dataSize = width * height * depth * pixelSize;

                        // TODO(cwallez@chromium.org): handle buffer offset when it is in the command.
                        if (dataSize > static_cast<uint64_t>(buffer->GetSize())) {
                            device->HandleError("Copy would read after end of the buffer");
                            return false;
                        }

                        if (x + width > static_cast<uint64_t>(texture->GetWidth()) ||
                            y + height > static_cast<uint64_t>(texture->GetHeight()) ||
                            z + depth > static_cast<uint64_t>(texture->GetDepth()) ||
                            level > texture->GetNumMipLevels()) {
                            device->HandleError("Copy would write outside of the texture");
                            return false;
                        }
                    }
                    break;

                case Command::Dispatch:
                    {
                        DispatchCmd* cmd = iterator.NextCommand<DispatchCmd>();

                        constexpr ValidationAspects requiredDispatchAspects =
                            1 << VALIDATION_ASPECT_COMPUTE_PIPELINE |
                            1 << VALIDATION_ASPECT_BINDGROUPS |
                            1 << VALIDATION_ASPECT_VERTEX_BUFFERS;

                        if ((requiredDispatchAspects & ~aspects).any()) {
                            // Compute the lazily computed aspects
                            if (bindgroupsSet.all()) {
                                aspects.set(VALIDATION_ASPECT_BINDGROUPS);
                            }

                            auto requiredInputs = lastPipeline->GetInputState()->GetInputsSetMask();
                            if ((inputsSet & ~requiredInputs).none()) {
                                aspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
                            }

                            // Check again if anything is missing
                            if ((requiredDispatchAspects & ~aspects).any()) {
                                device->HandleError("Some dispatch state is missing");
                                return false;
                            }
                        }
                    }
                    break;

                case Command::DrawArrays:
                case Command::DrawElements:
                    {
                        constexpr ValidationAspects requiredDrawAspects =
                            1 << VALIDATION_ASPECT_RENDER_PIPELINE |
                            1 << VALIDATION_ASPECT_BINDGROUPS |
                            1 << VALIDATION_ASPECT_VERTEX_BUFFERS;

                        if ((requiredDrawAspects & ~aspects).any()) {
                            // Compute the lazily computed aspects
                            if (bindgroupsSet.all()) {
                                aspects.set(VALIDATION_ASPECT_BINDGROUPS);
                            }

                            auto requiredInputs = lastPipeline->GetInputState()->GetInputsSetMask();
                            if ((inputsSet & ~requiredInputs).none()) {
                                aspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
                            }

                            // Check again if anything is missing
                            if ((requiredDrawAspects & ~aspects).any()) {
                                device->HandleError("Some draw state is missing");
                                return false;
                            }
                        }

                        if (type == Command::DrawArrays) {
                            DrawArraysCmd* draw = iterator.NextCommand<DrawArraysCmd>();
                        } else {
                            ASSERT(type == Command::DrawElements);
                            DrawElementsCmd* draw = iterator.NextCommand<DrawElementsCmd>();

                            if (!aspects[VALIDATION_ASPECT_INDEX_BUFFER]) {
                                device->HandleError("Draw elements requires an index buffer");
                                return false;
                            }
                        }
                    }
                    break;

                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = iterator.NextCommand<SetPipelineCmd>();
                        PipelineBase* pipeline = cmd->pipeline.Get();
                        PipelineLayoutBase* layout = pipeline->GetLayout();

                        if (pipeline->IsCompute()) {
                            aspects.set(VALIDATION_ASPECT_COMPUTE_PIPELINE);
                            aspects.reset(VALIDATION_ASPECT_RENDER_PIPELINE);
                        } else {
                            aspects.set(VALIDATION_ASPECT_RENDER_PIPELINE);
                            aspects.reset(VALIDATION_ASPECT_COMPUTE_PIPELINE);
                        }
                        aspects.reset(VALIDATION_ASPECT_BINDGROUPS);
                        aspects.reset(VALIDATION_ASPECT_VERTEX_BUFFERS);
                        bindgroupsSet = ~layout->GetBindGroupsLayoutMask();

                        // Only bindgroups that were not the same layout in the last pipeline need to be set again.
                        if (lastPipeline) {
                            PipelineLayoutBase* lastLayout = lastPipeline->GetLayout();
                            for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                                if (lastLayout->GetBindGroupLayout(i) == layout->GetBindGroupLayout(i)) {
                                    bindgroupsSet |= uint64_t(1) << i;
                                }
                            }
                        }

                        lastPipeline = pipeline;
                    }
                    break;

                case Command::SetPushConstants:
                    {
                        SetPushConstantsCmd* cmd = iterator.NextCommand<SetPushConstantsCmd>();
                        iterator.NextData<uint32_t>(cmd->count);
                        if (cmd->count + cmd->offset > kMaxPushConstants) {
                            device->HandleError("Setting pushconstants past the limit");
                            return false;
                        }
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = iterator.NextCommand<SetBindGroupCmd>();
                        uint32_t index = cmd->index;

                        if (cmd->group->GetLayout() != lastPipeline->GetLayout()->GetBindGroupLayout(index)) {
                            device->HandleError("Bind group layout mismatch");
                            return false;
                        }
                        if (!validateBindGroupUsages(cmd->group.Get())) {
                            return false;
                        }
                        bindgroupsSet |= uint64_t(1) << index;
                    }
                    break;

                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = iterator.NextCommand<SetIndexBufferCmd>();
                        auto buffer = cmd->buffer;
                        auto usage = nxt::BufferUsageBit::Index;
                        if (!bufferHasGuaranteedUsageBit(buffer.Get(), usage)) {
                            device->HandleError("Buffer needs the index usage bit to be guaranteed");
                            return false;
                        }

                        aspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
                    }
                    break;

                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = iterator.NextCommand<SetVertexBuffersCmd>();
                        auto buffers = iterator.NextData<Ref<BufferBase>>(cmd->count);
                        iterator.NextData<uint32_t>(cmd->count);

                        for (uint32_t i = 0; i < cmd->count; ++i) {
                            auto buffer = buffers[i];
                            auto usage = nxt::BufferUsageBit::Vertex;
                            if (!bufferHasGuaranteedUsageBit(buffer.Get(), usage)) {
                                device->HandleError("Buffer needs vertex usage bit to be guaranteed");
                                return false;
                            }
                            inputsSet.set(cmd->startSlot + i);
                        }
                    }
                    break;

                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = iterator.NextCommand<TransitionBufferUsageCmd>();
                        auto buffer = cmd->buffer.Get();
                        auto usage = cmd->usage;

                        if (!cmd->buffer->IsTransitionPossible(cmd->usage)) {
                            device->HandleError("Buffer frozen or usage not allowed");
                            return false;
                        }

                        mostRecentBufferUsages[buffer] = usage;

                        buffersTransitioned.insert(buffer);
                    }
                    break;

                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = iterator.NextCommand<TransitionTextureUsageCmd>();
                        auto texture = cmd->texture.Get();
                        auto usage = cmd->usage;

                        if (!cmd->texture->IsTransitionPossible(cmd->usage)) {
                            device->HandleError("Texture frozen or usage not allowed");
                            return false;
                        }

                        mostRecentTextureUsages[texture] = usage;

                        texturesTransitioned.insert(texture);
                    }
                    break;
            }
        }

        return true;
    }

    CommandIterator CommandBufferBuilder::AcquireCommands() {
        return std::move(iterator);
    }

    CommandBufferBase* CommandBufferBuilder::GetResult() {
        MoveToIterator();
        consumed = true;
        return device->CreateCommandBuffer(this);
    }

    void CommandBufferBuilder::CopyBufferToTexture(BufferBase* buffer, TextureBase* texture, uint32_t x, uint32_t y, uint32_t z,
                                                   uint32_t width, uint32_t height, uint32_t depth, uint32_t level) {
        CopyBufferToTextureCmd* copy = allocator.Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
        new(copy) CopyBufferToTextureCmd;
        copy->buffer = buffer;
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

    void CommandBufferBuilder::SetPipeline(PipelineBase* pipeline) {
        SetPipelineCmd* cmd = allocator.Allocate<SetPipelineCmd>(Command::SetPipeline);
        new(cmd) SetPipelineCmd;
        cmd->pipeline = pipeline;
    }

    void CommandBufferBuilder::SetPushConstants(nxt::ShaderStageBit stage, uint32_t offset, uint32_t count, const void* data) {
        if (offset + count > kMaxPushConstants) {
            device->HandleError("Setting too many push constants");
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

    void CommandBufferBuilder::SetBindGroup(uint32_t groupIndex, BindGroupBase* group) {
        if (groupIndex >= kMaxBindGroups) {
            device->HandleError("Setting bind group over the max");
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
