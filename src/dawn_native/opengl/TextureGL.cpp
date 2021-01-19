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

#include "dawn_native/opengl/TextureGL.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "dawn_native/EnumMaskIterator.h"
#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/UtilsGL.h"

namespace dawn_native { namespace opengl {

    namespace {

        GLenum TargetForTexture(const TextureDescriptor* descriptor) {
            switch (descriptor->dimension) {
                case wgpu::TextureDimension::e2D:
                    if (descriptor->size.depth > 1) {
                        ASSERT(descriptor->sampleCount == 1);
                        return GL_TEXTURE_2D_ARRAY;
                    } else {
                        if (descriptor->sampleCount > 1) {
                            return GL_TEXTURE_2D_MULTISAMPLE;
                        } else {
                            return GL_TEXTURE_2D;
                        }
                    }

                case wgpu::TextureDimension::e1D:
                case wgpu::TextureDimension::e3D:
                    UNREACHABLE();
            }
        }

        GLenum TargetForTextureViewDimension(wgpu::TextureViewDimension dimension,
                                             uint32_t arrayLayerCount,
                                             uint32_t sampleCount) {
            switch (dimension) {
                case wgpu::TextureViewDimension::e2D:
                    return (sampleCount > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
                case wgpu::TextureViewDimension::e2DArray:
                    if (arrayLayerCount == 1) {
                        return (sampleCount > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
                    }
                    ASSERT(sampleCount == 1);
                    return GL_TEXTURE_2D_ARRAY;
                case wgpu::TextureViewDimension::Cube:
                    return GL_TEXTURE_CUBE_MAP;
                case wgpu::TextureViewDimension::CubeArray:
                    return GL_TEXTURE_CUBE_MAP_ARRAY;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e3D:
                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
            }
        }

        GLuint GenTexture(const OpenGLFunctions& gl) {
            GLuint handle = 0;
            gl.GenTextures(1, &handle);
            return handle;
        }

        bool UsageNeedsTextureView(wgpu::TextureUsage usage) {
            constexpr wgpu::TextureUsage kUsageNeedingTextureView =
                wgpu::TextureUsage::Storage | wgpu::TextureUsage::Sampled;
            return usage & kUsageNeedingTextureView;
        }

        bool RequiresCreatingNewTextureView(const TextureBase* texture,
                                            const TextureViewDescriptor* textureViewDescriptor) {
            if (texture->GetFormat().format != textureViewDescriptor->format) {
                return true;
            }

            if (texture->GetArrayLayers() != textureViewDescriptor->arrayLayerCount) {
                return true;
            }

            if (texture->GetNumMipLevels() != textureViewDescriptor->mipLevelCount) {
                return true;
            }

            if (ToBackend(texture)->GetGLFormat().format == GL_DEPTH_STENCIL &&
                (texture->GetUsage() & wgpu::TextureUsage::Sampled) != 0 &&
                textureViewDescriptor->aspect == wgpu::TextureAspect::StencilOnly) {
                // We need a separate view for one of the depth or stencil planes
                // because each glTextureView needs it's own handle to set
                // GL_DEPTH_STENCIL_TEXTURE_MODE. Choose the stencil aspect for the
                // extra handle since it is likely sampled less often.
                return true;
            }

            switch (textureViewDescriptor->dimension) {
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    return true;
                default:
                    break;
            }

            return false;
        }

    }  // namespace

    // Texture

    Texture::Texture(Device* device, const TextureDescriptor* descriptor)
        : Texture(device, descriptor, GenTexture(device->gl), TextureState::OwnedInternal) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        uint32_t width = GetWidth();
        uint32_t height = GetHeight();
        uint32_t levels = GetNumMipLevels();
        uint32_t arrayLayers = GetArrayLayers();
        uint32_t sampleCount = GetSampleCount();

        const GLFormat& glFormat = GetGLFormat();

        gl.BindTexture(mTarget, mHandle);

        // glTextureView() requires the value of GL_TEXTURE_IMMUTABLE_FORMAT for origtexture to be
        // GL_TRUE, so the storage of the texture must be allocated with glTexStorage*D.
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTextureView.xhtml
        switch (GetDimension()) {
            case wgpu::TextureDimension::e2D:
                if (arrayLayers > 1) {
                    ASSERT(!IsMultisampledTexture());
                    gl.TexStorage3D(mTarget, levels, glFormat.internalFormat, width, height,
                                    arrayLayers);
                } else {
                    if (IsMultisampledTexture()) {
                        gl.TexStorage2DMultisample(mTarget, sampleCount, glFormat.internalFormat,
                                                   width, height, true);
                    } else {
                        gl.TexStorage2D(mTarget, levels, glFormat.internalFormat, width, height);
                    }
                }
                break;

            case wgpu::TextureDimension::e1D:
            case wgpu::TextureDimension::e3D:
                UNREACHABLE();
        }

        // The texture is not complete if it uses mipmapping and not all levels up to
        // MAX_LEVEL have been defined.
        gl.TexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, levels - 1);

