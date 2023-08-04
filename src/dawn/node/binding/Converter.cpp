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

#include "src/dawn/node/binding/Converter.h"

#include <cassert>

#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUPipelineLayout.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPUSampler.h"
#include "src/dawn/node/binding/GPUShaderModule.h"
#include "src/dawn/node/binding/GPUTexture.h"
#include "src/dawn/node/binding/GPUTextureView.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

Converter::~Converter() {
    for (auto& free : free_) {
        free();
    }
}

bool Converter::HasFeature(wgpu::FeatureName feature) {
    // Not all uses of the converter will have a device (for example for adapter-related
    // conversions).
    assert(device.Get() != nullptr);
    return device.HasFeature(feature);
}

bool Converter::Convert(wgpu::Extent3D& out, const interop::GPUExtent3D& in) {
    out = {};
    if (auto* dict = std::get_if<interop::GPUExtent3DDict>(&in)) {
        out.depthOrArrayLayers = dict->depthOrArrayLayers;
        out.width = dict->width;
        out.height = dict->height;
        return true;
    }
    if (auto* vec = std::get_if<std::vector<interop::GPUIntegerCoordinate>>(&in)) {
        switch (vec->size()) {
            default:
            case 3:
                out.depthOrArrayLayers = (*vec)[2];
            case 2:  // fallthrough
                out.height = (*vec)[1];
            case 1:  // fallthrough
                out.width = (*vec)[0];
                return true;
            case 0:
                break;
        }
    }
    Napi::Error::New(env, "invalid value for GPUExtent3D").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::Origin3D& out, const interop::GPUOrigin3DDict& in) {
    out = {};
    out.x = in.x;
    out.y = in.y;
    out.z = in.z;
    return true;
}

bool Converter::Convert(wgpu::Color& out, const interop::GPUColor& in) {
    out = {};
    if (auto* dict = std::get_if<interop::GPUColorDict>(&in)) {
        out.r = dict->r;
        out.g = dict->g;
        out.b = dict->b;
        out.a = dict->a;
        return true;
    }
    if (auto* vec = std::get_if<std::vector<double>>(&in)) {
        switch (vec->size()) {
            default:
            case 4:
                out.a = (*vec)[3];
            case 3:  // fallthrough
                out.b = (*vec)[2];
            case 2:  // fallthrough
                out.g = (*vec)[1];
            case 1:  // fallthrough
                out.r = (*vec)[0];
                return true;
            case 0:
                break;
        }
    }
    Napi::Error::New(env, "invalid value for GPUColor").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::Origin3D& out, const std::vector<interop::GPUIntegerCoordinate>& in) {
    out = {};
    switch (in.size()) {
        default:
        case 3:
            out.z = in[2];
        case 2:  // fallthrough
            out.y = in[1];
        case 1:  // fallthrough
            out.x = in[0];
        case 0:
            break;
    }
    return true;
}

