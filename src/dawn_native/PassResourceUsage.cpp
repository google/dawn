// Copyright 2021 The Dawn Authors
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

#include "dawn_native/PassResourceUsage.h"

#include "dawn_native/Format.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    PassTextureUsage::PassTextureUsage(const TextureBase* texture)
        : subresourceUsages(texture->GetFormat().aspects,
                            texture->GetArrayLayers(),
                            texture->GetNumMipLevels(),
                            wgpu::TextureUsage::None) {
    }

}  // namespace dawn_native
