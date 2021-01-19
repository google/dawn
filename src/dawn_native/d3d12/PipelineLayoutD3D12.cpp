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

#include "dawn_native/d3d12/PipelineLayoutD3D12.h"

#include "common/Assert.h"
#include "common/BitSetIterator.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"

using Microsoft::WRL::ComPtr;

namespace dawn_native { namespace d3d12 {
    namespace {
        D3D12_SHADER_VISIBILITY ShaderVisibilityType(wgpu::ShaderStage visibility) {
            ASSERT(visibility != wgpu::ShaderStage::None);

            if (visibility == wgpu::ShaderStage::Vertex) {
                return D3D12_SHADER_VISIBILITY_VERTEX;
            }

            if (visibility == wgpu::ShaderStage::Fragment) {
                return D3D12_SHADER_VISIBILITY_PIXEL;
            }

            // For compute or any two combination of stages, visibility must be ALL
            return D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_ROOT_PARAMETER_TYPE RootParameterType(wgpu::BufferBindingType type) {
            switch (type) {
                case wgpu::BufferBindingType::Uniform:
                    return D3D12_ROOT_PARAMETER_TYPE_CBV;
                case wgpu::BufferBindingType::Storage:
                    return D3D12_ROOT_PARAMETER_TYPE_UAV;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    return D3D12_ROOT_PARAMETER_TYPE_SRV;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    ResultOrError<PipelineLayout*> PipelineLayout::Create(
        Device* device,
        const PipelineLayoutDescriptor* descriptor) {
        Ref<PipelineLayout> layout = AcquireRef(new PipelineLayout(device, descriptor));
        DAWN_TRY(layout->Initialize());
        return layout.Detach();
    }

    MaybeError PipelineLayout::Initialize() {
        Device* device = ToBackend(GetDevice());
        // Parameters are D3D12_ROOT_PARAMETER_TYPE which is either a root table, constant, or
        // descriptor.
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;

        // Ranges are D3D12_DESCRIPTOR_RANGE_TYPE_(SRV|UAV|CBV|SAMPLER)
        // They are grouped together so each bind group has at most 4 ranges
        D3D12_DESCRIPTOR_RANGE ranges[kMaxBindGroups * 4];

        uint32_t rangeIndex = 0;

        for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bindGroupLayout = ToBackend(GetBindGroupLayout(group));

            // Set the root descriptor table parameter and copy ranges. Ranges are offset by the
            // bind group index Returns whether or not the parameter was set. A root parameter is
            // not set if the number of ranges is 0
            auto SetRootDescriptorTable =
                [&](uint32_t rangeCount, const D3D12_DESCRIPTOR_RANGE* descriptorRanges) -> bool {
                if (rangeCount == 0) {
                    return false;
                }

                D3D12_ROOT_PARAMETER rootParameter = {};
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParameter.DescriptorTable.NumDescriptorRanges = rangeCount;
                rootParameter.DescriptorTable.pDescriptorRanges = &ranges[rangeIndex];

                for (uint32_t i = 0; i < rangeCount; ++i) {
                    ranges[rangeIndex] = descriptorRanges[i];
                    ranges[rangeIndex].RegisterSpace = static_cast<uint32_t>(group);
                    rangeIndex++;
                }

                rootParameters.emplace_back(rootParameter);

                return true;
            };

            if (SetRootDescriptorTable(bindGroupLayout->GetCbvUavSrvDescriptorTableSize(),
                                       bindGroupLayout->GetCbvUavSrvDescriptorRanges())) {
                mCbvUavSrvRootParameterInfo[group] = rootParameters.size() - 1;
            }

            if (SetRootDescriptorTable(bindGroupLayout->GetSamplerDescriptorTableSize(),
                                       bindGroupLayout->GetSamplerDescriptorRanges())) {
                mSamplerRootParameterInfo[group] = rootParameters.size() - 1;
            }

            // Get calculated shader register for root descriptors
            const auto& shaderRegisters = bindGroupLayout->GetBindingOffsets();

            // Init root descriptors in root signatures for dynamic buffer bindings.
            // These are packed at the beginning of the layout binding info.
            for (BindingIndex dynamicBindingIndex{0};
                 dynamicBindingIndex < bindGroupLayout->GetDynamicBufferCount();
                 ++dynamicBindingIndex) {
                const BindingInfo& bindingInfo =
                    bindGroupLayout->GetBindingInfo(dynamicBindingIndex);

                if (bindingInfo.visibility == wgpu::ShaderStage::None) {
                    // Skip dynamic buffers that are not visible. D3D12 does not have None
                    // visibility.
                    continue;
                }

                D3D12_ROOT_PARAMETER rootParameter = {};

                // Setup root descriptor.
                D3D12_ROOT_DESCRIPTOR rootDescriptor;
                rootDescriptor.ShaderRegister = shaderRegisters[dynamicBindingIndex];
                rootDescriptor.RegisterSpace = static_cast<uint32_t>(group);

                // Set root descriptors in root signatures.
                rootParameter.Descriptor = rootDescriptor;
                mDynamicRootParameterIndices[group][dynamicBindingIndex] = rootParameters.size();

                // Set parameter types according to bind group layout descriptor.
                rootParameter.ParameterType = RootParameterType(bindingInfo.buffer.type);

                // Set visibilities according to bind group layout descriptor.
                rootParameter.ShaderVisibility = ShaderVisibilityType(bindingInfo.visibility);

                rootParameters.emplace_back(rootParameter);
            }
        }

        // Since Tint's HLSL writer doesn't currently map sets to spaces, we use the default space
        // (0).
        mFirstIndexOffsetRegisterSpace = 0;
        BindGroupIndex firstOffsetGroup{mFirstIndexOffsetRegisterSpace};
        if (GetBindGroupLayoutsMask()[firstOffsetGroup]) {
            // Find the last register used on firstOffsetGroup.
            uint32_t maxRegister = 0;
            for (uint32_t shaderRegister :
                 ToBackend(GetBindGroupLayout(firstOffsetGroup))->GetBindingOffsets()) {
                if (shaderRegister > maxRegister) {
                    maxRegister = shaderRegister;
                }
            }
            mFirstIndexOffsetShaderRegister = maxRegister + 1;
        } else {
            // firstOffsetGroup is not in use, we can use the first register.
            mFirstIndexOffsetShaderRegister = 0;
        }

        D3D12_ROOT_PARAMETER indexOffsetConstants{};
        indexOffsetConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        indexOffsetConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        // Always allocate 2 constants for vertex_index and instance_index
        // NOTE: We should consider delaying root signature creation until we know how many values
        // we need
        indexOffsetConstants.Constants.Num32BitValues = 2;
        indexOffsetConstants.Constants.RegisterSpace = mFirstIndexOffsetRegisterSpace;
        indexOffsetConstants.Constants.ShaderRegister = mFirstIndexOffsetShaderRegister;
        mFirstIndexOffsetParameterIndex = rootParameters.size();
        // NOTE: We should consider moving this entry to earlier in the root signature since offsets
        // would need to be updated often
        rootParameters.emplace_back(indexOffsetConstants);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
        rootSignatureDescriptor.NumParameters = rootParameters.size();
        rootSignatureDescriptor.pParameters = rootParameters.data();
        rootSignatureDescriptor.NumStaticSamplers = 0;
        rootSignatureDescriptor.pStaticSamplers = nullptr;
        rootSignatureDescriptor.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        DAWN_TRY(CheckHRESULT(
            device->GetFunctions()->d3d12SerializeRootSignature(
                &rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error),
            "D3D12 serialize root signature"));
        DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->CreateRootSignature(
                                  0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                  IID_PPV_ARGS(&mRootSignature)),
                              "D3D12 create root signature"));
        return {};
    }

