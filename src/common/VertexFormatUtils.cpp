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

#include "VertexFormatUtils.h"

#include "Assert.h"

namespace dawn {

    namespace {

        static constexpr wgpu::VertexFormat kFirstDeprecatedVertexFormat =
            wgpu::VertexFormat::UChar2;
        static constexpr uint32_t kFirstDeprecatedVertexFormatValue =
            static_cast<uint32_t>(kFirstDeprecatedVertexFormat);

    }  // namespace

    bool IsDeprecatedVertexFormat(wgpu::VertexFormat format) {
        return format >= kFirstDeprecatedVertexFormat;
    }

    wgpu::VertexFormat NormalizeVertexFormat(wgpu::VertexFormat format) {
        // If the specified format is from the deprecated range return the corresponding
        // non-deprecated format.
        if (format >= kFirstDeprecatedVertexFormat) {
            uint32_t formatValue = static_cast<uint32_t>(format);
            // Need to add one to account for the "undefined" enum with value 0
            return static_cast<wgpu::VertexFormat>(
                (formatValue - kFirstDeprecatedVertexFormatValue) + 1);
        }
        return format;
    }

    uint32_t VertexFormatNumComponents(wgpu::VertexFormat format) {
        format = NormalizeVertexFormat(format);
        switch (format) {
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Sint8x4:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Snorm8x4:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Sint16x4:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Float16x4:
            case wgpu::VertexFormat::Float32x4:
            case wgpu::VertexFormat::Uint32x4:
            case wgpu::VertexFormat::Sint32x4:
                return 4;
            case wgpu::VertexFormat::Float32x3:
            case wgpu::VertexFormat::Uint32x3:
            case wgpu::VertexFormat::Sint32x3:
                return 3;
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Sint8x2:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Sint16x2:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float32x2:
            case wgpu::VertexFormat::Uint32x2:
            case wgpu::VertexFormat::Sint32x2:
                return 2;
            case wgpu::VertexFormat::Float32:
            case wgpu::VertexFormat::Uint32:
            case wgpu::VertexFormat::Sint32:
                return 1;

            case wgpu::VertexFormat::Undefined:
                break;

            // Deprecated formats (should be unreachable after NormalizeVertexFormat call)
            case wgpu::VertexFormat::UChar2:
            case wgpu::VertexFormat::UChar4:
            case wgpu::VertexFormat::Char2:
            case wgpu::VertexFormat::Char4:
            case wgpu::VertexFormat::UChar2Norm:
            case wgpu::VertexFormat::UChar4Norm:
            case wgpu::VertexFormat::Char2Norm:
            case wgpu::VertexFormat::Char4Norm:
            case wgpu::VertexFormat::UShort2:
            case wgpu::VertexFormat::UShort4:
            case wgpu::VertexFormat::UShort2Norm:
            case wgpu::VertexFormat::UShort4Norm:
            case wgpu::VertexFormat::Short2:
            case wgpu::VertexFormat::Short4:
            case wgpu::VertexFormat::Short2Norm:
            case wgpu::VertexFormat::Short4Norm:
            case wgpu::VertexFormat::Half2:
            case wgpu::VertexFormat::Half4:
            case wgpu::VertexFormat::Float:
            case wgpu::VertexFormat::Float2:
            case wgpu::VertexFormat::Float3:
            case wgpu::VertexFormat::Float4:
            case wgpu::VertexFormat::UInt:
            case wgpu::VertexFormat::UInt2:
            case wgpu::VertexFormat::UInt3:
            case wgpu::VertexFormat::UInt4:
            case wgpu::VertexFormat::Int:
            case wgpu::VertexFormat::Int2:
            case wgpu::VertexFormat::Int3:
            case wgpu::VertexFormat::Int4:
                break;
        }
        UNREACHABLE();
    }

