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

#include "dawn/native/metal/TextureMTL.h"

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

#include <CoreVideo/CVPixelBuffer.h>

namespace dawn::native::metal {

namespace {

MTLTextureUsage MetalTextureUsage(const Format& format, wgpu::TextureUsage usage) {
    MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

    if (usage & (wgpu::TextureUsage::StorageBinding)) {
        result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    }

    if (usage & (wgpu::TextureUsage::TextureBinding)) {
        result |= MTLTextureUsageShaderRead;

        // For sampling stencil aspect of combined depth/stencil.
        // See TextureView::Initialize.
        // Depth views for depth/stencil textures in Metal simply use the original
        // texture's format, but stencil views require format reinterpretation.
        if (@available(macOS 10.12, iOS 10.0, *)) {
            if (IsSubset(Aspect::Depth | Aspect::Stencil, format.aspects)) {
                result |= MTLTextureUsagePixelFormatView;
            }
        }
    }

    if (usage & wgpu::TextureUsage::RenderAttachment) {
        result |= MTLTextureUsageRenderTarget;
    }

    return result;
}

MTLTextureType MetalTextureViewType(wgpu::TextureViewDimension dimension,
                                    unsigned int sampleCount) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e1D:
            return MTLTextureType1D;
        case wgpu::TextureViewDimension::e2D:
            return (sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
        case wgpu::TextureViewDimension::e2DArray:
            return MTLTextureType2DArray;
        case wgpu::TextureViewDimension::Cube:
            return MTLTextureTypeCube;
        case wgpu::TextureViewDimension::CubeArray:
            return MTLTextureTypeCubeArray;
        case wgpu::TextureViewDimension::e3D:
            return MTLTextureType3D;

        case wgpu::TextureViewDimension::Undefined:
            UNREACHABLE();
    }
}

bool RequiresCreatingNewTextureView(const TextureBase* texture,
                                    const TextureViewDescriptor* textureViewDescriptor) {
    constexpr wgpu::TextureUsage kShaderUsageNeedsView =
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
    constexpr wgpu::TextureUsage kUsageNeedsView =
        kShaderUsageNeedsView | wgpu::TextureUsage::RenderAttachment;
    if ((texture->GetInternalUsage() & kUsageNeedsView) == 0) {
        return false;
    }

    if (texture->GetFormat().format != textureViewDescriptor->format &&
        !texture->GetFormat().HasDepthOrStencil()) {
        // Color format reinterpretation required.
        // Note: Depth/stencil formats don't support reinterpretation.
        // See also TextureView::GetAttachmentInfo when modifying this condition.
        return true;
    }

    // Reinterpretation not required. Now, we only need a new view if the view dimension or
    // set of subresources for the shader is different from the base texture.
    if ((texture->GetInternalUsage() & kShaderUsageNeedsView) == 0) {
        return false;
    }

    if (texture->GetArrayLayers() != textureViewDescriptor->arrayLayerCount ||
        (texture->GetArrayLayers() == 1 && texture->GetDimension() == wgpu::TextureDimension::e2D &&
         textureViewDescriptor->dimension == wgpu::TextureViewDimension::e2DArray)) {
        // If the view has a different number of array layers, we need a new view.
        // And, if the original texture is a 2D texture with one array layer, we need a new
        // view to view it as a 2D array texture.
        return true;
    }

    if (texture->GetNumMipLevels() != textureViewDescriptor->mipLevelCount) {
        return true;
    }

    // If the texture is created with MTLTextureUsagePixelFormatView, we need
    // a new view to perform format reinterpretation.
    if ((MetalTextureUsage(texture->GetFormat(), texture->GetInternalUsage()) &
         MTLTextureUsagePixelFormatView) != 0) {
        return true;
    }

    switch (textureViewDescriptor->dimension) {
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return true;
        default:
            break;
    }

    return false;
}

// Metal only allows format reinterpretation to happen on swizzle pattern or conversion
// between linear space and sRGB without setting MTLTextureUsagePixelFormatView flag. For
// example, creating bgra8Unorm texture view on rgba8Unorm texture or creating
// rgba8Unorm_srgb texture view on rgab8Unorm texture.
bool AllowFormatReinterpretationWithoutFlag(MTLPixelFormat origin,
                                            MTLPixelFormat reinterpretation) {
#define SRGB_PAIR(a, b)               \
    case a:                           \
        return reinterpretation == b; \
    case b:                           \
        return reinterpretation == a

    switch (origin) {
        case MTLPixelFormatRGBA8Unorm:
            return reinterpretation == MTLPixelFormatBGRA8Unorm ||
                   reinterpretation == MTLPixelFormatRGBA8Unorm_sRGB;
        case MTLPixelFormatBGRA8Unorm:
            return reinterpretation == MTLPixelFormatRGBA8Unorm ||
                   reinterpretation == MTLPixelFormatBGRA8Unorm_sRGB;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            return reinterpretation == MTLPixelFormatBGRA8Unorm_sRGB ||
                   reinterpretation == MTLPixelFormatRGBA8Unorm;
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return reinterpretation == MTLPixelFormatRGBA8Unorm_sRGB ||
                   reinterpretation == MTLPixelFormatBGRA8Unorm;

#if DAWN_PLATFORM_IS(MACOS)
            SRGB_PAIR(MTLPixelFormatBC1_RGBA, MTLPixelFormatBC1_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC2_RGBA, MTLPixelFormatBC2_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC3_RGBA, MTLPixelFormatBC3_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC7_RGBAUnorm, MTLPixelFormatBC7_RGBAUnorm_sRGB);
#endif
        default:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                switch (origin) {
                    SRGB_PAIR(MTLPixelFormatEAC_RGBA8, MTLPixelFormatEAC_RGBA8_sRGB);

                    SRGB_PAIR(MTLPixelFormatETC2_RGB8, MTLPixelFormatETC2_RGB8_sRGB);
                    SRGB_PAIR(MTLPixelFormatETC2_RGB8A1, MTLPixelFormatETC2_RGB8A1_sRGB);

                    SRGB_PAIR(MTLPixelFormatASTC_4x4_LDR, MTLPixelFormatASTC_4x4_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_5x4_LDR, MTLPixelFormatASTC_5x4_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_5x5_LDR, MTLPixelFormatASTC_5x5_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_6x5_LDR, MTLPixelFormatASTC_6x5_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_6x6_LDR, MTLPixelFormatASTC_6x6_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_8x5_LDR, MTLPixelFormatASTC_8x5_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_8x6_LDR, MTLPixelFormatASTC_8x6_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_8x8_LDR, MTLPixelFormatASTC_8x8_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_10x5_LDR, MTLPixelFormatASTC_10x5_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_10x6_LDR, MTLPixelFormatASTC_10x6_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_10x8_LDR, MTLPixelFormatASTC_10x8_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_10x10_LDR, MTLPixelFormatASTC_10x10_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_12x10_LDR, MTLPixelFormatASTC_12x10_sRGB);
                    SRGB_PAIR(MTLPixelFormatASTC_12x12_LDR, MTLPixelFormatASTC_12x12_sRGB);

                    default:
                        break;
                }
            }

            return false;
    }
#undef SRGB_PAIR
}

