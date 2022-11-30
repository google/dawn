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

#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/d3d12/D3D11on12Util.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d12 {

ExternalImageDXGIImpl::ExternalImageDXGIImpl(Device* backendDevice,
                                             Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
                                             const TextureDescriptor* textureDescriptor,
                                             bool useFenceSynchronization)
    : mBackendDevice(backendDevice),
      mD3D12Resource(std::move(d3d12Resource)),
      mUseFenceSynchronization(useFenceSynchronization),
      mD3D11on12ResourceCache(std::make_unique<D3D11on12ResourceCache>()),
      mUsage(textureDescriptor->usage),
      mDimension(textureDescriptor->dimension),
      mSize(textureDescriptor->size),
      mFormat(textureDescriptor->format),
      mMipLevelCount(textureDescriptor->mipLevelCount),
      mSampleCount(textureDescriptor->sampleCount),
      mViewFormats(textureDescriptor->viewFormats,
                   textureDescriptor->viewFormats + textureDescriptor->viewFormatCount) {
    ASSERT(mBackendDevice != nullptr);
    ASSERT(mD3D12Resource != nullptr);
    ASSERT(!textureDescriptor->nextInChain || textureDescriptor->nextInChain->sType ==
                                                  wgpu::SType::DawnTextureInternalUsageDescriptor);
    if (textureDescriptor->nextInChain) {
        mUsageInternal = reinterpret_cast<const wgpu::DawnTextureInternalUsageDescriptor*>(
                             textureDescriptor->nextInChain)
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
        mBackendDevice = nullptr;
        mD3D12Resource = nullptr;
        mD3D11on12ResourceCache = nullptr;
    }
}

WGPUTexture ExternalImageDXGIImpl::BeginAccess(
    const ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    ASSERT(mBackendDevice != nullptr);
    ASSERT(descriptor != nullptr);
    // Ensure the texture usage is allowed
    if (!IsSubset(descriptor->usage, static_cast<WGPUTextureUsageFlags>(mUsage))) {
        dawn::ErrorLog() << "Texture usage is not valid for external image";
        return nullptr;
    }

    TextureDescriptor textureDescriptor = {};
    textureDescriptor.usage = static_cast<wgpu::TextureUsage>(descriptor->usage);
    textureDescriptor.dimension = mDimension;
    textureDescriptor.size = {mSize.width, mSize.height, mSize.depthOrArrayLayers};
    textureDescriptor.format = mFormat;
    textureDescriptor.mipLevelCount = mMipLevelCount;
    textureDescriptor.sampleCount = mSampleCount;
    textureDescriptor.viewFormats = mViewFormats.data();
    textureDescriptor.viewFormatCount = static_cast<uint32_t>(mViewFormats.size());

    DawnTextureInternalUsageDescriptor internalDesc = {};
    if (mUsageInternal != wgpu::TextureUsage::None) {
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = mUsageInternal;
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;
    }

    std::vector<Ref<Fence>> waitFences;
    Ref<D3D11on12ResourceCacheEntry> d3d11on12Resource;
    if (mUseFenceSynchronization) {
        for (const ExternalImageDXGIFenceDescriptor& fenceDescriptor : descriptor->waitFences) {
            ASSERT(fenceDescriptor.fenceHandle != nullptr);
            // TODO(sunnyps): Use a fence cache instead of re-importing fences on each BeginAccess.
            Ref<Fence> fence;
            if (mBackendDevice->ConsumedError(
                    Fence::CreateFromHandle(mBackendDevice->GetD3D12Device(),
                                            fenceDescriptor.fenceHandle,
                                            fenceDescriptor.fenceValue),
                    &fence)) {
                dawn::ErrorLog() << "Unable to create D3D12 fence for external image";
                return nullptr;
            }
            waitFences.push_back(std::move(fence));
        }
    } else {
        d3d11on12Resource = mD3D11on12ResourceCache->GetOrCreateD3D11on12Resource(
            mBackendDevice.Get(), mD3D12Resource.Get());
        if (d3d11on12Resource == nullptr) {
            dawn::ErrorLog() << "Unable to create 11on12 resource for external image";
            return nullptr;
        }
    }

    Ref<TextureBase> texture = mBackendDevice->CreateD3D12ExternalTexture(
        &textureDescriptor, mD3D12Resource, std::move(waitFences), std::move(d3d11on12Resource),
        descriptor->isSwapChainTexture, descriptor->isInitialized);
    return ToAPI(texture.Detach());
}

void ExternalImageDXGIImpl::EndAccess(WGPUTexture texture,
                                      ExternalImageDXGIFenceDescriptor* signalFence) {
    ASSERT(signalFence != nullptr);

    Texture* backendTexture = ToBackend(FromAPI(texture));
    ASSERT(backendTexture != nullptr);

    if (mUseFenceSynchronization) {
        ExecutionSerial fenceValue;
        if (mBackendDevice->ConsumedError(backendTexture->EndAccess(), &fenceValue)) {
            dawn::ErrorLog() << "D3D12 fence end access failed";
            return;
        }
        signalFence->fenceHandle = mBackendDevice->GetFenceHandle();
        signalFence->fenceValue = static_cast<uint64_t>(fenceValue);
    }
}

}  // namespace dawn::native::d3d12
