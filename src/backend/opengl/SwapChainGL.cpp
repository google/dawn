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

#include "backend/opengl/SwapChainGL.h"

#include "backend/opengl/TextureGL.h"

#include <nxt/nxt_wsi.h>

namespace backend {
namespace opengl {

    SwapChain::SwapChain(SwapChainBuilder* builder)
        : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        nxtWSIContextGL wsiContext = {};
        // TODO(kainino@chromium.org): set up wsiContext
        im.Init(im.userData, &wsiContext);

        // TODO(kainino@chromium.org): set up FBO
    }

    SwapChain::~SwapChain() {
        // TODO(kainino@chromium.org): clean up FBO
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        return new Texture(builder, nativeTexture);
    }

}
}
