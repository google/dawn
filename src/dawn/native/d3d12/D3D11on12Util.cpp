// Copyright 2021 The Dawn Authors
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

// D3D12Backend.cpp: contains the definition of symbols exported by D3D12Backend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/d3d12/D3D11on12Util.h"

#include <utility>

#include "dawn/common/HashUtils.h"
#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d12 {

void Flush11On12DeviceToAvoidLeaks(ComPtr<ID3D11On12Device> d3d11on12Device) {
    if (d3d11on12Device == nullptr) {
        return;
    }

    ComPtr<ID3D11Device> d3d11Device;
    if (FAILED(d3d11on12Device.As(&d3d11Device))) {
        return;
    }

    ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    d3d11Device->GetImmediateContext(&d3d11DeviceContext);

    ASSERT(d3d11DeviceContext != nullptr);

    // 11on12 has a bug where D3D12 resources used only for keyed shared mutexes
    // are not released until work is submitted to the device context and flushed.
    // The most minimal work we can get away with is issuing a TiledResourceBarrier.

    // ID3D11DeviceContext2 is available in Win8.1 and above. This suffices for a
    // D3D12 backend since both D3D12 and 11on12 first appeared in Windows 10.
    ComPtr<ID3D11DeviceContext2> d3d11DeviceContext2;
    if (FAILED(d3d11DeviceContext.As(&d3d11DeviceContext2))) {
        return;
    }

    d3d11DeviceContext2->TiledResourceBarrier(nullptr, nullptr);
    d3d11DeviceContext2->Flush();
}

D3D11on12ResourceCacheEntry::D3D11on12ResourceCacheEntry(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex,
                                                         ComPtr<ID3D11On12Device> d3d11On12Device)
    : mDXGIKeyedMutex(std::move(dxgiKeyedMutex)), mD3D11on12Device(std::move(d3d11On12Device)) {}

D3D11on12ResourceCacheEntry::D3D11on12ResourceCacheEntry(ComPtr<ID3D11On12Device> d3d11On12Device)
    : mD3D11on12Device(std::move(d3d11On12Device)) {}

D3D11on12ResourceCacheEntry::~D3D11on12ResourceCacheEntry() {
    if (mDXGIKeyedMutex == nullptr) {
        return;
    }

    if (mAcquireCount > 0) {
        mDXGIKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireReleaseKey);
    }

    ComPtr<ID3D11Resource> d3d11Resource;
    if (FAILED(mDXGIKeyedMutex.As(&d3d11Resource))) {
        return;
    }

    ASSERT(mD3D11on12Device != nullptr);

    ID3D11Resource* d3d11ResourceRaw = d3d11Resource.Get();
    mD3D11on12Device->ReleaseWrappedResources(&d3d11ResourceRaw, 1);

    d3d11Resource.Reset();
    mDXGIKeyedMutex.Reset();

    Flush11On12DeviceToAvoidLeaks(std::move(mD3D11on12Device));
}

MaybeError D3D11on12ResourceCacheEntry::AcquireKeyedMutex() {
    ASSERT(mDXGIKeyedMutex != nullptr);
    ASSERT(mAcquireCount >= 0);
    if (mAcquireCount == 0) {
        DAWN_TRY(
            CheckHRESULT(mDXGIKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireReleaseKey, INFINITE),
                         "D3D12 acquiring shared mutex"));
    }
    mAcquireCount++;
    return {};
}

MaybeError D3D11on12ResourceCacheEntry::ReleaseKeyedMutex() {
    ASSERT(mDXGIKeyedMutex != nullptr);
    ASSERT(mAcquireCount > 0);
    mAcquireCount--;
    if (mAcquireCount == 0) {
        DAWN_TRY(CheckHRESULT(mDXGIKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireReleaseKey),
                              "D3D12 releasing keyed mutex"));
    }
    return {};
}

size_t D3D11on12ResourceCacheEntry::HashFunc::operator()(
    const Ref<D3D11on12ResourceCacheEntry> a) const {
    size_t hash = 0;
    HashCombine(&hash, a->mD3D11on12Device.Get());
    return hash;
}

bool D3D11on12ResourceCacheEntry::EqualityFunc::operator()(
    const Ref<D3D11on12ResourceCacheEntry> a,
    const Ref<D3D11on12ResourceCacheEntry> b) const {
    return a->mD3D11on12Device == b->mD3D11on12Device;
}

D3D11on12ResourceCache::D3D11on12ResourceCache() = default;

D3D11on12ResourceCache::~D3D11on12ResourceCache() = default;

Ref<D3D11on12ResourceCacheEntry> D3D11on12ResourceCache::GetOrCreateD3D11on12Resource(
    Device* backendDevice,
    ID3D12Resource* d3d12Resource) {
    // The Dawn and 11on12 device share the same D3D12 command queue whereas this external image
    // could be accessed/produced with multiple Dawn devices. To avoid cross-queue sharing
    // restrictions, the 11 wrapped resource is forbidden to be shared between Dawn devices by
    // using the 11on12 device as the cache key.
    ComPtr<ID3D11On12Device> d3d11on12Device = backendDevice->GetOrCreateD3D11on12Device();
    if (d3d11on12Device == nullptr) {
        dawn::ErrorLog() << "Unable to create 11on12 device for external image";
        return nullptr;
    }

    D3D11on12ResourceCacheEntry blueprint(d3d11on12Device);
    auto iter = mCache.find(&blueprint);
    if (iter != mCache.end()) {
        return *iter;
    }

    // We use IDXGIKeyedMutexes to synchronize access between D3D11 and D3D12. D3D11/12 fences
    // are a viable alternative but are, unfortunately, not available on all versions of Windows
    // 10. Since D3D12 does not directly support keyed mutexes, we need to wrap the D3D12
    // resource using 11on12 and QueryInterface the D3D11 representation for the keyed mutex.
    ComPtr<ID3D11Texture2D> d3d11Texture;
    D3D11_RESOURCE_FLAGS resourceFlags;
    resourceFlags.BindFlags = 0;
    resourceFlags.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    resourceFlags.CPUAccessFlags = 0;
    resourceFlags.StructureByteStride = 0;
    if (FAILED(d3d11on12Device->CreateWrappedResource(
            d3d12Resource, &resourceFlags, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COMMON,
            IID_PPV_ARGS(&d3d11Texture)))) {
        return nullptr;
    }

    ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
    if (FAILED(d3d11Texture.As(&dxgiKeyedMutex))) {
        return nullptr;
    }

    // Keep this cache from growing unbounded.
    // TODO(dawn:625): Consider using a replacement policy based cache.
    if (mCache.size() > kMaxD3D11on12ResourceCacheSize) {
        mCache.clear();
    }

    Ref<D3D11on12ResourceCacheEntry> entry =
        AcquireRef(new D3D11on12ResourceCacheEntry(dxgiKeyedMutex, std::move(d3d11on12Device)));
    mCache.insert(entry);

    return entry;
}

}  // namespace dawn::native::d3d12
