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

#include "dawn_native/SwapChain.h"

#include "dawn_native/Device.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    // SwapChain

    SwapChainBase::SwapChainBase(SwapChainBuilder* builder)
        : ObjectBase(builder->GetDevice()), mImplementation(builder->mImplementation) {
    }

    SwapChainBase::~SwapChainBase() {
        const auto& im = GetImplementation();
        im.Destroy(im.userData);
    }

    void SwapChainBase::Configure(dawn::TextureFormat format,
                                  dawn::TextureUsageBit allowedUsage,
                                  uint32_t width,
                                  uint32_t height) {
        if (width == 0 || height == 0) {
            GetDevice()->HandleError("Swap chain cannot be configured to zero size");
            return;
        }
        allowedUsage |= dawn::TextureUsageBit::Present;

        mFormat = format;
        mAllowedUsage = allowedUsage;
        mWidth = width;
        mHeight = height;
        mImplementation.Configure(mImplementation.userData, static_cast<dawnTextureFormat>(format),
                                  static_cast<dawnTextureUsageBit>(allowedUsage), width, height);
    }

    TextureBase* SwapChainBase::GetNextTexture() {
        if (mWidth == 0) {
            // If width is 0, it implies swap chain has never been configured
            GetDevice()->HandleError("Swap chain needs to be configured before GetNextTexture");
            return nullptr;
        }

        TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = mWidth;
        descriptor.size.height = mHeight;
        descriptor.size.depth = 1;
        descriptor.arraySize = 1;
        descriptor.sampleCount = 1;
        descriptor.format = mFormat;
        descriptor.levelCount = 1;
        descriptor.usage = mAllowedUsage;

        auto* texture = GetNextTextureImpl(&descriptor);
        mLastNextTexture = texture;
        return texture;
    }

    void SwapChainBase::Present(TextureBase* texture) {
        if (texture != mLastNextTexture) {
            GetDevice()->HandleError("Tried to present something other than the last NextTexture");
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
        return GetDevice()->CreateSwapChain(this);
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
}  // namespace dawn_native