    uint32_t PipelineLayout::GetCbvUavSrvRootParameterIndex(BindGroupIndex group) const {
        ASSERT(group < kMaxBindGroupsTyped);
        return mCbvUavSrvRootParameterInfo[group];
    }

    uint32_t PipelineLayout::GetSamplerRootParameterIndex(BindGroupIndex group) const {
        ASSERT(group < kMaxBindGroupsTyped);
        return mSamplerRootParameterInfo[group];
    }

    ID3D12RootSignature* PipelineLayout::GetRootSignature() const {
        return mRootSignature.Get();
    }

    uint32_t PipelineLayout::GetDynamicRootParameterIndex(BindGroupIndex group,
                                                          BindingIndex bindingIndex) const {
        ASSERT(group < kMaxBindGroupsTyped);
        ASSERT(bindingIndex < kMaxDynamicBuffersPerPipelineLayoutTyped);
        ASSERT(GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).buffer.hasDynamicOffset);
        ASSERT(GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).visibility !=
               wgpu::ShaderStage::None);
        return mDynamicRootParameterIndices[group][bindingIndex];
    }

    uint32_t PipelineLayout::GetFirstIndexOffsetRegisterSpace() const {
        return mFirstIndexOffsetRegisterSpace;
    }

    uint32_t PipelineLayout::GetFirstIndexOffsetShaderRegister() const {
        return mFirstIndexOffsetShaderRegister;
    }

    uint32_t PipelineLayout::GetFirstIndexOffsetParameterIndex() const {
        return mFirstIndexOffsetParameterIndex;
    }
}}  // namespace dawn_native::d3d12
