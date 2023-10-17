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

#include "dawn/native/d3d12/SharedTextureMemoryD3D12.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SharedFenceD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    const char* label,
    const SharedTextureMemoryDXGISharedHandleDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle == nullptr, "shared HANDLE is missing.");

    ComPtr<ID3D12Resource> d3d12Resource;
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->OpenSharedHandle(descriptor->handle,
                                                                     IID_PPV_ARGS(&d3d12Resource)),
                          "D3D12 open shared handle"));

    D3D12_RESOURCE_DESC desc = d3d12Resource->GetDesc();
    DAWN_INVALID_IF(desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                    "Resource dimension (%d) was not Texture2D", desc.Dimension);
    DAWN_INVALID_IF(desc.DepthOrArraySize != 1, "Resource DepthOrArraySize (%d) was not 1",
                    desc.DepthOrArraySize);
    DAWN_INVALID_IF(desc.MipLevels != 1, "Resource MipLevels (%d) was not 1", desc.MipLevels);
    DAWN_INVALID_IF(desc.SampleDesc.Count != 1, "Resource SampleDesc.Count (%d) was not 1",
                    desc.SampleDesc.Count);

    const CombinedLimits& limits = device->GetLimits();
    DAWN_INVALID_IF(desc.Width > limits.v1.maxTextureDimension2D,
                    "Resource Width (%u) exceeds maxTextureDimension2D (%u).", desc.Width,
                    limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(desc.Height > limits.v1.maxTextureDimension2D,
                    "Resource Height (%u) exceeds maxTextureDimension2D (%u).", desc.Height,
                    limits.v1.maxTextureDimension2D);

    DAWN_INVALID_IF(!(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS),
                    "Resource did not have D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS flag");

    SharedTextureMemoryProperties properties;
    properties.size = {static_cast<uint32_t>(desc.Width), static_cast<uint32_t>(desc.Height), 1};

    DAWN_TRY_ASSIGN(properties.format, d3d::FromUncompressedColorDXGITextureFormat(desc.Format));

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    wgpu::TextureUsage storageBindingUsage =
        internalFormat->supportsStorageUsage &&
                (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            ? wgpu::TextureUsage::StorageBinding
            : wgpu::TextureUsage::None;

    wgpu::TextureUsage renderAttachmentUsage =
        internalFormat->isRenderable && (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
            ? wgpu::TextureUsage::RenderAttachment
            : wgpu::TextureUsage::None;

    if (internalFormat->IsMultiPlanar()) {
        properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;
    } else {
        properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                           wgpu::TextureUsage::TextureBinding | storageBindingUsage |
                           renderAttachmentUsage;
    }

    auto result =
        AcquireRef(new SharedTextureMemory(device, label, properties, std::move(d3d12Resource)));
    result->Initialize();
    return result;
}

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         const char* label,
                                         SharedTextureMemoryProperties properties,
                                         ComPtr<ID3D12Resource> resource)
    : d3d::SharedTextureMemory(device, label, properties, resource.Get()),
      mResource(std::move(resource)) {}

void SharedTextureMemory::DestroyImpl() {
    ToBackend(GetDevice())->ReferenceUntilUnused(std::move(mResource));
    mResource = nullptr;
}

ID3D12Resource* SharedTextureMemory::GetD3DResource() const {
    return mResource.Get();
}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const TextureDescriptor* descriptor) {
    return Texture::CreateFromSharedTextureMemory(this, descriptor);
}

ResultOrError<Ref<SharedFenceBase>> SharedTextureMemory::CreateFenceImpl(
    const SharedFenceDXGISharedHandleDescriptor* desc) {
    return SharedFence::Create(ToBackend(GetDevice()), "Internal shared DXGI fence", desc);
}

MaybeError SharedTextureMemory::BeginAccessImpl(TextureBase* texture,
                                                const BeginAccessDescriptor* descriptor) {
    DAWN_TRY(d3d::SharedTextureMemory::BeginAccessImpl(texture, descriptor));
    // Reset state to COMMON. BeginAccess contains a list of fences to wait on after
    // which the texture's usage will complete on the GPU.
    // All textures created from SharedTextureMemory must have
    // flag D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS. All textures with that flag are eligible
    // to decay to COMMON.
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#state-decay-to-common
    ToBackend(texture)->ResetSubresourceStateAndDecayToCommon();
    return {};
}

}  // namespace dawn::native::d3d12
