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

#include "backend/SwapChain.h"

#include "backend/Device.h"
#include "backend/Texture.h"

namespace backend {

    // SwapChain

    SwapChainBase::SwapChainBase(SwapChainBuilder* builder)
        : mDevice(builder->mDevice), mImplementation(builder->mImplementation) {
    }

    SwapChainBase::~SwapChainBase() {
        const auto& im = GetImplementation();
        im.Destroy(im.userData);
    }

    DeviceBase* SwapChainBase::GetDevice() {
        return mDevice;
    }

    void SwapChainBase::Configure(dawn::TextureFormat format,
                                  dawn::TextureUsageBit allowedUsage,
                                  uint32_t width,
                                  uint32_t height) {
        if (width == 0 || height == 0) {
            mDevice->HandleError("Swap chain cannot be configured to zero size");
            return;
        }
        allowedUsage |= dawn::TextureUsageBit::Present;

        mFormat = format;
        mAllowedUsage = allowedUsage;
        mWidth = width;
        mHeight = height;
        mImplementation.Configure(mImplementation.userData, static_cast<nxtTextureFormat>(format),
                                  static_cast<nxtTextureUsageBit>(allowedUsage), width, height);
    }

    TextureBase* SwapChainBase::GetNextTexture() {
        if (mWidth == 0) {
            // If width is 0, it implies swap chain has never been configured
            mDevice->HandleError("Swap chain needs to be configured before GetNextTexture");
            return nullptr;
        }

        auto* builder = mDevice->CreateTextureBuilder();
        builder->SetDimension(dawn::TextureDimension::e2D);
        builder->SetExtent(mWidth, mHeight, 1);
        builder->SetFormat(mFormat);
        builder->SetMipLevels(1);
        builder->SetAllowedUsage(mAllowedUsage);

        auto* texture = GetNextTextureImpl(builder);
        mLastNextTexture = texture;
        builder->Release();
        return texture;
    }

    void SwapChainBase::Present(TextureBase* texture) {
        if (texture != mLastNextTexture) {
            mDevice->HandleError("Tried to present something other than the last NextTexture");
            return;
        }

        OnBeforePresent(texture);

        mImplementation.Present(mImplementation.userData);
    }

    const dawnSwapChainImplementation& SwapChainBase::GetImplementation() {
        return mImplementation;
    }

    // SwapChain Builder

    SwapChainBuilder::SwapChainBuilder(DeviceBase* device) : Builder(device) {
    }

    SwapChainBase* SwapChainBuilder::GetResultImpl() {
        if (!mImplementation.Init) {
            HandleError("Implementation not set");
            return nullptr;
        }
        return mDevice->CreateSwapChain(this);
    }

    void SwapChainBuilder::SetImplementation(uint64_t implementation) {
        if (!implementation) {
            HandleError("Implementation pointer is invalid");
            return;
        }

        dawnSwapChainImplementation& impl =
            *reinterpret_cast<dawnSwapChainImplementation*>(implementation);

        if (!impl.Init || !impl.Destroy || !impl.Configure || !impl.GetNextTexture ||
            !impl.Present) {
            HandleError("Implementation is incomplete");
            return;
        }

        mImplementation = impl;
    }
}  // namespace backend
