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

#ifndef SRC_DAWN_NATIVE_D3D12_D3D11ON12UTIL_H_
#define SRC_DAWN_NATIVE_D3D12_D3D11ON12UTIL_H_

#include <memory>
#include <unordered_set>

#include "dawn/common/RefCounted.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d12/d3d12_platform.h"

struct ID3D11On12Device;
struct IDXGIKeyedMutex;

namespace dawn::native::d3d12 {

class Device;

// Wraps 11 wrapped resources in a cache.
class D3D11on12ResourceCacheEntry : public RefCounted {
  public:
    explicit D3D11on12ResourceCacheEntry(ComPtr<ID3D11On12Device> d3d11on12Device);
    D3D11on12ResourceCacheEntry(ComPtr<IDXGIKeyedMutex> d3d11on12Resource,
                                ComPtr<ID3D11On12Device> d3d11on12Device);
    ~D3D11on12ResourceCacheEntry() override;

    MaybeError AcquireKeyedMutex();
    MaybeError ReleaseKeyedMutex();

    // Functors necessary for the
    // unordered_set<D3D11on12ResourceCacheEntry&>-based cache.
    struct HashFunc {
        size_t operator()(const Ref<D3D11on12ResourceCacheEntry> a) const;
    };

    struct EqualityFunc {
        bool operator()(const Ref<D3D11on12ResourceCacheEntry> a,
                        const Ref<D3D11on12ResourceCacheEntry> b) const;
    };

  private:
    ComPtr<IDXGIKeyedMutex> mDXGIKeyedMutex;
    ComPtr<ID3D11On12Device> mD3D11on12Device;
    int64_t mAcquireCount = 0;
};

// |D3D11on12ResourceCache| maintains a cache of 11 wrapped resources.
// Each entry represents a 11 resource that is exclusively accessed by Dawn device.
// Since each Dawn device creates and stores a 11on12 device, the 11on12 device
// is used as the key for the cache entry which ensures only the same 11 wrapped
// resource is re-used and also fully released.
//
// The cache is primarily needed to avoid repeatedly calling CreateWrappedResource
// and special release code per ProduceTexture(device).
class D3D11on12ResourceCache {
  public:
    D3D11on12ResourceCache();
    ~D3D11on12ResourceCache();

    Ref<D3D11on12ResourceCacheEntry> GetOrCreateD3D11on12Resource(Device* backendDevice,
                                                                  ID3D12Resource* d3d12Resource);

  private:
    // TODO(dawn:625): Figure out a large enough cache size.
    static constexpr uint64_t kMaxD3D11on12ResourceCacheSize = 5;

    // 11on12 resource cache entries are refcounted to ensure if the ExternalImage outlives the
    // Dawn texture (or vice-versa), we always fully release the 11 wrapped resource without
    // waiting until Dawn device to shutdown.
    using Cache = std::unordered_set<Ref<D3D11on12ResourceCacheEntry>,
                                     D3D11on12ResourceCacheEntry::HashFunc,
                                     D3D11on12ResourceCacheEntry::EqualityFunc>;

    Cache mCache;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_D3D11ON12UTIL_H_
