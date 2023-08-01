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

#include "dawn/native/d3d12/PipelineLayoutD3D12.h"

#include <limits>
#include <sstream>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"

using Microsoft::WRL::ComPtr;

namespace dawn::native::d3d12 {
namespace {

// Reserve register names for internal use. This registers map to bindings in the shader,
// but are not directly related to allocation of the root signature.
// In the root signature, it the index of the root parameter where these registers are
// used that determines the layout of the root signature.
static constexpr uint32_t kRenderOrComputeInternalRegisterSpace = kMaxBindGroups + 1;
static constexpr uint32_t kRenderOrComputeInternalBaseRegister = 0;

static constexpr uint32_t kDynamicStorageBufferLengthsRegisterSpace = kMaxBindGroups + 2;
static constexpr uint32_t kDynamicStorageBufferLengthsBaseRegister = 0;

static constexpr uint32_t kInvalidDynamicStorageBufferLengthsParameterIndex =
    std::numeric_limits<uint32_t>::max();

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

HRESULT SerializeRootParameter1_0(Device* device,
                                  const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& rootSignature1_1,
                                  ID3DBlob** ppBlob,
                                  ID3DBlob** ppErrorBlob) {
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> allDescriptorRanges1_0;
    std::vector<D3D12_ROOT_PARAMETER> rootParameters1_0(rootSignature1_1.Desc_1_1.NumParameters);
    for (size_t i = 0; i < rootParameters1_0.size(); ++i) {
        const D3D12_ROOT_PARAMETER1& rootParameter1_1 = rootSignature1_1.Desc_1_1.pParameters[i];

        rootParameters1_0[i].ParameterType = rootParameter1_1.ParameterType;
        rootParameters1_0[i].ShaderVisibility = rootParameter1_1.ShaderVisibility;

        switch (rootParameters1_0[i].ParameterType) {
            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                rootParameters1_0[i].Constants = rootParameter1_1.Constants;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                rootParameters1_0[i].Descriptor.RegisterSpace =
                    rootParameter1_1.Descriptor.RegisterSpace;
                rootParameters1_0[i].Descriptor.ShaderRegister =
                    rootParameter1_1.Descriptor.ShaderRegister;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                rootParameters1_0[i].DescriptorTable.NumDescriptorRanges =
                    rootParameter1_1.DescriptorTable.NumDescriptorRanges;
                if (rootParameters1_0[i].DescriptorTable.NumDescriptorRanges > 0) {
                    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges1_0(
                        rootParameters1_0[i].DescriptorTable.NumDescriptorRanges);
                    for (uint32_t index = 0;
                         index < rootParameter1_1.DescriptorTable.NumDescriptorRanges; ++index) {
                        const D3D12_DESCRIPTOR_RANGE1& descriptorRange1_1 =
                            rootParameter1_1.DescriptorTable.pDescriptorRanges[index];
                        descriptorRanges1_0[index].BaseShaderRegister =
                            descriptorRange1_1.BaseShaderRegister;
                        descriptorRanges1_0[index].NumDescriptors =
                            descriptorRange1_1.NumDescriptors;
                        descriptorRanges1_0[index].OffsetInDescriptorsFromTableStart =
                            descriptorRange1_1.OffsetInDescriptorsFromTableStart;
                        descriptorRanges1_0[index].RangeType = descriptorRange1_1.RangeType;
                        descriptorRanges1_0[index].RegisterSpace = descriptorRange1_1.RegisterSpace;
                    }
                    allDescriptorRanges1_0.push_back(descriptorRanges1_0);
                    rootParameters1_0[i].DescriptorTable.pDescriptorRanges =
                        allDescriptorRanges1_0.back().data();
                }
                break;

            default:
                UNREACHABLE();
                break;
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
    rootSignatureDescriptor.NumParameters = rootParameters1_0.size();
    rootSignatureDescriptor.pParameters = rootParameters1_0.data();
    rootSignatureDescriptor.NumStaticSamplers = 0;
    rootSignatureDescriptor.pStaticSamplers = nullptr;
    rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    return device->GetFunctions()->d3d12SerializeRootSignature(
        &rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
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
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters;

    size_t rangesCount = 0;
    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bindGroupLayout =
            ToBackend(GetBindGroupLayout(group)->GetInternalBindGroupLayout());
        rangesCount += bindGroupLayout->GetCbvUavSrvDescriptorRanges().size() +
                       bindGroupLayout->GetSamplerDescriptorRanges().size();
    }

    // We are taking pointers to `ranges`, so we cannot let it resize while we're pushing to it.
    std::vector<D3D12_DESCRIPTOR_RANGE1> ranges(rangesCount);

    uint32_t rangeIndex = 0;

    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bindGroupLayout =
            ToBackend(GetBindGroupLayout(group)->GetInternalBindGroupLayout());

        // Set the root descriptor table parameter and copy ranges. Ranges are offset by the
        // bind group index Returns whether or not the parameter was set. A root parameter is
        // not set if the number of ranges is 0
        auto SetRootDescriptorTable =
            [&](const std::vector<D3D12_DESCRIPTOR_RANGE1>& descriptorRanges) -> bool {
            auto rangeCount = descriptorRanges.size();
            if (rangeCount == 0) {
                return false;
            }

            D3D12_ROOT_PARAMETER1 rootParameter = {};
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
        mDynamicRootParameterIndices[group].resize(bindGroupLayout->GetDynamicBufferCount());
        for (BindingIndex dynamicBindingIndex{0};
             dynamicBindingIndex < bindGroupLayout->GetDynamicBufferCount();
             ++dynamicBindingIndex) {
            const BindingInfo& bindingInfo = bindGroupLayout->GetBindingInfo(dynamicBindingIndex);

            if (bindingInfo.visibility == wgpu::ShaderStage::None) {
                // Skip dynamic buffers that are not visible. D3D12 does not have None
                // visibility.
                continue;
            }

            D3D12_ROOT_PARAMETER1 rootParameter = {};

            // Setup root descriptor.
            D3D12_ROOT_DESCRIPTOR1 rootDescriptor;
            rootDescriptor.ShaderRegister = bindGroupLayout->GetShaderRegister(dynamicBindingIndex);
            rootDescriptor.RegisterSpace = static_cast<uint32_t>(group);
            // Using D3D12_ROOT_DESCRIPTOR_FLAG_NONE means using DATA_STATIC_WHILE_SET_AT_EXECUTE
            // for CBV/SRV and using DATA_VOLATILE for UAV, which is allowed because currently in
            // Dawn the views with dynamic offsets are always re-applied every time before a draw or
            // a dispatch call.
            rootDescriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;

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

    D3D12_ROOT_PARAMETER1 renderOrComputeInternalConstants{};
    renderOrComputeInternalConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    renderOrComputeInternalConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    // Always allocate 3 constants for either:
    //  - vertex_index and instance_index
    //  - num_workgroups_x, num_workgroups_y and num_workgroups_z
    // NOTE: We should consider delaying root signature creation until we know how many values
    // we need
    renderOrComputeInternalConstants.Constants.Num32BitValues = 3;
    renderOrComputeInternalConstants.Constants.RegisterSpace =
        kRenderOrComputeInternalRegisterSpace;
    renderOrComputeInternalConstants.Constants.ShaderRegister =
        kRenderOrComputeInternalBaseRegister;
    mFirstIndexOffsetParameterIndex = rootParameters.size();
    mNumWorkgroupsParameterIndex = rootParameters.size();
    // NOTE: We should consider moving this entry to earlier in the root signature since offsets
    // would need to be updated often
    rootParameters.emplace_back(renderOrComputeInternalConstants);

    // Loops over all of the dynamic storage buffer bindings in the layout and build
    // a mapping from the binding to the next offset into the root constant array where
    // that dynamic storage buffer's binding size will be stored. The next register offset
    // to use is tracked with |dynamicStorageBufferLengthsShaderRegisterOffset|.
    // This data will be used by shader translation to emit a load from the root constant
    // array to use as the binding's size in runtime array calculations.
    // Each bind group's length data is stored contiguously in the root constant array,
    // so the loop also computes the first register offset for each group where the
    // data should start.
    uint32_t dynamicStorageBufferLengthsShaderRegisterOffset = 0;
    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bgl = GetBindGroupLayout(group);

        mDynamicStorageBufferLengthInfo[group].firstRegisterOffset =
            dynamicStorageBufferLengthsShaderRegisterOffset;
        mDynamicStorageBufferLengthInfo[group].bindingAndRegisterOffsets.reserve(
            bgl->GetBindingCountInfo().dynamicStorageBufferCount);

        for (BindingIndex bindingIndex(0); bindingIndex < bgl->GetDynamicBufferCount();
             ++bindingIndex) {
            if (bgl->IsStorageBufferBinding(bindingIndex)) {
                mDynamicStorageBufferLengthInfo[group].bindingAndRegisterOffsets.push_back(
                    {bgl->GetBindingInfo(bindingIndex).binding,
                     dynamicStorageBufferLengthsShaderRegisterOffset++});
            }
        }

        ASSERT(mDynamicStorageBufferLengthInfo[group].bindingAndRegisterOffsets.size() ==
               bgl->GetBindingCountInfo().dynamicStorageBufferCount);
    }

    if (dynamicStorageBufferLengthsShaderRegisterOffset > 0) {
        D3D12_ROOT_PARAMETER1 dynamicStorageBufferLengthConstants{};
        dynamicStorageBufferLengthConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        dynamicStorageBufferLengthConstants.ParameterType =
            D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        dynamicStorageBufferLengthConstants.Constants.Num32BitValues =
            dynamicStorageBufferLengthsShaderRegisterOffset;
        dynamicStorageBufferLengthConstants.Constants.RegisterSpace =
            kDynamicStorageBufferLengthsRegisterSpace;
        dynamicStorageBufferLengthConstants.Constants.ShaderRegister =
            kDynamicStorageBufferLengthsBaseRegister;
        mDynamicStorageBufferLengthsParameterIndex = rootParameters.size();
        rootParameters.emplace_back(dynamicStorageBufferLengthConstants);
    } else {
        mDynamicStorageBufferLengthsParameterIndex =
            kInvalidDynamicStorageBufferLengthsParameterIndex;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescriptor = {};
    versionedRootSignatureDescriptor.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    versionedRootSignatureDescriptor.Desc_1_1.NumParameters = rootParameters.size();
    versionedRootSignatureDescriptor.Desc_1_1.pParameters = rootParameters.data();
    versionedRootSignatureDescriptor.Desc_1_1.NumStaticSamplers = 0;
    versionedRootSignatureDescriptor.Desc_1_1.pStaticSamplers = nullptr;
    versionedRootSignatureDescriptor.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> error;
    HRESULT hr;
    if (device->IsToggleEnabled(Toggle::D3D12UseRootSignatureVersion1_1)) {
        hr = device->GetFunctions()->d3d12SerializeVersionedRootSignature(
            &versionedRootSignatureDescriptor, &mRootSignatureBlob, &error);
    } else {
        hr = SerializeRootParameter1_0(device, versionedRootSignatureDescriptor,
                                       &mRootSignatureBlob, &error);
    }
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
                              0, mRootSignatureBlob->GetBufferPointer(),
                              mRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)),
                          "D3D12 create root signature"));
    StreamIn(&mCacheKey, mRootSignatureBlob.Get());
    return {};
}

void PipelineLayout::DestroyImpl() {
    PipelineLayoutBase::DestroyImpl();

    Device* device = ToBackend(GetDevice());
    device->ReferenceUntilUnused(mRootSignature);

    // The ID3D12CommandSignature object should not be referenced by GPU operations in-flight on
    // Command Queue when it is being deleted. According to D3D12 debug layer, "it is not safe to
    // final-release objects that may have GPU operations pending. This can result in application
    // instability (921)".
    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get()) {
        device->ReferenceUntilUnused(mDispatchIndirectCommandSignatureWithNumWorkgroups);
    }
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        device->ReferenceUntilUnused(mDrawIndirectCommandSignatureWithInstanceVertexOffsets);
    }
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get()) {
        device->ReferenceUntilUnused(mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets);
    }
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

