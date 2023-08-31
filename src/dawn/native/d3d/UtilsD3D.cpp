// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d/UtilsD3D.h"

#include <utility>

namespace dawn::native::d3d {

ResultOrError<std::wstring> ConvertStringToWstring(std::string_view s) {
    size_t len = s.length();
    if (len == 0) {
        return std::wstring();
    }
    int numChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, nullptr, 0);
    if (numChars == 0) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    std::wstring result;
    result.resize(numChars);
    int numConvertedChars =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, &result[0], numChars);
    if (numConvertedChars != numChars) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    return std::move(result);
}

bool IsTypeless(DXGI_FORMAT format) {
    // List generated from <dxgiformat.h>
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
            return true;
        default:
            return false;
    }
}

bool IsDepthStencil(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return true;
        default:
            return false;
    }
}

uint64_t MakeDXCVersion(uint64_t majorVersion, uint64_t minorVersion) {
    return (majorVersion << 32) + minorVersion;
}

DXGI_FORMAT DXGITypelessTextureFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
            return DXGI_FORMAT_R8_TYPELESS;

        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::Depth16Unorm:
            return DXGI_FORMAT_R16_TYPELESS;

        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
            return DXGI_FORMAT_R8G8_TYPELESS;

        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::R32Float:
            return DXGI_FORMAT_R32_TYPELESS;

        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
            return DXGI_FORMAT_R16G16_TYPELESS;

        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;

        case wgpu::TextureFormat::RGB10A2Unorm:
            return DXGI_FORMAT_R10G10B10A2_TYPELESS;

        case wgpu::TextureFormat::RG11B10Ufloat:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RG32Float:
            return DXGI_FORMAT_R32G32_TYPELESS;

        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::RGBA16Snorm:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;

        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::RGBA32Float:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;

        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
            return DXGI_FORMAT_R32_TYPELESS;

        // DXGI_FORMAT_D24_UNORM_S8_UINT is the smallest format supported on D3D12 that has stencil,
        // for which the typeless equivalent is DXGI_FORMAT_R24G8_TYPELESS.
        case wgpu::TextureFormat::Stencil8:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_R32G8X24_TYPELESS;

        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            return DXGI_FORMAT_BC1_TYPELESS;

        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            return DXGI_FORMAT_BC2_TYPELESS;

        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            return DXGI_FORMAT_BC3_TYPELESS;

        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC4RUnorm:
            return DXGI_FORMAT_BC4_TYPELESS;

        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC5RGUnorm:
            return DXGI_FORMAT_BC5_TYPELESS;

        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC6HRGBUfloat:
            return DXGI_FORMAT_BC6H_TYPELESS;

        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return DXGI_FORMAT_BC7_TYPELESS;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:

        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::Undefined:
            UNREACHABLE();
    }
}

