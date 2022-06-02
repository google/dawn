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

#ifndef SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static ResultOrError<Ref<PipelineLayout>> Create(Device* device,
                                                     const PipelineLayoutDescriptor* descriptor);

    uint32_t GetCbvUavSrvRootParameterIndex(BindGroupIndex group) const;
    uint32_t GetSamplerRootParameterIndex(BindGroupIndex group) const;

    // Returns the index of the root parameter reserved for a dynamic buffer binding
    uint32_t GetDynamicRootParameterIndex(BindGroupIndex group, BindingIndex bindingIndex) const;

    uint32_t GetFirstIndexOffsetRegisterSpace() const;
    uint32_t GetFirstIndexOffsetShaderRegister() const;
    uint32_t GetFirstIndexOffsetParameterIndex() const;

    uint32_t GetNumWorkgroupsRegisterSpace() const;
    uint32_t GetNumWorkgroupsShaderRegister() const;
    uint32_t GetNumWorkgroupsParameterIndex() const;

    uint32_t GetDynamicStorageBufferLengthsRegisterSpace() const;
    uint32_t GetDynamicStorageBufferLengthsShaderRegister() const;
    uint32_t GetDynamicStorageBufferLengthsParameterIndex() const;

    ID3D12RootSignature* GetRootSignature() const;

    ID3DBlob* GetRootSignatureBlob() const;

    ID3D12CommandSignature* GetDispatchIndirectCommandSignatureWithNumWorkgroups();

    ID3D12CommandSignature* GetDrawIndirectCommandSignatureWithInstanceVertexOffsets();

    ID3D12CommandSignature* GetDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets();

    struct PerBindGroupDynamicStorageBufferLengthInfo {
        // First register offset for a bind group's dynamic storage buffer lengths.
        // This is the index into the array of root constants where this bind group's
        // lengths start.
        uint32_t firstRegisterOffset;

        struct BindingAndRegisterOffset {
            BindingNumber binding;
            uint32_t registerOffset;
        };
        // Associative list of (BindingNumber,registerOffset) pairs, which is passed into
        // the shader to map the BindingPoint(thisGroup, BindingNumber) to the registerOffset
        // into the root constant array which holds the dynamic storage buffer lengths.
        std::vector<BindingAndRegisterOffset> bindingAndRegisterOffsets;
    };

    // Flat map from bind group index to the list of (BindingNumber,Register) pairs.
    // Each pair is used in shader translation to
    using DynamicStorageBufferLengthInfo =
        ityp::array<BindGroupIndex, PerBindGroupDynamicStorageBufferLengthInfo, kMaxBindGroups>;

    const DynamicStorageBufferLengthInfo& GetDynamicStorageBufferLengthInfo() const;

  private:
    ~PipelineLayout() override = default;
    using PipelineLayoutBase::PipelineLayoutBase;
    MaybeError Initialize();
    void DestroyImpl() override;

    ityp::array<BindGroupIndex, uint32_t, kMaxBindGroups> mCbvUavSrvRootParameterInfo;
    ityp::array<BindGroupIndex, uint32_t, kMaxBindGroups> mSamplerRootParameterInfo;
    ityp::array<BindGroupIndex,
                ityp::array<BindingIndex, uint32_t, kMaxDynamicBuffersPerPipelineLayout>,
                kMaxBindGroups>
        mDynamicRootParameterIndices;
    DynamicStorageBufferLengthInfo mDynamicStorageBufferLengthInfo;
    uint32_t mFirstIndexOffsetParameterIndex;
    uint32_t mNumWorkgroupsParameterIndex;
    uint32_t mDynamicStorageBufferLengthsParameterIndex;
    ComPtr<ID3D12RootSignature> mRootSignature;
    // Store the root signature blob to put in pipeline cachekey
    ComPtr<ID3DBlob> mRootSignatureBlob;
    ComPtr<ID3D12CommandSignature> mDispatchIndirectCommandSignatureWithNumWorkgroups;
    ComPtr<ID3D12CommandSignature> mDrawIndirectCommandSignatureWithInstanceVertexOffsets;
    ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectCommandSignatureWithInstanceVertexOffsets;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
