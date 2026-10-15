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

#include "src/dawn/native/d3d11/SwapChainD3D11.h"

#if defined(DAWN_USE_WINDOWS_UI)
#include <windows.ui.xaml.media.dxinterop.h>
#endif  // defined(DAWN_USE_WINDOWS_UI)

#include <limits>
#include <utility>

#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Surface.h"
#include "src/dawn/native/d3d/D3DError.h"
#include "src/dawn/native/d3d/UtilsD3D.h"
#include "src/dawn/native/d3d11/DeviceD3D11.h"
#include "src/dawn/native/d3d11/QueueD3D11.h"
#include "src/dawn/native/d3d11/TextureD3D11.h"

namespace dawn::native::d3d11 {
// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, config));
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
    DAWN_ASSERT(GetDXGISwapChain() != nullptr);
    DAWN_ASSERT(!mBuffer);

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

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    // Create the API side objects for this use of the swapchain's buffer.
    TextureDescriptor descriptor = GetSwapChainBaseTextureDescriptor(this);
    DAWN_TRY_ASSIGN(mApiTexture,
                    Texture::Create(ToBackend(GetDevice()), Unpack(&descriptor), mBuffer));
    SwapChainTextureInfo info;
    info.texture = mApiTexture;
    // TODO(dawn:2320): Check for optimality
    info.status = wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
    return info;
}

MaybeError SwapChain::DetachAndWaitForDeallocation() {
    DetachFromSurface();

    // DetachFromSurfaceImpl calls Flush() on the immediate context to submit D3D11's deferred
    // destruction of the swapchain and its backbuffer/views, but Flush() is asynchronous. When
    // recreating a swapchain on the same HWND, we must also wait for in-flight GPU work on the
    // previous swapchain to finish, otherwise presenting on the new swapchain can fail with
    // DXGI_ERROR_DEVICE_REMOVED.
    Queue* queue = ToBackend(GetDevice()->GetQueue());
    return queue->WaitForQueueSerial(queue->GetPendingCommandSerial(),
                                     std::numeric_limits<Nanoseconds>::max());
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mApiTexture != nullptr) {
        mApiTexture->APIDestroy();
        mApiTexture = nullptr;
    }

    mBuffer = nullptr;
    ReleaseDXGISwapChain();

    // D3D11 defers the destruction of resources and flip-model swapchains until Flush() is called
    // on the immediate context after all references are released.
    Queue* queue = ToBackend(GetDevice()->GetQueue());
    queue->GetScopedPendingCommandContext(ExecutionQueueBase::SubmitMode::Passive).Flush();
}

}  // namespace dawn::native::d3d11