#define UNCOMPRESSED_COLOR_FORMATS(X)                                       \
    X(wgpu::TextureFormat::R8Unorm, DXGI_FORMAT_R8_UNORM)                   \
    X(wgpu::TextureFormat::R8Snorm, DXGI_FORMAT_R8_SNORM)                   \
    X(wgpu::TextureFormat::R8Uint, DXGI_FORMAT_R8_UINT)                     \
    X(wgpu::TextureFormat::R8Sint, DXGI_FORMAT_R8_SINT)                     \
                                                                            \
    X(wgpu::TextureFormat::R16Unorm, DXGI_FORMAT_R16_UNORM)                 \
    X(wgpu::TextureFormat::R16Snorm, DXGI_FORMAT_R16_SNORM)                 \
    X(wgpu::TextureFormat::R16Uint, DXGI_FORMAT_R16_UINT)                   \
    X(wgpu::TextureFormat::R16Sint, DXGI_FORMAT_R16_SINT)                   \
    X(wgpu::TextureFormat::R16Float, DXGI_FORMAT_R16_FLOAT)                 \
    X(wgpu::TextureFormat::RG8Unorm, DXGI_FORMAT_R8G8_UNORM)                \
    X(wgpu::TextureFormat::RG8Snorm, DXGI_FORMAT_R8G8_SNORM)                \
    X(wgpu::TextureFormat::RG8Uint, DXGI_FORMAT_R8G8_UINT)                  \
    X(wgpu::TextureFormat::RG8Sint, DXGI_FORMAT_R8G8_SINT)                  \
                                                                            \
    X(wgpu::TextureFormat::R32Uint, DXGI_FORMAT_R32_UINT)                   \
    X(wgpu::TextureFormat::R32Sint, DXGI_FORMAT_R32_SINT)                   \
    X(wgpu::TextureFormat::R32Float, DXGI_FORMAT_R32_FLOAT)                 \
    X(wgpu::TextureFormat::RG16Unorm, DXGI_FORMAT_R16G16_UNORM)             \
    X(wgpu::TextureFormat::RG16Snorm, DXGI_FORMAT_R16G16_SNORM)             \
    X(wgpu::TextureFormat::RG16Uint, DXGI_FORMAT_R16G16_UINT)               \
    X(wgpu::TextureFormat::RG16Sint, DXGI_FORMAT_R16G16_SINT)               \
    X(wgpu::TextureFormat::RG16Float, DXGI_FORMAT_R16G16_FLOAT)             \
    X(wgpu::TextureFormat::RGBA8Unorm, DXGI_FORMAT_R8G8B8A8_UNORM)          \
    X(wgpu::TextureFormat::RGBA8UnormSrgb, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) \
    X(wgpu::TextureFormat::RGBA8Snorm, DXGI_FORMAT_R8G8B8A8_SNORM)          \
    X(wgpu::TextureFormat::RGBA8Uint, DXGI_FORMAT_R8G8B8A8_UINT)            \
    X(wgpu::TextureFormat::RGBA8Sint, DXGI_FORMAT_R8G8B8A8_SINT)            \
    X(wgpu::TextureFormat::BGRA8Unorm, DXGI_FORMAT_B8G8R8A8_UNORM)          \
    X(wgpu::TextureFormat::BGRA8UnormSrgb, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) \
    X(wgpu::TextureFormat::RGB10A2Unorm, DXGI_FORMAT_R10G10B10A2_UNORM)     \
    X(wgpu::TextureFormat::RG11B10Ufloat, DXGI_FORMAT_R11G11B10_FLOAT)      \
    X(wgpu::TextureFormat::RGB9E5Ufloat, DXGI_FORMAT_R9G9B9E5_SHAREDEXP)    \
                                                                            \
    X(wgpu::TextureFormat::RG32Uint, DXGI_FORMAT_R32G32_UINT)               \
    X(wgpu::TextureFormat::RG32Sint, DXGI_FORMAT_R32G32_SINT)               \
    X(wgpu::TextureFormat::RG32Float, DXGI_FORMAT_R32G32_FLOAT)             \
    X(wgpu::TextureFormat::RGBA16Unorm, DXGI_FORMAT_R16G16B16A16_UNORM)     \
    X(wgpu::TextureFormat::RGBA16Snorm, DXGI_FORMAT_R16G16B16A16_SNORM)     \
    X(wgpu::TextureFormat::RGBA16Uint, DXGI_FORMAT_R16G16B16A16_UINT)       \
    X(wgpu::TextureFormat::RGBA16Sint, DXGI_FORMAT_R16G16B16A16_SINT)       \
    X(wgpu::TextureFormat::RGBA16Float, DXGI_FORMAT_R16G16B16A16_FLOAT)     \
                                                                            \
    X(wgpu::TextureFormat::RGBA32Uint, DXGI_FORMAT_R32G32B32A32_UINT)       \
    X(wgpu::TextureFormat::RGBA32Sint, DXGI_FORMAT_R32G32B32A32_SINT)       \
    X(wgpu::TextureFormat::RGBA32Float, DXGI_FORMAT_R32G32B32A32_FLOAT)     \
                                                                            \
    X(wgpu::TextureFormat::R8BG8Biplanar420Unorm, DXGI_FORMAT_NV12)         \
    X(wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm, DXGI_FORMAT_P010)

