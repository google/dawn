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

#ifndef SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_

#include <vector>

#include "dawn/native/BindGroupLayout.h"

#include "dawn/common/SlabAllocator.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class CPUDescriptorHeapAllocation;
class Device;
class StagingDescriptorAllocator;

// A purposefully invalid register space.
//
// We use the bind group index as the register space, but don't know the bind group index until
// pipeline layout creation time. This value should be replaced in PipelineLayoutD3D12.
static constexpr uint32_t kRegisterSpacePlaceholder =
    D3D12_DRIVER_RESERVED_REGISTER_SPACE_VALUES_START;

class BindGroupLayout final : public BindGroupLayoutBase {
  public:
    static Ref<BindGroupLayout> Create(Device* device,
                                       const BindGroupLayoutDescriptor* descriptor,
                                       PipelineCompatibilityToken pipelineCompatibilityToken);

    ResultOrError<Ref<BindGroup>> AllocateBindGroup(Device* device,
                                                    const BindGroupDescriptor* descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup, CPUDescriptorHeapAllocation* viewAllocation);

    // The offset (in descriptor count) into the corresponding descriptor heap. Not valid for
    // dynamic binding indexes.
    ityp::span<BindingIndex, const uint32_t> GetDescriptorHeapOffsets() const;

    // The D3D shader register that the Dawn binding index is mapped to by this bind group
    // layout.
    uint32_t GetShaderRegister(BindingIndex bindingIndex) const;

    // Counts of descriptors in the descriptor tables.
    uint32_t GetCbvUavSrvDescriptorCount() const;
    uint32_t GetSamplerDescriptorCount() const;

    const std::vector<D3D12_DESCRIPTOR_RANGE>& GetCbvUavSrvDescriptorRanges() const;
    const std::vector<D3D12_DESCRIPTOR_RANGE>& GetSamplerDescriptorRanges() const;

  private:
    BindGroupLayout(Device* device,
                    const BindGroupLayoutDescriptor* descriptor,
                    PipelineCompatibilityToken pipelineCompatibilityToken);
    ~BindGroupLayout() override = default;

    // Contains the offset into the descriptor heap for the given resource view. Samplers and
    // non-samplers are stored in separate descriptor heaps, so the offsets should be unique
    // within each group and tightly packed.
    //
    // Dynamic resources are not used here since their descriptors are placed directly in root
    // parameters.
    ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup> mDescriptorHeapOffsets;

    // Contains the shader register this binding is mapped to.
    ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup> mShaderRegisters;

    uint32_t mCbvUavSrvDescriptorCount;
    uint32_t mSamplerDescriptorCount;

    std::vector<D3D12_DESCRIPTOR_RANGE> mCbvUavSrvDescriptorRanges;
    std::vector<D3D12_DESCRIPTOR_RANGE> mSamplerDescriptorRanges;

    SlabAllocator<BindGroup> mBindGroupAllocator;

    StagingDescriptorAllocator* mSamplerAllocator = nullptr;
    StagingDescriptorAllocator* mViewAllocator = nullptr;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BINDGROUPLAYOUTD3D12_H_
