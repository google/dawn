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

#include "dawn_native/opengl/SwapChainGL.h"

#include "dawn_native/Device.h"
#include "dawn_native/opengl/TextureGL.h"

#include <dawn/dawn_wsi.h>

namespace dawn_native { namespace opengl {

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        im.Init(im.userData, nullptr);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        const auto& im = GetImplementation();
        dawnSwapChainNextTexture next = {};
        dawnSwapChainError error = im.GetNextTexture(im.userData, &next);
        if (error) {
            GetDevice()->HandleError(error);
            return nullptr;
        }
        GLuint nativeTexture = next.texture.u32;
        return new Texture(builder, nativeTexture);
    }

    void SwapChain::OnBeforePresent(TextureBase*) {
    }

}}  // namespace dawn_native::opengl