DXGI_FORMAT DXGITextureFormat(wgpu::TextureFormat format) {
    switch (format) {
#define X(wgpuFormat, dxgiFormat) \
    case wgpuFormat:              \
        return dxgiFormat;
        UNCOMPRESSED_COLOR_FORMATS(X)
#undef X

        case wgpu::TextureFormat::Depth16Unorm:
            return DXGI_FORMAT_D16_UNORM;
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
            return DXGI_FORMAT_D32_FLOAT;
        // DXGI_FORMAT_D24_UNORM_S8_UINT is the smallest format supported on D3D12 that has stencil.
        case wgpu::TextureFormat::Stencil8:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case wgpu::TextureFormat::BC1RGBAUnorm:
            return DXGI_FORMAT_BC1_UNORM;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case wgpu::TextureFormat::BC2RGBAUnorm:
            return DXGI_FORMAT_BC2_UNORM;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case wgpu::TextureFormat::BC3RGBAUnorm:
            return DXGI_FORMAT_BC3_UNORM;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            return DXGI_FORMAT_BC3_UNORM_SRGB;
        case wgpu::TextureFormat::BC4RSnorm:
            return DXGI_FORMAT_BC4_SNORM;
        case wgpu::TextureFormat::BC4RUnorm:
            return DXGI_FORMAT_BC4_UNORM;
        case wgpu::TextureFormat::BC5RGSnorm:
            return DXGI_FORMAT_BC5_SNORM;
        case wgpu::TextureFormat::BC5RGUnorm:
            return DXGI_FORMAT_BC5_UNORM;
        case wgpu::TextureFormat::BC6HRGBFloat:
            return DXGI_FORMAT_BC6H_SF16;
        case wgpu::TextureFormat::BC6HRGBUfloat:
            return DXGI_FORMAT_BC6H_UF16;
        case wgpu::TextureFormat::BC7RGBAUnorm:
            return DXGI_FORMAT_BC7_UNORM;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return DXGI_FORMAT_BC7_UNORM_SRGB;

        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:

        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:

        case wgpu::TextureFormat::Undefined:
            UNREACHABLE();
    }
}

ResultOrError<wgpu::TextureFormat> FromUncompressedColorDXGITextureFormat(DXGI_FORMAT format) {
    switch (format) {
#define X(wgpuFormat, dxgiFormat) \
    case dxgiFormat:              \
        return wgpuFormat;
        UNCOMPRESSED_COLOR_FORMATS(X)
#undef X

        default:
            return DAWN_VALIDATION_ERROR("Unsupported DXGI format %x", format);
    }
}

#undef UNCOMPRESSED_COLOR_FORMATS

DXGI_FORMAT DXGIVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8x2:
            return DXGI_FORMAT_R8G8_UINT;
        case wgpu::VertexFormat::Uint8x4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case wgpu::VertexFormat::Sint8x2:
            return DXGI_FORMAT_R8G8_SINT;
        case wgpu::VertexFormat::Sint8x4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case wgpu::VertexFormat::Unorm8x2:
            return DXGI_FORMAT_R8G8_UNORM;
        case wgpu::VertexFormat::Unorm8x4:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case wgpu::VertexFormat::Snorm8x2:
            return DXGI_FORMAT_R8G8_SNORM;
        case wgpu::VertexFormat::Snorm8x4:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case wgpu::VertexFormat::Uint16x2:
            return DXGI_FORMAT_R16G16_UINT;
        case wgpu::VertexFormat::Uint16x4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case wgpu::VertexFormat::Sint16x2:
            return DXGI_FORMAT_R16G16_SINT;
        case wgpu::VertexFormat::Sint16x4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case wgpu::VertexFormat::Unorm16x2:
            return DXGI_FORMAT_R16G16_UNORM;
        case wgpu::VertexFormat::Unorm16x4:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case wgpu::VertexFormat::Snorm16x2:
            return DXGI_FORMAT_R16G16_SNORM;
        case wgpu::VertexFormat::Snorm16x4:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case wgpu::VertexFormat::Float16x2:
            return DXGI_FORMAT_R16G16_FLOAT;
        case wgpu::VertexFormat::Float16x4:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case wgpu::VertexFormat::Float32:
            return DXGI_FORMAT_R32_FLOAT;
        case wgpu::VertexFormat::Float32x2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case wgpu::VertexFormat::Float32x3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case wgpu::VertexFormat::Float32x4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case wgpu::VertexFormat::Uint32:
            return DXGI_FORMAT_R32_UINT;
        case wgpu::VertexFormat::Uint32x2:
            return DXGI_FORMAT_R32G32_UINT;
        case wgpu::VertexFormat::Uint32x3:
            return DXGI_FORMAT_R32G32B32_UINT;
        case wgpu::VertexFormat::Uint32x4:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case wgpu::VertexFormat::Sint32:
            return DXGI_FORMAT_R32_SINT;
        case wgpu::VertexFormat::Sint32x2:
            return DXGI_FORMAT_R32G32_SINT;
        case wgpu::VertexFormat::Sint32x3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case wgpu::VertexFormat::Sint32x4:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        default:
            UNREACHABLE();
    }
}

}  // namespace dawn::native::d3d
