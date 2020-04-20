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
            for (BindingIndex bindingIndex = 0;
                 bindingIndex < group->GetLayout()->GetBindingCount(); ++bindingIndex) {
                wgpu::BindingType type = group->GetLayout()->GetBindingInfo(bindingIndex).type;

                switch (type) {
                    case wgpu::BindingType::UniformBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(bindingIndex).buffer;
                        usageTracker->BufferUsedAs(buffer, wgpu::BufferUsage::Uniform);
                        break;
                    }

                    case wgpu::BindingType::StorageBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(bindingIndex).buffer;
                        usageTracker->BufferUsedAs(buffer, wgpu::BufferUsage::Storage);
                        break;
                    }

                    case wgpu::BindingType::SampledTexture: {
                        TextureBase* texture =
                            group->GetBindingAsTextureView(bindingIndex)->GetTexture();
                        usageTracker->TextureUsedAs(texture, wgpu::TextureUsage::Sampled);
                        break;
                    }

                    case wgpu::BindingType::ReadonlyStorageBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(bindingIndex).buffer;
                        usageTracker->BufferUsedAs(buffer, kReadOnlyStorage);
                        break;
                    }

                    case wgpu::BindingType::Sampler:
                        break;

                    case wgpu::BindingType::StorageTexture:
                    case wgpu::BindingType::ReadonlyStorageTexture:
                    case wgpu::BindingType::WriteonlyStorageTexture:
                        UNREACHABLE();
                        break;
                }
            }
        }
    }  // namespace

    ProgrammablePassEncoder::ProgrammablePassEncoder(DeviceBase* device,
                                                     EncodingContext* encodingContext)
        : ObjectBase(device), mEncodingContext(encodingContext) {
    }

    ProgrammablePassEncoder::ProgrammablePassEncoder(DeviceBase* device,
                                                     EncodingContext* encodingContext,
                                                     ErrorTag errorTag)
        : ObjectBase(device, errorTag), mEncodingContext(encodingContext) {
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

    void ProgrammablePassEncoder::SetBindGroup(uint32_t groupIndex,
                                               BindGroupBase* group,
                                               uint32_t dynamicOffsetCount,
                                               const uint32_t* dynamicOffsets) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(group));

                if (groupIndex >= kMaxBindGroups) {
                    return DAWN_VALIDATION_ERROR("Setting bind group over the max");
                }

                // Dynamic offsets count must match the number required by the layout perfectly.
                const BindGroupLayoutBase* layout = group->GetLayout();
                if (layout->GetDynamicBufferCount() != dynamicOffsetCount) {
                    return DAWN_VALIDATION_ERROR("dynamicOffset count mismatch");
                }

                for (BindingIndex i = 0; i < dynamicOffsetCount; ++i) {
                    const BindingInfo& bindingInfo = layout->GetBindingInfo(i);

                    // BGL creation sorts bindings such that the dynamic buffer bindings are first.
                    // ASSERT that this true.
                    ASSERT(bindingInfo.hasDynamicOffset);
                    switch (bindingInfo.type) {
                        case wgpu::BindingType::UniformBuffer:
                        case wgpu::BindingType::StorageBuffer:
                        case wgpu::BindingType::ReadonlyStorageBuffer:
                            break;
                        default:
                            UNREACHABLE();
                            break;
                    }

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
                        return DAWN_VALIDATION_ERROR("dynamic offset out of bounds");
                    }
                }
            }

            SetBindGroupCmd* cmd = allocator->Allocate<SetBindGroupCmd>(Command::SetBindGroup);
            cmd->index = groupIndex;
            cmd->group = group;
            cmd->dynamicOffsetCount = dynamicOffsetCount;
            if (dynamicOffsetCount > 0) {
                uint32_t* offsets = allocator->AllocateData<uint32_t>(cmd->dynamicOffsetCount);
                memcpy(offsets, dynamicOffsets, dynamicOffsetCount * sizeof(uint32_t));
            }

            TrackBindGroupResourceUsage(&mUsageTracker, group);

            return {};
        });
    }

}  // namespace dawn_native
