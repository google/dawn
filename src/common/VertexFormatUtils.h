// Copyright 2021 The Dawn Authors
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

#ifndef DAWNNATIVE_VERTEX_FORMAT_UTILS_H_
#define DAWNNATIVE_VERTEX_FORMAT_UTILS_H_

#include <array>

#include <dawn/webgpu_cpp.h>

namespace dawn {

    static constexpr std::array<wgpu::VertexFormat, 30> kAllVertexFormats = {
        wgpu::VertexFormat::Uint8x2,   wgpu::VertexFormat::Uint8x4,   wgpu::VertexFormat::Sint8x2,
        wgpu::VertexFormat::Sint8x4,   wgpu::VertexFormat::Unorm8x2,  wgpu::VertexFormat::Unorm8x4,
        wgpu::VertexFormat::Snorm8x2,  wgpu::VertexFormat::Snorm8x4,  wgpu::VertexFormat::Uint16x2,
        wgpu::VertexFormat::Uint16x4,  wgpu::VertexFormat::Unorm16x2, wgpu::VertexFormat::Unorm16x4,
        wgpu::VertexFormat::Sint16x2,  wgpu::VertexFormat::Sint16x4,  wgpu::VertexFormat::Snorm16x2,
        wgpu::VertexFormat::Snorm16x4, wgpu::VertexFormat::Float16x2, wgpu::VertexFormat::Float16x4,
        wgpu::VertexFormat::Float32,   wgpu::VertexFormat::Float32x2, wgpu::VertexFormat::Float32x3,
        wgpu::VertexFormat::Float32x4, wgpu::VertexFormat::Uint32,    wgpu::VertexFormat::Uint32x2,
        wgpu::VertexFormat::Uint32x3,  wgpu::VertexFormat::Uint32x4,  wgpu::VertexFormat::Sint32,
        wgpu::VertexFormat::Sint32x2,  wgpu::VertexFormat::Sint32x3,  wgpu::VertexFormat::Sint32x4,
    };

    static constexpr std::array<wgpu::VertexFormat, 30> kAllDeprecatedVertexFormats = {
        wgpu::VertexFormat::UChar2,      wgpu::VertexFormat::UChar4,
        wgpu::VertexFormat::Char2,       wgpu::VertexFormat::Char4,
        wgpu::VertexFormat::UChar2Norm,  wgpu::VertexFormat::UChar4Norm,
        wgpu::VertexFormat::Char2Norm,   wgpu::VertexFormat::Char4Norm,
        wgpu::VertexFormat::UShort2,     wgpu::VertexFormat::UShort4,
        wgpu::VertexFormat::UShort2Norm, wgpu::VertexFormat::UShort4Norm,
        wgpu::VertexFormat::Short2,      wgpu::VertexFormat::Short4,
        wgpu::VertexFormat::Short2Norm,  wgpu::VertexFormat::Short4Norm,
        wgpu::VertexFormat::Half2,       wgpu::VertexFormat::Half4,
        wgpu::VertexFormat::Float,       wgpu::VertexFormat::Float2,
        wgpu::VertexFormat::Float3,      wgpu::VertexFormat::Float4,
        wgpu::VertexFormat::UInt,        wgpu::VertexFormat::UInt2,
        wgpu::VertexFormat::UInt3,       wgpu::VertexFormat::UInt4,
        wgpu::VertexFormat::Int,         wgpu::VertexFormat::Int2,
        wgpu::VertexFormat::Int3,        wgpu::VertexFormat::Int4,
    };

    bool IsDeprecatedVertexFormat(wgpu::VertexFormat format);
    wgpu::VertexFormat NormalizeVertexFormat(wgpu::VertexFormat format);
    uint32_t VertexFormatNumComponents(wgpu::VertexFormat format);
    size_t VertexFormatComponentSize(wgpu::VertexFormat format);
    size_t VertexFormatSize(wgpu::VertexFormat format);

    const char* GetWGSLVertexFormatType(wgpu::VertexFormat textureFormat);

}  // namespace dawn

#endif