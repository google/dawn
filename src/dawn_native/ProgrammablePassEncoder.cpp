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

#include "dawn_native/ProgrammablePassEncoder.h"

#include "common/BitSetIterator.h"
#include "common/ityp_array.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <cstring>

namespace dawn_native {

    namespace {
        void TrackBindGroupResourceUsage(PassResourceUsageTracker* usageTracker,
                                         BindGroupBase* group) {
            for (BindingIndex bindingIndex{0}; bindingIndex < group->GetLayout()->GetBindingCount();
                 ++bindingIndex) {
                const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

                switch (bindingInfo.bindingType) {
                    case BindingInfoType::Buffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(bindingIndex).buffer;
                        switch (bindingInfo.buffer.type) {
                            case wgpu::BufferBindingType::Uniform:
                                usageTracker->BufferUsedAs(buffer, wgpu::BufferUsage::Uniform);
                                break;
                            case wgpu::BufferBindingType::Storage:
                                usageTracker->BufferUsedAs(buffer, wgpu::BufferUsage::Storage);
                                break;
                            case wgpu::BufferBindingType::ReadOnlyStorage:
                                usageTracker->BufferUsedAs(buffer, kReadOnlyStorageBuffer);
                                break;
                            case wgpu::BufferBindingType::Undefined:
                                UNREACHABLE();
                        }
                        break;
                    }

                    case BindingInfoType::Texture: {
                        TextureViewBase* view = group->GetBindingAsTextureView(bindingIndex);
                        usageTracker->TextureViewUsedAs(view, wgpu::TextureUsage::Sampled);
                        break;
                    }

                    case BindingInfoType::StorageTexture: {
                        TextureViewBase* view = group->GetBindingAsTextureView(bindingIndex);
                        switch (bindingInfo.storageTexture.access) {
                            case wgpu::StorageTextureAccess::ReadOnly:
                                usageTracker->TextureViewUsedAs(view, kReadonlyStorageTexture);
                                break;
                            case wgpu::StorageTextureAccess::WriteOnly:
                                usageTracker->TextureViewUsedAs(view, wgpu::TextureUsage::Storage);
                                break;
                            case wgpu::StorageTextureAccess::Undefined:
                                UNREACHABLE();
                        }
                        break;
                    }

                    case BindingInfoType::Sampler:
                        break;
                }
            }
        }
    }  // namespace

    ProgrammablePassEncoder::ProgrammablePassEncoder(DeviceBase* device,
                                                     EncodingContext* encodingContext,
                                                     PassType passType)
        : ObjectBase(device), mEncodingContext(encodingContext), mUsageTracker(passType) {
    }

    ProgrammablePassEncoder::ProgrammablePassEncoder(DeviceBase* device,
                                                     EncodingContext* encodingContext,
                                                     ErrorTag errorTag,
                                                     PassType passType)
        : ObjectBase(device, errorTag), mEncodingContext(encodingContext), mUsageTracker(passType) {
    }

    void ProgrammablePassEncoder::InsertDebugMarker(const char* groupLabel) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        });
    }

    void ProgrammablePassEncoder::PopDebugGroup() {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);

            return {};
        });
    }

    void ProgrammablePassEncoder::PushDebugGroup(const char* groupLabel) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        });
    }

    void ProgrammablePassEncoder::SetBindGroup(uint32_t groupIndexIn,
                                               BindGroupBase* group,
                                               uint32_t dynamicOffsetCountIn,
                                               const uint32_t* dynamicOffsetsIn) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            BindGroupIndex groupIndex(groupIndexIn);

            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(group));

                if (groupIndex >= kMaxBindGroupsTyped) {
                    return DAWN_VALIDATION_ERROR("Setting bind group over the max");
                }

                ityp::span<BindingIndex, const uint32_t> dynamicOffsets(
                    dynamicOffsetsIn, BindingIndex(dynamicOffsetCountIn));

                // Dynamic offsets count must match the number required by the layout perfectly.
                const BindGroupLayoutBase* layout = group->GetLayout();
                if (layout->GetDynamicBufferCount() != dynamicOffsets.size()) {
                    return DAWN_VALIDATION_ERROR("dynamicOffset count mismatch");
                }

                for (BindingIndex i{0}; i < dynamicOffsets.size(); ++i) {
                    const BindingInfo& bindingInfo = layout->GetBindingInfo(i);

                    // BGL creation sorts bindings such that the dynamic buffer bindings are first.
                    // ASSERT that this true.
                    ASSERT(bindingInfo.bindingType == BindingInfoType::Buffer);
                    ASSERT(bindingInfo.buffer.hasDynamicOffset);

                    if (dynamicOffsets[i] % kMinDynamicBufferOffsetAlignment != 0) {
                        return DAWN_VALIDATION_ERROR("Dynamic Buffer Offset need to be aligned");
                    }

                    BufferBinding bufferBinding = group->GetBindingAsBufferBinding(i);

                    // During BindGroup creation, validation ensures binding offset + binding size
                    // <= buffer size.
                    ASSERT(bufferBinding.buffer->GetSize() >= bufferBinding.size);
                    ASSERT(bufferBinding.buffer->GetSize() - bufferBinding.size >=
                           bufferBinding.offset);

                    if ((dynamicOffsets[i] > bufferBinding.buffer->GetSize() -
                                                 bufferBinding.offset - bufferBinding.size)) {
                        if ((bufferBinding.buffer->GetSize() - bufferBinding.offset) ==
                            bufferBinding.size) {
                            return DAWN_VALIDATION_ERROR(
                                "Dynamic offset out of bounds. The binding goes to the end of the "
                                "buffer even with a dynamic offset of 0. Did you forget to specify "
                                "the binding's size?");
                        } else {
                            return DAWN_VALIDATION_ERROR("Dynamic offset out of bounds");
                        }
                    }
                }
            }

            SetBindGroupCmd* cmd = allocator->Allocate<SetBindGroupCmd>(Command::SetBindGroup);
            cmd->index = groupIndex;
            cmd->group = group;
            cmd->dynamicOffsetCount = dynamicOffsetCountIn;
            if (dynamicOffsetCountIn > 0) {
                uint32_t* offsets = allocator->AllocateData<uint32_t>(cmd->dynamicOffsetCount);
                memcpy(offsets, dynamicOffsetsIn, dynamicOffsetCountIn * sizeof(uint32_t));
            }

            TrackBindGroupResourceUsage(&mUsageTracker, group);

            return {};
        });
    }

}  // namespace dawn_native
