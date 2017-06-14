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
        MTLPixelFormat TextureFormatPixelFormat(nxt::TextureFormat format) {
            switch (format) {
                case nxt::TextureFormat::R8G8B8A8Unorm:
                    return MTLPixelFormatRGBA8Unorm;
            }
        }
    }

    Texture::Texture(TextureBuilder* builder)
        : TextureBase(builder) {
        auto desc = [MTLTextureDescriptor new];
        [desc autorelease];
        switch (GetDimension()) {
            case nxt::TextureDimension::e2D:
                desc.textureType = MTLTextureType2D;
                break;
        }
        desc.usage = MTLTextureUsageShaderRead;
        desc.pixelFormat = TextureFormatPixelFormat(GetFormat());
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
