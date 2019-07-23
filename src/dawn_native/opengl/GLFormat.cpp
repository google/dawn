// Copyright 2019 The Dawn Authors
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

#include "dawn_native/opengl/GLFormat.h"

namespace dawn_native { namespace opengl {

    GLFormatTable BuildGLFormatTable() {
        GLFormatTable table;

        auto AddFormat = [&table](dawn::TextureFormat dawnFormat, GLenum internalFormat,
                                  GLenum format, GLenum type) {
            size_t index = ComputeFormatIndex(dawnFormat);
            ASSERT(index < table.size());

            table[index].internalFormat = internalFormat;
            table[index].format = format;
            table[index].type = type;
            table[index].isSupportedOnBackend = true;
        };

        AddFormat(dawn::TextureFormat::RGBA8Unorm, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        AddFormat(dawn::TextureFormat::RG8Unorm, GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
        AddFormat(dawn::TextureFormat::R8Unorm, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
        AddFormat(dawn::TextureFormat::RGBA8Uint, GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_INT);
        AddFormat(dawn::TextureFormat::RG8Uint, GL_RG8UI, GL_RG, GL_UNSIGNED_INT);
        AddFormat(dawn::TextureFormat::R8Uint, GL_R8UI, GL_RED, GL_UNSIGNED_INT);
        // This doesn't have an enum for the internal format in OpenGL, so use RGBA8.
        AddFormat(dawn::TextureFormat::BGRA8Unorm, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE);

        AddFormat(dawn::TextureFormat::Depth24PlusStencil8, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL,
                  GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

        return table;
    }

}}  // namespace dawn_native::opengl
