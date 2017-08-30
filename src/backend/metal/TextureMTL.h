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

#ifndef BACKEND_METAL_TEXTUREMTL_H_
#define BACKEND_METAL_TEXTUREMTL_H_

#include "backend/Texture.h"

#import <Metal/Metal.h>

namespace backend {
namespace metal {

    MTLPixelFormat MetalPixelFormat(nxt::TextureFormat format);

    class Texture : public TextureBase {
        public:
            Texture(TextureBuilder* builder);
            Texture(TextureBuilder* builder, id<MTLTexture> mtlTexture);
            ~Texture();

            id<MTLTexture> GetMTLTexture();

            void TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) override;

        private:
            id<MTLTexture> mtlTexture = nil;
    };

    class TextureView : public TextureViewBase {
        public:
            TextureView(TextureViewBuilder* builder);
    };

}
}

#endif // BACKEND_METAL_TEXTUREMTL_H_
