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
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/UtilsGL.h"

namespace dawn_native { namespace opengl {

    namespace {

        GLenum TargetForTexture(const TextureDescriptor* descriptor) {
            switch (descriptor->dimension) {
                case dawn::TextureDimension::e2D:
                    if (descriptor->arrayLayerCount > 1) {
                        ASSERT(descriptor->sampleCount == 1);
                        return GL_TEXTURE_2D_ARRAY;
                    } else {
                        if (descriptor->sampleCount > 1) {
                            return GL_TEXTURE_2D_MULTISAMPLE;
                        } else {
                            return GL_TEXTURE_2D;
                        }
                    }

                default:
                    UNREACHABLE();
                    return GL_TEXTURE_2D;
            }
        }

        GLenum TargetForTextureViewDimension(dawn::TextureViewDimension dimension,
                                             uint32_t sampleCount) {
            switch (dimension) {
                case dawn::TextureViewDimension::e2D:
                    return (sampleCount > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
                case dawn::TextureViewDimension::e2DArray:
                    ASSERT(sampleCount == 1);
                    return GL_TEXTURE_2D_ARRAY;
                case dawn::TextureViewDimension::Cube:
                    return GL_TEXTURE_CUBE_MAP;
                case dawn::TextureViewDimension::CubeArray:
                    return GL_TEXTURE_CUBE_MAP_ARRAY;
                default:
                    UNREACHABLE();
                    return GL_TEXTURE_2D;
            }
        }

        GLuint GenTexture(const OpenGLFunctions& gl) {
            GLuint handle = 0;
            gl.GenTextures(1, &handle);
            return handle;
        }

        bool UsageNeedsTextureView(dawn::TextureUsageBit usage) {
            constexpr dawn::TextureUsageBit kUsageNeedingTextureView =
                dawn::TextureUsageBit::Storage | dawn::TextureUsageBit::Sampled;
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

            switch (textureViewDescriptor->dimension) {
                case dawn::TextureViewDimension::Cube:
                case dawn::TextureViewDimension::CubeArray:
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

        uint32_t width = GetSize().width;
        uint32_t height = GetSize().height;
        uint32_t levels = GetNumMipLevels();
        uint32_t arrayLayers = GetArrayLayers();
        uint32_t sampleCount = GetSampleCount();

        const GLFormat& glFormat = GetGLFormat();

        gl.BindTexture(mTarget, mHandle);

        // glTextureView() requires the value of GL_TEXTURE_IMMUTABLE_FORMAT for origtexture to be
        // GL_TRUE, so the storage of the texture must be allocated with glTexStorage*D.
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTextureView.xhtml
        switch (GetDimension()) {
            case dawn::TextureDimension::e2D:
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
            default:
                UNREACHABLE();
        }

        // The texture is not complete if it uses mipmapping and not all levels up to
        // MAX_LEVEL have been defined.
        gl.TexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, levels - 1);

        if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            static constexpr uint32_t MAX_TEXEL_SIZE = 16;
            ASSERT(GetFormat().blockByteSize <= MAX_TEXEL_SIZE);
            GLubyte clearColor[MAX_TEXEL_SIZE];
            std::fill(clearColor, clearColor + MAX_TEXEL_SIZE, 255);
            // TODO(natlee@microsoft.com): clear all subresources
            for (uint32_t i = 0; i < GetNumMipLevels(); i++) {
                gl.ClearTexImage(mHandle, i, glFormat.format, glFormat.type, clearColor);
            }
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
        ToBackend(GetDevice())->gl.DeleteTextures(1, &mHandle);
        mHandle = 0;
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

    void Texture::ClearTexture(GLint baseMipLevel,
                               GLint levelCount,
                               GLint baseArrayLayer,
                               uint32_t layerCount) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;
        // TODO(jiawei.shao@intel.com): initialize the textures with compressed formats.
        if (GetFormat().isCompressed) {
            return;
        }

        if (GetFormat().HasDepthOrStencil()) {
            bool doDepthClear = GetFormat().HasDepth();
            bool doStencilClear = GetFormat().HasStencil();
            GLfloat depth = 0.0f;
            GLint stencil = 0u;
            if (doDepthClear) {
                gl.DepthMask(GL_TRUE);
            }
            if (doStencilClear) {
                gl.StencilMask(GetStencilMaskFromStencilFormat(GetFormat().format));
            }

            GLuint framebuffer = 0;
            gl.GenFramebuffers(1, &framebuffer);
            gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
            gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GetGLTarget(),
                                    GetHandle(), 0);
            if (doDepthClear && doStencilClear) {
                gl.ClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
            } else if (doDepthClear) {
                gl.ClearBufferfv(GL_DEPTH, 0, &depth);
            } else if (doStencilClear) {
                gl.ClearBufferiv(GL_STENCIL, 0, &stencil);
            }
            gl.DeleteFramebuffers(1, &framebuffer);
        } else {
            const GLFormat& glFormat = GetGLFormat();
            for (GLint level = baseMipLevel; level < baseMipLevel + levelCount; ++level) {
                Extent3D mipSize = GetMipLevelPhysicalSize(level);
                gl.ClearTexSubImage(mHandle, level, 0, 0, baseArrayLayer, mipSize.width,
                                    mipSize.height, layerCount, glFormat.format, glFormat.type,
                                    nullptr);
            }
        }
    }

    void Texture::EnsureSubresourceContentInitialized(uint32_t baseMipLevel,
                                                      uint32_t levelCount,
                                                      uint32_t baseArrayLayer,
                                                      uint32_t layerCount,
                                                      bool isLazyClear) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer,
                                             layerCount)) {
            ClearTexture(baseMipLevel, levelCount, baseArrayLayer, layerCount);
            if (isLazyClear) {
                GetDevice()->IncrementLazyClearCountForTesting();
            }
            SetIsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer,
                                               layerCount);
        }
    }

    // TextureView

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor), mOwnsHandle(false) {
        mTarget = TargetForTextureViewDimension(descriptor->dimension, texture->GetSampleCount());

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
