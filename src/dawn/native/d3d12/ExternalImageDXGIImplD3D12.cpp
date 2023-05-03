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

#include "dawn/native/d3d12/ExternalImageDXGIImplD3D12.h"

#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/Forward.h"

namespace dawn::native::d3d12 {

ExternalImageDXGIImpl::ExternalImageDXGIImpl(Device* backendDevice,
                                             Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
                                             const TextureDescriptor* textureDescriptor)
    : Base(backendDevice, textureDescriptor), mD3D12Resource(std::move(d3d12Resource)) {
    ASSERT(mD3D12Resource != nullptr);
}

ExternalImageDXGIImpl::~ExternalImageDXGIImpl() = default;

void ExternalImageDXGIImpl::DestroyInternal() {
    if (IsInList()) {
        mD3D12Resource = nullptr;
    }
    Base::DestroyInternal();
}

WGPUTexture ExternalImageDXGIImpl::BeginAccess(
    const d3d::ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    ASSERT(descriptor != nullptr);

    auto deviceLock(GetScopedDeviceLock());

    if (!IsInList()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return nullptr;
    }

    // Ensure the texture usage is allowed
    if (!IsSubset(descriptor->usage, static_cast<WGPUTextureUsageFlags>(mUsage))) {
        dawn::ErrorLog() << "Texture usage is not valid for external image";
        return nullptr;
    }

    ASSERT(mBackendDevice != nullptr);

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
    for (const d3d::ExternalImageDXGIFenceDescriptor& fenceDescriptor : descriptor->waitFences) {
        ASSERT(fenceDescriptor.fenceHandle != nullptr);
        // TODO(sunnyps): Use a fence cache instead of re-importing fences on each BeginAccess.
        Ref<Fence> fence;
        if (mBackendDevice->ConsumedError(
                Fence::CreateFromHandle(ToBackend(mBackendDevice.Get())->GetD3D12Device(),
                                        fenceDescriptor.fenceHandle, fenceDescriptor.fenceValue),
                &fence)) {
            dawn::ErrorLog() << "Unable to create D3D12 fence for external image";
            return nullptr;
        }
        waitFences.push_back(std::move(fence));
    }

    Ref<TextureBase> texture =
        ToBackend(mBackendDevice.Get())
            ->CreateD3D12ExternalTexture(&textureDescriptor, mD3D12Resource, std::move(waitFences),
                                         descriptor->isSwapChainTexture, descriptor->isInitialized);
    return ToAPI(texture.Detach());
}

void ExternalImageDXGIImpl::EndAccess(WGPUTexture texture,
                                      d3d::ExternalImageDXGIFenceDescriptor* signalFence) {
    auto deviceLock(GetScopedDeviceLock());

    if (!IsInList()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return;
    }

    ASSERT(mBackendDevice != nullptr);
    ASSERT(signalFence != nullptr);

    Texture* backendTexture = ToBackend(FromAPI(texture));
    ASSERT(backendTexture != nullptr);

    ExecutionSerial fenceValue;
    if (mBackendDevice->ConsumedError(backendTexture->EndAccess(), &fenceValue)) {
        dawn::ErrorLog() << "D3D12 fence end access failed";
        return;
    }
    signalFence->fenceHandle = ToBackend(mBackendDevice.Get())->GetFenceHandle();
    signalFence->fenceValue = static_cast<uint64_t>(fenceValue);
}

}  // namespace dawn::native::d3d12
