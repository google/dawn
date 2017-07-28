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

#include "backend/metal/SwapChainMTL.h"

#include "backend/metal/MetalBackend.h"
#include "backend/metal/TextureMTL.h"

#include <nxt/nxt_wsi.h>

namespace backend {
namespace metal {

    SwapChain::SwapChain(SwapChainBuilder* builder)
        : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        nxtWSIContextMetal wsiContext = {};
        wsiContext.device = ToBackend(GetDevice())->GetMTLDevice();
        im.Init(im.userData, &wsiContext);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        const auto& im = GetImplementation();
        nxtSwapChainNextTexture next = {};
        nxtSwapChainError error = im.GetNextTexture(im.userData, &next);
        if (error) {
            GetDevice()->HandleError(error);
            return nullptr;
        }

        id<MTLTexture> nativeTexture = reinterpret_cast<id<MTLTexture>>(next.texture);
        return new Texture(builder, nativeTexture);
    }

}
}
