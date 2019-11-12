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

#ifndef DAWNNATIVE_SWAPCHAIN_H_
#define DAWNNATIVE_SWAPCHAIN_H_

#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn/dawn_wsi.h"
#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    MaybeError ValidateSwapChainDescriptor(const DeviceBase* device,
                                           const SwapChainDescriptor* descriptor);

    class SwapChainBase : public ObjectBase {
      public:
        SwapChainBase(DeviceBase* device, const SwapChainDescriptor* descriptor);
        ~SwapChainBase();

        static SwapChainBase* MakeError(DeviceBase* device);

        // Dawn API
        void Configure(wgpu::TextureFormat format,
                       wgpu::TextureUsage allowedUsage,
                       uint32_t width,
                       uint32_t height);
        TextureViewBase* GetCurrentTextureView();
        void Present();

      protected:
        SwapChainBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        const DawnSwapChainImplementation& GetImplementation();
        virtual TextureBase* GetNextTextureImpl(const TextureDescriptor*) = 0;
        virtual MaybeError OnBeforePresent(TextureBase* texture) = 0;

      private:
        MaybeError ValidateConfigure(wgpu::TextureFormat format,
                                     wgpu::TextureUsage allowedUsage,
                                     uint32_t width,
                                     uint32_t height) const;
        MaybeError ValidateGetCurrentTextureView() const;
        MaybeError ValidatePresent() const;

        DawnSwapChainImplementation mImplementation = {};
        wgpu::TextureFormat mFormat = {};
        wgpu::TextureUsage mAllowedUsage;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        Ref<TextureBase> mCurrentTexture;
        Ref<TextureViewBase> mCurrentTextureView;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_SWAPCHAIN_H_
