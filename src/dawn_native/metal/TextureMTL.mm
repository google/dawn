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

#include "dawn_native/metal/TextureMTL.h"

#include "dawn_native/metal/DeviceMTL.h"

namespace dawn_native { namespace metal {
    MTLPixelFormat MetalPixelFormat(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::R8G8B8A8Unorm:
                return MTLPixelFormatRGBA8Unorm;
            case dawn::TextureFormat::R8G8Unorm:
                return MTLPixelFormatRG8Unorm;
            case dawn::TextureFormat::R8Unorm:
                return MTLPixelFormatR8Unorm;
            case dawn::TextureFormat::R8G8B8A8Uint:
                return MTLPixelFormatRGBA8Uint;
            case dawn::TextureFormat::R8G8Uint:
                return MTLPixelFormatRG8Uint;
            case dawn::TextureFormat::R8Uint:
                return MTLPixelFormatR8Uint;
            case dawn::TextureFormat::B8G8R8A8Unorm:
                return MTLPixelFormatBGRA8Unorm;
            case dawn::TextureFormat::D32FloatS8Uint:
                return MTLPixelFormatDepth32Float_Stencil8;
        }
    }

    namespace {
        bool UsageNeedsTextureView(dawn::TextureUsageBit usage) {
            constexpr dawn::TextureUsageBit kUsageNeedsTextureView =
                dawn::TextureUsageBit::Storage | dawn::TextureUsageBit::Sampled;
            return usage & kUsageNeedsTextureView;
        }

        MTLTextureUsage MetalTextureUsage(dawn::TextureUsageBit usage) {
            MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

            if (usage & (dawn::TextureUsageBit::Storage)) {
                result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            }

            if (usage & (dawn::TextureUsageBit::Sampled)) {
                result |= MTLTextureUsageShaderRead;
            }

            if (usage & (dawn::TextureUsageBit::OutputAttachment)) {
                result |= MTLTextureUsageRenderTarget;
            }

            if (UsageNeedsTextureView(usage)) {
                result |= MTLTextureUsagePixelFormatView;
            }

            return result;
        }

        MTLTextureType MetalTextureType(dawn::TextureDimension dimension,
                                        unsigned int arrayLayers) {
            switch (dimension) {
                case dawn::TextureDimension::e2D:
                    return (arrayLayers > 1) ? MTLTextureType2DArray : MTLTextureType2D;
            }
        }

        MTLTextureType MetalTextureViewType(dawn::TextureViewDimension dimension) {
            switch (dimension) {
                case dawn::TextureViewDimension::e2D:
                    return MTLTextureType2D;
                case dawn::TextureViewDimension::e2DArray:
                    return MTLTextureType2DArray;
                case dawn::TextureViewDimension::Cube:
                    return MTLTextureTypeCube;
                case dawn::TextureViewDimension::CubeArray:
                    return MTLTextureTypeCubeArray;
                default:
                    UNREACHABLE();
                    return MTLTextureType2D;
            }
        }

        bool RequiresCreatingNewTextureView(const TextureBase* texture,
                                            const TextureViewDescriptor* textureViewDescriptor) {
            if (texture->GetFormat() != textureViewDescriptor->format) {
                return true;
            }

            if (texture->GetArrayLayers() != textureViewDescriptor->layerCount) {
                return true;
            }

            if (texture->GetNumMipLevels() != textureViewDescriptor->levelCount) {
                return true;
            }

            switch (textureViewDescriptor->dimension) {
                case dawn::TextureViewDimension::Cube:
                case dawn::TextureViewDimension::CubeArray:
                    return true;
                default:
                    break;
            }

            return false;
        }
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor)
        : TextureBase(device, descriptor) {
        auto desc = [MTLTextureDescriptor new];
        [desc autorelease];
        desc.textureType = MetalTextureType(GetDimension(), GetArrayLayers());
        desc.usage = MetalTextureUsage(GetUsage());
        desc.pixelFormat = MetalPixelFormat(GetFormat());

        const Extent3D& size = GetSize();
        desc.width = size.width;
        desc.height = size.height;
        desc.depth = size.depth;

        desc.mipmapLevelCount = GetNumMipLevels();
        desc.arrayLength = GetArrayLayers();
        desc.storageMode = MTLStorageModePrivate;

        auto mtlDevice = device->GetMTLDevice();
        mMtlTexture = [mtlDevice newTextureWithDescriptor:desc];
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor, id<MTLTexture> mtlTexture)
        : TextureBase(device, descriptor), mMtlTexture(mtlTexture) {
        [mMtlTexture retain];
    }

    Texture::~Texture() {
        [mMtlTexture release];
    }

    id<MTLTexture> Texture::GetMTLTexture() {
        return mMtlTexture;
    }

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor) {
        id<MTLTexture> mtlTexture = ToBackend(texture)->GetMTLTexture();

        if (!UsageNeedsTextureView(texture->GetUsage())) {
            mMtlTextureView = nil;
        } else if (!RequiresCreatingNewTextureView(texture, descriptor)) {
            mMtlTextureView = [mtlTexture retain];
        } else {
            MTLPixelFormat format = MetalPixelFormat(descriptor->format);
            MTLTextureType textureViewType = MetalTextureViewType(descriptor->dimension);
            auto mipLevelRange = NSMakeRange(descriptor->baseMipLevel, descriptor->levelCount);
            auto arrayLayerRange = NSMakeRange(descriptor->baseArrayLayer, descriptor->layerCount);

            mMtlTextureView = [mtlTexture newTextureViewWithPixelFormat:format
                                                            textureType:textureViewType
                                                                 levels:mipLevelRange
                                                                 slices:arrayLayerRange];
        }
    }

    TextureView::~TextureView() {
        [mMtlTextureView release];
    }

    id<MTLTexture> TextureView::GetMTLTexture() {
        ASSERT(mMtlTextureView != nil);
        return mMtlTextureView;
    }
}}  // namespace dawn_native::metal
