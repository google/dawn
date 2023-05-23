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

#ifndef SRC_DAWN_NATIVE_METAL_SWAPCHAINMTL_H_
#define SRC_DAWN_NATIVE_METAL_SWAPCHAINMTL_H_

#include "dawn/native/SwapChain.h"

#include "dawn/common/NSRef.h"

@class CAMetalLayer;
@protocol CAMetalDrawable;

namespace dawn::native::metal {

class Device;
class Texture;

class SwapChain final : public SwapChainBase {
  public:
    static ResultOrError<Ref<SwapChain>> Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor);

    SwapChain(DeviceBase* device, Surface* surface, const SwapChainDescriptor* descriptor);
    ~SwapChain() override;

  private:
    void DestroyImpl() override;

    using SwapChainBase::SwapChainBase;
    MaybeError Initialize(SwapChainBase* previousSwapChain);

    NSRef<CAMetalLayer> mLayer;

    NSPRef<id<CAMetalDrawable>> mCurrentDrawable;
    Ref<Texture> mTexture;

    MaybeError PresentImpl() override;
    ResultOrError<Ref<TextureBase>> GetCurrentTextureImpl() override;
    void DetachFromSurfaceImpl() override;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SWAPCHAINMTL_H_
