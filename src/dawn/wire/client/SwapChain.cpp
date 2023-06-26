// Copyright 2023 The Dawn Authors
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

#include "dawn/wire/client/SwapChain.h"

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"
#include "dawn/wire/client/Texture.h"

namespace dawn::wire::client {

SwapChain::SwapChain(const ObjectBaseParams& params,
                     WGPUSurface,
                     const WGPUSwapChainDescriptor* descriptor)
    : ObjectBase(params) {
    mTextureDescriptor = {};
    mTextureDescriptor.size = {descriptor->width, descriptor->height, 1};
    mTextureDescriptor.format = descriptor->format;
    mTextureDescriptor.usage = descriptor->usage;
    mTextureDescriptor.dimension = WGPUTextureDimension_2D;
    mTextureDescriptor.mipLevelCount = 1;
    mTextureDescriptor.sampleCount = 1;
}

SwapChain::~SwapChain() = default;

WGPUTexture SwapChain::GetCurrentTexture() {
    Client* wireClient = GetClient();
    Texture* texture = wireClient->Make<Texture>(&mTextureDescriptor);

    SwapChainGetCurrentTextureCmd cmd;
    cmd.self = ToAPI(this);
    cmd.selfId = GetWireId();
    cmd.result = texture->GetWireHandle();
    wireClient->SerializeCommand(cmd);

    return ToAPI(texture);
}

}  // namespace dawn::wire::client