ResultOrError<wgpu::TextureFormat> GetFormatEquivalentToIOSurfaceFormat(uint32_t format) {
    switch (format) {
        case kCVPixelFormatType_64RGBAHalf:
            return wgpu::TextureFormat::RGBA16Float;
        case kCVPixelFormatType_TwoComponent16Half:
            return wgpu::TextureFormat::RG16Float;
        case kCVPixelFormatType_OneComponent16Half:
            return wgpu::TextureFormat::R16Float;
        case kCVPixelFormatType_ARGB2101010LEPacked:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case kCVPixelFormatType_32RGBA:
            return wgpu::TextureFormat::RGBA8Unorm;
        case kCVPixelFormatType_32BGRA:
            return wgpu::TextureFormat::BGRA8Unorm;
        case kCVPixelFormatType_TwoComponent8:
            return wgpu::TextureFormat::RG8Unorm;
        case kCVPixelFormatType_OneComponent8:
            return wgpu::TextureFormat::R8Unorm;
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
            return wgpu::TextureFormat::R8BG8Biplanar420Unorm;
        default:
            return DAWN_VALIDATION_ERROR("Unsupported IOSurface format (%x).", format);
    }
}

uint32_t GetIOSurfacePlane(wgpu::TextureAspect aspect) {
    switch (aspect) {
        case wgpu::TextureAspect::Plane0Only:
            return 0;
        case wgpu::TextureAspect::Plane1Only:
            return 1;
        default:
            UNREACHABLE();
    }
}

#if DAWN_PLATFORM_IS(MACOS)
MTLStorageMode kIOSurfaceStorageMode = MTLStorageModeManaged;
#elif DAWN_PLATFORM_IS(IOS)
MTLStorageMode kIOSurfaceStorageMode = MTLStorageModePrivate;
#else
#error "Unsupported Apple platform."
#endif
}  // namespace

