// Copyright 2017 The NXT Authors
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

#include "BindGroupLayoutD3D12.h"

#include "common/BitSetIterator.h"
#include "D3D12Backend.h"

namespace backend {
namespace d3d12 {

    BindGroupLayout::BindGroupLayout(Device* device, BindGroupLayoutBuilder* builder)
        : BindGroupLayoutBase(builder), device(device), descriptorCounts {}  {

        const auto& groupInfo = GetBindingInfo();

        for (uint32_t binding : IterateBitSet(groupInfo.mask)) {
            switch (groupInfo.types[binding]) {
                case nxt::BindingType::UniformBuffer:
                    bindingOffsets[binding] = descriptorCounts[CBV]++;
                    break;
                case nxt::BindingType::StorageBuffer:
                    bindingOffsets[binding] = descriptorCounts[UAV]++;
                    break;
                case nxt::BindingType::SampledTexture:
                    bindingOffsets[binding] = descriptorCounts[SRV]++;
                    break;
                case nxt::BindingType::Sampler:
                    bindingOffsets[binding] = descriptorCounts[Sampler]++;
                    break;
            }
        }

        auto SetDescriptorRange = [&](uint32_t index, uint32_t count, D3D12_DESCRIPTOR_RANGE_TYPE type) -> bool {
            if (count == 0) {
                return false;
            }

            auto& range = ranges[index];
            range.RangeType = type;
            range.NumDescriptors = count;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            // These ranges will be copied and range.BaseShaderRegister will be set in d3d12::PipelineLayout to account for bind group register offsets
            return true;
        };

        uint32_t rangeIndex = 0;

        // Ranges 0-2 contain the CBV, UAV, and SRV ranges, if they exist, tightly packed
        // Range 3 contains the Sampler range, if there is one
        if (SetDescriptorRange(rangeIndex, descriptorCounts[CBV], D3D12_DESCRIPTOR_RANGE_TYPE_CBV)) {
            rangeIndex++;
        }
        if (SetDescriptorRange(rangeIndex, descriptorCounts[UAV], D3D12_DESCRIPTOR_RANGE_TYPE_UAV)) {
            rangeIndex++;
        }
        if (SetDescriptorRange(rangeIndex, descriptorCounts[SRV], D3D12_DESCRIPTOR_RANGE_TYPE_SRV)) {
            rangeIndex++;
        }
        SetDescriptorRange(Sampler, descriptorCounts[Sampler], D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);

        // descriptors ranges are offset by the offset + size of the previous range
        std::array<uint32_t, DescriptorType::Count> descriptorOffsets;
        descriptorOffsets[CBV] = 0;
        descriptorOffsets[UAV] = descriptorOffsets[CBV] + descriptorCounts[CBV];
        descriptorOffsets[SRV] = descriptorOffsets[UAV] + descriptorCounts[UAV];
        descriptorOffsets[Sampler] = 0; // samplers are in a different heap

        for (uint32_t binding : IterateBitSet(groupInfo.mask)) {
            switch (groupInfo.types[binding]) {
                case nxt::BindingType::UniformBuffer:
                    bindingOffsets[binding] += descriptorOffsets[CBV];
                    break;
                case nxt::BindingType::StorageBuffer:
                    bindingOffsets[binding] += descriptorOffsets[UAV];
                    break;
                case nxt::BindingType::SampledTexture:
                    bindingOffsets[binding] += descriptorOffsets[SRV];
                    break;
                case nxt::BindingType::Sampler:
                    bindingOffsets[binding] += descriptorOffsets[Sampler];
                    break;
            }
        }
    }

    const std::array<uint32_t, kMaxBindingsPerGroup>& BindGroupLayout::GetBindingOffsets() const {
        return bindingOffsets;
    }

    uint32_t BindGroupLayout::GetCbvUavSrvDescriptorTableSize() const {
        return (
            static_cast<uint32_t>(descriptorCounts[CBV] > 0) +
            static_cast<uint32_t>(descriptorCounts[UAV] > 0) +
            static_cast<uint32_t>(descriptorCounts[SRV] > 0)
        );
    }

    uint32_t BindGroupLayout::GetSamplerDescriptorTableSize() const {
        return descriptorCounts[Sampler] > 0;
    }

    uint32_t BindGroupLayout::GetCbvUavSrvDescriptorCount() const {
        return descriptorCounts[CBV] + descriptorCounts[UAV] + descriptorCounts[SRV];
    }

    uint32_t BindGroupLayout::GetSamplerDescriptorCount() const {
        return descriptorCounts[Sampler];
    }

    const D3D12_DESCRIPTOR_RANGE* BindGroupLayout::GetCbvUavSrvDescriptorRanges() const {
        return ranges;
    }

    const D3D12_DESCRIPTOR_RANGE* BindGroupLayout::GetSamplerDescriptorRanges() const {
        return &ranges[Sampler];
    }

}
}
