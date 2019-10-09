// Copyright 2019 The Dawn Authors
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

#include "dawn_native/d3d12/ResourceHeapAllocationD3D12.h"

namespace dawn_native { namespace d3d12 {
    ResourceHeapAllocation::ResourceHeapAllocation(const AllocationInfo& info,
                                                   uint64_t offset,
                                                   ComPtr<ID3D12Resource> resource)
        : ResourceMemoryAllocation(info, offset, nullptr), mResource(std::move(resource)) {
    }

    ComPtr<ID3D12Resource> ResourceHeapAllocation::GetD3D12Resource() const {
        return mResource;
    }

    D3D12_GPU_VIRTUAL_ADDRESS ResourceHeapAllocation::GetGPUPointer() const {
        return mResource->GetGPUVirtualAddress();
    }
}}  // namespace dawn_native::d3d12