MTLPixelFormat MetalPixelFormat(const DeviceBase* device, wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:
            return MTLPixelFormatR8Unorm;
        case wgpu::TextureFormat::R8Snorm:
            return MTLPixelFormatR8Snorm;
        case wgpu::TextureFormat::R8Uint:
            return MTLPixelFormatR8Uint;
        case wgpu::TextureFormat::R8Sint:
            return MTLPixelFormatR8Sint;

        case wgpu::TextureFormat::R16Uint:
            return MTLPixelFormatR16Uint;
        case wgpu::TextureFormat::R16Sint:
            return MTLPixelFormatR16Sint;
        case wgpu::TextureFormat::R16Float:
            return MTLPixelFormatR16Float;
        case wgpu::TextureFormat::RG8Unorm:
            return MTLPixelFormatRG8Unorm;
        case wgpu::TextureFormat::RG8Snorm:
            return MTLPixelFormatRG8Snorm;
        case wgpu::TextureFormat::RG8Uint:
            return MTLPixelFormatRG8Uint;
        case wgpu::TextureFormat::RG8Sint:
            return MTLPixelFormatRG8Sint;

        case wgpu::TextureFormat::R32Uint:
            return MTLPixelFormatR32Uint;
        case wgpu::TextureFormat::R32Sint:
            return MTLPixelFormatR32Sint;
        case wgpu::TextureFormat::R32Float:
            return MTLPixelFormatR32Float;
        case wgpu::TextureFormat::RG16Uint:
            return MTLPixelFormatRG16Uint;
        case wgpu::TextureFormat::RG16Sint:
            return MTLPixelFormatRG16Sint;
        case wgpu::TextureFormat::RG16Float:
            return MTLPixelFormatRG16Float;
        case wgpu::TextureFormat::RGBA8Unorm:
            return MTLPixelFormatRGBA8Unorm;
        case wgpu::TextureFormat::RGBA8UnormSrgb:
            return MTLPixelFormatRGBA8Unorm_sRGB;
        case wgpu::TextureFormat::RGBA8Snorm:
            return MTLPixelFormatRGBA8Snorm;
        case wgpu::TextureFormat::RGBA8Uint:
            return MTLPixelFormatRGBA8Uint;
        case wgpu::TextureFormat::RGBA8Sint:
            return MTLPixelFormatRGBA8Sint;
        case wgpu::TextureFormat::BGRA8Unorm:
            return MTLPixelFormatBGRA8Unorm;
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return MTLPixelFormatBGRA8Unorm_sRGB;
        case wgpu::TextureFormat::RGB10A2Unorm:
            return MTLPixelFormatRGB10A2Unorm;
        case wgpu::TextureFormat::RG11B10Ufloat:
            return MTLPixelFormatRG11B10Float;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            return MTLPixelFormatRGB9E5Float;

        case wgpu::TextureFormat::RG32Uint:
            return MTLPixelFormatRG32Uint;
        case wgpu::TextureFormat::RG32Sint:
            return MTLPixelFormatRG32Sint;
        case wgpu::TextureFormat::RG32Float:
            return MTLPixelFormatRG32Float;
        case wgpu::TextureFormat::RGBA16Uint:
            return MTLPixelFormatRGBA16Uint;
        case wgpu::TextureFormat::RGBA16Sint:
            return MTLPixelFormatRGBA16Sint;
        case wgpu::TextureFormat::RGBA16Float:
            return MTLPixelFormatRGBA16Float;

        case wgpu::TextureFormat::RGBA32Uint:
            return MTLPixelFormatRGBA32Uint;
        case wgpu::TextureFormat::RGBA32Sint:
            return MTLPixelFormatRGBA32Sint;
        case wgpu::TextureFormat::RGBA32Float:
            return MTLPixelFormatRGBA32Float;

        case wgpu::TextureFormat::Depth32Float:
            return MTLPixelFormatDepth32Float;
        case wgpu::TextureFormat::Depth24Plus:
            return MTLPixelFormatDepth32Float;
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return MTLPixelFormatDepth32Float_Stencil8;
        case wgpu::TextureFormat::Depth16Unorm:
            if (@available(macOS 10.12, iOS 13.0, *)) {
                return MTLPixelFormatDepth16Unorm;
            } else {
                // TODO(dawn:1181): Allow non-conformant implementation on macOS 10.11
                UNREACHABLE();
            }
        case wgpu::TextureFormat::Stencil8:
            if (device->IsToggleEnabled(Toggle::MetalUseCombinedDepthStencilFormatForStencil8)) {
                return MTLPixelFormatDepth32Float_Stencil8;
            }
            return MTLPixelFormatStencil8;

#if DAWN_PLATFORM_IS(MACOS)
        case wgpu::TextureFormat::BC1RGBAUnorm:
            return MTLPixelFormatBC1_RGBA;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            return MTLPixelFormatBC1_RGBA_sRGB;
        case wgpu::TextureFormat::BC2RGBAUnorm:
            return MTLPixelFormatBC2_RGBA;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            return MTLPixelFormatBC2_RGBA_sRGB;
        case wgpu::TextureFormat::BC3RGBAUnorm:
            return MTLPixelFormatBC3_RGBA;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            return MTLPixelFormatBC3_RGBA_sRGB;
        case wgpu::TextureFormat::BC4RSnorm:
            return MTLPixelFormatBC4_RSnorm;
        case wgpu::TextureFormat::BC4RUnorm:
            return MTLPixelFormatBC4_RUnorm;
        case wgpu::TextureFormat::BC5RGSnorm:
            return MTLPixelFormatBC5_RGSnorm;
        case wgpu::TextureFormat::BC5RGUnorm:
            return MTLPixelFormatBC5_RGUnorm;
        case wgpu::TextureFormat::BC6HRGBFloat:
            return MTLPixelFormatBC6H_RGBFloat;
        case wgpu::TextureFormat::BC6HRGBUfloat:
            return MTLPixelFormatBC6H_RGBUfloat;
        case wgpu::TextureFormat::BC7RGBAUnorm:
            return MTLPixelFormatBC7_RGBAUnorm;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            return MTLPixelFormatBC7_RGBAUnorm_sRGB;
#else
        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
#endif

        case wgpu::TextureFormat::ETC2RGB8Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatETC2_RGB8;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatETC2_RGB8_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatETC2_RGB8A1;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatETC2_RGB8A1_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_RGBA8;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_RGBA8_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::EACR11Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_R11Unorm;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::EACR11Snorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_R11Snorm;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::EACRG11Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_RG11Unorm;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::EACRG11Snorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatEAC_RG11Snorm;
            } else {
                UNREACHABLE();
            }

        case wgpu::TextureFormat::ASTC4x4Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_4x4_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_4x4_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC5x4Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_5x4_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_5x4_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC5x5Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_5x5_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_5x5_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC6x5Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_6x5_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_6x5_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC6x6Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_6x6_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_6x6_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x5Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x5_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x5_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x6Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x6_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x6_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x8Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x8_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_8x8_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x5Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x5_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x5_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x6Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x6_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x6_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x8Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x8_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x8_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x10Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x10_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_10x10_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC12x10Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_12x10_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_12x10_sRGB;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC12x12Unorm:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_12x12_LDR;
            } else {
                UNREACHABLE();
            }
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            if (@available(macOS 11.0, iOS 8.0, *)) {
                return MTLPixelFormatASTC_12x12_sRGB;
            } else {
                UNREACHABLE();
            }

        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::Undefined:
            UNREACHABLE();
    }
}

