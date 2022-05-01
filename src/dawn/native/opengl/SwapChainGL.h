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

#ifndef SRC_DAWN_NATIVE_OPENGL_SWAPCHAINGL_H_
#define SRC_DAWN_NATIVE_OPENGL_SWAPCHAINGL_H_

#include "dawn/native/SwapChain.h"

#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;

class SwapChain final : public OldSwapChainBase {
  public:
    SwapChain(Device* device, const SwapChainDescriptor* descriptor);

  protected:
    ~SwapChain() override;
    TextureBase* GetNextTextureImpl(const TextureDescriptor* descriptor) override;
    MaybeError OnBeforePresent(TextureViewBase* view) override;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_SWAPCHAINGL_H_