        if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            GetDevice()->ConsumedError(
                ClearTexture(GetAllSubresources(), TextureBase::ClearValue::NonZero));
        }
    }

    Texture::Texture(Device* device,
                     const TextureDescriptor* descriptor,
                     GLuint handle,
                     TextureState state)
        : TextureBase(device, descriptor, state), mHandle(handle) {
        mTarget = TargetForTexture(descriptor);
    }

    Texture::~Texture() {
        DestroyInternal();
    }

    void Texture::DestroyImpl() {
        if (GetTextureState() == TextureState::OwnedInternal) {
            ToBackend(GetDevice())->gl.DeleteTextures(1, &mHandle);
            mHandle = 0;
        }
    }

    GLuint Texture::GetHandle() const {
        return mHandle;
    }

    GLenum Texture::GetGLTarget() const {
        return mTarget;
    }

    const GLFormat& Texture::GetGLFormat() const {
        return ToBackend(GetDevice())->GetGLFormat(GetFormat());
    }

    MaybeError Texture::ClearTexture(const SubresourceRange& range,
                                     TextureBase::ClearValue clearValue) {
        // TODO(jiawei.shao@intel.com): initialize the textures with compressed formats.
        if (GetFormat().isCompressed) {
            return {};
        }

        Device* device = ToBackend(GetDevice());
        const OpenGLFunctions& gl = device->gl;

        uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
        float fClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.f : 1.f;

        if (GetFormat().isRenderable) {
            if ((range.aspects & (Aspect::Depth | Aspect::Stencil)) != 0) {
                GLfloat depth = fClearColor;
                GLint stencil = clearColor;
                if (range.aspects & Aspect::Depth) {
                    gl.DepthMask(GL_TRUE);
                }
                if (range.aspects & Aspect::Stencil) {
                    gl.StencilMask(GetStencilMaskFromStencilFormat(GetFormat().format));
                }

                auto DoClear = [&](Aspect aspects) {
                    if (aspects == (Aspect::Depth | Aspect::Stencil)) {
                        gl.ClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
                    } else if (aspects == Aspect::Depth) {
                        gl.ClearBufferfv(GL_DEPTH, 0, &depth);
                    } else if (aspects == Aspect::Stencil) {
                        gl.ClearBufferiv(GL_STENCIL, 0, &stencil);
                    } else {
                        UNREACHABLE();
                    }
                };

                GLuint framebuffer = 0;
                gl.GenFramebuffers(1, &framebuffer);
                gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    switch (GetDimension()) {
                        case wgpu::TextureDimension::e2D:
                            if (GetArrayLayers() == 1) {
                                Aspect aspectsToClear = Aspect::None;
                                for (Aspect aspect : IterateEnumMask(range.aspects)) {
                                    if (clearValue == TextureBase::ClearValue::Zero &&
                                        IsSubresourceContentInitialized(
                                            SubresourceRange::SingleMipAndLayer(level, 0,
                                                                                aspect))) {
                                        // Skip lazy clears if already initialized.
                                        continue;
                                    }
                                    aspectsToClear |= aspect;
                                }

                                if (aspectsToClear == Aspect::None) {
                                    continue;
                                }

                                gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                                        GL_DEPTH_STENCIL_ATTACHMENT, GetGLTarget(),
                                                        GetHandle(), static_cast<GLint>(level));
                                DoClear(aspectsToClear);
                            } else {
                                for (uint32_t layer = range.baseArrayLayer;
                                     layer < range.baseArrayLayer + range.layerCount; ++layer) {
                                    Aspect aspectsToClear = Aspect::None;
                                    for (Aspect aspect : IterateEnumMask(range.aspects)) {
                                        if (clearValue == TextureBase::ClearValue::Zero &&
                                            IsSubresourceContentInitialized(
                                                SubresourceRange::SingleMipAndLayer(level, layer,
                                                                                    aspect))) {
                                            // Skip lazy clears if already initialized.
                                            continue;
                                        }
                                        aspectsToClear |= aspect;
                                    }

                                    if (aspectsToClear == Aspect::None) {
                                        continue;
                                    }

                                    gl.FramebufferTextureLayer(
                                        GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                        GetHandle(), static_cast<GLint>(level),
                                        static_cast<GLint>(layer));
                                    DoClear(aspectsToClear);
                                }
                            }
                            break;

                        case wgpu::TextureDimension::e1D:
                        case wgpu::TextureDimension::e3D:
                            UNREACHABLE();
                    }
                }

                gl.DeleteFramebuffers(1, &framebuffer);
            } else {
                ASSERT(range.aspects == Aspect::Color);

                static constexpr uint32_t MAX_TEXEL_SIZE = 16;
                const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(Aspect::Color).block;
                ASSERT(blockInfo.byteSize <= MAX_TEXEL_SIZE);

                std::array<GLbyte, MAX_TEXEL_SIZE> clearColorData;
                clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 255;
                clearColorData.fill(clearColor);

                const GLFormat& glFormat = GetGLFormat();
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    Extent3D mipSize = GetMipLevelPhysicalSize(level);
                    for (uint32_t layer = range.baseArrayLayer;
                         layer < range.baseArrayLayer + range.layerCount; ++layer) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, Aspect::Color))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }
                        if (gl.IsAtLeastGL(4, 4)) {
                            gl.ClearTexSubImage(mHandle, static_cast<GLint>(level), 0, 0,
                                                static_cast<GLint>(layer), mipSize.width,
                                                mipSize.height, 1, glFormat.format, glFormat.type,
                                                clearColorData.data());
                        } else {
                            GLuint framebuffer = 0;
                            gl.GenFramebuffers(1, &framebuffer);
                            gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
                            gl.FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR, GetHandle(),
                                                       static_cast<int>(level),
                                                       static_cast<int>(layer));
                            gl.Disable(GL_SCISSOR_TEST);
                            gl.ClearBufferiv(GL_COLOR, 0,
                                             reinterpret_cast<const GLint*>(clearColorData.data()));
                            gl.Enable(GL_SCISSOR_TEST);
                            gl.DeleteFramebuffers(1, &framebuffer);
                        }
                    }
                }
            }
        } else {
            ASSERT(range.aspects == Aspect::Color);

            // create temp buffer with clear color to copy to the texture image
            const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(Aspect::Color).block;
            ASSERT(kTextureBytesPerRowAlignment % blockInfo.byteSize == 0);
            uint32_t bytesPerRow = Align((GetWidth() / blockInfo.width) * blockInfo.byteSize,
                                         kTextureBytesPerRowAlignment);

            // Make sure that we are not rounding
            ASSERT(bytesPerRow % blockInfo.byteSize == 0);
            ASSERT(GetHeight() % blockInfo.height == 0);

            dawn_native::BufferDescriptor descriptor = {};
            descriptor.mappedAtCreation = true;
            descriptor.usage = wgpu::BufferUsage::CopySrc;
            descriptor.size = bytesPerRow * (GetHeight() / blockInfo.height);
            if (descriptor.size > std::numeric_limits<uint32_t>::max()) {
                return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
            }

            // We don't count the lazy clear of srcBuffer because it is an internal buffer.
            // TODO(natlee@microsoft.com): use Dynamic Uploader here for temp buffer
            Ref<Buffer> srcBuffer;
            DAWN_TRY_ASSIGN(srcBuffer, Buffer::CreateInternalBuffer(device, &descriptor, false));

            // Fill the buffer with clear color
            memset(srcBuffer->GetMappedRange(0, descriptor.size), clearColor, descriptor.size);
            srcBuffer->Unmap();

            // Bind buffer and texture, and make the buffer to texture copy
            gl.PixelStorei(GL_UNPACK_ROW_LENGTH,
                           (bytesPerRow / blockInfo.byteSize) * blockInfo.width);
            gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER, srcBuffer->GetHandle());
                gl.ActiveTexture(GL_TEXTURE0);
                gl.BindTexture(GetGLTarget(), GetHandle());

                Extent3D size = GetMipLevelPhysicalSize(level);
                switch (GetDimension()) {
                    case wgpu::TextureDimension::e2D:
                        if (GetArrayLayers() == 1) {
                            if (clearValue == TextureBase::ClearValue::Zero &&
                                IsSubresourceContentInitialized(
                                    SubresourceRange::SingleMipAndLayer(level, 0, Aspect::Color))) {
                                // Skip lazy clears if already initialized.
                                continue;
                            }
                            gl.TexSubImage2D(GetGLTarget(), static_cast<GLint>(level), 0, 0,
                                             size.width, size.height, GetGLFormat().format,
                                             GetGLFormat().type, 0);
                        } else {
                            for (uint32_t layer = range.baseArrayLayer;
                                 layer < range.baseArrayLayer + range.layerCount; ++layer) {
                                if (clearValue == TextureBase::ClearValue::Zero &&
                                    IsSubresourceContentInitialized(
                                        SubresourceRange::SingleMipAndLayer(level, layer,
                                                                            Aspect::Color))) {
                                    // Skip lazy clears if already initialized.
                                    continue;
                                }
                                gl.TexSubImage3D(GetGLTarget(), static_cast<GLint>(level), 0, 0,
                                                 static_cast<GLint>(layer), size.width, size.height,
                                                 1, GetGLFormat().format, GetGLFormat().type, 0);
                            }
                        }
                        break;

                    case wgpu::TextureDimension::e1D:
                    case wgpu::TextureDimension::e3D:
                        UNREACHABLE();
                }
            }
            gl.PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

            gl.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
        if (clearValue == TextureBase::ClearValue::Zero) {
            SetIsSubresourceContentInitialized(true, range);
            device->IncrementLazyClearCountForTesting();
        }
        return {};
    }

    void Texture::EnsureSubresourceContentInitialized(const SubresourceRange& range) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(range)) {
            GetDevice()->ConsumedError(ClearTexture(range, TextureBase::ClearValue::Zero));
        }
    }

    // TextureView

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor), mOwnsHandle(false) {
        mTarget = TargetForTextureViewDimension(descriptor->dimension, descriptor->arrayLayerCount,
                                                texture->GetSampleCount());

        if (!UsageNeedsTextureView(texture->GetUsage())) {
            mHandle = 0;
        } else if (!RequiresCreatingNewTextureView(texture, descriptor)) {
            mHandle = ToBackend(texture)->GetHandle();
        } else {
            // glTextureView() is supported on OpenGL version >= 4.3
            // TODO(jiawei.shao@intel.com): support texture view on OpenGL version <= 4.2
            const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;
            mHandle = GenTexture(gl);
            const Texture* textureGL = ToBackend(texture);
            const GLFormat& glFormat = ToBackend(GetDevice())->GetGLFormat(GetFormat());
            gl.TextureView(mHandle, mTarget, textureGL->GetHandle(), glFormat.internalFormat,
                           descriptor->baseMipLevel, descriptor->mipLevelCount,
                           descriptor->baseArrayLayer, descriptor->arrayLayerCount);
            mOwnsHandle = true;
        }
    }

    TextureView::~TextureView() {
        if (mOwnsHandle) {
            ToBackend(GetDevice())->gl.DeleteTextures(1, &mHandle);
        }
    }

    GLuint TextureView::GetHandle() const {
        ASSERT(mHandle != 0);
        return mHandle;
    }

    GLenum TextureView::GetGLTarget() const {
        return mTarget;
    }

}}  // namespace dawn_native::opengl
