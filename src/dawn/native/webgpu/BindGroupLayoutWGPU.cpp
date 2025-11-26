// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"

#include <vector>

#include "dawn/common/MatchVariant.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/Forward.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) {
    return AcquireRef(new BindGroupLayout(device, descriptor));
}

BindGroupLayout::BindGroupLayout(Device* device,
                                 const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      RecordableObject(schema::ObjectType::BindGroupLayout),
      ObjectWGPU(device->wgpu.bindGroupLayoutRelease),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {
    // Rebuild the descriptor and resolve internal bindings to regular ones.
    std::vector<WGPUBindGroupLayoutEntry> entries(descriptor->entryCount);
    for (size_t i = 0; i < entries.size(); i++) {
        entries[i] = *ToAPI(&descriptor->entries[i]);

        switch (descriptor->entries[i].buffer.type) {
            case kInternalStorageBufferBinding:
                entries[i].buffer.type = WGPUBufferBindingType_Storage;
                break;
            case kInternalReadOnlyStorageBufferBinding:
                entries[i].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                break;
            default:
                break;
        }
    }

    WGPUBindGroupLayoutDescriptor desc = {};
    desc.nextInChain = nullptr;
    desc.label = ToOutputStringView(descriptor->label);
    desc.entryCount = descriptor->entryCount;
    desc.entries = entries.data();

    mInnerHandle = device->wgpu.deviceCreateBindGroupLayout(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
}

Ref<BindGroup> BindGroupLayout::AllocateBindGroup(
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    Device* device = ToBackend(GetDevice());
    return AcquireRef(mBindGroupAllocator->Allocate(device, descriptor));
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
    mBindGroupAllocator->Deallocate(bindGroup);
}

void BindGroupLayout::ReduceMemoryUsage() {
    mBindGroupAllocator->DeleteEmptySlabs();
}

MaybeError BindGroupLayout::AddReferenced(CaptureContext& captureContext) {
    // BindGroupLayouts don't reference anything.
    return {};
}

void BindGroupLayout::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError BindGroupLayout::CaptureCreationParameters(CaptureContext& captureContext) {
    const auto& bindingMap = GetBindingMap();

    schema::BindGroupLayout data{{
        .numEntries = uint32_t(bindingMap.size()),
    }};
    Serialize(captureContext, data);

    for (const auto& [bindingNumber, apiBindingIndex] : bindingMap) {
        const auto& bindingInfo = GetAPIBindingInfo(apiBindingIndex);

        schema::BindGroupLayoutBinding binding{{
            .binding = uint32_t(bindingNumber),
            .visibility = bindingInfo.visibility,
            .bindingArraySize = uint32_t(bindingInfo.arraySize),
        }};

        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& info) -> MaybeError {
                schema::BindGroupLayoutEntryTypeBufferBinding entry{{
                    .binding = binding,
                    .data{{
                        .type = info.type,
                        .minBindingSize = info.minBindingSize,
                        .hasDynamicOffset = info.hasDynamicOffset,
                    }},
                }};
                Serialize(captureContext, entry);
                return {};
            },
            [&](const SamplerBindingInfo& info) -> MaybeError {
                schema::BindGroupLayoutEntryTypeSamplerBinding entry{{
                    .binding = binding,
                    .data{{
                        .type = info.type,
                    }},
                }};
                Serialize(captureContext, entry);
                return {};
            },
            [&](const StorageTextureBindingInfo& info) -> MaybeError {
                schema::BindGroupLayoutEntryTypeStorageTextureBinding entry{{
                    .binding = binding,
                    .data{{
                        .format = info.format,
                        .viewDimension = info.viewDimension,
                        .access = info.access,
                    }},
                }};
                Serialize(captureContext, entry);
                return {};
            },
            [&](const TextureBindingInfo& info) -> MaybeError {
                schema::BindGroupLayoutEntryTypeTextureBinding entry{{
                    .binding = binding,
                    .data{{
                        .sampleType = info.sampleType,
                        .viewDimension = info.viewDimension,
                        .multisampled = info.multisampled,
                    }},
                }};
                Serialize(captureContext, entry);
                return {};
            },
            [&](const auto& info) -> MaybeError {
                return DAWN_INTERNAL_ERROR("Unsupported bind layout entry type");
            }));
    }

    return {};
}

}  // namespace dawn::native::webgpu