    size_t VertexFormatComponentSize(wgpu::VertexFormat format) {
        format = NormalizeVertexFormat(format);
        switch (format) {
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Sint8x2:
            case wgpu::VertexFormat::Sint8x4:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Snorm8x4:
                return sizeof(char);
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Sint16x2:
            case wgpu::VertexFormat::Sint16x4:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float16x4:
                return sizeof(uint16_t);
            case wgpu::VertexFormat::Float32:
            case wgpu::VertexFormat::Float32x2:
            case wgpu::VertexFormat::Float32x3:
            case wgpu::VertexFormat::Float32x4:
                return sizeof(float);
            case wgpu::VertexFormat::Uint32:
            case wgpu::VertexFormat::Uint32x2:
            case wgpu::VertexFormat::Uint32x3:
            case wgpu::VertexFormat::Uint32x4:
            case wgpu::VertexFormat::Sint32:
            case wgpu::VertexFormat::Sint32x2:
            case wgpu::VertexFormat::Sint32x3:
            case wgpu::VertexFormat::Sint32x4:
                return sizeof(int32_t);

            case wgpu::VertexFormat::Undefined:
                break;

            // Deprecated formats (should be unreachable after NormalizeVertexFormat call)
            case wgpu::VertexFormat::UChar2:
            case wgpu::VertexFormat::UChar4:
            case wgpu::VertexFormat::Char2:
            case wgpu::VertexFormat::Char4:
            case wgpu::VertexFormat::UChar2Norm:
            case wgpu::VertexFormat::UChar4Norm:
            case wgpu::VertexFormat::Char2Norm:
            case wgpu::VertexFormat::Char4Norm:
            case wgpu::VertexFormat::UShort2:
            case wgpu::VertexFormat::UShort4:
            case wgpu::VertexFormat::UShort2Norm:
            case wgpu::VertexFormat::UShort4Norm:
            case wgpu::VertexFormat::Short2:
            case wgpu::VertexFormat::Short4:
            case wgpu::VertexFormat::Short2Norm:
            case wgpu::VertexFormat::Short4Norm:
            case wgpu::VertexFormat::Half2:
            case wgpu::VertexFormat::Half4:
            case wgpu::VertexFormat::Float:
            case wgpu::VertexFormat::Float2:
            case wgpu::VertexFormat::Float3:
            case wgpu::VertexFormat::Float4:
            case wgpu::VertexFormat::UInt:
            case wgpu::VertexFormat::UInt2:
            case wgpu::VertexFormat::UInt3:
            case wgpu::VertexFormat::UInt4:
            case wgpu::VertexFormat::Int:
            case wgpu::VertexFormat::Int2:
            case wgpu::VertexFormat::Int3:
            case wgpu::VertexFormat::Int4:
                break;
        }
        UNREACHABLE();
    }

    size_t VertexFormatSize(wgpu::VertexFormat format) {
        return VertexFormatNumComponents(format) * VertexFormatComponentSize(format);
    }

    const char* GetWGSLVertexFormatType(wgpu::VertexFormat format) {
        format = NormalizeVertexFormat(format);
        switch (format) {
            case wgpu::VertexFormat::Float32:
                return "f32";
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float32x2:
                return "vec2<f32>";
            case wgpu::VertexFormat::Float32x3:
                return "vec3<f32>";
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Snorm8x4:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Float16x4:
            case wgpu::VertexFormat::Float32x4:
                return "vec4<f32>";
            case wgpu::VertexFormat::Uint32:
                return "u32";
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Uint32x2:
                return "vec2<u32>";
            case wgpu::VertexFormat::Uint32x3:
                return "vec3<u32>";
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Uint32x4:
                return "vec4<u32>";
            case wgpu::VertexFormat::Sint32:
                return "i32";
            case wgpu::VertexFormat::Sint8x2:
            case wgpu::VertexFormat::Sint16x2:
            case wgpu::VertexFormat::Sint32x2:
                return "vec2<i32>";
            case wgpu::VertexFormat::Sint32x3:
                return "vec3<i32>";
            case wgpu::VertexFormat::Sint8x4:
            case wgpu::VertexFormat::Sint16x4:
            case wgpu::VertexFormat::Sint32x4:
                return "vec4<i32>";

            case wgpu::VertexFormat::Undefined:
                break;

            // Deprecated formats (should be unreachable after NormalizeVertexFormat call)
            case wgpu::VertexFormat::UChar2:
            case wgpu::VertexFormat::UChar4:
            case wgpu::VertexFormat::Char2:
            case wgpu::VertexFormat::Char4:
            case wgpu::VertexFormat::UChar2Norm:
            case wgpu::VertexFormat::UChar4Norm:
            case wgpu::VertexFormat::Char2Norm:
            case wgpu::VertexFormat::Char4Norm:
            case wgpu::VertexFormat::UShort2:
            case wgpu::VertexFormat::UShort4:
            case wgpu::VertexFormat::UShort2Norm:
            case wgpu::VertexFormat::UShort4Norm:
            case wgpu::VertexFormat::Short2:
            case wgpu::VertexFormat::Short4:
            case wgpu::VertexFormat::Short2Norm:
            case wgpu::VertexFormat::Short4Norm:
            case wgpu::VertexFormat::Half2:
            case wgpu::VertexFormat::Half4:
            case wgpu::VertexFormat::Float:
            case wgpu::VertexFormat::Float2:
            case wgpu::VertexFormat::Float3:
            case wgpu::VertexFormat::Float4:
            case wgpu::VertexFormat::UInt:
            case wgpu::VertexFormat::UInt2:
            case wgpu::VertexFormat::UInt3:
            case wgpu::VertexFormat::UInt4:
            case wgpu::VertexFormat::Int:
            case wgpu::VertexFormat::Int2:
            case wgpu::VertexFormat::Int3:
            case wgpu::VertexFormat::Int4:
                break;
        }
        UNREACHABLE();
    }

}  // namespace dawn
