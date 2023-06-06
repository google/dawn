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

#include "dawn/native/d3d/ExternalImageDXGIImpl.h"

#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d/Fence.h"
#include "dawn/native/d3d/Forward.h"
#include "dawn/native/d3d/TextureD3D.h"

namespace dawn::native::d3d {

MaybeError ValidateTextureDescriptorCanBeWrapped(const TextureDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    return {};
}

ExternalImageDXGIImpl::ExternalImageDXGIImpl(Device* backendDevice,
                                             ComPtr<IUnknown> d3dResource,
                                             const TextureDescriptor* textureDescriptor)
    : mBackendDevice(backendDevice),
      mD3DResource(std::move(d3dResource)),
      mUsage(textureDescriptor->usage),
      mDimension(textureDescriptor->dimension),
      mSize(textureDescriptor->size),
      mFormat(textureDescriptor->format),
      mMipLevelCount(textureDescriptor->mipLevelCount),
      mSampleCount(textureDescriptor->sampleCount),
      mViewFormats(textureDescriptor->viewFormats,
                   textureDescriptor->viewFormats + textureDescriptor->viewFormatCount) {
    ASSERT(mBackendDevice != nullptr);
    ASSERT(!textureDescriptor->nextInChain || textureDescriptor->nextInChain->sType ==
                                                  wgpu::SType::DawnTextureInternalUsageDescriptor);
    if (textureDescriptor->nextInChain) {
        mUsageInternal = reinterpret_cast<const wgpu::DawnTextureInternalUsageDescriptor*>(
                             textureDescriptor->nextInChain)
                             ->internalUsage;
    }
}

ExternalImageDXGIImpl::~ExternalImageDXGIImpl() {
    auto deviceLock(GetScopedDeviceLock());
    DestroyInternal();
}

Mutex::AutoLock ExternalImageDXGIImpl::GetScopedDeviceLock() const {
    if (mBackendDevice != nullptr) {
        return mBackendDevice->GetScopedLock();
    }
    return Mutex::AutoLock();
}

bool ExternalImageDXGIImpl::IsValid() const {
    auto deviceLock(GetScopedDeviceLock());

    return IsInList();
}

void ExternalImageDXGIImpl::DestroyInternal() {
    if (IsInList()) {
        mD3DResource = nullptr;
    }

    // Linked list is not thread safe. A mutex must already be locked before
    // endtering this method. Either via Device::DestroyImpl() or ~ExternalImageDXGIImpl.
    ASSERT(mBackendDevice == nullptr || mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    if (IsInList()) {
        RemoveFromList();
    }
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
    if (mBackendDevice->GetValidInternalFormat(mFormat).IsMultiPlanar() &&
        !descriptor->isInitialized) {
        bool consumed = mBackendDevice->ConsumedError(DAWN_VALIDATION_ERROR(
            "External textures with multiplanar formats must be initialized."));
        DAWN_UNUSED(consumed);
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
    }

    std::vector<Ref<Fence>> waitFences;
    for (const d3d::ExternalImageDXGIFenceDescriptor& fenceDescriptor : descriptor->waitFences) {
        Ref<Fence> fence;
        if (mBackendDevice->ConsumedError(
                ToBackend(mBackendDevice.Get())->CreateFence(&fenceDescriptor), &fence)) {
            dawn::ErrorLog() << "Unable to create D3D11 fence for external image";
            return nullptr;
        }
        waitFences.push_back(std::move(fence));
    }

    Ref<TextureBase> texture =
        ToBackend(mBackendDevice.Get())
            ->CreateD3DExternalTexture(&textureDescriptor, mD3DResource, std::move(waitFences),
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
        dawn::ErrorLog() << "D3D11 fence end access failed";
        return;
    }
    signalFence->fenceHandle = ToBackend(mBackendDevice.Get())->GetFenceHandle();
    signalFence->fenceValue = static_cast<uint64_t>(fenceValue);
}

}  // namespace dawn::native::d3d
