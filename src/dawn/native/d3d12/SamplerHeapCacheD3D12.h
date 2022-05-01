// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_

#include <unordered_set>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"

// |SamplerHeapCacheEntry| maintains a cache of sampler descriptor heap allocations.
// Each entry represents one or more sampler descriptors that co-exist in a CPU and
// GPU descriptor heap. The CPU-side allocation is deallocated once the final reference
// has been released while the GPU-side allocation is deallocated when the GPU is finished.
//
// The BindGroupLayout hands out these entries upon constructing the bindgroup. If the entry is not
// invalid, it will allocate and initialize so it may be reused by another bindgroup.
//
// The cache is primary needed for the GPU sampler heap, which is much smaller than the view heap
// and switches incur expensive pipeline flushes.
namespace dawn::native::d3d12 {

class BindGroup;
class Device;
class Sampler;
class SamplerHeapCache;
class StagingDescriptorAllocator;
class ShaderVisibleDescriptorAllocator;

// Wraps sampler descriptor heap allocations in a cache.
class SamplerHeapCacheEntry : public RefCounted {
  public:
    SamplerHeapCacheEntry() = default;
    explicit SamplerHeapCacheEntry(std::vector<Sampler*> samplers);
    SamplerHeapCacheEntry(SamplerHeapCache* cache,
                          StagingDescriptorAllocator* allocator,
                          std::vector<Sampler*> samplers,
                          CPUDescriptorHeapAllocation allocation);
    ~SamplerHeapCacheEntry() override;

    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor() const;

    std::vector<Sampler*>&& AcquireSamplers();

    bool Populate(Device* device, ShaderVisibleDescriptorAllocator* allocator);

    // Functors necessary for the unordered_map<SamplerHeapCacheEntry*>-based cache.
    struct HashFunc {
        size_t operator()(const SamplerHeapCacheEntry* entry) const;
    };

    struct EqualityFunc {
        bool operator()(const SamplerHeapCacheEntry* a, const SamplerHeapCacheEntry* b) const;
    };

  private:
    CPUDescriptorHeapAllocation mCPUAllocation;
    GPUDescriptorHeapAllocation mGPUAllocation;

    // Storing raw pointer because the sampler object will be already hashed
    // by the device and will already be unique.
    std::vector<Sampler*> mSamplers;

    StagingDescriptorAllocator* mAllocator = nullptr;
    SamplerHeapCache* mCache = nullptr;
};

// Cache descriptor heap allocations so that we don't create duplicate ones for every
// BindGroup.
class SamplerHeapCache {
  public:
    explicit SamplerHeapCache(Device* device);
    ~SamplerHeapCache();

    ResultOrError<Ref<SamplerHeapCacheEntry>> GetOrCreate(
        const BindGroup* group,
        StagingDescriptorAllocator* samplerAllocator);

    void RemoveCacheEntry(SamplerHeapCacheEntry* entry);

  private:
    Device* mDevice;

    using Cache = std::unordered_set<SamplerHeapCacheEntry*,
                                     SamplerHeapCacheEntry::HashFunc,
                                     SamplerHeapCacheEntry::EqualityFunc>;

    Cache mCache;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_SAMPLERHEAPCACHED3D12_H_
