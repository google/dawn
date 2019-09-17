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
#include "dawn_native/BindGroupTracker.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/ComputePipelineGL.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/Forward.h"
#include "dawn_native/opengl/PersistentPipelineStateGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/RenderPipelineGL.h"
#include "dawn_native/opengl/SamplerGL.h"
#include "dawn_native/opengl/TextureGL.h"
#include "dawn_native/opengl/UtilsGL.h"

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

            void Apply(const OpenGLFunctions& gl) {
                if (mIndexBufferDirty && mIndexBuffer != nullptr) {
                    gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer->GetHandle());
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
                        gl.BindBuffer(GL_ARRAY_BUFFER, buffer);
                        if (VertexFormatIsInt(attribute.format)) {
                            gl.VertexAttribIPointer(location, components, formatType, input.stride,
                                                    reinterpret_cast<void*>(static_cast<intptr_t>(
                                                        offset + attribute.offset)));
                        } else {
                            gl.VertexAttribPointer(
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

        class BindGroupTracker : public BindGroupTrackerBase<BindGroupBase*, false> {
          public:
            void OnSetPipeline(RenderPipeline* pipeline) {
                BindGroupTrackerBase::OnSetPipeline(pipeline);
                mPipeline = pipeline;
            }

            void OnSetPipeline(ComputePipeline* pipeline) {
                BindGroupTrackerBase::OnSetPipeline(pipeline);
                mPipeline = pipeline;
            }

            void Apply(const OpenGLFunctions& gl) {
                for (uint32_t index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
                    ApplyBindGroup(gl, index, mBindGroups[index], mDynamicOffsetCounts[index],
                                   mDynamicOffsets[index].data());
                }
                DidApply();
            }

          private:
            void ApplyBindGroup(const OpenGLFunctions& gl,
                                uint32_t index,
                                BindGroupBase* group,
                                uint32_t dynamicOffsetCount,
                                uint64_t* dynamicOffsets) {
                const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];
                const auto& layout = group->GetLayout()->GetBindingInfo();
                uint32_t currentDynamicIndex = 0;

                for (uint32_t bindingIndex : IterateBitSet(layout.mask)) {
                    switch (layout.types[bindingIndex]) {
                        case dawn::BindingType::UniformBuffer: {
                            BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                            GLuint buffer = ToBackend(binding.buffer)->GetHandle();
                            GLuint uboIndex = indices[bindingIndex];
                            GLuint offset = binding.offset;

                            if (layout.dynamic[bindingIndex]) {
                                offset += dynamicOffsets[currentDynamicIndex];
                                ++currentDynamicIndex;
                            }

                            gl.BindBufferRange(GL_UNIFORM_BUFFER, uboIndex, buffer, offset,
                                               binding.size);
                        } break;

                        case dawn::BindingType::Sampler: {
                            Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                            GLuint samplerIndex = indices[bindingIndex];

                            for (PipelineGL::SamplerUnit unit :
                                 mPipeline->GetTextureUnitsForSampler(samplerIndex)) {
                                // Only use filtering for certain texture units, because int and
                                // uint texture are only complete without filtering
                                if (unit.shouldUseFiltering) {
                                    gl.BindSampler(unit.unit, sampler->GetFilteringHandle());
                                } else {
                                    gl.BindSampler(unit.unit, sampler->GetNonFilteringHandle());
                                }
                            }
                        } break;

                        case dawn::BindingType::SampledTexture: {
                            TextureView* view =
                                ToBackend(group->GetBindingAsTextureView(bindingIndex));
                            GLuint handle = view->GetHandle();
                            GLenum target = view->GetGLTarget();
                            GLuint viewIndex = indices[bindingIndex];

                            for (auto unit : mPipeline->GetTextureUnitsForTextureView(viewIndex)) {
                                gl.ActiveTexture(GL_TEXTURE0 + unit);
                                gl.BindTexture(target, handle);
                            }
                        } break;

                        case dawn::BindingType::StorageBuffer: {
                            BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                            GLuint buffer = ToBackend(binding.buffer)->GetHandle();
                            GLuint ssboIndex = indices[bindingIndex];
                            GLuint offset = binding.offset;

                            if (layout.dynamic[bindingIndex]) {
                                offset += dynamicOffsets[currentDynamicIndex];
                                ++currentDynamicIndex;
                            }

                            gl.BindBufferRange(GL_SHADER_STORAGE_BUFFER, ssboIndex, buffer, offset,
                                               binding.size);
                        } break;

                        case dawn::BindingType::StorageTexture:
                        case dawn::BindingType::ReadonlyStorageBuffer:
                            UNREACHABLE();
                            break;

                            // TODO(shaobo.yan@intel.com): Implement dynamic buffer offset.
                    }
                }
            }

            PipelineGL* mPipeline = nullptr;
        };

        void ResolveMultisampledRenderTargets(const OpenGLFunctions& gl,
                                              const BeginRenderPassCmd* renderPass) {
            ASSERT(renderPass != nullptr);

            GLuint readFbo = 0;
            GLuint writeFbo = 0;

            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                if (renderPass->colorAttachments[i].resolveTarget.Get() != nullptr) {
                    if (readFbo == 0) {
                        ASSERT(writeFbo == 0);
                        gl.GenFramebuffers(1, &readFbo);
                        gl.GenFramebuffers(1, &writeFbo);
                    }

                    const TextureBase* colorTexture =
                        renderPass->colorAttachments[i].view->GetTexture();
                    ASSERT(colorTexture->IsMultisampledTexture());
                    ASSERT(colorTexture->GetArrayLayers() == 1);
                    ASSERT(renderPass->colorAttachments[i].view->GetBaseMipLevel() == 0);

                    GLuint colorHandle = ToBackend(colorTexture)->GetHandle();
                    gl.BindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
                    gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                            ToBackend(colorTexture)->GetGLTarget(), colorHandle, 0);

                    const TextureBase* resolveTexture =
                        renderPass->colorAttachments[i].resolveTarget->GetTexture();
                    GLuint resolveTextureHandle = ToBackend(resolveTexture)->GetHandle();
                    GLuint resolveTargetMipmapLevel =
                        renderPass->colorAttachments[i].resolveTarget->GetBaseMipLevel();
                    gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFbo);
                    if (resolveTexture->GetArrayLayers() == 1) {
                        gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                GL_TEXTURE_2D, resolveTextureHandle,
                                                resolveTargetMipmapLevel);
                    } else {
                        GLuint resolveTargetArrayLayer =
                            renderPass->colorAttachments[i].resolveTarget->GetBaseArrayLayer();
                        gl.FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                   resolveTextureHandle, resolveTargetMipmapLevel,
                                                   resolveTargetArrayLayer);
                    }

                    gl.BlitFramebuffer(0, 0, renderPass->width, renderPass->height, 0, 0,
                                       renderPass->width, renderPass->height, GL_COLOR_BUFFER_BIT,
                                       GL_NEAREST);
                }
            }

            gl.DeleteFramebuffers(1, &readFbo);
            gl.DeleteFramebuffers(1, &writeFbo);
        }

        // OpenGL SPEC requires the source/destination region must be a region that is contained
        // within srcImage/dstImage. Here the size of the image refers to the virtual size, while
        // Dawn validates texture copy extent with the physical size, so we need to re-calculate the
        // texture copy extent to ensure it should fit in the virtual size of the subresource.
        Extent3D ComputeTextureCopyExtent(const TextureCopy& textureCopy,
                                          const Extent3D& copySize) {
            Extent3D validTextureCopyExtent = copySize;
            const TextureBase* texture = textureCopy.texture.Get();
            Extent3D virtualSizeAtLevel = texture->GetMipLevelVirtualSize(textureCopy.mipLevel);
            if (textureCopy.origin.x + copySize.width > virtualSizeAtLevel.width) {
                ASSERT(texture->GetFormat().isCompressed);
                validTextureCopyExtent.width = virtualSizeAtLevel.width - textureCopy.origin.x;
            }
            if (textureCopy.origin.y + copySize.height > virtualSizeAtLevel.height) {
                ASSERT(texture->GetFormat().isCompressed);
                validTextureCopyExtent.height = virtualSizeAtLevel.height - textureCopy.origin.y;
            }

            return validTextureCopyExtent;
        }

    }  // namespace

    CommandBuffer::CommandBuffer(CommandEncoderBase* encoder,
                                 const CommandBufferDescriptor* descriptor)
        : CommandBufferBase(encoder, descriptor), mCommands(encoder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::Execute() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        auto TransitionForPass = [](const PassResourceUsage& usages) {
            for (size_t i = 0; i < usages.textures.size(); i++) {
                Texture* texture = ToBackend(usages.textures[i]);
                // We count the lazy clears for non output attachment textures in order to match the
                // backdoor lazy clear counts in Vulkan and D3D12.
                bool isLazyClear =
                    !(usages.textureUsages[i] & dawn::TextureUsage::OutputAttachment);
                texture->EnsureSubresourceContentInitialized(
                    0, texture->GetNumMipLevels(), 0, texture->GetArrayLayers(), isLazyClear);
            }
        };

        const std::vector<PassResourceUsage>& passResourceUsages = GetResourceUsages().perPass;
        uint32_t nextPassNumber = 0;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();
                    TransitionForPass(passResourceUsages[nextPassNumber]);
                    ExecuteComputePass();

                    nextPassNumber++;
                } break;

                case Command::BeginRenderPass: {
                    auto* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                    TransitionForPass(passResourceUsages[nextPassNumber]);
                    ExecuteRenderPass(cmd);

                    nextPassNumber++;
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();

                    gl.BindBuffer(GL_PIXEL_PACK_BUFFER, ToBackend(copy->source)->GetHandle());
                    gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER,
                                  ToBackend(copy->destination)->GetHandle());
                    gl.CopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER,
                                         copy->sourceOffset, copy->destinationOffset, copy->size);

                    gl.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Buffer* buffer = ToBackend(src.buffer.Get());
                    Texture* texture = ToBackend(dst.texture.Get());
                    GLenum target = texture->GetGLTarget();
                    const GLFormat& format = texture->GetGLFormat();
                    if (IsCompleteSubresourceCopiedTo(texture, copySize, dst.mipLevel)) {
                        texture->SetIsSubresourceContentInitialized(dst.mipLevel, 1, dst.arrayLayer,
                                                                    1);
                    } else {
                        texture->EnsureSubresourceContentInitialized(dst.mipLevel, 1,
                                                                     dst.arrayLayer, 1);
                    }

                    gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->GetHandle());
                    gl.ActiveTexture(GL_TEXTURE0);
                    gl.BindTexture(target, texture->GetHandle());

                    const Format& formatInfo = texture->GetFormat();
                    gl.PixelStorei(GL_UNPACK_ROW_LENGTH,
                                   src.rowPitch / formatInfo.blockByteSize * formatInfo.blockWidth);
                    gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, src.imageHeight);

                    if (formatInfo.isCompressed) {
                        gl.PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_SIZE, formatInfo.blockByteSize);
                        gl.PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_WIDTH, formatInfo.blockWidth);
                        gl.PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_HEIGHT, formatInfo.blockHeight);
                        gl.PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_DEPTH, 1);

                        ASSERT(texture->GetDimension() == dawn::TextureDimension::e2D);
                        uint64_t copyDataSize = (copySize.width / formatInfo.blockWidth) *
                                                (copySize.height / formatInfo.blockHeight) *
                                                formatInfo.blockByteSize;
                        Extent3D copyExtent = ComputeTextureCopyExtent(dst, copySize);

                        if (texture->GetArrayLayers() > 1) {
                            gl.CompressedTexSubImage3D(
                                target, dst.mipLevel, dst.origin.x, dst.origin.y, dst.arrayLayer,
                                copyExtent.width, copyExtent.height, 1, format.internalFormat,
                                copyDataSize,
                                reinterpret_cast<void*>(static_cast<uintptr_t>(src.offset)));
                        } else {
                            gl.CompressedTexSubImage2D(
                                target, dst.mipLevel, dst.origin.x, dst.origin.y, copyExtent.width,
                                copyExtent.height, format.internalFormat, copyDataSize,
                                reinterpret_cast<void*>(static_cast<uintptr_t>(src.offset)));
                        }
                    } else {
                        switch (texture->GetDimension()) {
                            case dawn::TextureDimension::e2D:
                                if (texture->GetArrayLayers() > 1) {
                                    gl.TexSubImage3D(target, dst.mipLevel, dst.origin.x,
                                                     dst.origin.y, dst.arrayLayer, copySize.width,
                                                     copySize.height, 1, format.format, format.type,
                                                     reinterpret_cast<void*>(
                                                         static_cast<uintptr_t>(src.offset)));
                                } else {
                                    gl.TexSubImage2D(target, dst.mipLevel, dst.origin.x,
                                                     dst.origin.y, copySize.width, copySize.height,
                                                     format.format, format.type,
                                                     reinterpret_cast<void*>(
                                                         static_cast<uintptr_t>(src.offset)));
                                }
                                break;

                            default:
                                UNREACHABLE();
                        }
                    }

                    gl.PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                    gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

                    gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Texture* texture = ToBackend(src.texture.Get());
                    Buffer* buffer = ToBackend(dst.buffer.Get());
                    const GLFormat& format = texture->GetGLFormat();
                    GLenum target = texture->GetGLTarget();

                    // TODO(jiawei.shao@intel.com): support texture-to-buffer copy with compressed
                    // texture formats.
                    if (texture->GetFormat().isCompressed) {
                        UNREACHABLE();
                    }

                    texture->EnsureSubresourceContentInitialized(src.mipLevel, 1, src.arrayLayer,
                                                                 1);
                    // The only way to move data from a texture to a buffer in GL is via
                    // glReadPixels with a pack buffer. Create a temporary FBO for the copy.
                    gl.BindTexture(target, texture->GetHandle());

                    GLuint readFBO = 0;
                    gl.GenFramebuffers(1, &readFBO);
                    gl.BindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
                    switch (texture->GetDimension()) {
                        case dawn::TextureDimension::e2D:
                            if (texture->GetArrayLayers() > 1) {
                                gl.FramebufferTextureLayer(
                                    GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->GetHandle(),
                                    src.mipLevel, src.arrayLayer);
                            } else {
                                gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                        GL_TEXTURE_2D, texture->GetHandle(),
                                                        src.mipLevel);
                            }
                            break;

                        default:
                            UNREACHABLE();
                    }

                    gl.BindBuffer(GL_PIXEL_PACK_BUFFER, buffer->GetHandle());
                    gl.PixelStorei(GL_PACK_ROW_LENGTH,
                                   dst.rowPitch / texture->GetFormat().blockByteSize);
                    gl.PixelStorei(GL_PACK_IMAGE_HEIGHT, dst.imageHeight);
                    ASSERT(copySize.depth == 1 && src.origin.z == 0);
                    void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(dst.offset));
                    gl.ReadPixels(src.origin.x, src.origin.y, copySize.width, copySize.height,
                                  format.format, format.type, offset);
                    gl.PixelStorei(GL_PACK_ROW_LENGTH, 0);
                    gl.PixelStorei(GL_PACK_IMAGE_HEIGHT, 0);

                    gl.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    gl.DeleteFramebuffers(1, &readFBO);
                } break;

                case Command::CopyTextureToTexture: {
                    CopyTextureToTextureCmd* copy =
                        mCommands.NextCommand<CopyTextureToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    // TODO(jiawei.shao@intel.com): add workaround for the case that imageExtentSrc
                    // is not equal to imageExtentDst. For example when copySize fits in the virtual
                    // size of the source image but does not fit in the one of the destination
                    // image.
                    Extent3D copySize = ComputeTextureCopyExtent(dst, copy->copySize);
                    Texture* srcTexture = ToBackend(src.texture.Get());
                    Texture* dstTexture = ToBackend(dst.texture.Get());
                    srcTexture->EnsureSubresourceContentInitialized(src.mipLevel, 1, src.arrayLayer,
                                                                    1);
                    if (IsCompleteSubresourceCopiedTo(dstTexture, copySize, dst.mipLevel)) {
                        dstTexture->SetIsSubresourceContentInitialized(dst.mipLevel, 1,
                                                                       dst.arrayLayer, 1);
                    } else {
                        dstTexture->EnsureSubresourceContentInitialized(dst.mipLevel, 1,
                                                                        dst.arrayLayer, 1);
                    }
                    gl.CopyImageSubData(srcTexture->GetHandle(), srcTexture->GetGLTarget(),
                                        src.mipLevel, src.origin.x, src.origin.y, src.arrayLayer,
                                        dstTexture->GetHandle(), dstTexture->GetGLTarget(),
                                        dst.mipLevel, dst.origin.x, dst.origin.y, dst.arrayLayer,
                                        copySize.width, copySize.height, 1);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::ExecuteComputePass() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;
        ComputePipeline* lastPipeline = nullptr;
        BindGroupTracker bindGroupTracker = {};

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    return;
                } break;

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    bindGroupTracker.Apply(gl);

                    gl.DispatchCompute(dispatch->x, dispatch->y, dispatch->z);
                    // TODO(cwallez@chromium.org): add barriers to the API
                    gl.MemoryBarrier(GL_ALL_BARRIER_BITS);
                } break;

                case Command::DispatchIndirect: {
                    DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();
                    bindGroupTracker.Apply(gl);

                    uint64_t indirectBufferOffset = dispatch->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(dispatch->indirectBuffer.Get());

                    gl.BindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    gl.DispatchComputeIndirect(static_cast<GLintptr>(indirectBufferOffset));
                    // TODO(cwallez@chromium.org): add barriers to the API
                    gl.MemoryBarrier(GL_ALL_BARRIER_BITS);
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();
                    lastPipeline->ApplyNow();

                    bindGroupTracker.OnSetPipeline(lastPipeline);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    uint64_t* dynamicOffsets = nullptr;
                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = mCommands.NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }
                    bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                    cmd->dynamicOffsetCount, dynamicOffsets);
                } break;

                case Command::InsertDebugMarker:
                case Command::PopDebugGroup:
                case Command::PushDebugGroup: {
                    // Due to lack of linux driver support for GL_EXT_debug_marker
                    // extension these functions are skipped.
                    SkipCommand(&mCommands, type);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndComputePass should have been called
        UNREACHABLE();
    }

    void CommandBuffer::ExecuteRenderPass(BeginRenderPassCmd* renderPass) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;
        GLuint fbo = 0;

        // Create the framebuffer used for this render pass and calls the correct glDrawBuffers
        {
            // TODO(kainino@chromium.org): This is added to possibly work around an issue seen on
            // Windows/Intel. It should break any feedback loop before the clears, even if there
            // shouldn't be any negative effects from this. Investigate whether it's actually
            // needed.
            gl.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            // TODO(kainino@chromium.org): possible future optimization: create these framebuffers
            // at Framebuffer build time (or maybe CommandBuffer build time) so they don't have to
            // be created and destroyed at draw time.
            gl.GenFramebuffers(1, &fbo);
            gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

            // Mapping from attachmentSlot to GL framebuffer attachment points. Defaults to zero
            // (GL_NONE).
            std::array<GLenum, kMaxColorAttachments> drawBuffers = {};

            // Construct GL framebuffer

            unsigned int attachmentCount = 0;
            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                TextureViewBase* textureView = renderPass->colorAttachments[i].view.Get();
                GLuint texture = ToBackend(textureView->GetTexture())->GetHandle();

                // Attach color buffers.
                if (textureView->GetTexture()->GetArrayLayers() == 1) {
                    GLenum target = ToBackend(textureView->GetTexture())->GetGLTarget();
                    gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target,
                                            texture, textureView->GetBaseMipLevel());
                } else {
                    gl.FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                               texture, textureView->GetBaseMipLevel(),
                                               textureView->GetBaseArrayLayer());
                }
                drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
                attachmentCount = i + 1;
            }
            gl.DrawBuffers(attachmentCount, drawBuffers.data());

            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                TextureViewBase* textureView = renderPass->depthStencilAttachment.view.Get();
                GLuint texture = ToBackend(textureView->GetTexture())->GetHandle();
                const Format& format = textureView->GetTexture()->GetFormat();

                // Attach depth/stencil buffer.
                GLenum glAttachment = 0;
                // TODO(kainino@chromium.org): it may be valid to just always use
                // GL_DEPTH_STENCIL_ATTACHMENT here.
                switch (format.aspect) {
                    case Format::Aspect::Depth:
                        glAttachment = GL_DEPTH_ATTACHMENT;
                        break;
                    case Format::Aspect::Stencil:
                        glAttachment = GL_STENCIL_ATTACHMENT;
                        break;
                    case Format::Aspect::DepthStencil:
                        glAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
                        break;
                    default:
                        UNREACHABLE();
                        break;
                }

                GLenum target = ToBackend(textureView->GetTexture())->GetGLTarget();
                gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachment, target, texture, 0);
            }
        }

        ASSERT(gl.CheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        // Set defaults for dynamic state before executing clears and commands.
        PersistentPipelineState persistentPipelineState;
        persistentPipelineState.SetDefaultState(gl);
        gl.BlendColor(0, 0, 0, 0);
        gl.Viewport(0, 0, renderPass->width, renderPass->height);
        gl.DepthRangef(0.0, 1.0);
        gl.Scissor(0, 0, renderPass->width, renderPass->height);

        // Clear framebuffer attachments as needed
        {
            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                const auto& attachmentInfo = renderPass->colorAttachments[i];

                // Load op - color
                // TODO(cwallez@chromium.org): Choose the clear function depending on the
                // componentType: things work for now because the clear color is always a float, but
                // when that's fixed will lose precision on integer formats when converting to
                // float.
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    gl.ColorMaski(i, true, true, true, true);
                    gl.ClearBufferfv(GL_COLOR, i, &attachmentInfo.clearColor.r);
                }
            }

            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                const auto& attachmentInfo = renderPass->depthStencilAttachment;
                const Format& attachmentFormat = attachmentInfo.view->GetTexture()->GetFormat();

                // Load op - depth/stencil
                bool doDepthClear = attachmentFormat.HasDepth() &&
                                    (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear);
                bool doStencilClear = attachmentFormat.HasStencil() &&
                                      (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear);

                if (doDepthClear) {
                    gl.DepthMask(GL_TRUE);
                }
                if (doStencilClear) {
                    gl.StencilMask(GetStencilMaskFromStencilFormat(attachmentFormat.format));
                }

                if (doDepthClear && doStencilClear) {
                    gl.ClearBufferfi(GL_DEPTH_STENCIL, 0, attachmentInfo.clearDepth,
                                     attachmentInfo.clearStencil);
                } else if (doDepthClear) {
                    gl.ClearBufferfv(GL_DEPTH, 0, &attachmentInfo.clearDepth);
                } else if (doStencilClear) {
                    const GLint clearStencil = attachmentInfo.clearStencil;
                    gl.ClearBufferiv(GL_STENCIL, 0, &clearStencil);
                }
            }
        }

        RenderPipeline* lastPipeline = nullptr;
        uint64_t indexBufferBaseOffset = 0;

        InputBufferTracker inputBuffers;
        BindGroupTracker bindGroupTracker = {};

        auto DoRenderBundleCommand = [&](CommandIterator* iter, Command type) {
            switch (type) {
                case Command::Draw: {
                    DrawCmd* draw = iter->NextCommand<DrawCmd>();
                    inputBuffers.Apply(gl);
                    bindGroupTracker.Apply(gl);

                    if (draw->firstInstance > 0) {
                        gl.DrawArraysInstancedBaseInstance(
                            lastPipeline->GetGLPrimitiveTopology(), draw->firstVertex,
                            draw->vertexCount, draw->instanceCount, draw->firstInstance);
                    } else {
                        // This branch is only needed on OpenGL < 4.2
                        gl.DrawArraysInstanced(lastPipeline->GetGLPrimitiveTopology(),
                                               draw->firstVertex, draw->vertexCount,
                                               draw->instanceCount);
                    }
                } break;

                case Command::DrawIndexed: {
                    DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();
                    inputBuffers.Apply(gl);
                    bindGroupTracker.Apply(gl);

                    dawn::IndexFormat indexFormat =
                        lastPipeline->GetVertexInputDescriptor()->indexFormat;
                    size_t formatSize = IndexFormatSize(indexFormat);
                    GLenum formatType = IndexFormatType(indexFormat);

                    if (draw->firstInstance > 0) {
                        gl.DrawElementsInstancedBaseVertexBaseInstance(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount, draw->baseVertex, draw->firstInstance);
                    } else {
                        // This branch is only needed on OpenGL < 4.2
                        gl.DrawElementsInstancedBaseVertex(
                            lastPipeline->GetGLPrimitiveTopology(), draw->indexCount, formatType,
                            reinterpret_cast<void*>(draw->firstIndex * formatSize +
                                                    indexBufferBaseOffset),
                            draw->instanceCount, draw->baseVertex);
                    }
                } break;

                case Command::DrawIndirect: {
                    DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();
                    inputBuffers.Apply(gl);
                    bindGroupTracker.Apply(gl);

                    uint64_t indirectBufferOffset = draw->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());

                    gl.BindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    gl.DrawArraysIndirect(
                        lastPipeline->GetGLPrimitiveTopology(),
                        reinterpret_cast<void*>(static_cast<intptr_t>(indirectBufferOffset)));
                } break;

                case Command::DrawIndexedIndirect: {
                    DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();
                    inputBuffers.Apply(gl);
                    bindGroupTracker.Apply(gl);

                    dawn::IndexFormat indexFormat =
                        lastPipeline->GetVertexInputDescriptor()->indexFormat;
                    GLenum formatType = IndexFormatType(indexFormat);

                    uint64_t indirectBufferOffset = draw->indirectOffset;
                    Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());

                    gl.BindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle());
                    gl.DrawElementsIndirect(
                        lastPipeline->GetGLPrimitiveTopology(), formatType,
                        reinterpret_cast<void*>(static_cast<intptr_t>(indirectBufferOffset)));
                } break;

                case Command::InsertDebugMarker:
                case Command::PopDebugGroup:
                case Command::PushDebugGroup: {
                    // Due to lack of linux driver support for GL_EXT_debug_marker
                    // extension these functions are skipped.
                    SkipCommand(iter, type);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();
                    lastPipeline->ApplyNow(persistentPipelineState);

                    inputBuffers.OnSetPipeline(lastPipeline);
                    bindGroupTracker.OnSetPipeline(lastPipeline);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                    uint64_t* dynamicOffsets = nullptr;
                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = iter->NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }
                    bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                    cmd->dynamicOffsetCount, dynamicOffsets);
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();
                    indexBufferBaseOffset = cmd->offset;
                    inputBuffers.OnSetIndexBuffer(cmd->buffer.Get());
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = iter->NextCommand<SetVertexBuffersCmd>();
                    auto buffers = iter->NextData<Ref<BufferBase>>(cmd->count);
                    auto offsets = iter->NextData<uint64_t>(cmd->count);
                    inputBuffers.OnSetVertexBuffers(cmd->startSlot, cmd->count, buffers, offsets);
                } break;

                default:
                    UNREACHABLE();
                    break;
            }
        };

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();

                    if (renderPass->attachmentState->GetSampleCount() > 1) {
                        ResolveMultisampledRenderTargets(gl, renderPass);
                    }
                    gl.DeleteFramebuffers(1, &fbo);
                    return;
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                    persistentPipelineState.SetStencilReference(gl, cmd->reference);
                } break;

                case Command::SetViewport: {
                    SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                    gl.ViewportIndexedf(0, cmd->x, cmd->y, cmd->width, cmd->height);
                    gl.DepthRangef(cmd->minDepth, cmd->maxDepth);
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    gl.Scissor(cmd->x, cmd->y, cmd->width, cmd->height);
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    gl.BlendColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
                } break;

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                    auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        CommandIterator* iter = bundles[i]->GetCommands();
                        iter->Reset();
                        while (iter->NextCommandId(&type)) {
                            DoRenderBundleCommand(iter, type);
                        }
                    }
                } break;

                default: { DoRenderBundleCommand(&mCommands, type); } break;
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::opengl
