// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_

#include "dawn/native/PipelineLayout.h"

#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;

// For D3D11, uniform buffers, samplers, sampled textures, and storage buffers are bind to
// different kind of slots. The number of slots for each type is limited by the D3D11 spec.
// So we need to pack the bindings by type into the slots tightly. UAV slots are a little
// different. They are assigned to storage buffers and textures decreasingly from the end,
// as color attachments also use them increasingly from the begin.
// And D3D11 uses SM 5.0 which doesn't support spaces(binding groups). so we need to flatten
// the binding groups into a single array.
class PipelineLayout final : public PipelineLayoutBase {
  public:
    // constant buffer slot reserved for index offsets and num workgroups.
    static constexpr uint32_t kReservedConstantBufferSlot =
        D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1;
    static constexpr uint32_t kFirstIndexOffsetConstantBufferSlot = kReservedConstantBufferSlot;
    static constexpr uint32_t kNumWorkgroupsConstantBufferSlot = kReservedConstantBufferSlot;

    // The reserved groups and bindings are for internal use only. They must not conflict with
    // external clients.
    static constexpr uint32_t kReservedConstantsBindGroupIndex = kMaxBindGroups;
    static constexpr uint32_t kFirstIndexOffsetBindingNumber = 0u;

    static ResultOrError<Ref<PipelineLayout>> Create(Device* device,
                                                     const PipelineLayoutDescriptor* descriptor);

    using BindingIndexInfo =
        ityp::array<BindGroupIndex, ityp::vector<BindingIndex, uint32_t>, kMaxBindGroups>;
    const BindingIndexInfo& GetBindingIndexInfo() const;

    uint32_t GetUnusedUAVBindingCount() const { return mUnusedUAVBindingCount; }
    uint32_t GetTotalUAVBindingCount() const { return mTotalUAVBindingCount; }

    // Get the bind groups that use one or more UAV slots.
    const BindGroupLayoutMask& GetUAVBindGroupLayoutsMask() const;

  private:
    using PipelineLayoutBase::PipelineLayoutBase;

    ~PipelineLayout() override = default;

    MaybeError Initialize(Device* device);

    BindingIndexInfo mIndexInfo;

    uint32_t mUnusedUAVBindingCount = 0u;
    uint32_t mTotalUAVBindingCount = 0u;
    BindGroupLayoutMask mUAVBindGroups = 0;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_PIPELINELAYOUTD3D11_H_
