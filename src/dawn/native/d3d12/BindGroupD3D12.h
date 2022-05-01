// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_

#include "dawn/common/PlacementAllocated.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"

namespace dawn::native::d3d12 {

class Device;
class SamplerHeapCacheEntry;
class ShaderVisibleDescriptorAllocator;

class BindGroup final : public BindGroupBase, public PlacementAllocated {
  public:
    static ResultOrError<Ref<BindGroup>> Create(Device* device,
                                                const BindGroupDescriptor* descriptor);

    BindGroup(Device* device,
              const BindGroupDescriptor* descriptor,
              uint32_t viewSizeIncrement,
              const CPUDescriptorHeapAllocation& viewAllocation);

    // Returns true if the BindGroup was successfully populated.
    bool PopulateViews(ShaderVisibleDescriptorAllocator* viewAllocator);
    bool PopulateSamplers(Device* device, ShaderVisibleDescriptorAllocator* samplerAllocator);

    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseViewDescriptor() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseSamplerDescriptor() const;

    void SetSamplerAllocationEntry(Ref<SamplerHeapCacheEntry> entry);

    using DynamicStorageBufferLengths =
        ityp::stack_vec<uint32_t, uint32_t, kMaxDynamicStorageBuffersPerPipelineLayout>;
    const DynamicStorageBufferLengths& GetDynamicStorageBufferLengths() const;

  private:
    ~BindGroup() override;

    void DestroyImpl() override;

    Ref<SamplerHeapCacheEntry> mSamplerAllocationEntry;

    GPUDescriptorHeapAllocation mGPUViewAllocation;
    CPUDescriptorHeapAllocation mCPUViewAllocation;

    DynamicStorageBufferLengths mDynamicStorageBufferLengths;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BINDGROUPD3D12_H_
