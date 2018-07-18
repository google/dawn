// Copyright 2018 The Dawn Authors
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

#include "backend/vulkan/SwapChainVk.h"

#include "backend/vulkan/DeviceVk.h"
#include "backend/vulkan/TextureVk.h"

namespace backend { namespace vulkan {

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        nxtWSIContextVulkan wsiContext = {};
        im.Init(im.userData, &wsiContext);

        ASSERT(im.textureUsage != NXT_TEXTURE_USAGE_BIT_NONE);
        mTextureUsage = static_cast<dawn::TextureUsageBit>(im.textureUsage);
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

        VkImage nativeTexture = VkImage::CreateFromHandle(next.texture.u64);
        return new Texture(builder, nativeTexture);
    }

    void SwapChain::OnBeforePresent(TextureBase* texture) {
        Device* device = ToBackend(GetDevice());

        // Perform the necessary pipeline barriers for the texture to be used with the usage
        // requested by the implementation.
        VkCommandBuffer commands = device->GetPendingCommandBuffer();
        ToBackend(texture)->TransitionUsageNow(commands, mTextureUsage);

        device->SubmitPendingCommands();
    }

}}  // namespace backend::vulkan
