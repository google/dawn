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

#include "backend/Forward.h"
#include "backend/Builder.h"
#include "backend/RefCounted.h"

#include "nxt/nxtcpp.h"
#include "nxt/nxt_wsi.h"

namespace backend {

    class SwapChainBase : public RefCounted {
        public:
            SwapChainBase(SwapChainBuilder* builder);
            ~SwapChainBase();

            DeviceBase* GetDevice();

            // NXT API
            void Configure(nxt::TextureFormat format, uint32_t width, uint32_t height);
            TextureBase* GetNextTexture();
            void Present(TextureBase* texture);

        protected:
            const nxtSwapChainImplementation& GetImplementation();
            virtual TextureBase* GetNextTextureImpl(TextureBuilder* builder) = 0;

        private:
            DeviceBase* device = nullptr;
            nxtSwapChainImplementation implementation = {};
            nxt::TextureFormat format = {};
            uint32_t width = 0;
            uint32_t height = 0;
            TextureBase* lastNextTexture = nullptr;
    };

    class SwapChainBuilder : public Builder<SwapChainBase> {
        public:
            SwapChainBuilder(DeviceBase* device);

            // NXT API
            SwapChainBase* GetResultImpl() override;
            void SetImplementation(uint64_t implementation);

        private:
            friend class SwapChainBase;

            nxtSwapChainImplementation implementation = {};
    };

}

#endif // BACKEND_SWAPCHAIN_H_
