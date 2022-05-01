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

#include <array>

#include "dawn/native/VertexFormat.h"

#include "dawn/common/Assert.h"

namespace dawn::native {

static constexpr std::array<VertexFormatInfo, 31> sVertexFormatTable = {{
    //
    {wgpu::VertexFormat::Undefined, 0, 0, 0, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Uint8x2, 2, 2, 1, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint8x4, 4, 4, 1, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint8x2, 2, 2, 1, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint8x4, 4, 4, 1, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Unorm8x2, 2, 2, 1, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Unorm8x4, 4, 4, 1, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm8x2, 2, 2, 1, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm8x4, 4, 4, 1, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Uint16x2, 4, 2, 2, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint16x4, 8, 4, 2, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint16x2, 4, 2, 2, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint16x4, 8, 4, 2, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Unorm16x2, 4, 2, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Unorm16x4, 8, 4, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm16x2, 4, 2, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Snorm16x4, 8, 4, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float16x2, 4, 2, 2, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float16x4, 8, 4, 2, VertexFormatBaseType::Float},

    {wgpu::VertexFormat::Float32, 4, 1, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x2, 8, 2, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x3, 12, 3, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Float32x4, 16, 4, 4, VertexFormatBaseType::Float},
    {wgpu::VertexFormat::Uint32, 4, 1, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x2, 8, 2, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x3, 12, 3, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Uint32x4, 16, 4, 4, VertexFormatBaseType::Uint},
    {wgpu::VertexFormat::Sint32, 4, 1, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x2, 8, 2, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x3, 12, 3, 4, VertexFormatBaseType::Sint},
    {wgpu::VertexFormat::Sint32x4, 16, 4, 4, VertexFormatBaseType::Sint},
    //
}};

const VertexFormatInfo& GetVertexFormatInfo(wgpu::VertexFormat format) {
    ASSERT(format != wgpu::VertexFormat::Undefined);
    ASSERT(static_cast<uint32_t>(format) < sVertexFormatTable.size());
    ASSERT(sVertexFormatTable[static_cast<uint32_t>(format)].format == format);
    return sVertexFormatTable[static_cast<uint32_t>(format)];
}

}  // namespace dawn::native