MaybeError ValidateIOSurfaceCanBeWrapped(const DeviceBase*,
                                         const TextureDescriptor* descriptor,
                                         IOSurfaceRef ioSurface) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    uint32_t surfaceWidth = IOSurfaceGetWidth(ioSurface);
    uint32_t surfaceHeight = IOSurfaceGetHeight(ioSurface);

    DAWN_INVALID_IF(
        descriptor->size.width != surfaceWidth || descriptor->size.height != surfaceHeight ||
            descriptor->size.depthOrArrayLayers != 1,
        "IOSurface size (width: %u, height %u, depth: 1) doesn't match descriptor size %s.",
        surfaceWidth, surfaceHeight, &descriptor->size);

    wgpu::TextureFormat ioSurfaceFormat;
    DAWN_TRY_ASSIGN(ioSurfaceFormat,
                    GetFormatEquivalentToIOSurfaceFormat(IOSurfaceGetPixelFormat(ioSurface)));
    DAWN_INVALID_IF(descriptor->format != ioSurfaceFormat,
                    "IOSurface format (%s) doesn't match the descriptor format (%s).",
                    ioSurfaceFormat, descriptor->format);

    return {};
}

NSRef<MTLTextureDescriptor> Texture::CreateMetalTextureDescriptor() const {
    NSRef<MTLTextureDescriptor> mtlDescRef = AcquireNSRef([MTLTextureDescriptor new]);
    MTLTextureDescriptor* mtlDesc = mtlDescRef.Get();

    mtlDesc.width = GetWidth();
    mtlDesc.sampleCount = GetSampleCount();
    // Metal only allows format reinterpretation to happen on swizzle pattern or conversion
    // between linear space and sRGB. For example, creating bgra8Unorm texture view on
    // rgba8Unorm texture or creating rgba8Unorm_srgb texture view on rgab8Unorm texture.
    mtlDesc.usage = MetalTextureUsage(GetFormat(), GetInternalUsage());
    mtlDesc.pixelFormat = MetalPixelFormat(GetDevice(), GetFormat().format);
    if (GetDevice()->IsToggleEnabled(Toggle::MetalUseCombinedDepthStencilFormatForStencil8) &&
        GetFormat().format == wgpu::TextureFormat::Stencil8) {
        // If we used a combined depth stencil format instead of stencil8, we need
        // MTLTextureUsagePixelFormatView to reinterpet as stencil8.
        mtlDesc.usage |= MTLTextureUsagePixelFormatView;
    }
    mtlDesc.mipmapLevelCount = GetNumMipLevels();
    mtlDesc.storageMode = MTLStorageModePrivate;

    // Choose the correct MTLTextureType and paper over differences in how the array layer count
    // is specified.
    switch (GetDimension()) {
        case wgpu::TextureDimension::e1D:
            mtlDesc.arrayLength = 1;
            mtlDesc.depth = 1;
            ASSERT(mtlDesc.sampleCount == 1);
            mtlDesc.textureType = MTLTextureType1D;
            break;

        case wgpu::TextureDimension::e2D:
            mtlDesc.height = GetHeight();
            mtlDesc.arrayLength = GetArrayLayers();
            mtlDesc.depth = 1;
            if (mtlDesc.arrayLength > 1) {
                ASSERT(mtlDesc.sampleCount == 1);
                mtlDesc.textureType = MTLTextureType2DArray;
            } else if (mtlDesc.sampleCount > 1) {
                mtlDesc.textureType = MTLTextureType2DMultisample;
            } else {
                mtlDesc.textureType = MTLTextureType2D;
            }
            break;
        case wgpu::TextureDimension::e3D:
            mtlDesc.height = GetHeight();
            mtlDesc.depth = GetDepth();
            mtlDesc.arrayLength = 1;
            ASSERT(mtlDesc.sampleCount == 1);
            mtlDesc.textureType = MTLTextureType3D;
            break;
    }

    return mtlDescRef;
}

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device, const TextureDescriptor* descriptor) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
    DAWN_TRY(texture->InitializeAsInternalTexture(descriptor));
    return texture;
}