bool Converter::Convert(wgpu::TextureAspect& out, const interop::GPUTextureAspect& in) {
    out = wgpu::TextureAspect::All;
    switch (in) {
        case interop::GPUTextureAspect::kAll:
            out = wgpu::TextureAspect::All;
            return true;
        case interop::GPUTextureAspect::kStencilOnly:
            out = wgpu::TextureAspect::StencilOnly;
            return true;
        case interop::GPUTextureAspect::kDepthOnly:
            out = wgpu::TextureAspect::DepthOnly;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUTextureAspect").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::ImageCopyTexture& out, const interop::GPUImageCopyTexture& in) {
    out = {};
    return Convert(out.texture, in.texture) && Convert(out.mipLevel, in.mipLevel) &&
           Convert(out.origin, in.origin) && Convert(out.aspect, in.aspect);
}

bool Converter::Convert(wgpu::ImageCopyBuffer& out, const interop::GPUImageCopyBuffer& in) {
    out = {};
    out.buffer = *in.buffer.As<GPUBuffer>();
    return Convert(out.layout.offset, in.offset) &&
           Convert(out.layout.bytesPerRow, in.bytesPerRow) &&
           Convert(out.layout.rowsPerImage, in.rowsPerImage);
}

bool Converter::Convert(BufferSource& out, interop::BufferSource in) {
    out = {};
    if (auto* view = std::get_if<interop::ArrayBufferView>(&in)) {
        std::visit(
            [&](auto&& v) {
                auto arr = v.ArrayBuffer();
                out.data = static_cast<uint8_t*>(arr.Data()) + v.ByteOffset();
                out.size = v.ByteLength();
                out.bytesPerElement = v.ElementSize();
            },
            *view);
        return true;
    }
    if (auto* arr = std::get_if<interop::ArrayBuffer>(&in)) {
        out.data = arr->Data();
        out.size = arr->ByteLength();
        out.bytesPerElement = 1;
        return true;
    }
    Napi::Error::New(env, "invalid value for BufferSource").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::TextureDataLayout& out, const interop::GPUImageDataLayout& in) {
    out = {};
    return Convert(out.bytesPerRow, in.bytesPerRow) && Convert(out.offset, in.offset) &&
           Convert(out.rowsPerImage, in.rowsPerImage);
}

bool Converter::Convert(wgpu::TextureFormat& out, const interop::GPUTextureFormat& in) {
    out = wgpu::TextureFormat::Undefined;
    wgpu::FeatureName requiredFeature = wgpu::FeatureName::Undefined;
    switch (in) {
        case interop::GPUTextureFormat::kR8Unorm:
            out = wgpu::TextureFormat::R8Unorm;
            return true;
        case interop::GPUTextureFormat::kR8Snorm:
            out = wgpu::TextureFormat::R8Snorm;
            return true;
        case interop::GPUTextureFormat::kR8Uint:
            out = wgpu::TextureFormat::R8Uint;
            return true;
        case interop::GPUTextureFormat::kR8Sint:
            out = wgpu::TextureFormat::R8Sint;
            return true;
        case interop::GPUTextureFormat::kR16Uint:
            out = wgpu::TextureFormat::R16Uint;
            return true;
        case interop::GPUTextureFormat::kR16Sint:
            out = wgpu::TextureFormat::R16Sint;
            return true;
        case interop::GPUTextureFormat::kR16Float:
            out = wgpu::TextureFormat::R16Float;
            return true;
        case interop::GPUTextureFormat::kRg8Unorm:
            out = wgpu::TextureFormat::RG8Unorm;
            return true;
        case interop::GPUTextureFormat::kRg8Snorm:
            out = wgpu::TextureFormat::RG8Snorm;
            return true;
        case interop::GPUTextureFormat::kRg8Uint:
            out = wgpu::TextureFormat::RG8Uint;
            return true;
        case interop::GPUTextureFormat::kRg8Sint:
            out = wgpu::TextureFormat::RG8Sint;
            return true;
        case interop::GPUTextureFormat::kR32Uint:
            out = wgpu::TextureFormat::R32Uint;
            return true;
        case interop::GPUTextureFormat::kR32Sint:
            out = wgpu::TextureFormat::R32Sint;
            return true;
        case interop::GPUTextureFormat::kR32Float:
            out = wgpu::TextureFormat::R32Float;
            return true;
        case interop::GPUTextureFormat::kRg16Uint:
            out = wgpu::TextureFormat::RG16Uint;
            return true;
        case interop::GPUTextureFormat::kRg16Sint:
            out = wgpu::TextureFormat::RG16Sint;
            return true;
        case interop::GPUTextureFormat::kRg16Float:
            out = wgpu::TextureFormat::RG16Float;
            return true;
        case interop::GPUTextureFormat::kRgba8Unorm:
            out = wgpu::TextureFormat::RGBA8Unorm;
            return true;
        case interop::GPUTextureFormat::kRgba8UnormSrgb:
            out = wgpu::TextureFormat::RGBA8UnormSrgb;
            return true;
        case interop::GPUTextureFormat::kRgba8Snorm:
            out = wgpu::TextureFormat::RGBA8Snorm;
            return true;
        case interop::GPUTextureFormat::kRgba8Uint:
            out = wgpu::TextureFormat::RGBA8Uint;
            return true;
        case interop::GPUTextureFormat::kRgba8Sint:
            out = wgpu::TextureFormat::RGBA8Sint;
            return true;
        case interop::GPUTextureFormat::kBgra8Unorm:
            out = wgpu::TextureFormat::BGRA8Unorm;
            return true;
        case interop::GPUTextureFormat::kBgra8UnormSrgb:
            out = wgpu::TextureFormat::BGRA8UnormSrgb;
            return true;
        case interop::GPUTextureFormat::kRgb9E5Ufloat:
            out = wgpu::TextureFormat::RGB9E5Ufloat;
            return true;
        case interop::GPUTextureFormat::kRgb10A2Unorm:
            out = wgpu::TextureFormat::RGB10A2Unorm;
            return true;
        case interop::GPUTextureFormat::kRg11B10Ufloat:
            out = wgpu::TextureFormat::RG11B10Ufloat;
            return true;
        case interop::GPUTextureFormat::kRg32Uint:
            out = wgpu::TextureFormat::RG32Uint;
            return true;
        case interop::GPUTextureFormat::kRg32Sint:
            out = wgpu::TextureFormat::RG32Sint;
            return true;
        case interop::GPUTextureFormat::kRg32Float:
            out = wgpu::TextureFormat::RG32Float;
            return true;
        case interop::GPUTextureFormat::kRgba16Uint:
            out = wgpu::TextureFormat::RGBA16Uint;
            return true;
        case interop::GPUTextureFormat::kRgba16Sint:
            out = wgpu::TextureFormat::RGBA16Sint;
            return true;
        case interop::GPUTextureFormat::kRgba16Float:
            out = wgpu::TextureFormat::RGBA16Float;
            return true;
        case interop::GPUTextureFormat::kRgba32Uint:
            out = wgpu::TextureFormat::RGBA32Uint;
            return true;
        case interop::GPUTextureFormat::kRgba32Sint:
            out = wgpu::TextureFormat::RGBA32Sint;
            return true;
        case interop::GPUTextureFormat::kRgba32Float:
            out = wgpu::TextureFormat::RGBA32Float;
            return true;
        case interop::GPUTextureFormat::kStencil8:
            out = wgpu::TextureFormat::Stencil8;
            return true;
        case interop::GPUTextureFormat::kDepth16Unorm:
            out = wgpu::TextureFormat::Depth16Unorm;
            return true;
        case interop::GPUTextureFormat::kDepth24Plus:
            out = wgpu::TextureFormat::Depth24Plus;
            return true;
        case interop::GPUTextureFormat::kDepth24PlusStencil8:
            out = wgpu::TextureFormat::Depth24PlusStencil8;
            return true;
        case interop::GPUTextureFormat::kDepth32Float:
            out = wgpu::TextureFormat::Depth32Float;
            return true;

        case interop::GPUTextureFormat::kDepth32FloatStencil8:
            out = wgpu::TextureFormat::Depth32FloatStencil8;
            requiredFeature = wgpu::FeatureName::Depth32FloatStencil8;
            break;
        case interop::GPUTextureFormat::kBc1RgbaUnorm:
            out = wgpu::TextureFormat::BC1RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc1RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC1RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc2RgbaUnorm:
            out = wgpu::TextureFormat::BC2RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc2RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC2RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc3RgbaUnorm:
            out = wgpu::TextureFormat::BC3RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc3RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC3RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc4RUnorm:
            out = wgpu::TextureFormat::BC4RUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc4RSnorm:
            out = wgpu::TextureFormat::BC4RSnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc5RgUnorm:
            out = wgpu::TextureFormat::BC5RGUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc5RgSnorm:
            out = wgpu::TextureFormat::BC5RGSnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc6HRgbUfloat:
            out = wgpu::TextureFormat::BC6HRGBUfloat;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc6HRgbFloat:
            out = wgpu::TextureFormat::BC6HRGBFloat;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc7RgbaUnorm:
            out = wgpu::TextureFormat::BC7RGBAUnorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kBc7RgbaUnormSrgb:
            out = wgpu::TextureFormat::BC7RGBAUnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionBC;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8Unorm:
            out = wgpu::TextureFormat::ETC2RGB8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGB8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8A1Unorm:
            out = wgpu::TextureFormat::ETC2RGB8A1Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgb8A1UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgba8Unorm:
            out = wgpu::TextureFormat::ETC2RGBA8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEtc2Rgba8UnormSrgb:
            out = wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacR11Unorm:
            out = wgpu::TextureFormat::EACR11Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacR11Snorm:
            out = wgpu::TextureFormat::EACR11Snorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacRg11Unorm:
            out = wgpu::TextureFormat::EACRG11Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kEacRg11Snorm:
            out = wgpu::TextureFormat::EACRG11Snorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionETC2;
            break;
        case interop::GPUTextureFormat::kAstc4X4Unorm:
            out = wgpu::TextureFormat::ASTC4x4Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc4X4UnormSrgb:
            out = wgpu::TextureFormat::ASTC4x4UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X4Unorm:
            out = wgpu::TextureFormat::ASTC5x4Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X4UnormSrgb:
            out = wgpu::TextureFormat::ASTC5x4UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X5Unorm:
            out = wgpu::TextureFormat::ASTC5x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc5X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC5x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X5Unorm:
            out = wgpu::TextureFormat::ASTC6x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC6x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X6Unorm:
            out = wgpu::TextureFormat::ASTC6x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc6X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC6x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X5Unorm:
            out = wgpu::TextureFormat::ASTC8x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X6Unorm:
            out = wgpu::TextureFormat::ASTC8x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X8Unorm:
            out = wgpu::TextureFormat::ASTC8x8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc8X8UnormSrgb:
            out = wgpu::TextureFormat::ASTC8x8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X5Unorm:
            out = wgpu::TextureFormat::ASTC10x5Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X5UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x5UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X6Unorm:
            out = wgpu::TextureFormat::ASTC10x6Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X6UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x6UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X8Unorm:
            out = wgpu::TextureFormat::ASTC10x8Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X8UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x8UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X10Unorm:
            out = wgpu::TextureFormat::ASTC10x10Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc10X10UnormSrgb:
            out = wgpu::TextureFormat::ASTC10x10UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X10Unorm:
            out = wgpu::TextureFormat::ASTC12x10Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X10UnormSrgb:
            out = wgpu::TextureFormat::ASTC12x10UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X12Unorm:
            out = wgpu::TextureFormat::ASTC12x12Unorm;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;
        case interop::GPUTextureFormat::kAstc12X12UnormSrgb:
            out = wgpu::TextureFormat::ASTC12x12UnormSrgb;
            requiredFeature = wgpu::FeatureName::TextureCompressionASTC;
            break;

        default:
            Napi::Error::New(env, "invalid value for GPUTextureFormat")
                .ThrowAsJavaScriptException();
            return false;
    }

    assert(requiredFeature != wgpu::FeatureName::Undefined);
    if (!HasFeature(requiredFeature)) {
        Napi::TypeError::New(env, "invalid value for GPUTextureFormat")
            .ThrowAsJavaScriptException();
        return false;
    }

    return true;
}

bool Converter::Convert(interop::GPUTextureFormat& out, wgpu::TextureFormat in) {
    switch (in) {
        case wgpu::TextureFormat::R8Unorm:
            out = interop::GPUTextureFormat::kR8Unorm;
            return true;
        case wgpu::TextureFormat::R8Snorm:
            out = interop::GPUTextureFormat::kR8Snorm;
            return true;
        case wgpu::TextureFormat::R8Uint:
            out = interop::GPUTextureFormat::kR8Uint;
            return true;
        case wgpu::TextureFormat::R8Sint:
            out = interop::GPUTextureFormat::kR8Sint;
            return true;
        case wgpu::TextureFormat::R16Uint:
            out = interop::GPUTextureFormat::kR16Uint;
            return true;
        case wgpu::TextureFormat::R16Sint:
            out = interop::GPUTextureFormat::kR16Sint;
            return true;
        case wgpu::TextureFormat::R16Float:
            out = interop::GPUTextureFormat::kR16Float;
            return true;
        case wgpu::TextureFormat::RG8Unorm:
            out = interop::GPUTextureFormat::kRg8Unorm;
            return true;
        case wgpu::TextureFormat::RG8Snorm:
            out = interop::GPUTextureFormat::kRg8Snorm;
            return true;
        case wgpu::TextureFormat::RG8Uint:
            out = interop::GPUTextureFormat::kRg8Uint;
            return true;
        case wgpu::TextureFormat::RG8Sint:
            out = interop::GPUTextureFormat::kRg8Sint;
            return true;
        case wgpu::TextureFormat::R32Uint:
            out = interop::GPUTextureFormat::kR32Uint;
            return true;
        case wgpu::TextureFormat::R32Sint:
            out = interop::GPUTextureFormat::kR32Sint;
            return true;
        case wgpu::TextureFormat::R32Float:
            out = interop::GPUTextureFormat::kR32Float;
            return true;
        case wgpu::TextureFormat::RG16Uint:
            out = interop::GPUTextureFormat::kRg16Uint;
            return true;
        case wgpu::TextureFormat::RG16Sint:
            out = interop::GPUTextureFormat::kRg16Sint;
            return true;
        case wgpu::TextureFormat::RG16Float:
            out = interop::GPUTextureFormat::kRg16Float;
            return true;
        case wgpu::TextureFormat::RGBA8Unorm:
            out = interop::GPUTextureFormat::kRgba8Unorm;
            return true;
        case wgpu::TextureFormat::RGBA8UnormSrgb:
            out = interop::GPUTextureFormat::kRgba8UnormSrgb;
            return true;
        case wgpu::TextureFormat::RGBA8Snorm:
            out = interop::GPUTextureFormat::kRgba8Snorm;
            return true;
        case wgpu::TextureFormat::RGBA8Uint:
            out = interop::GPUTextureFormat::kRgba8Uint;
            return true;
        case wgpu::TextureFormat::RGBA8Sint:
            out = interop::GPUTextureFormat::kRgba8Sint;
            return true;
        case wgpu::TextureFormat::BGRA8Unorm:
            out = interop::GPUTextureFormat::kBgra8Unorm;
            return true;
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            out = interop::GPUTextureFormat::kBgra8UnormSrgb;
            return true;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            out = interop::GPUTextureFormat::kRgb9E5Ufloat;
            return true;
        case wgpu::TextureFormat::RGB10A2Unorm:
            out = interop::GPUTextureFormat::kRgb10A2Unorm;
            return true;
        case wgpu::TextureFormat::RG11B10Ufloat:
            out = interop::GPUTextureFormat::kRg11B10Ufloat;
            return true;
        case wgpu::TextureFormat::RG32Uint:
            out = interop::GPUTextureFormat::kRg32Uint;
            return true;
        case wgpu::TextureFormat::RG32Sint:
            out = interop::GPUTextureFormat::kRg32Sint;
            return true;
        case wgpu::TextureFormat::RG32Float:
            out = interop::GPUTextureFormat::kRg32Float;
            return true;
        case wgpu::TextureFormat::RGBA16Uint:
            out = interop::GPUTextureFormat::kRgba16Uint;
            return true;
        case wgpu::TextureFormat::RGBA16Sint:
            out = interop::GPUTextureFormat::kRgba16Sint;
            return true;
        case wgpu::TextureFormat::RGBA16Float:
            out = interop::GPUTextureFormat::kRgba16Float;
            return true;
        case wgpu::TextureFormat::RGBA32Uint:
            out = interop::GPUTextureFormat::kRgba32Uint;
            return true;
        case wgpu::TextureFormat::RGBA32Sint:
            out = interop::GPUTextureFormat::kRgba32Sint;
            return true;
        case wgpu::TextureFormat::RGBA32Float:
            out = interop::GPUTextureFormat::kRgba32Float;
            return true;
        case wgpu::TextureFormat::Stencil8:
            out = interop::GPUTextureFormat::kStencil8;
            return true;
        case wgpu::TextureFormat::Depth16Unorm:
            out = interop::GPUTextureFormat::kDepth16Unorm;
            return true;
        case wgpu::TextureFormat::Depth24Plus:
            out = interop::GPUTextureFormat::kDepth24Plus;
            return true;
        case wgpu::TextureFormat::Depth24PlusStencil8:
            out = interop::GPUTextureFormat::kDepth24PlusStencil8;
            return true;
        case wgpu::TextureFormat::Depth32Float:
            out = interop::GPUTextureFormat::kDepth32Float;
            return true;
        case wgpu::TextureFormat::Depth32FloatStencil8:
            out = interop::GPUTextureFormat::kDepth32FloatStencil8;
            return true;
        case wgpu::TextureFormat::BC1RGBAUnorm:
            out = interop::GPUTextureFormat::kBc1RgbaUnorm;
            return true;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            out = interop::GPUTextureFormat::kBc1RgbaUnormSrgb;
            return true;
        case wgpu::TextureFormat::BC2RGBAUnorm:
            out = interop::GPUTextureFormat::kBc2RgbaUnorm;
            return true;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            out = interop::GPUTextureFormat::kBc2RgbaUnormSrgb;
            return true;
        case wgpu::TextureFormat::BC3RGBAUnorm:
            out = interop::GPUTextureFormat::kBc3RgbaUnorm;
            return true;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            out = interop::GPUTextureFormat::kBc3RgbaUnormSrgb;
            return true;
        case wgpu::TextureFormat::BC4RUnorm:
            out = interop::GPUTextureFormat::kBc4RUnorm;
            return true;
        case wgpu::TextureFormat::BC4RSnorm:
            out = interop::GPUTextureFormat::kBc4RSnorm;
            return true;
        case wgpu::TextureFormat::BC5RGUnorm:
            out = interop::GPUTextureFormat::kBc5RgUnorm;
            return true;
        case wgpu::TextureFormat::BC5RGSnorm:
            out = interop::GPUTextureFormat::kBc5RgSnorm;
            return true;
        case wgpu::TextureFormat::BC6HRGBUfloat:
            out = interop::GPUTextureFormat::kBc6HRgbUfloat;
            return true;
        case wgpu::TextureFormat::BC6HRGBFloat:
            out = interop::GPUTextureFormat::kBc6HRgbFloat;
            return true;
        case wgpu::TextureFormat::BC7RGBAUnorm:
            out = interop::GPUTextureFormat::kBc7RgbaUnorm;
            return true;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
            out = interop::GPUTextureFormat::kBc7RgbaUnormSrgb;
            return true;
        case wgpu::TextureFormat::ETC2RGB8Unorm:
            out = interop::GPUTextureFormat::kEtc2Rgb8Unorm;
            return true;
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            out = interop::GPUTextureFormat::kEtc2Rgb8UnormSrgb;
            return true;
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
            out = interop::GPUTextureFormat::kEtc2Rgb8A1Unorm;
            return true;
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
            out = interop::GPUTextureFormat::kEtc2Rgb8A1UnormSrgb;
            return true;
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
            out = interop::GPUTextureFormat::kEtc2Rgba8Unorm;
            return true;
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
            out = interop::GPUTextureFormat::kEtc2Rgba8UnormSrgb;
            return true;
        case wgpu::TextureFormat::EACR11Unorm:
            out = interop::GPUTextureFormat::kEacR11Unorm;
            return true;
        case wgpu::TextureFormat::EACR11Snorm:
            out = interop::GPUTextureFormat::kEacR11Snorm;
            return true;
        case wgpu::TextureFormat::EACRG11Unorm:
            out = interop::GPUTextureFormat::kEacRg11Unorm;
            return true;
        case wgpu::TextureFormat::EACRG11Snorm:
            out = interop::GPUTextureFormat::kEacRg11Snorm;
            return true;
        case wgpu::TextureFormat::ASTC4x4Unorm:
            out = interop::GPUTextureFormat::kAstc4X4Unorm;
            return true;
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            out = interop::GPUTextureFormat::kAstc4X4UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC5x4Unorm:
            out = interop::GPUTextureFormat::kAstc5X4Unorm;
            return true;
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            out = interop::GPUTextureFormat::kAstc5X4UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC5x5Unorm:
            out = interop::GPUTextureFormat::kAstc5X5Unorm;
            return true;
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            out = interop::GPUTextureFormat::kAstc5X5UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC6x5Unorm:
            out = interop::GPUTextureFormat::kAstc6X5Unorm;
            return true;
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            out = interop::GPUTextureFormat::kAstc6X5UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC6x6Unorm:
            out = interop::GPUTextureFormat::kAstc6X6Unorm;
            return true;
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            out = interop::GPUTextureFormat::kAstc6X6UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC8x5Unorm:
            out = interop::GPUTextureFormat::kAstc8X5Unorm;
            return true;
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            out = interop::GPUTextureFormat::kAstc8X5UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC8x6Unorm:
            out = interop::GPUTextureFormat::kAstc8X6Unorm;
            return true;
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            out = interop::GPUTextureFormat::kAstc8X6UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC8x8Unorm:
            out = interop::GPUTextureFormat::kAstc8X8Unorm;
            return true;
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            out = interop::GPUTextureFormat::kAstc8X8UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC10x5Unorm:
            out = interop::GPUTextureFormat::kAstc10X5Unorm;
            return true;
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            out = interop::GPUTextureFormat::kAstc10X5UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC10x6Unorm:
            out = interop::GPUTextureFormat::kAstc10X6Unorm;
            return true;
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            out = interop::GPUTextureFormat::kAstc10X6UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC10x8Unorm:
            out = interop::GPUTextureFormat::kAstc10X8Unorm;
            return true;
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            out = interop::GPUTextureFormat::kAstc10X8UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC10x10Unorm:
            out = interop::GPUTextureFormat::kAstc10X10Unorm;
            return true;
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            out = interop::GPUTextureFormat::kAstc10X10UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC12x10Unorm:
            out = interop::GPUTextureFormat::kAstc12X10Unorm;
            return true;
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            out = interop::GPUTextureFormat::kAstc12X10UnormSrgb;
            return true;
        case wgpu::TextureFormat::ASTC12x12Unorm:
            out = interop::GPUTextureFormat::kAstc12X12Unorm;
            return true;
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            out = interop::GPUTextureFormat::kAstc12X12UnormSrgb;
            return true;

        case wgpu::TextureFormat::Undefined:
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return false;
    }

    return false;
}

bool Converter::Convert(wgpu::TextureUsage& out, const interop::GPUTextureUsageFlags& in) {
    out = static_cast<wgpu::TextureUsage>(in.value);
    return true;
}

bool Converter::Convert(interop::GPUTextureUsageFlags& out, wgpu::TextureUsage in) {
    out = interop::GPUTextureUsageFlags(static_cast<uint32_t>(out));
    return true;
}

bool Converter::Convert(wgpu::ColorWriteMask& out, const interop::GPUColorWriteFlags& in) {
    out = static_cast<wgpu::ColorWriteMask>(in.value);
    return true;
}

bool Converter::Convert(wgpu::BufferUsage& out, const interop::GPUBufferUsageFlags& in) {
    out = static_cast<wgpu::BufferUsage>(in.value);
    return true;
}

bool Converter::Convert(interop::GPUBufferUsageFlags& out, wgpu::BufferUsage in) {
    out = interop::GPUBufferUsageFlags(static_cast<uint32_t>(out));
    return true;
}

bool Converter::Convert(wgpu::MapMode& out, const interop::GPUMapModeFlags& in) {
    out = static_cast<wgpu::MapMode>(in.value);
    return true;
}

bool Converter::Convert(wgpu::ShaderStage& out, const interop::GPUShaderStageFlags& in) {
    out = static_cast<wgpu::ShaderStage>(in.value);
    return true;
}

bool Converter::Convert(wgpu::TextureDimension& out, const interop::GPUTextureDimension& in) {
    out = wgpu::TextureDimension::e1D;
    switch (in) {
        case interop::GPUTextureDimension::k1D:
            out = wgpu::TextureDimension::e1D;
            return true;
        case interop::GPUTextureDimension::k2D:
            out = wgpu::TextureDimension::e2D;
            return true;
        case interop::GPUTextureDimension::k3D:
            out = wgpu::TextureDimension::e3D;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUTextureDimension").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(interop::GPUTextureDimension& out, wgpu::TextureDimension in) {
    switch (in) {
        case wgpu::TextureDimension::e1D:
            out = interop::GPUTextureDimension::k1D;
            return true;
        case wgpu::TextureDimension::e2D:
            out = interop::GPUTextureDimension::k2D;
            return true;
        case wgpu::TextureDimension::e3D:
            out = interop::GPUTextureDimension::k3D;
            return true;
    }
    return false;
}

bool Converter::Convert(wgpu::TextureViewDimension& out,
                        const interop::GPUTextureViewDimension& in) {
    out = wgpu::TextureViewDimension::Undefined;
    switch (in) {
        case interop::GPUTextureViewDimension::k1D:
            out = wgpu::TextureViewDimension::e1D;
            return true;
        case interop::GPUTextureViewDimension::k2D:
            out = wgpu::TextureViewDimension::e2D;
            return true;
        case interop::GPUTextureViewDimension::k2DArray:
            out = wgpu::TextureViewDimension::e2DArray;
            return true;
        case interop::GPUTextureViewDimension::kCube:
            out = wgpu::TextureViewDimension::Cube;
            return true;
        case interop::GPUTextureViewDimension::kCubeArray:
            out = wgpu::TextureViewDimension::CubeArray;
            return true;
        case interop::GPUTextureViewDimension::k3D:
            out = wgpu::TextureViewDimension::e3D;
            return true;
        default:
            break;
    }
    Napi::Error::New(env, "invalid value for GPUTextureViewDimension").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::ProgrammableStageDescriptor& out,
                        const interop::GPUProgrammableStage& in) {
    out = {};
    out.module = *in.module.As<GPUShaderModule>();

    // Replace nulls in the entryPoint name with another character that's disallowed in
    // identifiers. This is so that using "main\0" doesn't match an entryPoint named "main".
    // TODO(dawn:1345): Replace with a way to size strings explicitly in webgpu.h
    char* entryPoint = Allocate<char>(in.entryPoint.size() + 1);
    entryPoint[in.entryPoint.size()] = '\0';
    for (size_t i = 0; i < in.entryPoint.size(); i++) {
        if (in.entryPoint[i] == '\0') {
            entryPoint[i] = '#';
        } else {
            entryPoint[i] = in.entryPoint[i];
        }
    }
    out.entryPoint = entryPoint;

    return Convert(out.constants, out.constantCount, in.constants);
}

bool Converter::Convert(wgpu::ConstantEntry& out,
                        const std::string& in_name,
                        wgpu::interop::GPUPipelineConstantValue in_value) {
    out.key = in_name.c_str();
    out.value = in_value;
    return true;
}

bool Converter::Convert(wgpu::BlendComponent& out, const interop::GPUBlendComponent& in) {
    out = {};
    return Convert(out.operation, in.operation) && Convert(out.dstFactor, in.dstFactor) &&
           Convert(out.srcFactor, in.srcFactor);
}

bool Converter::Convert(wgpu::BlendFactor& out, const interop::GPUBlendFactor& in) {
    out = wgpu::BlendFactor::Zero;
    switch (in) {
        case interop::GPUBlendFactor::kZero:
            out = wgpu::BlendFactor::Zero;
            return true;
        case interop::GPUBlendFactor::kOne:
            out = wgpu::BlendFactor::One;
            return true;
        case interop::GPUBlendFactor::kSrc:
            out = wgpu::BlendFactor::Src;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrc:
            out = wgpu::BlendFactor::OneMinusSrc;
            return true;
        case interop::GPUBlendFactor::kSrcAlpha:
            out = wgpu::BlendFactor::SrcAlpha;
            return true;
        case interop::GPUBlendFactor::kOneMinusSrcAlpha:
            out = wgpu::BlendFactor::OneMinusSrcAlpha;
            return true;
        case interop::GPUBlendFactor::kDst:
            out = wgpu::BlendFactor::Dst;
            return true;
        case interop::GPUBlendFactor::kOneMinusDst:
            out = wgpu::BlendFactor::OneMinusDst;
            return true;
        case interop::GPUBlendFactor::kDstAlpha:
            out = wgpu::BlendFactor::DstAlpha;
            return true;
        case interop::GPUBlendFactor::kOneMinusDstAlpha:
            out = wgpu::BlendFactor::OneMinusDstAlpha;
            return true;
        case interop::GPUBlendFactor::kSrcAlphaSaturated:
            out = wgpu::BlendFactor::SrcAlphaSaturated;
            return true;
        case interop::GPUBlendFactor::kConstant:
            out = wgpu::BlendFactor::Constant;
            return true;
        case interop::GPUBlendFactor::kOneMinusConstant:
            out = wgpu::BlendFactor::OneMinusConstant;
            return true;
        default:
            break;
    }
    Napi::Error::New(env, "invalid value for GPUBlendFactor").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::BlendOperation& out, const interop::GPUBlendOperation& in) {
    out = wgpu::BlendOperation::Add;
    switch (in) {
        case interop::GPUBlendOperation::kAdd:
            out = wgpu::BlendOperation::Add;
            return true;
        case interop::GPUBlendOperation::kSubtract:
            out = wgpu::BlendOperation::Subtract;
            return true;
        case interop::GPUBlendOperation::kReverseSubtract:
            out = wgpu::BlendOperation::ReverseSubtract;
            return true;
        case interop::GPUBlendOperation::kMin:
            out = wgpu::BlendOperation::Min;
            return true;
        case interop::GPUBlendOperation::kMax:
            out = wgpu::BlendOperation::Max;
            return true;
        default:
            break;
    }
    Napi::Error::New(env, "invalid value for GPUBlendOperation").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::BlendState& out, const interop::GPUBlendState& in) {
    out = {};
    return Convert(out.alpha, in.alpha) && Convert(out.color, in.color);
}

bool Converter::Convert(wgpu::PrimitiveState& out, const interop::GPUPrimitiveState& in) {
    out = {};

    if (in.unclippedDepth) {
        wgpu::PrimitiveDepthClipControl* depthClip = Allocate<wgpu::PrimitiveDepthClipControl>();
        depthClip->unclippedDepth = true;
        out.nextInChain = depthClip;
    }

    return Convert(out.topology, in.topology) &&
           Convert(out.stripIndexFormat, in.stripIndexFormat) &&
           Convert(out.frontFace, in.frontFace) && Convert(out.cullMode, in.cullMode);
}

bool Converter::Convert(wgpu::ColorTargetState& out, const interop::GPUColorTargetState& in) {
    out = {};
    return Convert(out.format, in.format) && Convert(out.blend, in.blend) &&
           Convert(out.writeMask, in.writeMask);
}

bool Converter::Convert(wgpu::DepthStencilState& out, const interop::GPUDepthStencilState& in) {
    out = {};
    return Convert(out.format, in.format) && Convert(out.depthWriteEnabled, in.depthWriteEnabled) &&
           Convert(out.depthCompare, in.depthCompare) &&
           Convert(out.stencilFront, in.stencilFront) && Convert(out.stencilBack, in.stencilBack) &&
           Convert(out.stencilReadMask, in.stencilReadMask) &&
           Convert(out.stencilWriteMask, in.stencilWriteMask) &&
           Convert(out.depthBias, in.depthBias) &&
           Convert(out.depthBiasSlopeScale, in.depthBiasSlopeScale) &&
           Convert(out.depthBiasClamp, in.depthBiasClamp);
}

bool Converter::Convert(wgpu::MultisampleState& out, const interop::GPUMultisampleState& in) {
    out = {};
    return Convert(out.count, in.count) && Convert(out.mask, in.mask) &&
           Convert(out.alphaToCoverageEnabled, in.alphaToCoverageEnabled);
}

bool Converter::Convert(wgpu::FragmentState& out, const interop::GPUFragmentState& in) {
    out = {};
    return Convert(out.targets, out.targetCount, in.targets) &&  //
           Convert(out.module, in.module) &&                     //
           Convert(out.entryPoint, in.entryPoint) &&             //
           Convert(out.constants, out.constantCount, in.constants);
}

bool Converter::Convert(wgpu::PrimitiveTopology& out, const interop::GPUPrimitiveTopology& in) {
    out = wgpu::PrimitiveTopology::LineList;
    switch (in) {
        case interop::GPUPrimitiveTopology::kPointList:
            out = wgpu::PrimitiveTopology::PointList;
            return true;
        case interop::GPUPrimitiveTopology::kLineList:
            out = wgpu::PrimitiveTopology::LineList;
            return true;
        case interop::GPUPrimitiveTopology::kLineStrip:
            out = wgpu::PrimitiveTopology::LineStrip;
            return true;
        case interop::GPUPrimitiveTopology::kTriangleList:
            out = wgpu::PrimitiveTopology::TriangleList;
            return true;
        case interop::GPUPrimitiveTopology::kTriangleStrip:
            out = wgpu::PrimitiveTopology::TriangleStrip;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUPrimitiveTopology").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::FrontFace& out, const interop::GPUFrontFace& in) {
    out = wgpu::FrontFace::CW;
    switch (in) {
        case interop::GPUFrontFace::kCw:
            out = wgpu::FrontFace::CW;
            return true;
        case interop::GPUFrontFace::kCcw:
            out = wgpu::FrontFace::CCW;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUFrontFace").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::CullMode& out, const interop::GPUCullMode& in) {
    out = wgpu::CullMode::None;
    switch (in) {
        case interop::GPUCullMode::kNone:
            out = wgpu::CullMode::None;
            return true;
        case interop::GPUCullMode::kFront:
            out = wgpu::CullMode::Front;
            return true;
        case interop::GPUCullMode::kBack:
            out = wgpu::CullMode::Back;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUCullMode").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::CompareFunction& out, const interop::GPUCompareFunction& in) {
    out = wgpu::CompareFunction::Undefined;
    switch (in) {
        case interop::GPUCompareFunction::kNever:
            out = wgpu::CompareFunction::Never;
            return true;
        case interop::GPUCompareFunction::kLess:
            out = wgpu::CompareFunction::Less;
            return true;
        case interop::GPUCompareFunction::kLessEqual:
            out = wgpu::CompareFunction::LessEqual;
            return true;
        case interop::GPUCompareFunction::kGreater:
            out = wgpu::CompareFunction::Greater;
            return true;
        case interop::GPUCompareFunction::kGreaterEqual:
            out = wgpu::CompareFunction::GreaterEqual;
            return true;
        case interop::GPUCompareFunction::kEqual:
            out = wgpu::CompareFunction::Equal;
            return true;
        case interop::GPUCompareFunction::kNotEqual:
            out = wgpu::CompareFunction::NotEqual;
            return true;
        case interop::GPUCompareFunction::kAlways:
            out = wgpu::CompareFunction::Always;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUCompareFunction").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::IndexFormat& out, const interop::GPUIndexFormat& in) {
    out = wgpu::IndexFormat::Undefined;
    switch (in) {
        case interop::GPUIndexFormat::kUint16:
            out = wgpu::IndexFormat::Uint16;
            return true;
        case interop::GPUIndexFormat::kUint32:
            out = wgpu::IndexFormat::Uint32;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUIndexFormat").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::StencilOperation& out, const interop::GPUStencilOperation& in) {
    out = wgpu::StencilOperation::Zero;
    switch (in) {
        case interop::GPUStencilOperation::kKeep:
            out = wgpu::StencilOperation::Keep;
            return true;
        case interop::GPUStencilOperation::kZero:
            out = wgpu::StencilOperation::Zero;
            return true;
        case interop::GPUStencilOperation::kReplace:
            out = wgpu::StencilOperation::Replace;
            return true;
        case interop::GPUStencilOperation::kInvert:
            out = wgpu::StencilOperation::Invert;
            return true;
        case interop::GPUStencilOperation::kIncrementClamp:
            out = wgpu::StencilOperation::IncrementClamp;
            return true;
        case interop::GPUStencilOperation::kDecrementClamp:
            out = wgpu::StencilOperation::DecrementClamp;
            return true;
        case interop::GPUStencilOperation::kIncrementWrap:
            out = wgpu::StencilOperation::IncrementWrap;
            return true;
        case interop::GPUStencilOperation::kDecrementWrap:
            out = wgpu::StencilOperation::DecrementWrap;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUStencilOperation").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::StencilFaceState& out, const interop::GPUStencilFaceState& in) {
    return Convert(out.compare, in.compare) && Convert(out.failOp, in.failOp) &&
           Convert(out.depthFailOp, in.depthFailOp) && Convert(out.passOp, in.passOp);
}

bool Converter::Convert(wgpu::VertexBufferLayout& out, const interop::GPUVertexBufferLayout& in) {
    out = {};
    return Convert(out.attributes, out.attributeCount, in.attributes) &&
           Convert(out.arrayStride, in.arrayStride) && Convert(out.stepMode, in.stepMode);
}

bool Converter::Convert(wgpu::VertexState& out, const interop::GPUVertexState& in) {
    out = {};
    wgpu::VertexBufferLayout* outBuffers = nullptr;
    if (!Convert(out.module, in.module) ||                    //
        !Convert(outBuffers, out.bufferCount, in.buffers) ||  //
        !Convert(out.entryPoint, in.entryPoint) ||            //
        !Convert(out.constants, out.constantCount, in.constants)) {
        return false;
    }

    // Patch up the unused vertex buffer layouts to use wgpu::VertexStepMode::VertexBufferNotUsed.
    // The converter for optional value will have put the default value of wgpu::VertexBufferLayout
    // that has wgpu::VertexStepMode::Vertex.
    out.buffers = outBuffers;
    for (size_t i = 0; i < in.buffers.size(); i++) {
        if (!in.buffers[i].has_value()) {
            outBuffers[i].stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
        }
    }

    return true;
}

bool Converter::Convert(wgpu::VertexStepMode& out, const interop::GPUVertexStepMode& in) {
    out = wgpu::VertexStepMode::Instance;
    switch (in) {
        case interop::GPUVertexStepMode::kInstance:
            out = wgpu::VertexStepMode::Instance;
            return true;
        case interop::GPUVertexStepMode::kVertex:
            out = wgpu::VertexStepMode::Vertex;
            return true;
        default:
            break;
    }
    Napi::Error::New(env, "invalid value for GPUVertexStepMode").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::VertexAttribute& out, const interop::GPUVertexAttribute& in) {
    return Convert(out.format, in.format) && Convert(out.offset, in.offset) &&
           Convert(out.shaderLocation, in.shaderLocation);
}

bool Converter::Convert(wgpu::VertexFormat& out, const interop::GPUVertexFormat& in) {
    out = wgpu::VertexFormat::Undefined;
    switch (in) {
        case interop::GPUVertexFormat::kUint8X2:
            out = wgpu::VertexFormat::Uint8x2;
            return true;
        case interop::GPUVertexFormat::kUint8X4:
            out = wgpu::VertexFormat::Uint8x4;
            return true;
        case interop::GPUVertexFormat::kSint8X2:
            out = wgpu::VertexFormat::Sint8x2;
            return true;
        case interop::GPUVertexFormat::kSint8X4:
            out = wgpu::VertexFormat::Sint8x4;
            return true;
        case interop::GPUVertexFormat::kUnorm8X2:
            out = wgpu::VertexFormat::Unorm8x2;
            return true;
        case interop::GPUVertexFormat::kUnorm8X4:
            out = wgpu::VertexFormat::Unorm8x4;
            return true;
        case interop::GPUVertexFormat::kSnorm8X2:
            out = wgpu::VertexFormat::Snorm8x2;
            return true;
        case interop::GPUVertexFormat::kSnorm8X4:
            out = wgpu::VertexFormat::Snorm8x4;
            return true;
        case interop::GPUVertexFormat::kUint16X2:
            out = wgpu::VertexFormat::Uint16x2;
            return true;
        case interop::GPUVertexFormat::kUint16X4:
            out = wgpu::VertexFormat::Uint16x4;
            return true;
        case interop::GPUVertexFormat::kSint16X2:
            out = wgpu::VertexFormat::Sint16x2;
            return true;
        case interop::GPUVertexFormat::kSint16X4:
            out = wgpu::VertexFormat::Sint16x4;
            return true;
        case interop::GPUVertexFormat::kUnorm16X2:
            out = wgpu::VertexFormat::Unorm16x2;
            return true;
        case interop::GPUVertexFormat::kUnorm16X4:
            out = wgpu::VertexFormat::Unorm16x4;
            return true;
        case interop::GPUVertexFormat::kSnorm16X2:
            out = wgpu::VertexFormat::Snorm16x2;
            return true;
        case interop::GPUVertexFormat::kSnorm16X4:
            out = wgpu::VertexFormat::Snorm16x4;
            return true;
        case interop::GPUVertexFormat::kFloat16X2:
            out = wgpu::VertexFormat::Float16x2;
            return true;
        case interop::GPUVertexFormat::kFloat16X4:
            out = wgpu::VertexFormat::Float16x4;
            return true;
        case interop::GPUVertexFormat::kFloat32:
            out = wgpu::VertexFormat::Float32;
            return true;
        case interop::GPUVertexFormat::kFloat32X2:
            out = wgpu::VertexFormat::Float32x2;
            return true;
        case interop::GPUVertexFormat::kFloat32X3:
            out = wgpu::VertexFormat::Float32x3;
            return true;
        case interop::GPUVertexFormat::kFloat32X4:
            out = wgpu::VertexFormat::Float32x4;
            return true;
        case interop::GPUVertexFormat::kUint32:
            out = wgpu::VertexFormat::Uint32;
            return true;
        case interop::GPUVertexFormat::kUint32X2:
            out = wgpu::VertexFormat::Uint32x2;
            return true;
        case interop::GPUVertexFormat::kUint32X3:
            out = wgpu::VertexFormat::Uint32x3;
            return true;
        case interop::GPUVertexFormat::kUint32X4:
            out = wgpu::VertexFormat::Uint32x4;
            return true;
        case interop::GPUVertexFormat::kSint32:
            out = wgpu::VertexFormat::Sint32;
            return true;
        case interop::GPUVertexFormat::kSint32X2:
            out = wgpu::VertexFormat::Sint32x2;
            return true;
        case interop::GPUVertexFormat::kSint32X3:
            out = wgpu::VertexFormat::Sint32x3;
            return true;
        case interop::GPUVertexFormat::kSint32X4:
            out = wgpu::VertexFormat::Sint32x4;
            return true;
        default:
            break;
    }
    Napi::Error::New(env, "invalid value for GPUVertexFormat").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::RenderPassColorAttachment& out,
                        const interop::GPURenderPassColorAttachment& in) {
    out = {};
    return Convert(out.view, in.view) &&                    //
           Convert(out.resolveTarget, in.resolveTarget) &&  //
           Convert(out.clearValue, in.clearValue) &&        //
           Convert(out.loadOp, in.loadOp) &&                //
           Convert(out.storeOp, in.storeOp);
}

bool Converter::Convert(wgpu::RenderPassDepthStencilAttachment& out,
                        const interop::GPURenderPassDepthStencilAttachment& in) {
    out = {};
    return Convert(out.view, in.view) &&                            //
           Convert(out.depthClearValue, in.depthClearValue) &&      //
           Convert(out.depthLoadOp, in.depthLoadOp) &&              //
           Convert(out.depthStoreOp, in.depthStoreOp) &&            //
           Convert(out.depthReadOnly, in.depthReadOnly) &&          //
           Convert(out.stencilClearValue, in.stencilClearValue) &&  //
           Convert(out.stencilLoadOp, in.stencilLoadOp) &&          //
           Convert(out.stencilStoreOp, in.stencilStoreOp) &&        //
           Convert(out.stencilReadOnly, in.stencilReadOnly);
}

bool Converter::Convert(wgpu::LoadOp& out, const interop::GPULoadOp& in) {
    out = wgpu::LoadOp::Clear;
    switch (in) {
        case interop::GPULoadOp::kLoad:
            out = wgpu::LoadOp::Load;
            return true;
        case interop::GPULoadOp::kClear:
            out = wgpu::LoadOp::Clear;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPULoadOp").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::StoreOp& out, const interop::GPUStoreOp& in) {
    out = wgpu::StoreOp::Store;
    switch (in) {
        case interop::GPUStoreOp::kStore:
            out = wgpu::StoreOp::Store;
            return true;
        case interop::GPUStoreOp::kDiscard:
            out = wgpu::StoreOp::Discard;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUStoreOp").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::BindGroupEntry& out, const interop::GPUBindGroupEntry& in) {
    out = {};
    if (!Convert(out.binding, in.binding)) {
        return false;
    }

    if (auto* res = std::get_if<interop::Interface<interop::GPUSampler>>(&in.resource)) {
        return Convert(out.sampler, *res);
    }
    if (auto* res = std::get_if<interop::Interface<interop::GPUTextureView>>(&in.resource)) {
        return Convert(out.textureView, *res);
    }
    if (auto* res = std::get_if<interop::GPUBufferBinding>(&in.resource)) {
        auto buffer = res->buffer.As<GPUBuffer>();
        out.size = wgpu::kWholeSize;
        if (!buffer || !Convert(out.offset, res->offset) || !Convert(out.size, res->size)) {
            return false;
        }
        out.buffer = *buffer;
        return true;
    }
    if (auto* res = std::get_if<interop::Interface<interop::GPUExternalTexture>>(&in.resource)) {
        // TODO(crbug.com/dawn/1129): External textures
        UNIMPLEMENTED(env, {});
    }
    Napi::Error::New(env, "invalid value for GPUBindGroupEntry.resource")
        .ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::BindGroupLayoutEntry& out,
                        const interop::GPUBindGroupLayoutEntry& in) {
    // TODO(crbug.com/dawn/1129): External textures
    return Convert(out.binding, in.binding) && Convert(out.visibility, in.visibility) &&
           Convert(out.buffer, in.buffer) && Convert(out.sampler, in.sampler) &&
           Convert(out.texture, in.texture) && Convert(out.storageTexture, in.storageTexture);
}

bool Converter::Convert(wgpu::BufferBindingLayout& out, const interop::GPUBufferBindingLayout& in) {
    return Convert(out.type, in.type) && Convert(out.hasDynamicOffset, in.hasDynamicOffset) &&
           Convert(out.minBindingSize, in.minBindingSize);
}

bool Converter::Convert(wgpu::SamplerBindingLayout& out,
                        const interop::GPUSamplerBindingLayout& in) {
    return Convert(out.type, in.type);
}

bool Converter::Convert(wgpu::TextureBindingLayout& out,
                        const interop::GPUTextureBindingLayout& in) {
    return Convert(out.sampleType, in.sampleType) && Convert(out.viewDimension, in.viewDimension) &&
           Convert(out.multisampled, in.multisampled);
}

bool Converter::Convert(wgpu::StorageTextureBindingLayout& out,
                        const interop::GPUStorageTextureBindingLayout& in) {
    return Convert(out.access, in.access) && Convert(out.format, in.format) &&
           Convert(out.viewDimension, in.viewDimension);
}

bool Converter::Convert(wgpu::BufferBindingType& out, const interop::GPUBufferBindingType& in) {
    out = wgpu::BufferBindingType::Undefined;
    switch (in) {
        case interop::GPUBufferBindingType::kUniform:
            out = wgpu::BufferBindingType::Uniform;
            return true;
        case interop::GPUBufferBindingType::kStorage:
            out = wgpu::BufferBindingType::Storage;
            return true;
        case interop::GPUBufferBindingType::kReadOnlyStorage:
            out = wgpu::BufferBindingType::ReadOnlyStorage;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUBufferBindingType").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::TextureSampleType& out, const interop::GPUTextureSampleType& in) {
    out = wgpu::TextureSampleType::Undefined;
    switch (in) {
        case interop::GPUTextureSampleType::kFloat:
            out = wgpu::TextureSampleType::Float;
            return true;
        case interop::GPUTextureSampleType::kUnfilterableFloat:
            out = wgpu::TextureSampleType::UnfilterableFloat;
            return true;
        case interop::GPUTextureSampleType::kDepth:
            out = wgpu::TextureSampleType::Depth;
            return true;
        case interop::GPUTextureSampleType::kSint:
            out = wgpu::TextureSampleType::Sint;
            return true;
        case interop::GPUTextureSampleType::kUint:
            out = wgpu::TextureSampleType::Uint;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUTextureSampleType").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::SamplerBindingType& out, const interop::GPUSamplerBindingType& in) {
    out = wgpu::SamplerBindingType::Undefined;
    switch (in) {
        case interop::GPUSamplerBindingType::kFiltering:
            out = wgpu::SamplerBindingType::Filtering;
            return true;
        case interop::GPUSamplerBindingType::kNonFiltering:
            out = wgpu::SamplerBindingType::NonFiltering;
            return true;
        case interop::GPUSamplerBindingType::kComparison:
            out = wgpu::SamplerBindingType::Comparison;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUSamplerBindingType").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::StorageTextureAccess& out,
                        const interop::GPUStorageTextureAccess& in) {
    out = wgpu::StorageTextureAccess::Undefined;
    switch (in) {
        case interop::GPUStorageTextureAccess::kWriteOnly:
            out = wgpu::StorageTextureAccess::WriteOnly;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUStorageTextureAccess").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::QueryType& out, const interop::GPUQueryType& in) {
    out = wgpu::QueryType::Occlusion;
    switch (in) {
        case interop::GPUQueryType::kOcclusion:
            out = wgpu::QueryType::Occlusion;
            return true;
        case interop::GPUQueryType::kTimestamp:
            if (HasFeature(wgpu::FeatureName::TimestampQuery)) {
                out = wgpu::QueryType::Timestamp;
                return true;
            } else {
                Napi::TypeError::New(env, "invalid value for GPUQueryType")
                    .ThrowAsJavaScriptException();
                return false;
            }
    }
    Napi::Error::New(env, "invalid value for GPUQueryType").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::FeatureName& out, interop::GPUFeatureName in) {
    switch (in) {
        case interop::GPUFeatureName::kTextureCompressionBc:
            out = wgpu::FeatureName::TextureCompressionBC;
            return true;
        case interop::GPUFeatureName::kTextureCompressionEtc2:
            out = wgpu::FeatureName::TextureCompressionETC2;
            return true;
        case interop::GPUFeatureName::kTextureCompressionAstc:
            out = wgpu::FeatureName::TextureCompressionASTC;
            return true;
        case interop::GPUFeatureName::kTimestampQuery:
            out = wgpu::FeatureName::TimestampQuery;
            // TODO(dawn:1800): Reenable once Dawn's interface is updated.
            return false;
        case interop::GPUFeatureName::kDepth32FloatStencil8:
            out = wgpu::FeatureName::Depth32FloatStencil8;
            return true;
        case interop::GPUFeatureName::kDepthClipControl:
            out = wgpu::FeatureName::DepthClipControl;
            return true;
        case interop::GPUFeatureName::kIndirectFirstInstance:
            out = wgpu::FeatureName::IndirectFirstInstance;
            return true;
        case interop::GPUFeatureName::kShaderF16:
            out = wgpu::FeatureName::ShaderF16;
            return true;
        case interop::GPUFeatureName::kRg11B10UfloatRenderable:
            out = wgpu::FeatureName::RG11B10UfloatRenderable;
            return true;
        case interop::GPUFeatureName::kBgra8UnormStorage:
            out = wgpu::FeatureName::BGRA8UnormStorage;
            return true;
        case interop::GPUFeatureName::kFloat32Filterable:
            out = wgpu::FeatureName::Float32Filterable;
            return true;
    }
    return false;
}

bool Converter::Convert(interop::GPUFeatureName& out, wgpu::FeatureName in) {
    switch (in) {
        case wgpu::FeatureName::Depth32FloatStencil8:
            out = interop::GPUFeatureName::kDepth32FloatStencil8;
            return true;
        case wgpu::FeatureName::TimestampQuery:
            out = interop::GPUFeatureName::kTimestampQuery;
            return true;
        case wgpu::FeatureName::TextureCompressionBC:
            out = interop::GPUFeatureName::kTextureCompressionBc;
            return true;
        case wgpu::FeatureName::TextureCompressionETC2:
            out = interop::GPUFeatureName::kTextureCompressionEtc2;
            return true;
        case wgpu::FeatureName::TextureCompressionASTC:
            out = interop::GPUFeatureName::kTextureCompressionAstc;
            return true;
        case wgpu::FeatureName::IndirectFirstInstance:
            out = interop::GPUFeatureName::kIndirectFirstInstance;
            return true;
        case wgpu::FeatureName::DepthClipControl:
            out = interop::GPUFeatureName::kDepthClipControl;
            return true;
        case wgpu::FeatureName::ShaderF16:
            out = interop::GPUFeatureName::kShaderF16;
            return true;
        case wgpu::FeatureName::RG11B10UfloatRenderable:
            out = interop::GPUFeatureName::kRg11B10UfloatRenderable;
            return true;
        case wgpu::FeatureName::BGRA8UnormStorage:
            out = interop::GPUFeatureName::kBgra8UnormStorage;
            return true;
        case wgpu::FeatureName::Float32Filterable:
            out = interop::GPUFeatureName::kFloat32Filterable;
            return true;

        case wgpu::FeatureName::PipelineStatisticsQuery:
        case wgpu::FeatureName::DawnShaderFloat16:
        case wgpu::FeatureName::DawnInternalUsages:
        case wgpu::FeatureName::DawnMultiPlanarFormats:
        case wgpu::FeatureName::DawnNative:
        case wgpu::FeatureName::ChromiumExperimentalDp4a:
        case wgpu::FeatureName::TimestampQueryInsidePasses:
        case wgpu::FeatureName::ImplicitDeviceSynchronization:
        case wgpu::FeatureName::SurfaceCapabilities:
        case wgpu::FeatureName::TransientAttachments:
        case wgpu::FeatureName::MSAARenderToSingleSampled:
        case wgpu::FeatureName::Undefined:
            return false;
    }
    return false;
}

bool Converter::Convert(interop::GPUQueryType& out, wgpu::QueryType in) {
    switch (in) {
        case wgpu::QueryType::Occlusion:
            out = interop::GPUQueryType::kOcclusion;
            return true;
        case wgpu::QueryType::Timestamp:
            out = interop::GPUQueryType::kTimestamp;
            return true;
        case wgpu::QueryType::PipelineStatistics:
            // TODO(dawn:1123): Add support for pipeline statistics if they are in WebGPU one day.
            return false;
    }
    return false;
}

bool Converter::Convert(interop::GPUBufferMapState& out, wgpu::BufferMapState in) {
    switch (in) {
        case wgpu::BufferMapState::Unmapped:
            out = interop::GPUBufferMapState::kUnmapped;
            return true;
        case wgpu::BufferMapState::Pending:
            out = interop::GPUBufferMapState::kPending;
            return true;
        case wgpu::BufferMapState::Mapped:
            out = interop::GPUBufferMapState::kMapped;
            return true;
    }
    return false;
}

bool Converter::Convert(wgpu::AddressMode& out, const interop::GPUAddressMode& in) {
    out = wgpu::AddressMode::Repeat;
    switch (in) {
        case interop::GPUAddressMode::kClampToEdge:
            out = wgpu::AddressMode::ClampToEdge;
            return true;
        case interop::GPUAddressMode::kRepeat:
            out = wgpu::AddressMode::Repeat;
            return true;
        case interop::GPUAddressMode::kMirrorRepeat:
            out = wgpu::AddressMode::MirrorRepeat;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUAddressMode").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::FilterMode& out, const interop::GPUFilterMode& in) {
    out = wgpu::FilterMode::Nearest;
    switch (in) {
        case interop::GPUFilterMode::kNearest:
            out = wgpu::FilterMode::Nearest;
            return true;
        case interop::GPUFilterMode::kLinear:
            out = wgpu::FilterMode::Linear;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUFilterMode").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::MipmapFilterMode& out, const interop::GPUMipmapFilterMode& in) {
    out = wgpu::MipmapFilterMode::Nearest;
    switch (in) {
        case interop::GPUMipmapFilterMode::kNearest:
            out = wgpu::MipmapFilterMode::Nearest;
            return true;
        case interop::GPUMipmapFilterMode::kLinear:
            out = wgpu::MipmapFilterMode::Linear;
            return true;
    }
    Napi::Error::New(env, "invalid value for GPUMipmapFilterMode").ThrowAsJavaScriptException();
    return false;
}

bool Converter::Convert(wgpu::ComputePipelineDescriptor& out,
                        const interop::GPUComputePipelineDescriptor& in) {
    return Convert(out.label, in.label) &&    //
           Convert(out.layout, in.layout) &&  //
           Convert(out.compute, in.compute);
}

bool Converter::Convert(wgpu::RenderPipelineDescriptor& out,
                        const interop::GPURenderPipelineDescriptor& in) {
    return Convert(out.label, in.label) &&                //
           Convert(out.layout, in.layout) &&              //
           Convert(out.vertex, in.vertex) &&              //
           Convert(out.primitive, in.primitive) &&        //
           Convert(out.depthStencil, in.depthStencil) &&  //
           Convert(out.multisample, in.multisample) &&    //
           Convert(out.fragment, in.fragment);
}

bool Converter::Convert(wgpu::PipelineLayout& out, const interop::GPUAutoLayoutMode& in) {
    out = nullptr;
    return true;
}

bool Converter::Convert(wgpu::Bool& out, const bool& in) {
    out = in;
    return true;
}

}  // namespace wgpu::binding
