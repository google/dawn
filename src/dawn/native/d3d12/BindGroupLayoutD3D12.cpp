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

#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"

#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"

namespace dawn::native::d3d12 {
namespace {
D3D12_DESCRIPTOR_RANGE_TYPE WGPUBindingInfoToDescriptorRangeType(const BindingInfo& bindingInfo) {
    switch (bindingInfo.bindingType) {
        case BindingInfoType::Buffer:
            switch (bindingInfo.buffer.type) {
                case wgpu::BufferBindingType::Uniform:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }

        case BindingInfoType::Sampler:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

        case BindingInfoType::Texture:
        case BindingInfoType::ExternalTexture:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

        case BindingInfoType::StorageTexture:
            switch (bindingInfo.storageTexture.access) {
                case wgpu::StorageTextureAccess::WriteOnly:
                    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                case wgpu::StorageTextureAccess::Undefined:
                    UNREACHABLE();
            }
    }
}
}  // anonymous namespace

// static
Ref<BindGroupLayout> BindGroupLayout::Create(
    Device* device,
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    return AcquireRef(new BindGroupLayout(device, descriptor, pipelineCompatibilityToken));
}

BindGroupLayout::BindGroupLayout(Device* device,
                                 const BindGroupLayoutDescriptor* descriptor,
                                 PipelineCompatibilityToken pipelineCompatibilityToken)
    : BindGroupLayoutBase(device, descriptor, pipelineCompatibilityToken),
      mDescriptorHeapOffsets(GetBindingCount()),
      mShaderRegisters(GetBindingCount()),
      mCbvUavSrvDescriptorCount(0),
      mSamplerDescriptorCount(0),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {
    for (BindingIndex bindingIndex{0}; bindingIndex < GetBindingCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);

        D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType =
            WGPUBindingInfoToDescriptorRangeType(bindingInfo);
        mShaderRegisters[bindingIndex] = uint32_t(bindingInfo.binding);

        // For dynamic resources, Dawn uses root descriptor in D3D12 backend. So there is no
        // need to allocate the descriptor from descriptor heap or create descriptor ranges.
        if (bindingIndex < GetDynamicBufferCount()) {
            continue;
        }
        ASSERT(!bindingInfo.buffer.hasDynamicOffset);

        mDescriptorHeapOffsets[bindingIndex] =
            descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
                ? mSamplerDescriptorCount++
                : mCbvUavSrvDescriptorCount++;

        D3D12_DESCRIPTOR_RANGE range;
        range.RangeType = descriptorRangeType;
        range.NumDescriptors = 1;
        range.BaseShaderRegister = GetShaderRegister(bindingIndex);
        range.RegisterSpace = kRegisterSpacePlaceholder;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges =
            descriptorRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ? mSamplerDescriptorRanges
                                                                       : mCbvUavSrvDescriptorRanges;

        // Try to join this range with the previous one, if the current range is a continuation
        // of the previous. This is possible because the binding infos in the base type are
        // sorted.
        if (descriptorRanges.size() >= 2) {
            D3D12_DESCRIPTOR_RANGE& previous = descriptorRanges.back();
            if (previous.RangeType == range.RangeType &&
                previous.BaseShaderRegister + previous.NumDescriptors == range.BaseShaderRegister) {
                previous.NumDescriptors += range.NumDescriptors;
                continue;
            }
        }

        descriptorRanges.push_back(range);
    }

    mViewAllocator = device->GetViewStagingDescriptorAllocator(GetCbvUavSrvDescriptorCount());
    mSamplerAllocator = device->GetSamplerStagingDescriptorAllocator(GetSamplerDescriptorCount());
}

ResultOrError<Ref<BindGroup>> BindGroupLayout::AllocateBindGroup(
    Device* device,
    const BindGroupDescriptor* descriptor) {
    uint32_t viewSizeIncrement = 0;
    CPUDescriptorHeapAllocation viewAllocation;
    if (GetCbvUavSrvDescriptorCount() > 0) {
        DAWN_TRY_ASSIGN(viewAllocation, mViewAllocator->AllocateCPUDescriptors());
        viewSizeIncrement = mViewAllocator->GetSizeIncrement();
    }

    Ref<BindGroup> bindGroup = AcquireRef<BindGroup>(
        mBindGroupAllocator.Allocate(device, descriptor, viewSizeIncrement, viewAllocation));

    if (GetSamplerDescriptorCount() > 0) {
        Ref<SamplerHeapCacheEntry> samplerHeapCacheEntry;
        DAWN_TRY_ASSIGN(samplerHeapCacheEntry, device->GetSamplerHeapCache()->GetOrCreate(
                                                   bindGroup.Get(), mSamplerAllocator));
        bindGroup->SetSamplerAllocationEntry(std::move(samplerHeapCacheEntry));
    }

    return bindGroup;
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup,
                                          CPUDescriptorHeapAllocation* viewAllocation) {
    if (viewAllocation->IsValid()) {
        mViewAllocator->Deallocate(viewAllocation);
    }

    mBindGroupAllocator.Deallocate(bindGroup);
}

ityp::span<BindingIndex, const uint32_t> BindGroupLayout::GetDescriptorHeapOffsets() const {
    return {mDescriptorHeapOffsets.data(), mDescriptorHeapOffsets.size()};
}

uint32_t BindGroupLayout::GetShaderRegister(BindingIndex bindingIndex) const {
    return mShaderRegisters[bindingIndex];
}

uint32_t BindGroupLayout::GetCbvUavSrvDescriptorCount() const {
    return mCbvUavSrvDescriptorCount;
}

uint32_t BindGroupLayout::GetSamplerDescriptorCount() const {
    return mSamplerDescriptorCount;
}

const std::vector<D3D12_DESCRIPTOR_RANGE>& BindGroupLayout::GetCbvUavSrvDescriptorRanges() const {
    return mCbvUavSrvDescriptorRanges;
}

const std::vector<D3D12_DESCRIPTOR_RANGE>& BindGroupLayout::GetSamplerDescriptorRanges() const {
    return mSamplerDescriptorRanges;
}

}  // namespace dawn::native::d3d12
