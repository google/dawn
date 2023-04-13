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

#include "dawn/native/d3d12/SwapChainD3D12.h"

#include <windows.ui.xaml.media.dxinterop.h>

#include <utility>

#include "dawn/native/Surface.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {
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
    return ToBackend(GetDevice())->GetCommandQueue().Get();
}

void SwapChain::ReuseBuffers(SwapChainBase* previousSwapChain) {
    SwapChain* previousD3DSwapChain = ToBackend(previousSwapChain);
    mBuffers = std::move(previousD3DSwapChain->mBuffers);
    mBufferLastUsedSerials = std::move(previousD3DSwapChain->mBufferLastUsedSerials);
}

MaybeError SwapChain::CollectSwapChainBuffers() {
    ASSERT(GetDXGISwapChain() != nullptr);
    ASSERT(mBuffers.empty());

    IDXGISwapChain3* dxgiSwapChain = GetDXGISwapChain();
    const auto& config = GetConfig();

    mBuffers.resize(config.bufferCount);
    for (uint32_t i = 0; i < config.bufferCount; i++) {
        DAWN_TRY(CheckHRESULT(dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBuffers[i])),
                              "Getting IDXGISwapChain buffer"));
    }

    // Pretend all the buffers were last used at the beginning of time.
    mBufferLastUsedSerials.resize(config.bufferCount, ExecutionSerial(0));
    return {};
}

MaybeError SwapChain::PresentImpl() {
    Device* device = ToBackend(GetDevice());

    // Transition the texture to the present state as required by IDXGISwapChain1::Present()
    // TODO(crbug.com/dawn/269): Remove the need for this by eagerly transitioning the
    // presentable texture to present at the end of submits that use them.
    CommandRecordingContext* commandContext;
    DAWN_TRY_ASSIGN(commandContext, device->GetPendingCommandContext());
    mApiTexture->TrackUsageAndTransitionNow(commandContext, kPresentTextureUsage,
                                            mApiTexture->GetAllSubresources());
    DAWN_TRY(device->ExecutePendingCommandContext());

    DAWN_TRY(PresentDXGISwapChain());

    // Record that "new" is the last time the buffer has been used.
    DAWN_TRY(device->NextSerial());
    mBufferLastUsedSerials[mCurrentBuffer] = device->GetPendingCommandSerial();

    mApiTexture->APIDestroy();
    mApiTexture = nullptr;

    return {};
}

ResultOrError<Ref<TextureViewBase>> SwapChain::GetCurrentTextureViewImpl() {
    Device* device = ToBackend(GetDevice());

    // Synchronously wait until previous operations on the next swapchain buffer are finished.
    // This is the logic that performs frame pacing.
    // TODO(crbug.com/dawn/269): Consider whether this should  be lifted for Mailbox so that
    // there is not frame pacing.
    mCurrentBuffer = GetDXGISwapChain()->GetCurrentBackBufferIndex();
    DAWN_TRY(device->WaitForSerial(mBufferLastUsedSerials[mCurrentBuffer]));

    // Create the API side objects for this use of the swapchain's buffer.
    TextureDescriptor descriptor = GetSwapChainBaseTextureDescriptor(this);
    DAWN_TRY_ASSIGN(mApiTexture,
                    Texture::Create(ToBackend(GetDevice()), &descriptor, mBuffers[mCurrentBuffer]));
    return mApiTexture->CreateView();
}

MaybeError SwapChain::DetachAndWaitForDeallocation() {
    DetachFromSurface();

    // DetachFromSurface calls Texture->Destroy that enqueues the D3D12 resource in a
    // SerialQueue with the current "pending serial" so that we don't destroy the texture
    // before it is finished being used. Flush the commands and wait for that serial to be
    // passed, then Tick the device to make sure the reference to the D3D12 texture is removed.
    Device* device = ToBackend(GetDevice());
    DAWN_TRY(device->NextSerial());
    DAWN_TRY(device->WaitForSerial(device->GetLastSubmittedCommandSerial()));
    return device->TickImpl();
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mApiTexture != nullptr) {
        mApiTexture->APIDestroy();
        mApiTexture = nullptr;
    }
    mBuffers.clear();

    ReleaseDXGISwapChain();
}

}  // namespace dawn::native::d3d12
