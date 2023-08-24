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

#include "dawn/native/d3d11/PipelineLayoutD3D11.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

// static
ResultOrError<Ref<PipelineLayout>> PipelineLayout::Create(
    Device* device,
    const PipelineLayoutDescriptor* descriptor) {
    Ref<PipelineLayout> pipelineLayout = AcquireRef(new PipelineLayout(device, descriptor));
    DAWN_TRY(pipelineLayout->Initialize(device));
    return pipelineLayout;
}

MaybeError PipelineLayout::Initialize(Device* device) {
    unsigned int constantBufferIndex = 0;
    unsigned int samplerIndex = 0;
    unsigned int shaderResourceViewIndex = 0;
    // For d3d11 pixel shaders, the render targets and unordered-access views share the same
    // resource slots when being written out. So we assign UAV binding index decreasingly here.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargetsandunorderedaccessviews
    // TODO(dawn:1818): Support testing on both FL11_0 and FL11_1.
    uint32_t unorderedAccessViewIndex = device->GetUAVSlotCount();
    mTotalUAVBindingCount = unorderedAccessViewIndex;

    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayoutInternalBase* bgl = GetBindGroupLayout(group);
        mIndexInfo[group].resize(bgl->GetBindingCount());

        for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Buffer:
                    switch (bindingInfo.buffer.type) {
                        case wgpu::BufferBindingType::Uniform:
                            mIndexInfo[group][bindingIndex] = constantBufferIndex++;
                            break;
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                            mIndexInfo[group][bindingIndex] = --unorderedAccessViewIndex;
                            mUAVBindGroups.set(group);
                            break;
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                            mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                            break;
                        case wgpu::BufferBindingType::Undefined:
                            UNREACHABLE();
                    }
                    break;

                case BindingInfoType::Sampler:
                    mIndexInfo[group][bindingIndex] = samplerIndex++;
                    break;

                case BindingInfoType::Texture:
                case BindingInfoType::ExternalTexture:
                    mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                    break;

                case BindingInfoType::StorageTexture:
                    switch (bindingInfo.storageTexture.access) {
                        case wgpu::StorageTextureAccess::ReadWrite:
                        case wgpu::StorageTextureAccess::WriteOnly:
                            mIndexInfo[group][bindingIndex] = --unorderedAccessViewIndex;
                            mUAVBindGroups.set(group);
                            break;
                        case wgpu::StorageTextureAccess::ReadOnly:
                            mIndexInfo[group][bindingIndex] = shaderResourceViewIndex++;
                            break;
                        case wgpu::StorageTextureAccess::Undefined:
                            UNREACHABLE();
                    }
                    break;
            }
        }
    }
    mUnusedUAVBindingCount = unorderedAccessViewIndex;
    ASSERT(constantBufferIndex <= kReservedConstantBufferSlot);

    return {};
}

const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo() const {
    return mIndexInfo;
}

const BindGroupLayoutMask& PipelineLayout::GetUAVBindGroupLayoutsMask() const {
    return mUAVBindGroups;
}

}  // namespace dawn::native::d3d11
