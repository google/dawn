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

#include "backend/SwapChain.h"

#include "backend/Device.h"
#include "backend/Texture.h"

namespace backend {

    // SwapChain

    SwapChainBase::SwapChainBase(SwapChainBuilder* builder)
            : device(builder->device), implementation(builder->implementation) {
    }

    SwapChainBase::~SwapChainBase() {
        const auto& im = GetImplementation();
        im.Destroy(im.userData);
    }

    DeviceBase* SwapChainBase::GetDevice() {
        return device;
    }

    void SwapChainBase::Configure(nxt::TextureFormat format, uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            device->HandleError("Swap chain cannot be configured to zero size");
            return;
        }

        this->format = format;
        this->width = width;
        this->height = height;
        implementation.Configure(implementation.userData,
                static_cast<nxtTextureFormat>(format), width, height);
    }

    TextureBase* SwapChainBase::GetNextTexture() {
        if (width == 0) {
            // If width is 0, it implies swap chain has never been configured
            device->HandleError("Swap chain needs to be configured before GetNextTexture");
            return nullptr;
        }

        auto* builder = device->CreateTextureBuilder();
        builder->SetDimension(nxt::TextureDimension::e2D);
        builder->SetExtent(width, height, 1);
        builder->SetFormat(format);
        builder->SetMipLevels(1);
        builder->SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::Present);
        builder->SetInitialUsage(nxt::TextureUsageBit::OutputAttachment);

        auto* texture = GetNextTextureImpl(builder);
        lastNextTexture = texture;
        return texture;
    }

    void SwapChainBase::Present(TextureBase* texture) {
        if (texture != lastNextTexture) {
            device->HandleError("Tried to present something other than the last NextTexture");
            return;
        }
        if (texture->GetUsage() != nxt::TextureUsageBit::Present) {
            device->HandleError("Texture has not been transitioned to the Present usage");
            return;
        }

        implementation.Present(implementation.userData);
    }

    const nxtSwapChainImplementation& SwapChainBase::GetImplementation() {
        return implementation;
    }

    // SwapChain Builder

    SwapChainBuilder::SwapChainBuilder(DeviceBase* device)
        : Builder(device) {
    }

    SwapChainBase* SwapChainBuilder::GetResultImpl() {
        if (!implementation.Init) {
            HandleError("Implementation not set");
            return nullptr;
        }
        return device->CreateSwapChain(this);
    }

    void SwapChainBuilder::SetImplementation(uint64_t implementation) {
        if (!implementation) {
            HandleError("Implementation pointer is invalid");
            return;
        }

        nxtSwapChainImplementation& impl = *reinterpret_cast<nxtSwapChainImplementation*>(implementation);

        if (!impl.Init || !impl.Destroy || !impl.Configure ||
                !impl.GetNextTexture || !impl.Present) {
            HandleError("Implementation is incomplete");
            return;
        }

        this->implementation = impl;
    }
}
