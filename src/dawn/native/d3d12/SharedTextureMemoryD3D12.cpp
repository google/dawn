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

    return AcquireRef(new SharedTextureMemory(device, label, properties, std::move(d3d12Resource)));
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