// static
ResultOrError<Ref<Texture>> Texture::CreateFromIOSurface(
    Device* device,
    const ExternalImageDescriptor* descriptor,
    IOSurfaceRef ioSurface,
    std::vector<MTLSharedEventAndSignalValue> waitEvents) {
    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

    Ref<Texture> texture =
        AcquireRef(new Texture(device, textureDescriptor, TextureState::OwnedExternal));
    DAWN_TRY(texture->InitializeFromIOSurface(descriptor, textureDescriptor, ioSurface,
                                              std::move(waitEvents)));
    return texture;
}

// static
Ref<Texture> Texture::CreateWrapping(Device* device,
                                     const TextureDescriptor* descriptor,
                                     NSPRef<id<MTLTexture>> wrapped) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
    texture->InitializeAsWrapping(descriptor, std::move(wrapped));
    return texture;
}

MaybeError Texture::InitializeAsInternalTexture(const TextureDescriptor* descriptor) {
    Device* device = ToBackend(GetDevice());

    NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
    mMtlUsage = [*mtlDesc usage];
    mMtlTexture = AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()]);

    if (mMtlTexture == nil) {
        return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate texture.");
    }

    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(ClearTexture(device->GetPendingCommandContext(), GetAllSubresources(),
                              TextureBase::ClearValue::NonZero));
    } else if (ShouldKeepInitialized()) {
        DAWN_TRY(ClearTexture(device->GetPendingCommandContext(), GetAllSubresources(),
                              TextureBase::ClearValue::Zero));
    }

    return {};
}

void Texture::InitializeAsWrapping(const TextureDescriptor* descriptor,
                                   NSPRef<id<MTLTexture>> wrapped) {
    NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
    mMtlUsage = [*mtlDesc usage];
    mMtlTexture = std::move(wrapped);
}

MaybeError Texture::InitializeFromIOSurface(const ExternalImageDescriptor* descriptor,
                                            const TextureDescriptor* textureDescriptor,
                                            IOSurfaceRef ioSurface,
                                            std::vector<MTLSharedEventAndSignalValue> waitEvents) {
    mIOSurface = ioSurface;
    mWaitEvents = std::move(waitEvents);

    // Uses WGPUTexture which wraps multiplanar ioSurface needs to create
    // texture view explicitly. Wrap the ioSurface and delay to extract
    // MTLTexture from the plane of it when creating texture view.
    // WGPUTexture which wraps non-multplanar ioSurface needs to support
    // ops that doesn't require creating texture view(e.g. copy). Extract
    // MTLTexture from such ioSurface to support this.
    if (!GetFormat().IsMultiPlanar()) {
        Device* device = ToBackend(GetDevice());

        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
        [*mtlDesc setStorageMode:kIOSurfaceStorageMode];

        mMtlUsage = [*mtlDesc usage];
        mMtlTexture = AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()
                                                                           iosurface:ioSurface
                                                                               plane:0]);
    }
    SetIsSubresourceContentInitialized(descriptor->isInitialized, GetAllSubresources());
    return {};
}

void Texture::SynchronizeTextureBeforeUse(CommandRecordingContext* commandContext) {
    if (@available(macOS 10.14, *)) {
        if (!mWaitEvents.empty()) {
            // There may be an open blit encoder from a copy command or writeBuffer.
            // Wait events are only allowed if there is no encoder open.
            commandContext->EndBlit();
        }
        auto commandBuffer = commandContext->GetCommands();
        // Consume the wait events on the texture. They will be empty after this loop.
        for (auto waitEvent : std::move(mWaitEvents)) {
            id rawEvent = *waitEvent.sharedEvent;
            id<MTLSharedEvent> sharedEvent = static_cast<id<MTLSharedEvent>>(rawEvent);
            [commandBuffer encodeWaitForEvent:sharedEvent value:waitEvent.signaledValue];
        }
    }
}

void Texture::IOSurfaceEndAccess(ExternalImageIOSurfaceEndAccessDescriptor* descriptor) {
    ASSERT(descriptor);
    ToBackend(GetDevice())->ExportLastSignaledEvent(descriptor);
    descriptor->isInitialized = IsSubresourceContentInitialized(GetAllSubresources());
    // Destroy the texture as it should not longer be used after EndAccess.
    Destroy();
}

Texture::Texture(DeviceBase* dev, const TextureDescriptor* desc, TextureState st)
    : TextureBase(dev, desc, st) {}

Texture::~Texture() {}

void Texture::DestroyImpl() {
    TextureBase::DestroyImpl();
    mMtlTexture = nullptr;
    mIOSurface = nullptr;
}

