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

#ifndef SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
#define SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_

#import <Metal/Metal.h>

#include <utility>

#include "src/dawn/common/Constants.h"
#include "src/dawn/common/ityp_stack_vec.h"
#include "src/dawn/native/BindingInfo.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/PerStage.h"
#include "src/dawn/native/PipelineLayout.h"
#include "src/utils/typed_integer.h"

namespace dawn::native::metal {

class Device;

namespace detail {

// This struct is not used directly, it's just an ergonomic way of writing out the layout of buffers
// in the 31 Metal buffer slots.
struct BufferSlots {
    char mVertexBuffers[kMaxVertexBuffers];

    // This layout is for when Argument Buffers are enabled. When disabled, this block of 22 gets
    // reused as space for uniform + storage buffer bindings. (Except, when Argument Buffers are
    // disabled *and* ResourceTables are enabled, there's an effective limit of just 20
    // uniform+storage buffer bindings despite our advertised limits.)
    char mPadding[12];
    char mArgumentBuffersAndDynamicOffsetBuffers[kMaxBindGroups * 2];
    char mResourceTableStorage;
    char mResourceTableBinding;

    char mImmediates;
};
static_assert(sizeof(BufferSlots) == 31);

}  // namespace detail

// Metal buffer index: [[buffer(n)]].
using MetalBufferIndex = TypedInteger<struct MetalBufferIndexT, uint32_t>;

// Immediate blocks contain pipeline immediates followed by tightly packed storage buffer sizes.
inline constexpr MetalBufferIndex kImmediateBlockBufferSlot{
    offsetof(detail::BufferSlots, mImmediates)};

// Note, the kResourceTable*Slot's can conflict with buffer bindings from bindgroups (when not using
// argument buffers - see kMaxVertexStageBuffersWithResourceTableWithoutArgBufs).
// TODO(crbug.com/363031535): Update when the ResourceTable + non-argbufs path is removed.
inline constexpr MetalBufferIndex kResourceTableBindingSlot{
    offsetof(detail::BufferSlots, mResourceTableBinding)};
inline constexpr MetalBufferIndex kResourceTableStorageSlot{
    offsetof(detail::BufferSlots, mResourceTableStorage)};

// Get the [[buffer()]] number for an argument buffer from the SetBindGroup() slot number.
inline std::pair<MetalBufferIndex, MetalBufferIndex> BindGroupIndexToArgumentBufferIndex(
    BindGroupIndex slot) {
    constexpr MetalBufferIndex kArgumentBufferSlot0{
        offsetof(detail::BufferSlots, mArgumentBuffersAndDynamicOffsetBuffers[0])};
    // Maps slots as follows (AB = Argument Buffer, DOB = Dynamic Offset Buffer):
    // AB0 DOB0 AB1 DOB1 AB2 DOB2 AB3 DOB3
    // TODO(crbug.com/363031535): Simplify after removing DOBs.
    DAWN_CHECK(slot < kMaxBindGroupsTyped);
    return {
        kArgumentBufferSlot0 + MetalBufferIndex{uint32_t{slot} * 2},
        kArgumentBufferSlot0 + MetalBufferIndex{uint32_t{slot} * 2 + 1},
    };
}

// Get the [[buffer()]] number for a vertex buffer from the SetVertexBuffer() slot number.
inline MetalBufferIndex VertexBufferSlotToMetalBufferIndex(VertexBufferSlot slot) {
    constexpr MetalBufferIndex kVertexBufferSlot0{offsetof(detail::BufferSlots, mVertexBuffers[0])};
    DAWN_ASSERT(slot < kMaxVertexBuffersTyped);
    return kVertexBufferSlot0 + MetalBufferIndex{uint32_t{slot}};
}

// Index type for when we pack buffers (vertex+uniform+storage) into a per-stage array:
// - Buffer length array for robustness
// - Metal buffer slots (when not using Argument Buffers)
using PerStageBufferIndex = TypedInteger<struct PerStageBufferIndexT, uint32_t>;
inline constexpr PerStageBufferIndex kBufferLengthArraySize{
    kMaxVertexBuffers + kMaxUniformBuffersPerShaderStage + kMaxStorageBuffersPerShaderStage};
// When using ResourceTable we are limited in how many buffers we can bind directly to the shader.
// TODO(crbug.com/363031535): Remove when the non-argbufs path is removed.
inline constexpr PerStageBufferIndex kMaxVertexStageBufferCountWithResourceTableWithoutArgBufs{
    uint32_t{kResourceTableStorageSlot}};

inline PerStageBufferIndex VertexBufferSlotToLengthArrayIndex(VertexBufferSlot slot) {
    DAWN_CHECK(slot < kMaxVertexBuffersTyped);
    return PerStageBufferIndex{uint32_t{slot}};
}

inline uint32_t GetImmediateBufferSizesByteOffset(ImmediateMask pipelineImmediateMask) {
    return static_cast<uint32_t>(pipelineImmediateMask.count()) * kImmediateElementByteSize;
}

// Placeholder values where we need to pass in a bind group index to Tint but won't actually use it.
// Used to bind vertex buffers for vertex pulling, and for the resource table, both of which are
// passed as [[buffer()]] arguments directly to the vertex shader. (These two IDs don't strictly
// have to be different, but it's clearer that way.)
// - Used for any place we need to pass a bind group ID that Tint won't actually use for MSL.
inline constexpr uint32_t kPlaceholderBindGroupID{0xFFFF'FFFF};
// - The vertex pulling transform moves vertex buffers to a binding point in this bind group. In MSL
//   this becomes a toplevel [[buffer()]] slot, so we also pass this to the pulling_group option so
//   it knows which bind points to collect those from.
inline constexpr uint32_t kPullingBufferBindGroupID{0xFFFF'FFFE};

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static Ref<PipelineLayout> Create(Device* device,
                                      const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    using BindingIndexInfo =
        PerBindGroup<ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup>>;
    const BindingIndexInfo& GetBindingIndexInfo(SingleShaderStage stage) const;

    // The number of Metal vertex stage buffers used for the whole pipeline layout.
    PerStageBufferIndex GetBufferBindingCount(SingleShaderStage stage) const;

  private:
    PipelineLayout(Device* device, const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);
    ~PipelineLayout() override;

    // TODO(https://crbug.com/363031535): Simplify when removing non-argument-buffers code.
    PerStage<BindingIndexInfo> mIndexInfo;
    PerStage<PerStageBufferIndex> mBufferBindingCount;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
