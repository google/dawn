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

        GLenum TargetForDimensionAndArrayLayers(dawn::TextureDimension dimension,
                                                uint32_t arrayLayer) {
            switch (dimension) {
                case dawn::TextureDimension::e2D:
                    return (arrayLayer > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
                default:
                    UNREACHABLE();
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
            }
        }

        GLuint GenTexture() {
            GLuint handle = 0;
            glGenTextures(1, &handle);
            return handle;
        }

    }  // namespace

    // Texture

    Texture::Texture(Device* device, const TextureDescriptor* descriptor)
        : Texture(device, descriptor, GenTexture()) {
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor, GLuint handle)
        : TextureBase(device, descriptor), mHandle(handle) {
        mTarget = TargetForDimensionAndArrayLayers(GetDimension(), GetArrayLayers());

        uint32_t width = GetWidth();
        uint32_t height = GetHeight();
        uint32_t levels = GetNumMipLevels();
        uint32_t arrayLayers = GetArrayLayers();

        auto formatInfo = GetGLFormatInfo(GetFormat());

        glBindTexture(mTarget, handle);

        for (uint32_t i = 0; i < levels; ++i) {
            switch (GetDimension()) {
                case dawn::TextureDimension::e2D:
                    if (arrayLayers > 1) {
                        glTexImage3D(mTarget, i, formatInfo.internalFormat, width, height,
                                     arrayLayers, 0, formatInfo.format, formatInfo.type, nullptr);
                    } else {
                        glTexImage2D(mTarget, i, formatInfo.internalFormat, width, height, 0,
                                     formatInfo.format, formatInfo.type, nullptr);
                    }
                    break;
                default:
                    UNREACHABLE();
            }

            width = std::max(uint32_t(1), width / 2);
            height = std::max(uint32_t(1), height / 2);
        }

        // The texture is not complete if it uses mipmapping and not all levels up to
        // MAX_LEVEL have been defined.
        glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, levels - 1);
    }

    Texture::~Texture() {
        // TODO(kainino@chromium.org): delete texture (but only when not using the native texture
        // constructor?)
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

    TextureView::TextureView(TextureBase* texture) : TextureViewBase(texture) {
    }

}}  // namespace dawn_native::opengl
