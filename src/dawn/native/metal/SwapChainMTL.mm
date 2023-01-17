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

#include "dawn/native/metal/SwapChainMTL.h"

#include "dawn/native/Surface.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/TextureMTL.h"

#include "dawn/dawn_wsi.h"

#import <QuartzCore/CAMetalLayer.h>

namespace dawn::native::metal {

// OldSwapChain

// static
Ref<OldSwapChain> OldSwapChain::Create(Device* device, const SwapChainDescriptor* descriptor) {
    return AcquireRef(new OldSwapChain(device, descriptor));
}

OldSwapChain::OldSwapChain(Device* device, const SwapChainDescriptor* descriptor)
    : OldSwapChainBase(device, descriptor) {
    const auto& im = GetImplementation();
    DawnWSIContextMetal wsiContext = {};
    wsiContext.device = ToBackend(GetDevice())->GetMTLDevice();
    wsiContext.queue = ToBackend(GetDevice())->GetMTLQueue();
    im.Init(im.userData, &wsiContext);
}

OldSwapChain::~OldSwapChain() {}

TextureBase* OldSwapChain::GetNextTextureImpl(const TextureDescriptor* descriptor) {
    const auto& im = GetImplementation();
    DawnSwapChainNextTexture next = {};
    DawnSwapChainError error = im.GetNextTexture(im.userData, &next);
    if (error) {
        GetDevice()->HandleError(InternalErrorType::Internal, error);
        return nullptr;
    }

    id<MTLTexture> nativeTexture = reinterpret_cast<id<MTLTexture>>(next.texture.ptr);

    return Texture::CreateWrapping(ToBackend(GetDevice()), descriptor, nativeTexture).Detach();
}

MaybeError OldSwapChain::OnBeforePresent(TextureViewBase*) {
    return {};
}

// SwapChain

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                NewSwapChainBase* previousSwapChain,
                                                const SwapChainDescriptor* descriptor) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, descriptor));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

SwapChain::SwapChain(DeviceBase* dev, Surface* sur, const SwapChainDescriptor* desc)
    : NewSwapChainBase(dev, sur, desc) {}

SwapChain::~SwapChain() = default;

void SwapChain::DestroyImpl() {
    SwapChainBase::DestroyImpl();
    DetachFromSurface();
}

MaybeError SwapChain::Initialize(NewSwapChainBase* previousSwapChain) {
    ASSERT(GetSurface()->GetType() == Surface::Type::MetalLayer);

    if (previousSwapChain != nullptr) {
        // TODO(crbug.com/dawn/269): figure out what should happen when surfaces are used by
        // multiple backends one after the other. It probably needs to block until the backend
        // and GPU are completely finished with the previous swapchain.
        DAWN_INVALID_IF(previousSwapChain->GetBackendType() != wgpu::BackendType::Metal,
                        "Metal SwapChain cannot switch backend types from %s to %s.",
                        previousSwapChain->GetBackendType(), wgpu::BackendType::Metal);

        previousSwapChain->DetachFromSurface();
    }

    mLayer = static_cast<CAMetalLayer*>(GetSurface()->GetMetalLayer());
    ASSERT(mLayer != nullptr);

    CGSize size = {};
    size.width = GetWidth();
    size.height = GetHeight();
    [*mLayer setDrawableSize:size];

    [*mLayer setFramebufferOnly:(GetUsage() == wgpu::TextureUsage::RenderAttachment)];
    [*mLayer setDevice:ToBackend(GetDevice())->GetMTLDevice()];
    [*mLayer setPixelFormat:MetalPixelFormat(GetDevice(), GetFormat())];

#if DAWN_PLATFORM_IS(MACOS)
    if (@available(macos 10.13, *)) {
        [*mLayer setDisplaySyncEnabled:(GetPresentMode() != wgpu::PresentMode::Immediate)];
    }
#endif  // DAWN_PLATFORM_IS(MACOS)

    // There is no way to control Fifo vs. Mailbox in Metal.

    return {};
}

MaybeError SwapChain::PresentImpl() {
    ASSERT(mCurrentDrawable != nullptr);
    [*mCurrentDrawable present];

    mTexture->APIDestroy();
    mTexture = nullptr;

    mCurrentDrawable = nullptr;

    return {};
}

ResultOrError<Ref<TextureViewBase>> SwapChain::GetCurrentTextureViewImpl() {
    ASSERT(mCurrentDrawable == nullptr);
    mCurrentDrawable = [*mLayer nextDrawable];

    TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(this);

    mTexture =
        Texture::CreateWrapping(ToBackend(GetDevice()), &textureDesc, [*mCurrentDrawable texture]);
    return mTexture->CreateView();
}

void SwapChain::DetachFromSurfaceImpl() {
    ASSERT((mTexture == nullptr) == (mCurrentDrawable == nullptr));

    if (mTexture != nullptr) {
        mTexture->APIDestroy();
        mTexture = nullptr;

        mCurrentDrawable = nullptr;
    }
}

}  // namespace dawn::native::metal
