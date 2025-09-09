// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/TextureWGPU.h"

#include <utility>

#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                            const UnpackedPtr<TextureDescriptor>& descriptor) {
    auto desc = ToAPI(*descriptor);

    WGPUTexture innerTexture = device->wgpu.deviceCreateTexture(device->GetInnerHandle(), desc);
    DAWN_ASSERT(innerTexture);

    return AcquireRef(new Texture(device, descriptor, innerTexture));
}

Texture::Texture(Device* device,
                 const UnpackedPtr<TextureDescriptor>& descriptor,
                 WGPUTexture innerTexture)
    : TextureBase(device, descriptor), ObjectWGPU(device->wgpu.textureRelease) {
    mInnerHandle = innerTexture;
}

void Texture::DestroyImpl() {
    TextureBase::DestroyImpl();
    auto& wgpu = ToBackend(GetDevice())->wgpu;
    wgpu.textureDestroy(mInnerHandle);
}

// TextureView

// static
ResultOrError<Ref<TextureView>> TextureView::Create(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    Device* device = ToBackend(texture->GetDevice());
    auto* desc = ToAPI(*descriptor);

    WGPUTextureView innerView =
        device->wgpu.textureCreateView(ToBackend(texture)->GetInnerHandle(), desc);
    DAWN_ASSERT(innerView);

    return AcquireRef(new TextureView(texture, descriptor, innerView));
}

TextureView::TextureView(TextureBase* texture,
                         const UnpackedPtr<TextureViewDescriptor>& descriptor,
                         WGPUTextureView innerView)
    : TextureViewBase(texture, descriptor),
      ObjectWGPU(ToBackend(texture->GetDevice())->wgpu.textureViewRelease) {
    mInnerHandle = innerView;
}

}  // namespace dawn::native::webgpu