id<MTLTexture> Texture::GetMTLTexture() const {
    return mMtlTexture.Get();
}

IOSurfaceRef Texture::GetIOSurface() {
    return mIOSurface.Get();
}

NSPRef<id<MTLTexture>> Texture::CreateFormatView(wgpu::TextureFormat format) {
    if (GetFormat().format == format) {
        return mMtlTexture;
    }

    ASSERT(AllowFormatReinterpretationWithoutFlag(MetalPixelFormat(GetDevice(), GetFormat().format),
                                                  MetalPixelFormat(GetDevice(), format)));
    return AcquireNSPRef(
        [mMtlTexture.Get() newTextureViewWithPixelFormat:MetalPixelFormat(GetDevice(), format)]);
}

bool Texture::ShouldKeepInitialized() const {
    return GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
           GetDevice()->IsToggleEnabled(
               Toggle::MetalKeepMultisubresourceDepthStencilTexturesInitialized) &&
           GetFormat().HasDepthOrStencil() && (GetArrayLayers() > 1 || GetNumMipLevels() > 1);
}

MaybeError Texture::ClearTexture(CommandRecordingContext* commandContext,
                                 const SubresourceRange& range,
                                 TextureBase::ClearValue clearValue) {
    Device* device = ToBackend(GetDevice());

    const uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
    const double dClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.0 : 1.0;

    if ((mMtlUsage & MTLTextureUsageRenderTarget) != 0) {
        ASSERT(GetFormat().isRenderable);

        // End the blit encoder if it is open.
        commandContext->EndBlit();

        if (GetFormat().HasDepthOrStencil()) {
            // Create a render pass to clear each subresource.
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                for (uint32_t arrayLayer = range.baseArrayLayer;
                     arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                            level, arrayLayer, range.aspects))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    // Note that this creates a descriptor that's autoreleased so we don't use
                    // AcquireNSRef
                    NSRef<MTLRenderPassDescriptor> descriptorRef =
                        [MTLRenderPassDescriptor renderPassDescriptor];
                    MTLRenderPassDescriptor* descriptor = descriptorRef.Get();

                    // At least one aspect needs clearing. Iterate the aspects individually to
                    // determine which to clear.
                    for (Aspect aspect : IterateEnumMask(range.aspects)) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, arrayLayer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
                        switch (aspect) {
                            case Aspect::Depth:
                                descriptor.depthAttachment.texture = GetMTLTexture();
                                descriptor.depthAttachment.level = level;
                                descriptor.depthAttachment.slice = arrayLayer;
                                descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                                descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                                descriptor.depthAttachment.clearDepth = dClearColor;
                                break;
                            case Aspect::Stencil:
                                descriptor.stencilAttachment.texture = GetMTLTexture();
                                descriptor.stencilAttachment.level = level;
                                descriptor.stencilAttachment.slice = arrayLayer;
                                descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                                descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                                descriptor.stencilAttachment.clearStencil =
                                    static_cast<uint32_t>(clearColor);
                                break;
                            default:
                                UNREACHABLE();
                        }
                    }

                    DAWN_TRY(
                        EncodeEmptyMetalRenderPass(device, commandContext, descriptor,
                                                   GetMipLevelSingleSubresourceVirtualSize(level)));
                }
            }
        } else {
            ASSERT(GetFormat().IsColor());
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                // Create multiple render passes with each subresource as a color attachment to
                // clear them all. Only do this for array layers to ensure all attachments have
                // the same size.
                NSRef<MTLRenderPassDescriptor> descriptor;
                uint32_t attachment = 0;

                uint32_t depth = GetMipLevelSingleSubresourceVirtualSize(level).depthOrArrayLayers;

                for (uint32_t arrayLayer = range.baseArrayLayer;
                     arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                            level, arrayLayer, Aspect::Color))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    for (uint32_t z = 0; z < depth; ++z) {
                        if (descriptor == nullptr) {
                            // Note that this creates a descriptor that's autoreleased so we
                            // don't use AcquireNSRef
                            descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
                        }

                        [*descriptor colorAttachments][attachment].texture = GetMTLTexture();
                        [*descriptor colorAttachments][attachment].loadAction = MTLLoadActionClear;
                        [*descriptor colorAttachments][attachment].storeAction =
                            MTLStoreActionStore;
                        [*descriptor colorAttachments][attachment].clearColor =
                            MTLClearColorMake(dClearColor, dClearColor, dClearColor, dClearColor);
                        [*descriptor colorAttachments][attachment].level = level;
                        [*descriptor colorAttachments][attachment].slice = arrayLayer;
                        [*descriptor colorAttachments][attachment].depthPlane = z;

                        attachment++;

                        if (attachment == kMaxColorAttachments) {
                            attachment = 0;
                            DAWN_TRY(EncodeEmptyMetalRenderPass(
                                device, commandContext, descriptor.Get(),
                                GetMipLevelSingleSubresourceVirtualSize(level)));
                            descriptor = nullptr;
                        }
                    }
                }

                if (descriptor != nullptr) {
                    DAWN_TRY(
                        EncodeEmptyMetalRenderPass(device, commandContext, descriptor.Get(),
                                                   GetMipLevelSingleSubresourceVirtualSize(level)));
                }
            }
        }
    } else {
        ASSERT(!IsMultisampledTexture());

        // Encode a buffer to texture copy to clear each subresource.
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            // Compute the buffer size big enough to fill the largest mip.
            const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(aspect).block;

            // Computations for the bytes per row / image height are done using the physical size
            // so that enough data is reserved for compressed textures.
            Extent3D largestMipSize = GetMipLevelSingleSubresourcePhysicalSize(range.baseMipLevel);
            uint32_t largestMipBytesPerRow =
                (largestMipSize.width / blockInfo.width) * blockInfo.byteSize;
            uint64_t largestMipBytesPerImage = static_cast<uint64_t>(largestMipBytesPerRow) *
                                               (largestMipSize.height / blockInfo.height);
            uint64_t bufferSize = largestMipBytesPerImage * largestMipSize.depthOrArrayLayers;

            if (bufferSize > std::numeric_limits<NSUInteger>::max()) {
                return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
            }

            DynamicUploader* uploader = device->GetDynamicUploader();
            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle,
                            uploader->Allocate(bufferSize, device->GetPendingCommandSerial(),
                                               blockInfo.byteSize));
            memset(uploadHandle.mappedBuffer, clearColor, bufferSize);

            id<MTLBuffer> uploadBuffer = ToBackend(uploadHandle.stagingBuffer)->GetMTLBuffer();

            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                Extent3D virtualSize = GetMipLevelSingleSubresourceVirtualSize(level);

                for (uint32_t arrayLayer = range.baseArrayLayer;
                     arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(
                            SubresourceRange::SingleMipAndLayer(level, arrayLayer, aspect))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    MTLBlitOption blitOption = ComputeMTLBlitOption(aspect);
                    [commandContext->EnsureBlit()
                             copyFromBuffer:uploadBuffer
                               sourceOffset:uploadHandle.startOffset
                          sourceBytesPerRow:largestMipBytesPerRow
                        sourceBytesPerImage:largestMipBytesPerImage
                                 sourceSize:MTLSizeMake(virtualSize.width, virtualSize.height,
                                                        virtualSize.depthOrArrayLayers)
                                  toTexture:GetMTLTexture()
                           destinationSlice:arrayLayer
                           destinationLevel:level
                          destinationOrigin:MTLOriginMake(0, 0, 0)
                                    options:blitOption];
                }
            }
        }
    }
    return {};
}

