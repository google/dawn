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

#include "dawn/native/d3d11/SwapChainD3D11.h"

#include <windows.ui.xaml.media.dxinterop.h>

#include <utility>

#include "dawn/native/Surface.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"

namespace dawn::native::d3d11 {
// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, descriptor));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

SwapChain::~SwapChain() = default;

IUnknown* SwapChain::GetD3DDeviceForCreatingSwapChain() {
    return ToBackend(GetDevice())->GetD3D11Device();
}

void SwapChain::ReuseBuffers(SwapChainBase* previousSwapChain) {
    SwapChain* previousD3DSwapChain = ToBackend(previousSwapChain);
    mBuffer = std::move(previousD3DSwapChain->mBuffer);
}

MaybeError SwapChain::CollectSwapChainBuffers() {
    ASSERT(GetDXGISwapChain() != nullptr);
    ASSERT(!mBuffer);

    // https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect
    // DXGISwapChain is created with DXGI_SWAP_EFFECT_FLIP_DISCARD, we can read and write to the
    // buffer 0 only for D3D11.
    DAWN_TRY(CheckHRESULT(GetDXGISwapChain()->GetBuffer(0, IID_PPV_ARGS(&mBuffer)),
                          "Getting IDXGISwapChain buffer"));

    return {};
}

MaybeError SwapChain::PresentImpl() {
    DAWN_TRY(PresentDXGISwapChain());

    mApiTexture->APIDestroy();
    mApiTexture = nullptr;

    return {};
}

ResultOrError<Ref<TextureViewBase>> SwapChain::GetCurrentTextureViewImpl() {
    // Create the API side objects for this use of the swapchain's buffer.
    TextureDescriptor descriptor = GetSwapChainBaseTextureDescriptor(this);
    DAWN_TRY_ASSIGN(mApiTexture, Texture::Create(ToBackend(GetDevice()), &descriptor, mBuffer));
    return mApiTexture->CreateView();
}

MaybeError SwapChain::DetachAndWaitForDeallocation() {
    DetachFromSurface();
    return {};
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mApiTexture != nullptr) {
        mApiTexture->APIDestroy();
        mApiTexture = nullptr;
    }

    mBuffer = nullptr;
    ReleaseDXGISwapChain();
}

}  // namespace dawn::native::d3d11
