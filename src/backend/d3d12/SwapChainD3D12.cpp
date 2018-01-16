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

#include "backend/d3d12/SwapChainD3D12.h"

#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/TextureD3D12.h"

#include <nxt/nxt_wsi.h>

namespace backend { namespace d3d12 {

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        nxtWSIContextD3D12 wsiContext = {};
        wsiContext.device = reinterpret_cast<nxtDevice>(GetDevice());
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

        ID3D12Resource* nativeTexture = reinterpret_cast<ID3D12Resource*>(next.texture.ptr);
        return new Texture(builder, nativeTexture);
    }

}}  // namespace backend::d3d12
