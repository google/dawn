// Copyright 2022 The Dawn & Tint Authors
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
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    DAWN_ASSERT(mBackendDevice != nullptr);
    DAWN_ASSERT(!textureDescriptor->nextInChain ||
                textureDescriptor->nextInChain->sType ==
                    wgpu::SType::DawnTextureInternalUsageDescriptor);
    if (textureDescriptor->nextInChain) {
        mUsageInternal = reinterpret_cast<const wgpu::DawnTextureInternalUsageDescriptor*>(
                             textureDescriptor->nextInChain)
                             ->internalUsage;
    }

    // If the resource has IDXGIKeyedMutex interface, it will be used for synchronization.
    // TODO(dawn:1906): remove the mDXGIKeyedMutex when it is not used in chrome.
    mD3DResource.As(&mDXGIKeyedMutex);
}

ExternalImageDXGIImpl::~ExternalImageDXGIImpl() {
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    mDXGIKeyedMutexReleaser.reset();
    DestroyInternal();
}

Mutex::AutoLock ExternalImageDXGIImpl::GetScopedDeviceLock() const {
    return mBackendDevice->GetScopedLock();
}

bool ExternalImageDXGIImpl::IsValid() const {
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    return IsInList();
}

void ExternalImageDXGIImpl::DestroyInternal() {
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    if (IsInList()) {
        mD3DResource = nullptr;
    }

    if (IsInList()) {
        RemoveFromList();
    }
}

WGPUTexture ExternalImageDXGIImpl::BeginAccess(
    const d3d::ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    DAWN_ASSERT(descriptor != nullptr);

    if (!IsInList()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return nullptr;
    }

    // Ensure the texture usage is allowed
    if (!IsSubset(descriptor->usage, static_cast<WGPUTextureUsageFlags>(mUsage))) {
        dawn::ErrorLog() << "Texture usage is not valid for external image";
        return nullptr;
    }

    DAWN_ASSERT(mBackendDevice != nullptr);
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
    textureDescriptor.viewFormatCount = mViewFormats.size();

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

    if (mDXGIKeyedMutex && mAccessCount == 0) {
        HRESULT hr = mDXGIKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireKey, INFINITE);
        if (FAILED(hr)) {
            dawn::ErrorLog() << "Failed to acquire keyed mutex for external image";
            return nullptr;
        }
        mDXGIKeyedMutexReleaser.emplace(mDXGIKeyedMutex);
    }
    ++mAccessCount;

    return ToAPI(texture.Detach());
}

void ExternalImageDXGIImpl::EndAccess(WGPUTexture texture,
                                      d3d::ExternalImageDXGIFenceDescriptor* signalFence) {
    DAWN_ASSERT(mBackendDevice->IsLockedByCurrentThreadIfNeeded());

    if (!IsInList()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return;
    }

    DAWN_ASSERT(mBackendDevice != nullptr);
    DAWN_ASSERT(signalFence != nullptr);

    Texture* backendTexture = ToBackend(FromAPI(texture));
    DAWN_ASSERT(backendTexture != nullptr);

    ExecutionSerial fenceValue;
    if (mBackendDevice->ConsumedError(backendTexture->EndAccess(), &fenceValue)) {
        dawn::ErrorLog() << "D3D11 fence end access failed";
        return;
    }
    signalFence->fenceHandle = ToBackend(mBackendDevice.Get())->GetFenceHandle();
    signalFence->fenceValue = static_cast<uint64_t>(fenceValue);

    --mAccessCount;
    if (mDXGIKeyedMutexReleaser && mAccessCount == 0) {
        mDXGIKeyedMutexReleaser.reset();
    }
}

}  // namespace dawn::native::d3d
