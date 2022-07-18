// Copyright 2022 The Dawn Authors
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

#include "dawn/native/d3d12/ExternalImageDXGIImpl.h"

#include <d3d12.h>

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/d3d12/D3D11on12Util.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d12 {

ExternalImageDXGIImpl::ExternalImageDXGIImpl(Device* backendDevice,
                                             Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
                                             Microsoft::WRL::ComPtr<ID3D12Fence> d3d12Fence,
                                             const WGPUTextureDescriptor* descriptor)
    : mBackendDevice(backendDevice),
      mD3D12Resource(std::move(d3d12Resource)),
      mD3D12Fence(std::move(d3d12Fence)),
      mD3D11on12ResourceCache(std::make_unique<D3D11on12ResourceCache>()),
      mUsage(descriptor->usage),
      mDimension(descriptor->dimension),
      mSize(descriptor->size),
      mFormat(descriptor->format),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount) {
    ASSERT(mBackendDevice != nullptr);
    ASSERT(mD3D12Resource != nullptr);
    ASSERT(!descriptor->nextInChain ||
           descriptor->nextInChain->sType == WGPUSType_DawnTextureInternalUsageDescriptor);
    if (descriptor->nextInChain) {
        mUsageInternal =
            reinterpret_cast<const WGPUDawnTextureInternalUsageDescriptor*>(descriptor->nextInChain)
                ->internalUsage;
    }
}

ExternalImageDXGIImpl::~ExternalImageDXGIImpl() {
    Destroy();
}

bool ExternalImageDXGIImpl::IsValid() const {
    return IsInList();
}

void ExternalImageDXGIImpl::Destroy() {
    if (IsInList()) {
        RemoveFromList();

        // Keep fence alive until any pending signal calls are done on the GPU.
        mBackendDevice->ConsumedError(mBackendDevice->NextSerial());
        mBackendDevice->ReferenceUntilUnused(mD3D12Fence.Get());
        mBackendDevice = nullptr;

        mD3D12Resource.Reset();
        mD3D12Fence.Reset();
        mD3D11on12ResourceCache.reset();
    }
}

WGPUTexture ExternalImageDXGIImpl::ProduceTexture(
    const ExternalImageAccessDescriptorDXGISharedHandle* descriptor) {
    ASSERT(mBackendDevice != nullptr);
    // Ensure the texture usage is allowed
    if (!IsSubset(descriptor->usage, mUsage)) {
        dawn::ErrorLog() << "Texture usage is not valid for external image";
        return nullptr;
    }

    TextureDescriptor textureDescriptor = {};
    textureDescriptor.usage = static_cast<wgpu::TextureUsage>(descriptor->usage);
    textureDescriptor.dimension = static_cast<wgpu::TextureDimension>(mDimension);
    textureDescriptor.size = {mSize.width, mSize.height, mSize.depthOrArrayLayers};
    textureDescriptor.format = static_cast<wgpu::TextureFormat>(mFormat);
    textureDescriptor.mipLevelCount = mMipLevelCount;
    textureDescriptor.sampleCount = mSampleCount;

    DawnTextureInternalUsageDescriptor internalDesc = {};
    if (mUsageInternal) {
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = static_cast<wgpu::TextureUsage>(mUsageInternal);
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;
    }

    Ref<D3D11on12ResourceCacheEntry> d3d11on12Resource;
    if (!mD3D12Fence) {
        d3d11on12Resource = mD3D11on12ResourceCache->GetOrCreateD3D11on12Resource(
            mBackendDevice, mD3D12Resource.Get());
        if (d3d11on12Resource == nullptr) {
            dawn::ErrorLog() << "Unable to create 11on12 resource for external image";
            return nullptr;
        }
    }

    Ref<TextureBase> texture = mBackendDevice->CreateD3D12ExternalTexture(
        &textureDescriptor, mD3D12Resource, mD3D12Fence, std::move(d3d11on12Resource),
        descriptor->fenceWaitValue, descriptor->fenceSignalValue, descriptor->isSwapChainTexture,
        descriptor->isInitialized);
    return ToAPI(texture.Detach());
}

}  // namespace dawn::native::d3d12
