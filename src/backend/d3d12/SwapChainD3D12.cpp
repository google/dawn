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

#include "backend/d3d12/TextureD3D12.h"

#include <nxt/nxt_wsi.h>

namespace backend {
namespace d3d12 {

    SwapChain::SwapChain(SwapChainBuilder* builder)
        : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        nxtWSIContextD3D12 wsiContext = {};
        // TODO(kainino@chromium.org): set up wsiContext
        im.Init(im.userData, &wsiContext);

        // TODO(kainino@chromium.org): set up D3D12 swapchain
    }

    SwapChain::~SwapChain() {
        // TODO(kainino@chromium.org): clean up D3D12 swapchain
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        ComPtr<ID3D12Resource> nativeTexture = nullptr;
        // TODO(kainino@chromium.org): obtain native texture from D3D12 swapchain
        return new Texture(builder, nativeTexture);
    }

}
}

