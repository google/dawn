// Copyright 2018 The Dawn Authors
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

#include "dawn/native/ProgrammableEncoder.h"

#include <cstring>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

ProgrammableEncoder::ProgrammableEncoder(DeviceBase* device,
                                         const char* label,
                                         EncodingContext* encodingContext)
    : ApiObjectBase(device, label),
      mEncodingContext(encodingContext),
      mValidationEnabled(device->IsValidationEnabled()) {}

ProgrammableEncoder::ProgrammableEncoder(DeviceBase* device,
                                         EncodingContext* encodingContext,
                                         ErrorTag errorTag)
    : ApiObjectBase(device, errorTag),
      mEncodingContext(encodingContext),
      mValidationEnabled(device->IsValidationEnabled()) {}

bool ProgrammableEncoder::IsValidationEnabled() const {
    return mValidationEnabled;
}

MaybeError ProgrammableEncoder::ValidateProgrammableEncoderEnd() const {
    DAWN_INVALID_IF(mDebugGroupStackSize != 0,
                    "PushDebugGroup called %u time(s) without a corresponding PopDebugGroup.",
                    mDebugGroupStackSize);
    return {};
}

void ProgrammableEncoder::APIInsertDebugMarker(const char* groupLabel) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        },
        "encoding %s.InsertDebugMarker(\"%s\").", this, groupLabel);
}

void ProgrammableEncoder::APIPopDebugGroup() {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(mDebugGroupStackSize == 0,
                                "PopDebugGroup called when no debug groups are currently pushed.");
            }
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);
            mDebugGroupStackSize--;
            mEncodingContext->PopDebugGroupLabel();

            return {};
        },
        "encoding %s.PopDebugGroup().", this);
}

void ProgrammableEncoder::APIPushDebugGroup(const char* groupLabel) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            mDebugGroupStackSize++;
            mEncodingContext->PushDebugGroupLabel(groupLabel);

            return {};
        },
        "encoding %s.PushDebugGroup(\"%s\").", this, groupLabel);
}

MaybeError ProgrammableEncoder::ValidateSetBindGroup(BindGroupIndex index,
                                                     BindGroupBase* group,
                                                     uint32_t dynamicOffsetCountIn,
                                                     const uint32_t* dynamicOffsetsIn) const {
    DAWN_TRY(GetDevice()->ValidateObject(group));

    DAWN_INVALID_IF(index >= kMaxBindGroupsTyped, "Bind group index (%u) exceeds the maximum (%u).",
                    static_cast<uint32_t>(index), kMaxBindGroups);

    ityp::span<BindingIndex, const uint32_t> dynamicOffsets(dynamicOffsetsIn,
                                                            BindingIndex(dynamicOffsetCountIn));

    // Dynamic offsets count must match the number required by the layout perfectly.
    const BindGroupLayoutBase* layout = group->GetLayout();
    DAWN_INVALID_IF(
        layout->GetDynamicBufferCount() != dynamicOffsets.size(),
        "The number of dynamic offsets (%u) does not match the number of dynamic buffers (%u) "
        "in %s.",
        static_cast<uint32_t>(dynamicOffsets.size()),
        static_cast<uint32_t>(layout->GetDynamicBufferCount()), layout);

    for (BindingIndex i{0}; i < dynamicOffsets.size(); ++i) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(i);

        // BGL creation sorts bindings such that the dynamic buffer bindings are first.
        // ASSERT that this true.
        ASSERT(bindingInfo.bindingType == BindingInfoType::Buffer);
        ASSERT(bindingInfo.buffer.hasDynamicOffset);

        uint64_t requiredAlignment;
        switch (bindingInfo.buffer.type) {
            case wgpu::BufferBindingType::Uniform:
                requiredAlignment = GetDevice()->GetLimits().v1.minUniformBufferOffsetAlignment;
                break;
            case wgpu::BufferBindingType::Storage:
            case wgpu::BufferBindingType::ReadOnlyStorage:
            case kInternalStorageBufferBinding:
                requiredAlignment = GetDevice()->GetLimits().v1.minStorageBufferOffsetAlignment;
                break;
            case wgpu::BufferBindingType::Undefined:
                UNREACHABLE();
        }

        DAWN_INVALID_IF(!IsAligned(dynamicOffsets[i], requiredAlignment),
                        "Dynamic Offset[%u] (%u) is not %u byte aligned.", static_cast<uint32_t>(i),
                        dynamicOffsets[i], requiredAlignment);

        BufferBinding bufferBinding = group->GetBindingAsBufferBinding(i);

        // During BindGroup creation, validation ensures binding offset + binding size
        // <= buffer size.
        ASSERT(bufferBinding.buffer->GetSize() >= bufferBinding.size);
        ASSERT(bufferBinding.buffer->GetSize() - bufferBinding.size >= bufferBinding.offset);

        if ((dynamicOffsets[i] >
             bufferBinding.buffer->GetSize() - bufferBinding.offset - bufferBinding.size)) {
            DAWN_INVALID_IF(
                (bufferBinding.buffer->GetSize() - bufferBinding.offset) == bufferBinding.size,
                "Dynamic Offset[%u] (%u) is out of bounds of %s with a size of %u and a bound "
                "range of (offset: %u, size: %u). The binding goes to the end of the buffer "
                "even with a dynamic offset of 0. Did you forget to specify "
                "the binding's size?",
                static_cast<uint32_t>(i), dynamicOffsets[i], bufferBinding.buffer,
                bufferBinding.buffer->GetSize(), bufferBinding.offset, bufferBinding.size);

            return DAWN_VALIDATION_ERROR(
                "Dynamic Offset[%u] (%u) is out of bounds of "
                "%s with a size of %u and a bound range of (offset: %u, size: %u).",
                static_cast<uint32_t>(i), dynamicOffsets[i], bufferBinding.buffer,
                bufferBinding.buffer->GetSize(), bufferBinding.offset, bufferBinding.size);
        }
    }

    return {};
}

void ProgrammableEncoder::RecordSetBindGroup(CommandAllocator* allocator,
                                             BindGroupIndex index,
                                             BindGroupBase* group,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets) const {
    SetBindGroupCmd* cmd = allocator->Allocate<SetBindGroupCmd>(Command::SetBindGroup);
    cmd->index = index;
    cmd->group = group;
    cmd->dynamicOffsetCount = dynamicOffsetCount;
    if (dynamicOffsetCount > 0) {
        uint32_t* offsets = allocator->AllocateData<uint32_t>(cmd->dynamicOffsetCount);
        memcpy(offsets, dynamicOffsets, dynamicOffsetCount * sizeof(uint32_t));
    }
}

}  // namespace dawn::native
