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

#ifndef SRC_DAWN_WIRE_CLIENT_SWAPCHAIN_H_
#define SRC_DAWN_WIRE_CLIENT_SWAPCHAIN_H_

#include "dawn/webgpu.h"

#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

class SwapChain final : public ObjectBase {
  public:
    SwapChain(const ObjectBaseParams& params,
              WGPUSurface surface,
              const WGPUSwapChainDescriptor* descriptor);
    ~SwapChain() override;

    WGPUTexture GetCurrentTexture();

  private:
    WGPUTextureDescriptor mTextureDescriptor;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_SWAPCHAIN_H_
