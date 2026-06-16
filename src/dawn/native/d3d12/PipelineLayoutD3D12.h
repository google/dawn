// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_

#include <optional>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/ityp_array.h"
#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/BindingInfo.h"
#include "src/dawn/native/PipelineLayout.h"
#include "src/dawn/native/d3d12/PipelineLayoutHandle.h"
#include "src/dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static ResultOrError<Ref<PipelineLayout>> Create(
        Device* device,
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    uint32_t GetResourceTableCbvUavSrvRootParameterIndex() const;
    uint32_t GetResourceTableSamplerRootParameterIndex() const;
    uint32_t GetBaseResourceTableRegisterSpace() const;

    uint32_t GetCbvUavSrvRootParameterIndex(BindGroupIndex group) const;
    uint32_t GetSamplerRootParameterIndex(BindGroupIndex group) const;

    // Returns the index of the root parameter reserved for a dynamic uniform buffer binding
    uint32_t GetDynamicUniformRootParameterIndex(BindGroupIndex group,
                                                 BindingIndex bindingIndex) const;

    uint32_t GetFirstIndexOffsetRegisterSpace() const;
    uint32_t GetFirstIndexOffsetShaderRegister() const;

    uint32_t GetNumWorkgroupsRegisterSpace() const;
    uint32_t GetNumWorkgroupsShaderRegister() const;

    uint32_t GetDynamicStorageBufferLengthsRegisterSpace() const;
    uint32_t GetDynamicStorageBufferLengthsShaderRegister() const;
    uint32_t GetDynamicStorageBufferLengthsParameterIndex() const;

    uint32_t GetDynamicStorageBufferOffsetsRegisterSpace() const;
    uint32_t GetDynamicStorageBufferOffsetsShaderRegister() const;
    uint32_t GetDynamicStorageBufferOffsetsParameterIndex() const;

    uint32_t GetImmediatesRegisterSpace() const;
    uint32_t GetImmediatesShaderRegister() const;

    ResultOrError<Ref<PipelineLayoutHandle>> GetOrCreatePipelineLayoutHandle(
        uint32_t immediateCounts);

    struct BindGroupDynamicStorageBufferInfo {
        // First register offset for a bind group's dynamic storage buffer lengths or offsets.
        // This is the index into the array of root constants where this bind group's
        // lengths or offsets start.
        uint32_t firstRegisterOffset;

        struct BindingAndRegisterOffset {
            BindingNumber binding;
            uint32_t registerOffset;
        };
        // Associative list of (BindingNumber,registerOffset) pairs, which is passed into
        // the shader to map the BindingPoint(thisGroup, BindingNumber) to the registerOffset
        // into the root constant array which holds the dynamic storage buffer lengths and offsets.
        std::vector<BindingAndRegisterOffset> bindingAndRegisterOffsets;
    };

    // Flat map from bind group index to the list of (BindingNumber,Register) pairs.
    using DynamicStorageBufferInfo = PerBindGroup<BindGroupDynamicStorageBufferInfo>;

    const DynamicStorageBufferInfo& GetDynamicStorageBufferInfo() const;

  private:
    ~PipelineLayout() override = default;
    using PipelineLayoutBase::PipelineLayoutBase;
    MaybeError Initialize();
    void DestroyImpl(DestroyReason reason) override;

    MaybeError BuildBaseRootParameters();
    ResultOrError<Ref<PipelineLayoutHandle>> CreatePipelineLayoutHandle(uint32_t immediateCounts);

    PerBindGroup<uint32_t> mCbvUavSrvRootParameterIndices;
    PerBindGroup<uint32_t> mSamplerRootParameterIndices;
    PerBindGroup<ityp::vector<BindingIndex, uint32_t>> mDynamicUniformRootParameterIndices;
    DynamicStorageBufferInfo mDynamicStorageBufferInfo;
    uint32_t mResourceTableCbvUavSrvRootParameterIndex;
    uint32_t mResourceTableSamplerRootParameterIndex;
    uint32_t mFirstIndexOffsetParameterIndex;
    uint32_t mNumWorkgroupsParameterIndex;
    uint32_t mDynamicStorageBufferLengthsParameterIndex;
    uint32_t mDynamicStorageBufferOffsetsParameterIndex;

    // Base root parameters shared by every PipelineLayoutHandle, built once in
    // BuildBaseRootParameters(). rootParameters points into ranges, so both are kept const and
    // bundled together; std::optional defers the single emplace to Initialize().
    // TODO(crbug.com/366291600): The C++ way to make immutable types is to make private mutable
    // data members, use a constructor to initialize them, and provide only const-access members.
    struct InvariantParams {
        const std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
        const std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;
        const std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
    };
    std::optional<InvariantParams> mInvariantParams;

    // Cache of PipelineLayoutHandles keyed by the number of immediate (32-bit root) constants the
    // pipeline uses. Today every root constant that shapes the root signature is either always
    // allocated (the firstVertex / firstInstance / num_workgroups block) or fully determined by the
    // bind group layouts (dynamic storage buffer lengths/offsets) and the layout's user immediate
    // range. None of them vary between pipelines that share a layout, so the immediate count is
    // constant per layout and a PipelineLayout currently maps 1:1 to a single PipelineLayoutHandle.
    //
    // TODO(crbug.com/366291600): Stop always allocating the internal root constants; allocate them
    // dynamically and track them through the immediate mask instead (as D3D11/Vulkan/GL already
    // do). After that, pipelines sharing a layout can have different immediate counts, so one
    // PipelineLayout may map to multiple PipelineLayoutHandles. At that point this key must capture
    // every per-pipeline input that changes the root signature.
    MutexProtected<absl::flat_hash_map<uint32_t, Ref<PipelineLayoutHandle>>> mPipelineLayoutHandles;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PIPELINELAYOUTD3D12_H_
