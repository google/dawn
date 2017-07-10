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

#include "backend/opengl/CommandBufferGL.h"

#include "backend/Commands.h"
#include "backend/opengl/OpenGLBackend.h"
#include "backend/opengl/PersistentPipelineStateGL.h"
#include "backend/opengl/PipelineGL.h"
#include "backend/opengl/PipelineLayoutGL.h"
#include "backend/opengl/SamplerGL.h"
#include "backend/opengl/TextureGL.h"

#include <cstring>

namespace backend {
namespace opengl {

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), commands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&commands);
    }

    static GLenum IndexFormatType(nxt::IndexFormat format) {
        switch (format) {
            case nxt::IndexFormat::Uint16:
                return GL_UNSIGNED_SHORT;
            case nxt::IndexFormat::Uint32:
                return GL_UNSIGNED_INT;
        }
    }

    static GLenum VertexFormatType(nxt::VertexFormat format) {
        switch (format) {
            case nxt::VertexFormat::FloatR32G32B32A32:
            case nxt::VertexFormat::FloatR32G32B32:
            case nxt::VertexFormat::FloatR32G32:
            case nxt::VertexFormat::FloatR32:
                return GL_FLOAT;
        }
    }

    void CommandBuffer::Execute() {
        Command type;
        Pipeline* lastPipeline = nullptr;
        uint32_t indexBufferOffset = 0;
        nxt::IndexFormat indexBufferFormat = nxt::IndexFormat::Uint16;

        PersistentPipelineState persistentPipelineState;
        persistentPipelineState.SetDefaultState();

        while(commands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass:
                    {
                        commands.NextCommand<BeginComputePassCmd>();
                    }
                    break;

                case Command::BeginRenderPass:
                    {
                        commands.NextCommand<BeginRenderPassCmd>();
                        // TODO(kainino@chromium.org): implement
                    }
                    break;

                case Command::BeginRenderSubpass:
                    {
                        commands.NextCommand<BeginRenderSubpassCmd>();
                        // TODO(kainino@chromium.org): implement
                    }
                    break;

                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = commands.NextCommand<CopyBufferToBufferCmd>();
                        auto& src = copy->source;
                        auto& dst = copy->destination;

                        glBindBuffer(GL_PIXEL_PACK_BUFFER, ToBackend(src.buffer)->GetHandle());
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ToBackend(dst.buffer)->GetHandle());
                        glCopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER, src.offset, dst.offset, copy->size);

                        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands.NextCommand<CopyBufferToTextureCmd>();
                        auto& src = copy->source;
                        auto& dst = copy->destination;
                        Buffer* buffer = ToBackend(src.buffer.Get());
                        Texture* texture = ToBackend(dst.texture.Get());
                        GLenum target = texture->GetGLTarget();
                        auto format = texture->GetGLFormat();

                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->GetHandle());
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(target, texture->GetHandle());

                        glTexSubImage2D(target, dst.level, dst.x, dst.y, dst.width, dst.height,
                                        format.format, format.type,
                                        reinterpret_cast<void*>(static_cast<uintptr_t>(src.offset)));
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    }
                    break;

                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = commands.NextCommand<CopyTextureToBufferCmd>();
                        // TODO(cwallez@chromium.org): implement using a temporary FBO and ReadPixels
                    }
                    break;

                case Command::Dispatch:
                    {
                        DispatchCmd* dispatch = commands.NextCommand<DispatchCmd>();
                        glDispatchCompute(dispatch->x, dispatch->y, dispatch->z);
                        // TODO(cwallez@chromium.org): add barriers to the API
                        glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    }
                    break;

                case Command::DrawArrays:
                    {
                        DrawArraysCmd* draw = commands.NextCommand<DrawArraysCmd>();
                        if (draw->firstInstance > 0) {
                            glDrawArraysInstancedBaseInstance(GL_TRIANGLES,
                                draw->firstVertex, draw->vertexCount, draw->instanceCount, draw->firstInstance);
                        } else {
                            // This branch is only needed on OpenGL < 4.2
                            glDrawArraysInstanced(GL_TRIANGLES,
                                draw->firstVertex, draw->vertexCount, draw->instanceCount);
                        }
                    }
                    break;

                case Command::DrawElements:
                    {
                        DrawElementsCmd* draw = commands.NextCommand<DrawElementsCmd>();
                        size_t formatSize = IndexFormatSize(indexBufferFormat);
                        GLenum formatType = IndexFormatType(indexBufferFormat);

                        if (draw->firstInstance > 0) {
                            glDrawElementsInstancedBaseInstance(GL_TRIANGLES,
                                draw->indexCount, formatType,
                                reinterpret_cast<void*>(draw->firstIndex * formatSize + indexBufferOffset),
                                draw->instanceCount, draw->firstInstance);
                        } else {
                            // This branch is only needed on OpenGL < 4.2
                            glDrawElementsInstanced(GL_TRIANGLES,
                                draw->indexCount, formatType,
                                reinterpret_cast<void*>(draw->firstIndex * formatSize + indexBufferOffset),
                                draw->instanceCount);
                        }
                    }
                    break;

                case Command::EndComputePass:
                    {
                        commands.NextCommand<EndComputePassCmd>();
                    }
                    break;

                case Command::EndRenderPass:
                    {
                        commands.NextCommand<EndRenderPassCmd>();
                        // TODO(kainino@chromium.org): implement
                    }
                    break;

                case Command::EndRenderSubpass:
                    {
                        commands.NextCommand<EndRenderSubpassCmd>();
                        // TODO(kainino@chromium.org): implement
                    }
                    break;

                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = commands.NextCommand<SetPipelineCmd>();
                        ToBackend(cmd->pipeline)->ApplyNow(persistentPipelineState);
                        lastPipeline = ToBackend(cmd->pipeline).Get();
                    }
                    break;

                case Command::SetPushConstants:
                    {
                        SetPushConstantsCmd* cmd = commands.NextCommand<SetPushConstantsCmd>();
                        uint32_t* valuesUInt = commands.NextData<uint32_t>(cmd->count);
                        int32_t* valuesInt = reinterpret_cast<int32_t*>(valuesUInt);
                        float* valuesFloat = reinterpret_cast<float*>(valuesUInt);

                        for (auto stage : IterateStages(cmd->stage)) {
                            const auto& pushConstants = lastPipeline->GetPushConstants(stage);
                            const auto& glPushConstants = lastPipeline->GetGLPushConstants(stage);
                            for (size_t i = 0; i < cmd->count; i++) {
                                GLint location = glPushConstants[cmd->offset + i];

                                switch (pushConstants.types[cmd->offset + i]) {
                                    case PushConstantType::Int:
                                        glUniform1i(location, valuesInt[i]);
                                        break;
                                    case PushConstantType::UInt:
                                        glUniform1ui(location, valuesUInt[i]);
                                        break;
                                    case PushConstantType::Float:
                                        glUniform1f(location, valuesFloat[i]);
                                        break;
                                }
                            }
                        }
                    }
                    break;

                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd* cmd = commands.NextCommand<SetStencilReferenceCmd>();
                        persistentPipelineState.SetStencilReference(cmd->reference);
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = commands.NextCommand<SetBindGroupCmd>();
                        size_t index = cmd->index;
                        BindGroup* group = ToBackend(cmd->group.Get());

                        const auto& indices = ToBackend(lastPipeline->GetLayout())->GetBindingIndexInfo()[index];
                        const auto& layout = group->GetLayout()->GetBindingInfo();

                        // TODO(cwallez@chromium.org): iterate over the layout bitmask instead
                        for (size_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                            if (!layout.mask[binding]) {
                                continue;
                            }

                            switch (layout.types[binding]) {
                                case nxt::BindingType::UniformBuffer:
                                    {
                                        BufferView* view = ToBackend(group->GetBindingAsBufferView(binding));
                                        GLuint buffer = ToBackend(view->GetBuffer())->GetHandle();
                                        GLuint index = indices[binding];

                                        glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, view->GetOffset(), view->GetSize());
                                    }
                                    break;

                                case nxt::BindingType::Sampler:
                                    {
                                        GLuint sampler = ToBackend(group->GetBindingAsSampler(binding))->GetHandle();
                                        GLuint index = indices[binding];

                                        for (auto unit : lastPipeline->GetTextureUnitsForSampler(index)) {
                                            glBindSampler(unit, sampler);
                                        }
                                    }
                                    break;

                                case nxt::BindingType::SampledTexture:
                                    {
                                        TextureView* view = ToBackend(group->GetBindingAsTextureView(binding));
                                        Texture* texture = ToBackend(view->GetTexture());
                                        GLuint handle = texture->GetHandle();
                                        GLenum target = texture->GetGLTarget();
                                        GLuint index = indices[binding];

                                        for (auto unit : lastPipeline->GetTextureUnitsForTexture(index)) {
                                            glActiveTexture(GL_TEXTURE0 + unit);
                                            glBindTexture(target, handle);
                                        }
                                    }
                                    break;

                                case nxt::BindingType::StorageBuffer:
                                    {
                                        BufferView* view = ToBackend(group->GetBindingAsBufferView(binding));
                                        GLuint buffer = ToBackend(view->GetBuffer())->GetHandle();
                                        GLuint index = indices[binding];

                                        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, buffer, view->GetOffset(), view->GetSize());
                                    }
                                    break;
                            }
                        }
                    }
                    break;

                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = commands.NextCommand<SetIndexBufferCmd>();

                        GLuint buffer = ToBackend(cmd->buffer.Get())->GetHandle();
                        indexBufferOffset = cmd->offset;
                        indexBufferFormat = cmd->format;
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                    }
                    break;

                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = commands.NextCommand<SetVertexBuffersCmd>();
                        auto buffers = commands.NextData<Ref<BufferBase>>(cmd->count);
                        auto offsets = commands.NextData<uint32_t>(cmd->count);

                        auto inputState = lastPipeline->GetInputState();

                        auto& attributesSetMask = inputState->GetAttributesSetMask();
                        for (uint32_t location = 0; location < attributesSetMask.size(); ++location) {
                            if (!attributesSetMask[location]) {
                                // This slot is not used in the input state
                                continue;
                            }
                            auto attribute = inputState->GetAttribute(location);
                            auto slot = attribute.bindingSlot;
                            ASSERT(slot < kMaxVertexInputs);
                            if (slot < cmd->startSlot || slot >= cmd->startSlot + cmd->count) {
                                // This slot is not affected by this call
                                continue;
                            }
                            size_t bufferIndex = slot - cmd->startSlot;
                            GLuint buffer = ToBackend(buffers[bufferIndex])->GetHandle();
                            uint32_t bufferOffset = offsets[bufferIndex];

                            auto input = inputState->GetInput(slot);

                            auto components = VertexFormatNumComponents(attribute.format);
                            auto formatType = VertexFormatType(attribute.format);

                            glBindBuffer(GL_ARRAY_BUFFER, buffer);
                            glVertexAttribPointer(
                                    location, components, formatType, GL_FALSE,
                                    input.stride,
                                    reinterpret_cast<void*>(static_cast<intptr_t>(bufferOffset + attribute.offset)));
                        }
                    }
                    break;

                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = commands.NextCommand<TransitionBufferUsageCmd>();

                        cmd->buffer->UpdateUsageInternal(cmd->usage);
                    }
                    break;

                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = commands.NextCommand<TransitionTextureUsageCmd>();

                        cmd->texture->UpdateUsageInternal(cmd->usage);
                    }
                    break;
            }
        }

        // HACK: cleanup a tiny bit of state to make this work with
        // virtualized contexts enabled in Chromium
        glBindSampler(0, 0);
    }

}
}
