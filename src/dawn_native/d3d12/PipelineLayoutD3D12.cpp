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
#include <sstream>

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
                case kInternalStorageBufferBinding:
                    return D3D12_ROOT_PARAMETER_TYPE_UAV;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    return D3D12_ROOT_PARAMETER_TYPE_SRV;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
        Device* device,
        const PipelineLayoutDescriptor* descriptor) {
        Ref<PipelineLayout> layout = AcquireRef(new PipelineLayout(device, descriptor));
        DAWN_TRY(layout->Initialize());
        return layout;
    }

    MaybeError PipelineLayout::Initialize() {
        Device* device = ToBackend(GetDevice());
        // Parameters are D3D12_ROOT_PARAMETER_TYPE which is either a root table, constant, or
        // descriptor.
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;

        size_t rangesCount = 0;
        for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bindGroupLayout = ToBackend(GetBindGroupLayout(group));
            rangesCount += bindGroupLayout->GetCbvUavSrvDescriptorRanges().size() +
                           bindGroupLayout->GetSamplerDescriptorRanges().size();
        }

        // We are taking pointers to `ranges`, so we cannot let it resize while we're pushing to it.
        std::vector<D3D12_DESCRIPTOR_RANGE> ranges(rangesCount);

        uint32_t rangeIndex = 0;

        for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bindGroupLayout = ToBackend(GetBindGroupLayout(group));

            // Set the root descriptor table parameter and copy ranges. Ranges are offset by the
            // bind group index Returns whether or not the parameter was set. A root parameter is
            // not set if the number of ranges is 0
            auto SetRootDescriptorTable =
                [&](const std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges) -> bool {
                auto rangeCount = descriptorRanges.size();
                if (rangeCount == 0) {
                    return false;
                }

                D3D12_ROOT_PARAMETER rootParameter = {};
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParameter.DescriptorTable.NumDescriptorRanges = rangeCount;
                rootParameter.DescriptorTable.pDescriptorRanges = &ranges[rangeIndex];

                for (auto& range : descriptorRanges) {
                    ASSERT(range.RegisterSpace == kRegisterSpacePlaceholder);
                    ranges[rangeIndex] = range;
                    ranges[rangeIndex].RegisterSpace = static_cast<uint32_t>(group);
                    rangeIndex++;
                }

                rootParameters.emplace_back(rootParameter);

                return true;
            };

            if (SetRootDescriptorTable(bindGroupLayout->GetCbvUavSrvDescriptorRanges())) {
                mCbvUavSrvRootParameterInfo[group] = rootParameters.size() - 1;
            }
            if (SetRootDescriptorTable(bindGroupLayout->GetSamplerDescriptorRanges())) {
                mSamplerRootParameterInfo[group] = rootParameters.size() - 1;
            }

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
                rootDescriptor.ShaderRegister =
                    bindGroupLayout->GetShaderRegister(dynamicBindingIndex);
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

        // Make sure that we added exactly the number of elements we expected. If we added more,
        // |ranges| will have resized and the pointers in the |rootParameter|s will be invalid.
        ASSERT(rangeIndex == rangesCount);

        D3D12_ROOT_PARAMETER indexOffsetConstants{};
        indexOffsetConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        indexOffsetConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        // Always allocate 2 constants for vertex_index and instance_index
        // NOTE: We should consider delaying root signature creation until we know how many values
        // we need
        indexOffsetConstants.Constants.Num32BitValues = 2;
        indexOffsetConstants.Constants.RegisterSpace = kReservedRegisterSpace;
        indexOffsetConstants.Constants.ShaderRegister = kFirstOffsetInfoBaseRegister;
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
        HRESULT hr = device->GetFunctions()->d3d12SerializeRootSignature(
            &rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (DAWN_UNLIKELY(FAILED(hr))) {
            std::ostringstream messageStream;
            if (error) {
                messageStream << static_cast<const char*>(error->GetBufferPointer());

                // |error| is observed to always end with a \n, but is not
                // specified to do so, so we add an extra newline just in case.
                messageStream << std::endl;
            }
            messageStream << "D3D12 serialize root signature";
            DAWN_TRY(CheckHRESULT(hr, messageStream.str().c_str()));
        }
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
        return kReservedRegisterSpace;
    }

    uint32_t PipelineLayout::GetFirstIndexOffsetShaderRegister() const {
        return kFirstOffsetInfoBaseRegister;
    }

    uint32_t PipelineLayout::GetFirstIndexOffsetParameterIndex() const {
        return mFirstIndexOffsetParameterIndex;
    }
}}  // namespace dawn_native::d3d12