MTLBlitOption Texture::ComputeMTLBlitOption(Aspect aspect) const {
    ASSERT(HasOneBit(aspect));
    ASSERT(GetFormat().aspects & aspect);
    MTLPixelFormat format = MetalPixelFormat(GetDevice(), GetFormat().format);

    if (format == MTLPixelFormatDepth32Float_Stencil8) {
        // We only provide a blit option if the format has both depth and stencil.
        // It is invalid to provide a blit option otherwise.
        switch (aspect) {
            case Aspect::Depth:
                return MTLBlitOptionDepthFromDepthStencil;
            case Aspect::Stencil:
                return MTLBlitOptionStencilFromDepthStencil;
            default:
                UNREACHABLE();
        }
    }
    return MTLBlitOptionNone;
}

void Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                  const SubresourceRange& range) {
    if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        return;
    }
    if (!IsSubresourceContentInitialized(range)) {
        // If subresource has not been initialized, clear it to black as it could
        // contain dirty bits from recycled memory
        GetDevice()->ConsumedError(
            ClearTexture(commandContext, range, TextureBase::ClearValue::Zero));
        SetIsSubresourceContentInitialized(true, range);
        GetDevice()->IncrementLazyClearCountForTesting();
    }
}

// static
ResultOrError<Ref<TextureView>> TextureView::Create(TextureBase* texture,
                                                    const TextureViewDescriptor* descriptor) {
    Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
    DAWN_TRY(view->Initialize(descriptor));
    return view;
}

