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

#include "backend/metal/TextureMTL.h"

#include "backend/metal/MetalBackend.h"

namespace backend { namespace metal {

    MTLPixelFormat MetalPixelFormat(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
                return MTLPixelFormatRGBA8Unorm;
            case nxt::TextureFormat::R8G8Unorm:
                return MTLPixelFormatRG8Unorm;
            case nxt::TextureFormat::R8Unorm:
                return MTLPixelFormatR8Unorm;
            case nxt::TextureFormat::R8G8B8A8Uint:
                return MTLPixelFormatRGBA8Uint;
            case nxt::TextureFormat::R8G8Uint:
                return MTLPixelFormatRG8Uint;
            case nxt::TextureFormat::R8Uint:
                return MTLPixelFormatR8Uint;
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return MTLPixelFormatBGRA8Unorm;
            case nxt::TextureFormat::D32FloatS8Uint:
                return MTLPixelFormatDepth32Float_Stencil8;
        }
    }

    namespace {
        MTLTextureUsage MetalTextureUsage(nxt::TextureUsageBit usage) {
            MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

            if (usage & (nxt::TextureUsageBit::Storage)) {
                result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            }

            if (usage & (nxt::TextureUsageBit::Sampled)) {
                result |= MTLTextureUsageShaderRead;
            }

            if (usage & (nxt::TextureUsageBit::OutputAttachment)) {
                result |= MTLTextureUsageRenderTarget;
            }

            return result;
        }

        MTLTextureType MetalTextureType(nxt::TextureDimension dimension) {
            switch (dimension) {
                case nxt::TextureDimension::e2D:
                    return MTLTextureType2D;
            }
        }
    }

    Texture::Texture(TextureBuilder* builder) : TextureBase(builder) {
        auto desc = [MTLTextureDescriptor new];
        [desc autorelease];
        desc.textureType = MetalTextureType(GetDimension());
        desc.usage = MetalTextureUsage(GetUsage());
        desc.pixelFormat = MetalPixelFormat(GetFormat());
        desc.width = GetWidth();
        desc.height = GetHeight();
        desc.depth = GetDepth();
        desc.mipmapLevelCount = GetNumMipLevels();
        desc.arrayLength = 1;
        desc.storageMode = MTLStorageModePrivate;

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();
        mMtlTexture = [mtlDevice newTextureWithDescriptor:desc];
    }

    Texture::Texture(TextureBuilder* builder, id<MTLTexture> mtlTexture)
        : TextureBase(builder), mMtlTexture(mtlTexture) {
        [mMtlTexture retain];
    }

    Texture::~Texture() {
        [mMtlTexture release];
    }

    id<MTLTexture> Texture::GetMTLTexture() {
        return mMtlTexture;
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit, nxt::TextureUsageBit) {
    }

    TextureView::TextureView(TextureViewBuilder* builder) : TextureViewBase(builder) {
    }

}}  // namespace backend::metal
