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
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d {

ExternalImageDXGIImpl::ExternalImageDXGIImpl(Device* backendDevice,
                                             const TextureDescriptor* textureDescriptor)
    : mBackendDevice(backendDevice),
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
    // Linked list is not thread safe. A mutex must already be locked before
    // endtering this method. Either via Device::DestroyImpl() or ~ExternalImageDXGIImpl.
    ASSERT(mBackendDevice == nullptr || mBackendDevice->IsLockedByCurrentThreadIfNeeded());
    if (IsInList()) {
        RemoveFromList();
    }
}

}  // namespace dawn::native::d3d