MaybeError TextureView::Initialize(const TextureViewDescriptor* descriptor) {
    DeviceBase* device = GetDevice();
    Texture* texture = ToBackend(GetTexture());

    // Texture could be destroyed by the time we make a view.
    if (GetTexture()->GetTextureState() == Texture::TextureState::Destroyed) {
        return {};
    }

    id<MTLTexture> mtlTexture = texture->GetMTLTexture();

    bool needsNewView = RequiresCreatingNewTextureView(texture, descriptor);
    if (device->IsToggleEnabled(Toggle::MetalUseCombinedDepthStencilFormatForStencil8) &&
        GetTexture()->GetFormat().format == wgpu::TextureFormat::Stencil8) {
        // If MetalUseCombinedDepthStencilFormatForStencil8 is true and the format is Stencil8,
        // we used a combined format instead on texture allocation.
        // We need a new view to view it as stencil8.
        needsNewView = true;
    }
    if (!needsNewView) {
        mMtlTextureView = mtlTexture;
    } else if (texture->GetFormat().IsMultiPlanar()) {
        NSRef<MTLTextureDescriptor> mtlDescRef = AcquireNSRef([MTLTextureDescriptor new]);
        MTLTextureDescriptor* mtlDesc = mtlDescRef.Get();

        mtlDesc.sampleCount = texture->GetSampleCount();
        mtlDesc.usage = MetalTextureUsage(texture->GetFormat(), texture->GetInternalUsage());
        mtlDesc.pixelFormat = MetalPixelFormat(device, descriptor->format);
        mtlDesc.mipmapLevelCount = texture->GetNumMipLevels();
        mtlDesc.storageMode = kIOSurfaceStorageMode;

        uint32_t plane = GetIOSurfacePlane(descriptor->aspect);
        mtlDesc.width = IOSurfaceGetWidthOfPlane(texture->GetIOSurface(), plane);
        mtlDesc.height = IOSurfaceGetHeightOfPlane(texture->GetIOSurface(), plane);

        // Multiplanar texture is validated to only have single layer, single mipLevel
        // and 2d textures (depth == 1)
        ASSERT(texture->GetArrayLayers() == 1 &&
               texture->GetDimension() == wgpu::TextureDimension::e2D &&
               texture->GetNumMipLevels() == 1);
        mtlDesc.arrayLength = 1;
        mtlDesc.depth = 1;

        mMtlTextureView = AcquireNSPRef([ToBackend(GetDevice())->GetMTLDevice()
            newTextureWithDescriptor:mtlDesc
                           iosurface:texture->GetIOSurface()
                               plane:plane]);
        if (mMtlTextureView == nil) {
            return DAWN_INTERNAL_ERROR("Failed to create MTLTexture view for external texture.");
        }
    } else {
        MTLPixelFormat viewFormat = MetalPixelFormat(device, descriptor->format);
        MTLPixelFormat textureFormat = MetalPixelFormat(device, GetTexture()->GetFormat().format);

        Aspect aspect = SelectFormatAspects(GetFormat(), descriptor->aspect);
        if (aspect == Aspect::Stencil && textureFormat != MTLPixelFormatStencil8) {
            if (@available(macOS 10.12, iOS 10.0, *)) {
                if (textureFormat == MTLPixelFormatDepth32Float_Stencil8) {
                    viewFormat = MTLPixelFormatX32_Stencil8;
                } else {
                    UNREACHABLE();
                }
            } else {
                // TODO(enga): Add a workaround to back combined depth/stencil textures
                // with Sampled usage using two separate textures.
                // Or, consider always using the workaround for D32S8.
                device->ConsumedError(
                    DAWN_DEVICE_LOST_ERROR("Cannot create stencil-only texture view of "
                                           "combined depth/stencil format."));
            }
        } else if (GetTexture()->GetFormat().HasDepth() && GetTexture()->GetFormat().HasStencil()) {
            // Depth-only views for depth/stencil textures in Metal simply use the original
            // texture's format.
            viewFormat = textureFormat;
        }

        MTLTextureType textureViewType =
            MetalTextureViewType(descriptor->dimension, texture->GetSampleCount());
        auto mipLevelRange = NSMakeRange(descriptor->baseMipLevel, descriptor->mipLevelCount);
        auto arrayLayerRange = NSMakeRange(descriptor->baseArrayLayer, descriptor->arrayLayerCount);

        mMtlTextureView = AcquireNSPRef([mtlTexture newTextureViewWithPixelFormat:viewFormat
                                                                      textureType:textureViewType
                                                                           levels:mipLevelRange
                                                                           slices:arrayLayerRange]);
        if (mMtlTextureView == nil) {
            return DAWN_INTERNAL_ERROR("Failed to create MTLTexture view.");
        }
    }

    return {};
}

void TextureView::DestroyImpl() {
    mMtlTextureView = nil;
}

id<MTLTexture> TextureView::GetMTLTexture() const {
    ASSERT(mMtlTextureView != nullptr);
    return mMtlTextureView.Get();
}

TextureView::AttachmentInfo TextureView::GetAttachmentInfo() const {
    ASSERT(GetTexture()->GetInternalUsage() & wgpu::TextureUsage::RenderAttachment);
    // Use our own view if the formats do not match.
    // If the formats do not match, format reinterpretation will be required.
    // Note: Depth/stencil formats don't support reinterpretation.
    // Also, we compute |useOwnView| here instead of relying on whether or not
    // a view was created in Initialize, because rendering to a depth/stencil
    // texture on Metal only works when using the original texture, not a view.
    bool useOwnView = GetFormat().format != GetTexture()->GetFormat().format &&
                      !GetTexture()->GetFormat().HasDepthOrStencil();
    if (useOwnView) {
        ASSERT(mMtlTextureView.Get());
        return {mMtlTextureView, 0, 0};
    }
    AttachmentInfo info;
    info.texture = ToBackend(GetTexture())->GetMTLTexture();
    info.baseMipLevel = GetBaseMipLevel();
    info.baseArrayLayer = GetBaseArrayLayer();
    return info;
}

}  // namespace dawn::native::metal
