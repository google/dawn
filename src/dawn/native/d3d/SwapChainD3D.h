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

#ifndef SRC_DAWN_NATIVE_D3D_SWAPCHAIND3D_H_
#define SRC_DAWN_NATIVE_D3D_SWAPCHAIND3D_H_

#include <vector>

#include "dawn/native/SwapChain.h"

#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class Device;
class Texture;

// SwapChain abstracts the swapchain creation for D3D12 and D3D11.
// D3D11 and D3D12 have different ways to present and get the backbuffer.
// D3D11 doesn't need to wait for the GPU to finish before presenting, but D3D12 does.
// D3D11 manages buffers and we can only read and write the the buffer with index 0, but for D3D12
// we need to manage all buffers by ourselves.
class SwapChain : public SwapChainBase {
  protected:
    using SwapChainBase::SwapChainBase;
    ~SwapChain() override;

    void DestroyImpl() override;

    MaybeError Initialize(SwapChainBase* previousSwapChain);

    virtual IUnknown* GetD3DDeviceForCreatingSwapChain() = 0;
    virtual void ReuseBuffers(SwapChainBase* previousSwapChain) = 0;
    // Does the swapchain initialization step of gathering the buffers.
    virtual MaybeError CollectSwapChainBuffers() = 0;
    // Calls DetachFromSurface but also synchronously waits until all references to the
    // swapchain and buffers are removed, as that's a constraint for some DXGI operations.
    virtual MaybeError DetachAndWaitForDeallocation() = 0;

    MaybeError PresentDXGISwapChain();
    void ReleaseDXGISwapChain();

    IDXGISwapChain3* GetDXGISwapChain() const;

    struct Config {
        // Information that's passed to the D3D12 swapchain creation call.
        UINT bufferCount;
        UINT swapChainFlags;
        DXGI_FORMAT format;
        DXGI_USAGE usage;
    };
    const Config& GetConfig() const;

  private:
    // Does the swapchain initialization steps assuming there is nothing we can reuse.
    MaybeError InitializeSwapChainFromScratch();

    Config mConfig;
    ComPtr<IDXGISwapChain3> mDXGISwapChain;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D12_SWAPCHAIND3D12_H_
