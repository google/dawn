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

#include "TextureMTL.h"

#include "MetalBackend.h"

namespace backend {
namespace metal {

    namespace {
        MTLPixelFormat MetalPixelFormat(nxt::TextureFormat format) {
            switch (format) {
                case nxt::TextureFormat::R8G8B8A8Unorm:
                    return MTLPixelFormatRGBA8Unorm;
            }
        }

        MTLTextureUsage MetalTextureUsage(nxt::TextureUsageBit usage) {
            MTLTextureUsage result = MTLTextureUsageUnknown; // This is 0

            if (usage & (nxt::TextureUsageBit::Storage)) {
                result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            }

            if (usage & (nxt::TextureUsageBit::Sampled)) {
                result |= MTLTextureUsageShaderRead;
            }

            if (usage & (nxt::TextureUsageBit::ColorAttachment | nxt::TextureUsageBit::DepthStencilAttachment)) {
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

    Texture::Texture(TextureBuilder* builder)
        : TextureBase(builder) {
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

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();
        mtlTexture = [mtlDevice newTextureWithDescriptor:desc];
    }

    Texture::~Texture() {
        [mtlTexture release];
    }

    id<MTLTexture> Texture::GetMTLTexture() {
        return mtlTexture;
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) {
    }

    TextureView::TextureView(TextureViewBuilder* builder)
        : TextureViewBase(builder) {
    }
}
}
