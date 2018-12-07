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

#include "dawn_native/opengl/CommandBufferGL.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/Commands.h"
#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/ComputePipelineGL.h"
#include "dawn_native/opengl/Forward.h"
#include "dawn_native/opengl/InputStateGL.h"
#include "dawn_native/opengl/PersistentPipelineStateGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/RenderPipelineGL.h"
#include "dawn_native/opengl/SamplerGL.h"
#include "dawn_native/opengl/TextureGL.h"

#include <cstring>

namespace dawn_native { namespace opengl {

    namespace {

        GLenum IndexFormatType(dawn::IndexFormat format) {
            switch (format) {
                case dawn::IndexFormat::Uint16:
                    return GL_UNSIGNED_SHORT;
                case dawn::IndexFormat::Uint32:
                    return GL_UNSIGNED_INT;
                default:
                    UNREACHABLE();
            }
        }

        GLenum VertexFormatType(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::FloatR32G32B32A32:
                case dawn::VertexFormat::FloatR32G32B32:
                case dawn::VertexFormat::FloatR32G32:
                case dawn::VertexFormat::FloatR32:
                    return GL_FLOAT;
                case dawn::VertexFormat::IntR32G32B32A32:
                case dawn::VertexFormat::IntR32G32B32:
                case dawn::VertexFormat::IntR32G32:
                case dawn::VertexFormat::IntR32:
                    return GL_INT;
                case dawn::VertexFormat::UshortR16G16B16A16:
                case dawn::VertexFormat::UshortR16G16:
                    return GL_UNSIGNED_SHORT;
                case dawn::VertexFormat::UnormR8G8B8A8:
                case dawn::VertexFormat::UnormR8G8:
                    return GL_UNSIGNED_BYTE;
                default:
                    UNREACHABLE();
            }
        }

        GLboolean VertexFormatIsNormalized(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::UnormR8G8B8A8:
                case dawn::VertexFormat::UnormR8G8:
                    return GL_TRUE;
                default:
                    return GL_FALSE;
            }
        }

        // Push constants are implemented using OpenGL uniforms, however they aren't part of the
        // global OpenGL state but are part of the program state instead. This means that we have to
        // reapply push constants on pipeline change.
        //
        // This structure tracks the current values of push constants as well as dirty bits for push
        // constants that should be applied before the next draw or dispatch.
        class PushConstantTracker {
          public:
            PushConstantTracker() {
                for (auto stage : IterateStages(kAllStages)) {
                    mValues[stage].fill(0);
                    // No need to set dirty bits as a pipeline will be set before the next operation
                    // using push constants.
                }
            }

            void OnSetPushConstants(dawn::ShaderStageBit stages,
                                    uint32_t count,
                                    uint32_t offset,
                                    const uint32_t* data) {
                for (auto stage : IterateStages(stages)) {
                    memcpy(&mValues[stage][offset], data, count * sizeof(uint32_t));

                    // Use 64 bit masks and make sure there are no shift UB
                    static_assert(kMaxPushConstants <= 8 * sizeof(unsigned long long) - 1, "");
                    mDirtyBits[stage] |= ((1ull << count) - 1ull) << offset;
                }
            }

            void OnSetPipeline(PipelineBase* pipeline) {
                for (auto stage : IterateStages(kAllStages)) {
                    mDirtyBits[stage] = pipeline->GetPushConstants(stage).mask;
                }
            }

            void Apply(PipelineBase* pipeline, PipelineGL* glPipeline) {
                for (auto stage : IterateStages(kAllStages)) {
                    const auto& pushConstants = pipeline->GetPushConstants(stage);
                    const auto& glPushConstants = glPipeline->GetGLPushConstants(stage);

                    for (uint32_t constant :
                         IterateBitSet(mDirtyBits[stage] & pushConstants.mask)) {
                        GLint location = glPushConstants[constant];
                        switch (pushConstants.types[constant]) {
                            case PushConstantType::Int:
                                glUniform1i(location,
                                            *reinterpret_cast<GLint*>(&mValues[stage][constant]));
                                break;
                            case PushConstantType::UInt:
                                glUniform1ui(location,
                                             *reinterpret_cast<GLuint*>(&mValues[stage][constant]));
                                break;
                            case PushConstantType::Float:
                                float value;
                                // Use a memcpy to avoid strict-aliasing warnings, even if it is
                                // still technically undefined behavior.
                                memcpy(&value, &mValues[stage][constant], sizeof(value));
                                glUniform1f(location, value);
                                break;
                        }
                    }

                    mDirtyBits[stage].reset();
                }
            }