ID3DBlob* PipelineLayout::GetRootSignatureBlob() const {
    return mRootSignatureBlob.Get();
}

const PipelineLayout::DynamicStorageBufferLengthInfo&
PipelineLayout::GetDynamicStorageBufferLengthInfo() const {
    return mDynamicStorageBufferLengthInfo;
}

uint32_t PipelineLayout::GetDynamicRootParameterIndex(BindGroupIndex group,
                                                      BindingIndex bindingIndex) const {
    ASSERT(group < kMaxBindGroupsTyped);
    ASSERT(GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).buffer.hasDynamicOffset);
    ASSERT(GetBindGroupLayout(group)->GetBindingInfo(bindingIndex).visibility !=
           wgpu::ShaderStage::None);
    return mDynamicRootParameterIndices[group][bindingIndex];
}

uint32_t PipelineLayout::GetFirstIndexOffsetRegisterSpace() const {
    return kRenderOrComputeInternalRegisterSpace;
}

uint32_t PipelineLayout::GetFirstIndexOffsetShaderRegister() const {
    return kRenderOrComputeInternalBaseRegister;
}

uint32_t PipelineLayout::GetFirstIndexOffsetParameterIndex() const {
    return mFirstIndexOffsetParameterIndex;
}

uint32_t PipelineLayout::GetNumWorkgroupsRegisterSpace() const {
    return kRenderOrComputeInternalRegisterSpace;
}

