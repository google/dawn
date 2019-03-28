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
#include "dawn_native/opengl/DeviceGL.h"

#include "common/Assert.h"

#include <algorithm>
#include <vector>

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

        TextureFormatInfo GetGLFormatInfo(dawn::TextureFormat format) {
            switch (format) {
                case dawn::TextureFormat::R8G8B8A8Unorm:
                    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
                case dawn::TextureFormat::R8G8Unorm:
                    return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE};
                case dawn::TextureFormat::R8Unorm:
                    return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};
                case dawn::TextureFormat::R8G8B8A8Uint:
                    return {GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_INT};
                case dawn::TextureFormat::R8G8Uint:
                    return {GL_RG8UI, GL_RG, GL_UNSIGNED_INT};
                case dawn::TextureFormat::R8Uint:
                    return {GL_R8UI, GL_RED, GL_UNSIGNED_INT};
                case dawn::TextureFormat::B8G8R8A8Unorm:
                    // This doesn't have an enum for the internal format in OpenGL, so use RGBA8.
                    return {GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE};
                case dawn::TextureFormat::D32FloatS8Uint:
                    return {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL,
                            GL_FLOAT_32_UNSIGNED_INT_24_8_REV};
                default:
                    UNREACHABLE();
                    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            }
        }

        GLuint GenTexture() {
            GLuint handle = 0;
            glGenTextures(1, &handle);
            return handle;
        }

        bool UsageNeedsTextureView(dawn::TextureUsageBit usage) {
            constexpr dawn::TextureUsageBit kUsageNeedingTextureView =
                dawn::TextureUsageBit::Storage | dawn::TextureUsageBit::Sampled;
            return usage & kUsageNeedingTextureView;
        }

        bool RequiresCreatingNewTextureView(const TextureBase* texture,
                                            const TextureViewDescriptor* textureViewDescriptor) {
            if (texture->GetFormat() != textureViewDescriptor->format) {
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
        : Texture(device, descriptor, GenTexture(), TextureState::OwnedInternal) {
        uint32_t width = GetSize().width;
        uint32_t height = GetSize().height;
        uint32_t levels = GetNumMipLevels();
        uint32_t arrayLayers = GetArrayLayers();
        uint32_t sampleCount = GetSampleCount();

        auto formatInfo = GetGLFormatInfo(GetFormat());

        glBindTexture(mTarget, mHandle);

        // glTextureView() requires the value of GL_TEXTURE_IMMUTABLE_FORMAT for origtexture to be
        // GL_TRUE, so the storage of the texture must be allocated with glTexStorage*D.
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTextureView.xhtml
        switch (GetDimension()) {
            case dawn::TextureDimension::e2D:
                if (arrayLayers > 1) {
                    ASSERT(!IsMultisampledTexture());
                    glTexStorage3D(mTarget, levels, formatInfo.internalFormat, width, height,
                                   arrayLayers);
                } else {
                    if (IsMultisampledTexture()) {
                        glTexStorage2DMultisample(mTarget, sampleCount, formatInfo.internalFormat,
                                                  width, height, true);
                    } else {
                        glTexStorage2D(mTarget, levels, formatInfo.internalFormat, width, height);
                    }
                }
                break;
            default:
                UNREACHABLE();
        }

        // The texture is not complete if it uses mipmapping and not all levels up to
        // MAX_LEVEL have been defined.
        glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, levels - 1);
    }

    Texture::Texture(Device* device,
                     const TextureDescriptor* descriptor,
                     GLuint handle,
                     TextureState state)
        : TextureBase(device, descriptor, state), mHandle(handle) {
        mTarget = TargetForTexture(descriptor);
    }

    Texture::~Texture() {
        Destroy();
    }

    void Texture::DestroyImpl() {
        glDeleteTextures(1, &mHandle);
        mHandle = 0;
    }

    GLuint Texture::GetHandle() const {
        return mHandle;
    }

    GLenum Texture::GetGLTarget() const {
        return mTarget;
    }

    TextureFormatInfo Texture::GetGLFormat() const {
        return GetGLFormatInfo(GetFormat());
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
            mHandle = GenTexture();
            const Texture* textureGL = ToBackend(texture);
            TextureFormatInfo textureViewFormat = GetGLFormatInfo(descriptor->format);
            glTextureView(mHandle, mTarget, textureGL->GetHandle(),
                          textureViewFormat.internalFormat, descriptor->baseMipLevel,
                          descriptor->mipLevelCount, descriptor->baseArrayLayer,
                          descriptor->arrayLayerCount);
            mOwnsHandle = true;
        }
    }

    TextureView::~TextureView() {
        if (mOwnsHandle) {
            glDeleteTextures(1, &mHandle);
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