          private:
            PerStage<std::array<uint32_t, kMaxPushConstants>> mValues;
            PerStage<std::bitset<kMaxPushConstants>> mDirtyBits;
        };

        // Vertex buffers and index buffers are implemented as part of an OpenGL VAO that
        // corresponds to an InputState. On the contrary in Dawn they are part of the global state.
        // This means that we have to re-apply these buffers on an InputState change.
        class InputBufferTracker {
          public:
            void OnSetIndexBuffer(BufferBase* buffer) {
                mIndexBufferDirty = true;
                mIndexBuffer = ToBackend(buffer);
            }

            void OnSetVertexBuffers(uint32_t startSlot,
                                    uint32_t count,
                                    Ref<BufferBase>* buffers,
                                    uint32_t* offsets) {
                for (uint32_t i = 0; i < count; ++i) {
                    uint32_t slot = startSlot + i;
                    mVertexBuffers[slot] = ToBackend(buffers[i].Get());
                    mVertexBufferOffsets[slot] = offsets[i];
                }

                // Use 64 bit masks and make sure there are no shift UB
                static_assert(kMaxVertexInputs <= 8 * sizeof(unsigned long long) - 1, "");
                mDirtyVertexBuffers |= ((1ull << count) - 1ull) << startSlot;
            }

            void OnSetPipeline(RenderPipelineBase* pipeline) {
                InputStateBase* inputState = pipeline->GetInputState();
                if (mLastInputState == inputState) {
                    return;
                }

                mIndexBufferDirty = true;
                mDirtyVertexBuffers |= inputState->GetInputsSetMask();

                mLastInputState = ToBackend(inputState);
            }

            void Apply() {
                if (mIndexBufferDirty && mIndexBuffer != nullptr) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer->GetHandle());
                    mIndexBufferDirty = false;
                }

                for (uint32_t slot :
                     IterateBitSet(mDirtyVertexBuffers & mLastInputState->GetInputsSetMask())) {
                    for (uint32_t location :
                         IterateBitSet(mLastInputState->GetAttributesUsingInput(slot))) {
                        auto attribute = mLastInputState->GetAttribute(location);

                        GLuint buffer = mVertexBuffers[slot]->GetHandle();
                        uint32_t offset = mVertexBufferOffsets[slot];

                        auto input = mLastInputState->GetInput(slot);
                        auto components = VertexFormatNumComponents(attribute.format);
                        auto formatType = VertexFormatType(attribute.format);

                        GLboolean normalized = VertexFormatIsNormalized(attribute.format);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        glVertexAttribPointer(
                            location, components, formatType, normalized, input.stride,
                            reinterpret_cast<void*>(
                                static_cast<intptr_t>(offset + attribute.offset)));
                    }
                }