uint32_t PipelineLayout::GetNumWorkgroupsShaderRegister() const {
    return kRenderOrComputeInternalBaseRegister;
}

uint32_t PipelineLayout::GetNumWorkgroupsParameterIndex() const {
    return mNumWorkgroupsParameterIndex;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsRegisterSpace() const {
    return kDynamicStorageBufferLengthsRegisterSpace;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsShaderRegister() const {
    return kDynamicStorageBufferLengthsBaseRegister;
}

uint32_t PipelineLayout::GetDynamicStorageBufferLengthsParameterIndex() const {
    ASSERT(mDynamicStorageBufferLengthsParameterIndex !=
           kInvalidDynamicStorageBufferLengthsParameterIndex);
    return mDynamicStorageBufferLengthsParameterIndex;
}

ID3D12CommandSignature* PipelineLayout::GetDispatchIndirectCommandSignatureWithNumWorkgroups() {
    // mDispatchIndirectCommandSignatureWithNumWorkgroups won't be created until it is needed.
    if (mDispatchIndirectCommandSignatureWithNumWorkgroups.Get() != nullptr) {
        return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetNumWorkgroupsParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 3;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(&programDesc, GetRootSignature(),
                                 IID_PPV_ARGS(&mDispatchIndirectCommandSignatureWithNumWorkgroups));
    return mDispatchIndirectCommandSignatureWithNumWorkgroups.Get();
}

ID3D12CommandSignature* PipelineLayout::GetDrawIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it is
    // needed.
    if (mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetFirstIndexOffsetParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 6 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(
            &programDesc, GetRootSignature(),
            IID_PPV_ARGS(&mDrawIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

ID3D12CommandSignature*
PipelineLayout::GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets() {
    // mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets won't be created until it
    // is needed.
    if (mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get() != nullptr) {
        return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
    }

    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    argumentDescs[0].Constant.RootParameterIndex = GetFirstIndexOffsetParameterIndex();
    argumentDescs[0].Constant.Num32BitValuesToSet = 2;
    argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;

    // A command signature must contain exactly 1 Draw / Dispatch / DispatchMesh / DispatchRays
    // command. That command must come last.
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 7 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 2;
    programDesc.pArgumentDescs = argumentDescs;

    // The root signature must be specified if and only if the command signature changes one of
    // the root arguments.
    ToBackend(GetDevice())
        ->GetD3D12Device()
        ->CreateCommandSignature(
            &programDesc, GetRootSignature(),
            IID_PPV_ARGS(&mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets));
    return mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets.Get();
}

}  // namespace dawn::native::d3d12
