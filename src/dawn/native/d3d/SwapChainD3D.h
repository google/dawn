// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_D3D_SWAPCHAIND3D_H_
#define SRC_DAWN_NATIVE_D3D_SWAPCHAIND3D_H_

#include <vector>

#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/SwapChain.h"
#include "src/dawn/native/d3d/d3d_platform.h"

// DirectComposition, for per-pixel-transparent HWND surfaces: DXGI only allows a non-opaque
// AlphaMode on composition swapchains, never on the ones created by CreateSwapChainForHwnd.
// Must come after d3d_platform.h, which brings in windows.h.
#include <dcomp.h>

namespace dawn::native::d3d {

class Device;
class Texture;

// SwapChain abstracts the swapchain creation for D3D12 and D3D11.
// D3D11 and D3D12 have different ways to present and get the backbuffer.
// D3D11 doesn't need to wait for the GPU to finish before presenting, but D3D12 does.
// D3D11 manages buffers and we can only read and write the buffer with index 0, but for D3D12
// we need to manage all buffers by ourselves.
class SwapChain : public SwapChainBase {
  protected:
    using SwapChainBase::SwapChainBase;
    ~SwapChain() override;

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
        UINT bufferCount = 0;
        UINT swapChainFlags = 0;
        DXGI_FORMAT format{};
        DXGI_USAGE usage = 0;
    };
    const Config& GetConfig() const;

    // True when the surface asked for a per-pixel-transparent presentation and we therefore
    // have to go through DirectComposition instead of CreateSwapChainForHwnd.
    bool UsesComposition() const;

  private:
    // Does the swapchain initialization steps assuming there is nothing we can reuse.
    MaybeError InitializeSwapChainFromScratch();

    // Builds the DirectComposition device / target / visual tree for `hwnd` and points it at
    // `swapChain`. Only called on the composition path.
    MaybeError InitializeDComp(HWND hwnd, IDXGISwapChain1* swapChain);

    Config mConfig;
    ComPtr<IDXGISwapChain3> mDXGISwapChain;

    // DirectComposition objects backing a transparent HWND swapchain; null on the opaque path.
    // These must outlive mDXGISwapChain, and must be moved across when a swapchain is recycled
    // onto a new SwapChain object — otherwise the visual tree is destroyed with the old object
    // and the window goes blank while still presenting successfully.
    ComPtr<IDCompositionDevice> mDCompDevice;
    ComPtr<IDCompositionTarget> mDCompTarget;
    ComPtr<IDCompositionVisual> mDCompVisual;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D12_SWAPCHAIND3D12_H_
