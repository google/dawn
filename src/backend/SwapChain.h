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

#ifndef BACKEND_SWAPCHAIN_H_
#define BACKEND_SWAPCHAIN_H_

#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"

#include "nxt/nxt_wsi.h"
#include "nxt/nxtcpp.h"

namespace backend {

    class SwapChainBase : public RefCounted {
      public:
        SwapChainBase(SwapChainBuilder* builder);
        ~SwapChainBase();

        DeviceBase* GetDevice();

        // NXT API
        void Configure(nxt::TextureFormat format,
                       nxt::TextureUsageBit allowedUsage,
                       uint32_t width,
                       uint32_t height);
        TextureBase* GetNextTexture();
        void Present(TextureBase* texture);

      protected:
        const nxtSwapChainImplementation& GetImplementation();
        virtual TextureBase* GetNextTextureImpl(TextureBuilder* builder) = 0;
        virtual void OnBeforePresent(TextureBase* texture) = 0;

      private:
        DeviceBase* mDevice = nullptr;
        nxtSwapChainImplementation mImplementation = {};
        nxt::TextureFormat mFormat = {};
        nxt::TextureUsageBit mAllowedUsage;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        TextureBase* mLastNextTexture = nullptr;
    };

    class SwapChainBuilder : public Builder<SwapChainBase> {
      public:
        SwapChainBuilder(DeviceBase* device);

        // NXT API
        SwapChainBase* GetResultImpl() override;
        void SetImplementation(uint64_t implementation);

      private:
        friend class SwapChainBase;

        nxtSwapChainImplementation mImplementation = {};
    };

}  // namespace backend

#endif  // BACKEND_SWAPCHAIN_H_