                mDirtyVertexBuffers.reset();
            }

          private:
            bool mIndexBufferDirty = false;
            Buffer* mIndexBuffer = nullptr;

            std::bitset<kMaxVertexInputs> mDirtyVertexBuffers;
            std::array<Buffer*, kMaxVertexInputs> mVertexBuffers;
            std::array<uint32_t, kMaxVertexInputs> mVertexBufferOffsets;

            InputState* mLastInputState = nullptr;
        };

        // Handles SetBindGroup commands with the specifics of translating to OpenGL texture and
        // buffer units
        void ApplyBindGroup(uint32_t index,
                            BindGroupBase* group,
                            PipelineLayout* pipelineLayout,
                            PipelineGL* pipeline) {
            const auto& indices = pipelineLayout->GetBindingIndexInfo()[index];
            const auto& layout = group->GetLayout()->GetBindingInfo();

            for (uint32_t bindingIndex : IterateBitSet(layout.mask)) {
                switch (layout.types[bindingIndex]) {
                    case dawn::BindingType::UniformBuffer: {
                        BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                        GLuint buffer = ToBackend(binding.buffer)->GetHandle();
                        GLuint uboIndex = indices[bindingIndex];

                        glBindBufferRange(GL_UNIFORM_BUFFER, uboIndex, buffer, binding.offset,
                                          binding.size);
                    } break;

                    case dawn::BindingType::Sampler: {
                        GLuint sampler =
                            ToBackend(group->GetBindingAsSampler(bindingIndex))->GetHandle();
                        GLuint samplerIndex = indices[bindingIndex];

                        for (auto unit : pipeline->GetTextureUnitsForSampler(samplerIndex)) {
                            glBindSampler(unit, sampler);
                        }
                    } break;

                    case dawn::BindingType::SampledTexture: {
                        TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                        GLuint handle = view->GetHandle();
                        GLenum target = view->GetGLTarget();
                        GLuint viewIndex = indices[bindingIndex];

                        for (auto unit : pipeline->GetTextureUnitsForTextureView(viewIndex)) {
                            glActiveTexture(GL_TEXTURE0 + unit);
                            glBindTexture(target, handle);
                        }
                    } break;

                    case dawn::BindingType::StorageBuffer: {
                        BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                        GLuint buffer = ToBackend(binding.buffer)->GetHandle();
                        GLuint ssboIndex = indices[bindingIndex];

                        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, ssboIndex, buffer,
                                          binding.offset, binding.size);
                    } break;
                }
            }
        }
    }  // namespace

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::Execute() {
        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();
                    ExecuteComputePass();
                } break;

                case Command::BeginRenderPass: {
                    auto* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                    ExecuteRenderPass(ToBackend(cmd->info.Get()));
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, ToBackend(src.buffer)->GetHandle());
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ToBackend(dst.buffer)->GetHandle());
                    glCopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER, src.offset,
                                        dst.offset, copy->size);

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Buffer* buffer = ToBackend(src.buffer.Get());
                    Texture* texture = ToBackend(dst.texture.Get());
                    GLenum target = texture->GetGLTarget();
                    auto format = texture->GetGLFormat();

                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->GetHandle());
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(target, texture->GetHandle());

                    glPixelStorei(GL_UNPACK_ROW_LENGTH,
                                  src.rowPitch / TextureFormatPixelSize(texture->GetFormat()));
                    switch (texture->GetDimension()) {
                        case dawn::TextureDimension::e2D:
                            if (texture->GetArrayLayers() > 1) {
                                glTexSubImage3D(
                                    target, dst.level, dst.origin.x, dst.origin.y, dst.slice,
                                    copySize.width, copySize.height, 1, format.format, format.type,
                                    reinterpret_cast<void*>(static_cast<uintptr_t>(src.offset)));
                            } else {
                                glTexSubImage2D(
                                    target, dst.level, dst.origin.x, dst.origin.y, copySize.width,
                                    copySize.height, format.format, format.type,
                                    reinterpret_cast<void*>(static_cast<uintptr_t>(src.offset)));
                            }
                            break;

                        default:
                            UNREACHABLE();
                    }
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Texture* texture = ToBackend(src.texture.Get());
                    Buffer* buffer = ToBackend(dst.buffer.Get());
                    auto format = texture->GetGLFormat();
                    GLenum target = texture->GetGLTarget();

                    // The only way to move data from a texture to a buffer in GL is via
                    // glReadPixels with a pack buffer. Create a temporary FBO for the copy.
                    glBindTexture(target, texture->GetHandle());

                    GLuint readFBO = 0;
                    glGenFramebuffers(1, &readFBO);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
                    switch (texture->GetDimension()) {
                        case dawn::TextureDimension::e2D:
                            if (texture->GetArrayLayers() > 1) {
                                glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                          texture->GetHandle(), src.level,
                                                          src.slice);
                            } else {
                                glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                       GL_TEXTURE_2D, texture->GetHandle(),
                                                       src.level);
                            }
                            break;

                        default:
                            UNREACHABLE();
                    }

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->GetHandle());
                    glPixelStorei(GL_PACK_ROW_LENGTH,
                                  dst.rowPitch / TextureFormatPixelSize(texture->GetFormat()));
                    ASSERT(copySize.depth == 1 && src.origin.z == 0);
                    void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(dst.offset));
                    glReadPixels(src.origin.x, src.origin.y, copySize.width, copySize.height,
                                 format.format, format.type, offset);
                    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    glDeleteFramebuffers(1, &readFBO);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::ExecuteComputePass() {
        PushConstantTracker pushConstants;
        ComputePipeline* lastPipeline = nullptr;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    return;
                } break;

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    pushConstants.Apply(lastPipeline, lastPipeline);
                    glDispatchCompute(dispatch->x, dispatch->y, dispatch->z);
                    // TODO(cwallez@chromium.org): add barriers to the API
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();

                    lastPipeline->ApplyNow();
                    pushConstants.OnSetPipeline(lastPipeline);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mCommands.NextCommand<SetPushConstantsCmd>();
                    uint32_t* data = mCommands.NextData<uint32_t>(cmd->count);
                    pushConstants.OnSetPushConstants(cmd->stages, cmd->count, cmd->offset, data);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    ApplyBindGroup(cmd->index, cmd->group.Get(),
                                   ToBackend(lastPipeline->GetLayout()), lastPipeline);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndComputePass should have been called
        UNREACHABLE();
    }

    void CommandBuffer::ExecuteRenderPass(RenderPassDescriptorBase* renderPass) {
        GLuint fbo = 0;

        // Create the framebuffer used for this render pass and calls the correct glDrawBuffers
        {
            // TODO(kainino@chromium.org): This is added to possibly work around an issue seen on
            // Windows/Intel. It should break any feedback loop before the clears, even if there
            // shouldn't be any negative effects from this. Investigate whether it's actually
            // needed.
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            // TODO(kainino@chromium.org): possible future optimization: create these framebuffers
            // at Framebuffer build time (or maybe CommandBuffer build time) so they don't have to
            // be created and destroyed at draw time.
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

            // Mapping from attachmentSlot to GL framebuffer attachment points. Defaults to zero
            // (GL_NONE).
            std::array<GLenum, kMaxColorAttachments> drawBuffers = {};

            // Construct GL framebuffer

            unsigned int attachmentCount = 0;
            for (uint32_t i : IterateBitSet(renderPass->GetColorAttachmentMask())) {
                TextureViewBase* textureView = renderPass->GetColorAttachment(i).view.Get();
                GLuint texture = ToBackend(textureView->GetTexture())->GetHandle();

                // Attach color buffers.
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                                       texture, 0);
                drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
                attachmentCount = i + 1;

                // TODO(kainino@chromium.org): the color clears (later in
                // this function) may be undefined for non-normalized integer formats.
                dawn::TextureFormat format = textureView->GetTexture()->GetFormat();
                ASSERT(format == dawn::TextureFormat::R8G8B8A8Unorm ||
                       format == dawn::TextureFormat::R8G8Unorm ||
                       format == dawn::TextureFormat::R8Unorm ||
                       format == dawn::TextureFormat::B8G8R8A8Unorm);
            }
            glDrawBuffers(attachmentCount, drawBuffers.data());

            if (renderPass->HasDepthStencilAttachment()) {
                TextureViewBase* textureView = renderPass->GetDepthStencilAttachment().view.Get();
                GLuint texture = ToBackend(textureView->GetTexture())->GetHandle();
                dawn::TextureFormat format = textureView->GetTexture()->GetFormat();

                // Attach depth/stencil buffer.
                GLenum glAttachment = 0;
                // TODO(kainino@chromium.org): it may be valid to just always use
                // GL_DEPTH_STENCIL_ATTACHMENT here.
                if (TextureFormatHasDepth(format)) {
                    if (TextureFormatHasStencil(format)) {
                        glAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
                    } else {
                        glAttachment = GL_DEPTH_ATTACHMENT;
                    }
                } else {
                    glAttachment = GL_STENCIL_ATTACHMENT;
                }

                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachment, GL_TEXTURE_2D, texture,
                                       0);

                // TODO(kainino@chromium.org): the depth/stencil clears (later in
                // this function) may be undefined for other texture formats.
                ASSERT(format == dawn::TextureFormat::D32FloatS8Uint);
            }
        }

        // Clear framebuffer attachments as needed
        {
            for (uint32_t i : IterateBitSet(renderPass->GetColorAttachmentMask())) {
                const auto& attachmentInfo = renderPass->GetColorAttachment(i);

                // Load op - color
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    glClearBufferfv(GL_COLOR, i, attachmentInfo.clearColor.data());
                }
            }

            if (renderPass->HasDepthStencilAttachment()) {
                const auto& attachmentInfo = renderPass->GetDepthStencilAttachment();
                dawn::TextureFormat attachmentFormat =
                    attachmentInfo.view->GetTexture()->GetFormat();

                // Load op - depth/stencil
                bool doDepthClear = TextureFormatHasDepth(attachmentFormat) &&
                                    (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear);
                bool doStencilClear = TextureFormatHasStencil(attachmentFormat) &&
                                      (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear);
                if (doDepthClear && doStencilClear) {
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, attachmentInfo.clearDepth,
                                    attachmentInfo.clearStencil);
                } else if (doDepthClear) {
                    glClearBufferfv(GL_DEPTH, 0, &attachmentInfo.clearDepth);
                } else if (doStencilClear) {
                    const GLint clearStencil = attachmentInfo.clearStencil;
                    glClearBufferiv(GL_STENCIL, 0, &clearStencil);
                }
            }
        }

        RenderPipeline* lastPipeline = nullptr;
        uint32_t indexBufferBaseOffset = 0;

        PersistentPipelineState persistentPipelineState;

        PushConstantTracker pushConstants;
        InputBufferTracker inputBuffers;

        // Set defaults for dynamic state
        persistentPipelineState.SetDefaultState();
        glBlendColor(0, 0, 0, 0);
        glViewport(0, 0, renderPass->GetWidth(), renderPass->GetHeight());
        glScissor(0, 0, renderPass->GetWidth(), renderPass->GetHeight());

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    glDeleteFramebuffers(1, &fbo);
                    return;
                } break;

                case Command::DrawArrays: {
                    DrawArraysCmd* draw = mCommands.NextCommand<DrawArraysCmd>();
                    pushConstants.Apply(lastPipeline, lastPipeline);
                    inputBuffers.Apply();

                    if (draw->firstInstance > 0) {
                        glDrawArraysInstancedBaseInstance(lastPipeline->GetGLPrimitiveTopology(),
                                                          draw->firstVertex, draw->vertexCount,
                                                          draw->instanceCount, draw->firstInstance);
                    } else {
                        // This branch is only needed on OpenGL < 4.2
                        glDrawArraysInstanced(lastPipeline->GetGLPrimitiveTopology(),
                                              draw->firstVertex, draw->vertexCount,
                                              draw->instanceCount);
                    }
                } break;

                case Command::DrawElements: {
                    DrawElementsCmd* draw = mCommands.NextCommand<DrawElementsCmd>();
                    pushConstants.Apply(lastPipeline, lastPipeline);
                    inputBuffers.Apply();

                    dawn::IndexFormat indexFormat = lastPipeline->GetIndexFormat();
                    size_t formatSize = IndexFormatSize(indexFormat);
                    GLenum formatType = IndexFormatType(indexFormat);

                    if (draw->firstInstance > 0) {
                        glDrawElementsInstancedBaseInstance(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount, draw->firstInstance);
                    } else {
                        // This branch is only needed on OpenGL < 4.2
                        glDrawElementsInstanced(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount);
                    }
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();
                    lastPipeline->ApplyNow(persistentPipelineState);

                    pushConstants.OnSetPipeline(lastPipeline);
                    inputBuffers.OnSetPipeline(lastPipeline);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mCommands.NextCommand<SetPushConstantsCmd>();
                    uint32_t* data = mCommands.NextData<uint32_t>(cmd->count);
                    pushConstants.OnSetPushConstants(cmd->stages, cmd->count, cmd->offset, data);
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                    persistentPipelineState.SetStencilReference(cmd->reference);
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    glScissor(cmd->x, cmd->y, cmd->width, cmd->height);
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    glBlendColor(cmd->r, cmd->g, cmd->b, cmd->a);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    ApplyBindGroup(cmd->index, cmd->group.Get(),
                                   ToBackend(lastPipeline->GetLayout()), lastPipeline);
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mCommands.NextCommand<SetIndexBufferCmd>();
                    indexBufferBaseOffset = cmd->offset;
                    inputBuffers.OnSetIndexBuffer(cmd->buffer.Get());
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mCommands.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mCommands.NextData<Ref<BufferBase>>(cmd->count);
                    auto offsets = mCommands.NextData<uint32_t>(cmd->count);
                    inputBuffers.OnSetVertexBuffers(cmd->startSlot, cmd->count, buffers, offsets);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::opengl
