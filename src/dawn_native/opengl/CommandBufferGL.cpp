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
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/ComputePipelineGL.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/Forward.h"
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
                case dawn::VertexFormat::UChar2:
                case dawn::VertexFormat::UChar4:
                case dawn::VertexFormat::UChar2Norm:
                case dawn::VertexFormat::UChar4Norm:
                    return GL_UNSIGNED_BYTE;
                case dawn::VertexFormat::Char2:
                case dawn::VertexFormat::Char4:
                case dawn::VertexFormat::Char2Norm:
                case dawn::VertexFormat::Char4Norm:
                    return GL_BYTE;
                case dawn::VertexFormat::UShort2:
                case dawn::VertexFormat::UShort4:
                case dawn::VertexFormat::UShort2Norm:
                case dawn::VertexFormat::UShort4Norm:
                    return GL_UNSIGNED_SHORT;
                case dawn::VertexFormat::Short2:
                case dawn::VertexFormat::Short4:
                case dawn::VertexFormat::Short2Norm:
                case dawn::VertexFormat::Short4Norm:
                    return GL_SHORT;
                case dawn::VertexFormat::Half2:
                case dawn::VertexFormat::Half4:
                    return GL_HALF_FLOAT;
                case dawn::VertexFormat::Float:
                case dawn::VertexFormat::Float2:
                case dawn::VertexFormat::Float3:
                case dawn::VertexFormat::Float4:
                    return GL_FLOAT;
                case dawn::VertexFormat::UInt:
                case dawn::VertexFormat::UInt2:
                case dawn::VertexFormat::UInt3:
                case dawn::VertexFormat::UInt4:
                    return GL_UNSIGNED_INT;
                case dawn::VertexFormat::Int:
                case dawn::VertexFormat::Int2:
                case dawn::VertexFormat::Int3:
                case dawn::VertexFormat::Int4:
                    return GL_INT;
                default:
                    UNREACHABLE();
            }
        }

        GLboolean VertexFormatIsNormalized(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::UChar2Norm:
                case dawn::VertexFormat::UChar4Norm:
                case dawn::VertexFormat::Char2Norm:
                case dawn::VertexFormat::Char4Norm:
                case dawn::VertexFormat::UShort2Norm:
                case dawn::VertexFormat::UShort4Norm:
                case dawn::VertexFormat::Short2Norm:
                case dawn::VertexFormat::Short4Norm:
                    return GL_TRUE;
                default:
                    return GL_FALSE;
            }
        }

        bool VertexFormatIsInt(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::UChar2:
                case dawn::VertexFormat::UChar4:
                case dawn::VertexFormat::Char2:
                case dawn::VertexFormat::Char4:
                case dawn::VertexFormat::UShort2:
                case dawn::VertexFormat::UShort4:
                case dawn::VertexFormat::Short2:
                case dawn::VertexFormat::Short4:
                case dawn::VertexFormat::UInt:
                case dawn::VertexFormat::UInt2:
                case dawn::VertexFormat::UInt3:
                case dawn::VertexFormat::UInt4:
                case dawn::VertexFormat::Int:
                case dawn::VertexFormat::Int2:
                case dawn::VertexFormat::Int3:
                case dawn::VertexFormat::Int4:
                    return true;
                default:
                    return false;
            }
        }

        GLint GetStencilMaskFromStencilFormat(dawn::TextureFormat depthStencilFormat) {
            switch (depthStencilFormat) {
                case dawn::TextureFormat::D32FloatS8Uint:
                    return 0xFF;
                default:
                    UNREACHABLE();
            }
        }

        // Vertex buffers and index buffers are implemented as part of an OpenGL VAO that
        // corresponds to an VertexInput. On the contrary in Dawn they are part of the global state.
        // This means that we have to re-apply these buffers on an VertexInput change.
        class InputBufferTracker {
          public:
            void OnSetIndexBuffer(BufferBase* buffer) {
                mIndexBufferDirty = true;
                mIndexBuffer = ToBackend(buffer);
            }

            void OnSetVertexBuffers(uint32_t startSlot,
                                    uint32_t count,
                                    Ref<BufferBase>* buffers,
                                    uint64_t* offsets) {
                for (uint32_t i = 0; i < count; ++i) {
                    uint32_t slot = startSlot + i;
                    mVertexBuffers[slot] = ToBackend(buffers[i].Get());
                    mVertexBufferOffsets[slot] = offsets[i];
                }

                // Use 64 bit masks and make sure there are no shift UB
                static_assert(kMaxVertexBuffers <= 8 * sizeof(unsigned long long) - 1, "");
                mDirtyVertexBuffers |= ((1ull << count) - 1ull) << startSlot;
            }

            void OnSetPipeline(RenderPipelineBase* pipeline) {
                if (mLastPipeline == pipeline) {
                    return;
                }

                mIndexBufferDirty = true;
                mDirtyVertexBuffers |= pipeline->GetInputsSetMask();

                mLastPipeline = pipeline;
            }

            void Apply() {
                if (mIndexBufferDirty && mIndexBuffer != nullptr) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer->GetHandle());
                    mIndexBufferDirty = false;
                }

                for (uint32_t slot :
                     IterateBitSet(mDirtyVertexBuffers & mLastPipeline->GetInputsSetMask())) {
                    for (uint32_t location :
                         IterateBitSet(mLastPipeline->GetAttributesUsingInput(slot))) {
                        auto attribute = mLastPipeline->GetAttribute(location);

                        GLuint buffer = mVertexBuffers[slot]->GetHandle();
                        uint64_t offset = mVertexBufferOffsets[slot];

                        auto input = mLastPipeline->GetInput(slot);
                        auto components = VertexFormatNumComponents(attribute.format);
                        auto formatType = VertexFormatType(attribute.format);

                        GLboolean normalized = VertexFormatIsNormalized(attribute.format);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        if (VertexFormatIsInt(attribute.format)) {
                            glVertexAttribIPointer(location, components, formatType, input.stride,
                                                   reinterpret_cast<void*>(static_cast<intptr_t>(
                                                       offset + attribute.offset)));
                        } else {
                            glVertexAttribPointer(
                                location, components, formatType, normalized, input.stride,
                                reinterpret_cast<void*>(
                                    static_cast<intptr_t>(offset + attribute.offset)));
                        }
                    }
                }

                mDirtyVertexBuffers.reset();
            }

          private:
            bool mIndexBufferDirty = false;
            Buffer* mIndexBuffer = nullptr;

            std::bitset<kMaxVertexBuffers> mDirtyVertexBuffers;
            std::array<Buffer*, kMaxVertexBuffers> mVertexBuffers;
            std::array<uint64_t, kMaxVertexBuffers> mVertexBufferOffsets;

            RenderPipelineBase* mLastPipeline = nullptr;
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

                    // TODO(shaobo.yan@intel.com): Implement dynamic buffer offset.
                    case dawn::BindingType::DynamicUniformBuffer:
                    case dawn::BindingType::DynamicStorageBuffer:
                        UNREACHABLE();
                        break;
                }
            }
        }

        void ResolveMultisampledRenderTargets(const BeginRenderPassCmd* renderPass) {
            ASSERT(renderPass != nullptr);

            GLuint readFbo = 0;
            GLuint writeFbo = 0;

            for (uint32_t i : IterateBitSet(renderPass->colorAttachmentsSet)) {
                if (renderPass->colorAttachments[i].resolveTarget.Get() != nullptr) {
                    if (readFbo == 0) {
                        ASSERT(writeFbo == 0);
                        glGenFramebuffers(1, &readFbo);
                        glGenFramebuffers(1, &writeFbo);
                    }

                    const TextureBase* colorTexture =
                        renderPass->colorAttachments[i].view->GetTexture();
                    ASSERT(colorTexture->IsMultisampledTexture());
                    ASSERT(colorTexture->GetArrayLayers() == 1);
                    ASSERT(renderPass->colorAttachments[i].view->GetBaseMipLevel() == 0);

                    GLuint colorHandle = ToBackend(colorTexture)->GetHandle();
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
                    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           ToBackend(colorTexture)->GetGLTarget(), colorHandle, 0);

                    const TextureBase* resolveTexture =
                        renderPass->colorAttachments[i].resolveTarget->GetTexture();
                    GLuint resolveTextureHandle = ToBackend(resolveTexture)->GetHandle();
                    GLuint resolveTargetMipmapLevel =
                        renderPass->colorAttachments[i].resolveTarget->GetBaseMipLevel();
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFbo);
                    if (resolveTexture->GetArrayLayers() == 1) {
                        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                               GL_TEXTURE_2D, resolveTextureHandle,
                                               resolveTargetMipmapLevel);
                    } else {
                        GLuint resolveTargetArrayLayer =
                            renderPass->colorAttachments[i].resolveTarget->GetBaseArrayLayer();
                        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                  resolveTextureHandle, resolveTargetMipmapLevel,
                                                  resolveTargetArrayLayer);
                    }

                    glBlitFramebuffer(0, 0, renderPass->width, renderPass->height, 0, 0,
                                      renderPass->width, renderPass->height, GL_COLOR_BUFFER_BIT,
                                      GL_NEAREST);
                }
            }

            glDeleteFramebuffers(1, &readFbo);
            glDeleteFramebuffers(1, &writeFbo);
        }
    }  // namespace

    CommandBuffer::CommandBuffer(Device* device, CommandEncoderBase* encoder)
        : CommandBufferBase(device, encoder), mCommands(encoder->AcquireCommands()) {
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
                    ExecuteRenderPass(cmd);
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, ToBackend(copy->source)->GetHandle());
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ToBackend(copy->destination)->GetHandle());
                    glCopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER,
                                        copy->sourceOffset, copy->destinationOffset, copy->size);

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
                    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, src.imageHeight);
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
                    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

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
                    glPixelStorei(GL_PACK_IMAGE_HEIGHT, dst.imageHeight);
                    ASSERT(copySize.depth == 1 && src.origin.z == 0);
                    void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(dst.offset));
                    glReadPixels(src.origin.x, src.origin.y, copySize.width, copySize.height,
                                 format.format, format.type, offset);
                    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
                    glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);

                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    glDeleteFramebuffers(1, &readFBO);
                } break;

                case Command::CopyTextureToTexture: {
                    CopyTextureToTextureCmd* copy =
                        mCommands.NextCommand<CopyTextureToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Texture* srcTexture = ToBackend(src.texture.Get());
                    Texture* dstTexture = ToBackend(dst.texture.Get());

                    glCopyImageSubData(srcTexture->GetHandle(), srcTexture->GetGLTarget(),
                                       src.level, src.origin.x, src.origin.y, src.slice,
                                       dstTexture->GetHandle(), dstTexture->GetGLTarget(),
                                       dst.level, dst.origin.x, dst.origin.y, dst.slice,
                                       copySize.width, copySize.height, 1);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::ExecuteComputePass() {
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
                    glDispatchCompute(dispatch->x, dispatch->y, dispatch->z);
                    // TODO(cwallez@chromium.org): add barriers to the API
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                } break;

                case Command::DispatchIndirect: {
                    DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                    uint64_t indirectBufferOffset = dispatch->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(dispatch->indirectBuffer.Get());

                    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    glDispatchComputeIndirect(static_cast<GLintptr>(indirectBufferOffset));
                    // TODO(cwallez@chromium.org): add barriers to the API
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();
                    lastPipeline->ApplyNow();
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

    void CommandBuffer::ExecuteRenderPass(BeginRenderPassCmd* renderPass) {
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
            for (uint32_t i : IterateBitSet(renderPass->colorAttachmentsSet)) {
                TextureViewBase* textureView = renderPass->colorAttachments[i].view.Get();
                GLuint texture = ToBackend(textureView->GetTexture())->GetHandle();

                // Attach color buffers.
                if (textureView->GetTexture()->GetArrayLayers() == 1) {
                    GLenum target = ToBackend(textureView->GetTexture())->GetGLTarget();
                    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target,
                                           texture, textureView->GetBaseMipLevel());
                } else {
                    glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                              texture, textureView->GetBaseMipLevel(),
                                              textureView->GetBaseArrayLayer());
                }
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

            if (renderPass->hasDepthStencilAttachment) {
                TextureViewBase* textureView = renderPass->depthStencilAttachment.view.Get();
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

                GLenum target = ToBackend(textureView->GetTexture())->GetGLTarget();
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachment, target, texture, 0);

                // TODO(kainino@chromium.org): the depth/stencil clears (later in
                // this function) may be undefined for other texture formats.
                ASSERT(format == dawn::TextureFormat::D32FloatS8Uint);
            }
        }

        // Set defaults for dynamic state before executing clears and commands.
        PersistentPipelineState persistentPipelineState;
        persistentPipelineState.SetDefaultState();
        glBlendColor(0, 0, 0, 0);
        glViewport(0, 0, renderPass->width, renderPass->height);
        glScissor(0, 0, renderPass->width, renderPass->height);

        // Clear framebuffer attachments as needed
        {
            for (uint32_t i : IterateBitSet(renderPass->colorAttachmentsSet)) {
                const auto& attachmentInfo = renderPass->colorAttachments[i];

                // Load op - color
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    glColorMaski(i, true, true, true, true);
                    glClearBufferfv(GL_COLOR, i, &attachmentInfo.clearColor.r);
                }
            }

            if (renderPass->hasDepthStencilAttachment) {
                const auto& attachmentInfo = renderPass->depthStencilAttachment;
                dawn::TextureFormat attachmentFormat =
                    attachmentInfo.view->GetTexture()->GetFormat();

                // Load op - depth/stencil
                bool doDepthClear = TextureFormatHasDepth(attachmentFormat) &&
                                    (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear);
                bool doStencilClear = TextureFormatHasStencil(attachmentFormat) &&
                                      (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear);

                if (doDepthClear) {
                    glDepthMask(GL_TRUE);
                }
                if (doStencilClear) {
                    glStencilMask(GetStencilMaskFromStencilFormat(attachmentFormat));
                }

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
        uint64_t indexBufferBaseOffset = 0;

        InputBufferTracker inputBuffers;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();

                    if (renderPass->sampleCount > 1) {
                        ResolveMultisampledRenderTargets(renderPass);
                    }

                    glDeleteFramebuffers(1, &fbo);
                    return;
                } break;

                case Command::Draw: {
                    DrawCmd* draw = mCommands.NextCommand<DrawCmd>();
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

                case Command::DrawIndexed: {
                    DrawIndexedCmd* draw = mCommands.NextCommand<DrawIndexedCmd>();
                    inputBuffers.Apply();

                    dawn::IndexFormat indexFormat =
                        lastPipeline->GetVertexInputDescriptor()->indexFormat;
                    size_t formatSize = IndexFormatSize(indexFormat);
                    GLenum formatType = IndexFormatType(indexFormat);

                    if (draw->firstInstance > 0) {
                        glDrawElementsInstancedBaseVertexBaseInstance(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount, draw->baseVertex, draw->firstInstance);
                    } else {
                        // This branch is only needed on OpenGL < 4.2
                        glDrawElementsInstancedBaseVertex(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount, draw->baseVertex);
                    }
                } break;

                case Command::DrawIndirect: {
                    DrawIndirectCmd* draw = mCommands.NextCommand<DrawIndirectCmd>();
                    inputBuffers.Apply();

                    uint64_t indirectBufferOffset = draw->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());

                    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    glDrawArraysIndirect(
                        lastPipeline->GetGLPrimitiveTopology(),
                        reinterpret_cast<void*>(static_cast<intptr_t>(indirectBufferOffset)));
                } break;

                case Command::DrawIndexedIndirect: {
                    DrawIndexedIndirectCmd* draw = mCommands.NextCommand<DrawIndexedIndirectCmd>();
                    inputBuffers.Apply();

                    dawn::IndexFormat indexFormat =
                        lastPipeline->GetVertexInputDescriptor()->indexFormat;
                    GLenum formatType = IndexFormatType(indexFormat);

                    uint64_t indirectBufferOffset = draw->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());

                    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    glDrawElementsIndirect(
                        lastPipeline->GetGLPrimitiveTopology(), formatType,
                        reinterpret_cast<void*>(static_cast<intptr_t>(indirectBufferOffset)));
                } break;

                case Command::InsertDebugMarker:
                case Command::PopDebugGroup:
                case Command::PushDebugGroup: {
                    // Due to lack of linux driver support for GL_EXT_debug_marker
                    // extension these functions are skipped.
                    SkipCommand(&mCommands, type);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();
                    lastPipeline->ApplyNow(persistentPipelineState);

                    inputBuffers.OnSetPipeline(lastPipeline);
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
                    glBlendColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
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
                    auto offsets = mCommands.NextData<uint64_t>(cmd->count);
                    inputBuffers.OnSetVertexBuffers(cmd->startSlot, cmd->count, buffers, offsets);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::opengl
