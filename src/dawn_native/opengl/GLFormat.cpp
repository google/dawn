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

        using Type = GLFormat::ComponentType;

        auto AddFormat = [&table](dawn::TextureFormat dawnFormat, GLenum internalFormat,
                                  GLenum format, GLenum type, Type componentType) {
            size_t index = ComputeFormatIndex(dawnFormat);
            ASSERT(index < table.size());

            table[index].internalFormat = internalFormat;
            table[index].format = format;
            table[index].type = type;
            table[index].componentType = componentType;
            table[index].isSupportedOnBackend = true;
        };

        // It's dangerous to go alone, take this:
        //
        //     [ANGLE's formatutils.cpp]
        //     [ANGLE's formatutilsgl.cpp]
        //
        // The format tables in these files are extremely complete and the best reference on GL
        // format support, enums, etc.

        // clang-format off

        // 1 byte color formats
        AddFormat(dawn::TextureFormat::R8Unorm, GL_R8, GL_RED, GL_UNSIGNED_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::R8Snorm, GL_R8_SNORM, GL_RED, GL_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::R8Uint, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
        AddFormat(dawn::TextureFormat::R8Sint, GL_R8I, GL_RED_INTEGER, GL_BYTE, Type::Int);

        // 2 bytes color formats
        AddFormat(dawn::TextureFormat::R16Uint, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
        AddFormat(dawn::TextureFormat::R16Sint, GL_R16I, GL_RED_INTEGER, GL_SHORT, Type::Int);
        AddFormat(dawn::TextureFormat::R16Float, GL_R16F, GL_RED, GL_HALF_FLOAT, Type::Float);
        AddFormat(dawn::TextureFormat::RG8Unorm, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RG8Snorm, GL_RG8_SNORM, GL_RG, GL_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RG8Uint, GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
        AddFormat(dawn::TextureFormat::RG8Sint, GL_RG8I, GL_RG_INTEGER, GL_BYTE, Type::Int);

        // 4 bytes color formats
        AddFormat(dawn::TextureFormat::R32Uint, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, Type::Uint);
        AddFormat(dawn::TextureFormat::R32Sint, GL_R32I, GL_RED_INTEGER, GL_INT, Type::Int);
        AddFormat(dawn::TextureFormat::R32Float, GL_R32F, GL_RED, GL_FLOAT, Type::Float);
        AddFormat(dawn::TextureFormat::RG16Uint, GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
        AddFormat(dawn::TextureFormat::RG16Sint, GL_RG16I, GL_RG_INTEGER, GL_SHORT, Type::Int);
        AddFormat(dawn::TextureFormat::RG16Float, GL_RG16F, GL_RG, GL_HALF_FLOAT, Type::Float);
        AddFormat(dawn::TextureFormat::RGBA8Unorm, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RGBA8UnormSrgb, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RGBA8Snorm, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RGBA8Uint, GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, Type::Uint);
        AddFormat(dawn::TextureFormat::RGBA8Sint, GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, Type::Int);

        // This doesn't have an enum for the internal format in OpenGL, so use RGBA8.
        AddFormat(dawn::TextureFormat::BGRA8Unorm, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE, Type::Float);
        AddFormat(dawn::TextureFormat::RGB10A2Unorm, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, Type::Float);
        AddFormat(dawn::TextureFormat::RG11B10Float, GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, Type::Float);

        // 8 bytes color formats
        AddFormat(dawn::TextureFormat::RG32Uint, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, Type::Uint);
        AddFormat(dawn::TextureFormat::RG32Sint, GL_RG32I, GL_RG_INTEGER, GL_INT, Type::Int);
        AddFormat(dawn::TextureFormat::RG32Float, GL_RG32F, GL_RG, GL_FLOAT, Type::Float);
        AddFormat(dawn::TextureFormat::RGBA16Uint, GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, Type::Uint);
        AddFormat(dawn::TextureFormat::RGBA16Sint, GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, Type::Int);
        AddFormat(dawn::TextureFormat::RGBA16Float, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, Type::Float);

        // 16 bytes color formats
        AddFormat(dawn::TextureFormat::RGBA32Uint, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, Type::Uint);
        AddFormat(dawn::TextureFormat::RGBA32Sint, GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, Type::Int);
        AddFormat(dawn::TextureFormat::RGBA32Float, GL_RGBA32F, GL_RGBA, GL_FLOAT, Type::Float);

        // Depth stencil formats
        AddFormat(dawn::TextureFormat::Depth32Float, GL_DEPTH_COMPONENT32F, GL_DEPTH, GL_FLOAT, Type::DepthStencil);
        AddFormat(dawn::TextureFormat::Depth24Plus, GL_DEPTH_COMPONENT32F, GL_DEPTH, GL_FLOAT, Type::DepthStencil);
        AddFormat(dawn::TextureFormat::Depth24PlusStencil8, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, Type::DepthStencil);

        // clang-format on

        return table;
    }

}}  // namespace dawn_native::opengl
