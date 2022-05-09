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

#ifndef SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_
#define SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"

namespace dawn::native {

class CommandBufferStateTracker {
  public:
    CommandBufferStateTracker();
    CommandBufferStateTracker(const CommandBufferStateTracker&);
    CommandBufferStateTracker(CommandBufferStateTracker&&);
    ~CommandBufferStateTracker();

    CommandBufferStateTracker& operator=(const CommandBufferStateTracker&);
    CommandBufferStateTracker& operator=(CommandBufferStateTracker&&);

    // Non-state-modifying validation functions
    MaybeError ValidateCanDispatch();
    MaybeError ValidateCanDraw();
    MaybeError ValidateCanDrawIndexed();
    MaybeError ValidateBufferInRangeForVertexBuffer(uint32_t vertexCount, uint32_t firstVertex);
    MaybeError ValidateBufferInRangeForInstanceBuffer(uint32_t instanceCount,
                                                      uint32_t firstInstance);
    MaybeError ValidateIndexBufferInRange(uint32_t indexCount, uint32_t firstIndex);

    // State-modifying methods
    void SetComputePipeline(ComputePipelineBase* pipeline);
    void SetRenderPipeline(RenderPipelineBase* pipeline);
    void SetBindGroup(BindGroupIndex index,
                      BindGroupBase* bindgroup,
                      uint32_t dynamicOffsetCount,
                      const uint32_t* dynamicOffsets);
    void SetIndexBuffer(wgpu::IndexFormat format, uint64_t size);
    void SetVertexBuffer(VertexBufferSlot slot, uint64_t size);

    static constexpr size_t kNumAspects = 4;
    using ValidationAspects = std::bitset<kNumAspects>;

    BindGroupBase* GetBindGroup(BindGroupIndex index) const;
    const std::vector<uint32_t>& GetDynamicOffsets(BindGroupIndex index) const;
    bool HasPipeline() const;
    RenderPipelineBase* GetRenderPipeline() const;
    ComputePipelineBase* GetComputePipeline() const;
    PipelineLayoutBase* GetPipelineLayout() const;
    wgpu::IndexFormat GetIndexFormat() const;
    uint64_t GetIndexBufferSize() const;

  private:
    MaybeError ValidateOperation(ValidationAspects requiredAspects);
    void RecomputeLazyAspects(ValidationAspects aspects);
    MaybeError CheckMissingAspects(ValidationAspects aspects);

    void SetPipelineCommon(PipelineBase* pipeline);

    ValidationAspects mAspects;

    ityp::array<BindGroupIndex, BindGroupBase*, kMaxBindGroups> mBindgroups = {};
    ityp::array<BindGroupIndex, std::vector<uint32_t>, kMaxBindGroups> mDynamicOffsets = {};
    ityp::bitset<VertexBufferSlot, kMaxVertexBuffers> mVertexBufferSlotsUsed;
    bool mIndexBufferSet = false;
    wgpu::IndexFormat mIndexFormat;
    uint64_t mIndexBufferSize = 0;

    ityp::array<VertexBufferSlot, uint64_t, kMaxVertexBuffers> mVertexBufferSizes = {};

    PipelineLayoutBase* mLastPipelineLayout = nullptr;
    PipelineBase* mLastPipeline = nullptr;

    const RequiredBufferSizes* mMinBufferSizes = nullptr;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_
