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

#include "TextureGL.h"

#include <algorithm>
#include <vector>

namespace backend {
namespace opengl {

    namespace {

        GLenum TargetForDimension(nxt::TextureDimension dimension) {
            switch (dimension) {
                case nxt::TextureDimension::e2D:
                    return GL_TEXTURE_2D;
            }
        }

        TextureFormatInfo GetGLFormatInfo(nxt::TextureFormat format) {
            switch (format) {
                case nxt::TextureFormat::R8G8B8A8Unorm:
                    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            }
        }

    }

    // Texture

    Texture::Texture(Device* device, TextureBuilder* builder)
        : TextureBase(builder), device(device) {
        target = TargetForDimension(GetDimension());

        uint32_t width = GetWidth();
        uint32_t height = GetHeight();
        uint32_t levels = GetNumMipLevels();

        auto formatInfo = GetGLFormatInfo(GetFormat());

        glGenTextures(1, &handle);
        glBindTexture(target, handle);

        for (uint32_t i = 0; i < levels; ++i) {
            glTexImage2D(target, i, formatInfo.internalFormat, width, height, 0, formatInfo.format, formatInfo.type, nullptr);
            width = std::max(uint32_t(1), width / 2);
            height = std::max(uint32_t(1), height / 2);
        }

        // The texture is not complete if it uses mipmapping and not all levels up to
        // MAX_LEVEL have been defined.
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1);
    }

    GLuint Texture::GetHandle() const {
        return handle;
    }

    GLenum Texture::GetGLTarget() const {
        return target;
    }

    TextureFormatInfo Texture::GetGLFormat() const {
        return GetGLFormatInfo(GetFormat());
    }

    // TextureView

    TextureView::TextureView(Device* device, TextureViewBuilder* builder)
        : TextureViewBase(builder), device(device) {
    }

}
}
