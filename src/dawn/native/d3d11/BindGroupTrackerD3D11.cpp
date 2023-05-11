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

#include "dawn/native/d3d11/BindGroupTrackerD3D11.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BindGroupD3D11.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/SamplerD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"

namespace dawn::native::d3d11 {
namespace {

bool CheckAllSlotsAreEmpty(CommandRecordingContext* commandContext) {
    ID3D11DeviceContext1* deviceContext1 = commandContext->GetD3D11DeviceContext1();

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;

    // Check constant buffer slots
    for (UINT slot = 0;
         slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots; ++slot) {
        ID3D11Buffer* buffer = nullptr;
        deviceContext1->VSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        ASSERT(buffer == nullptr);
        deviceContext1->PSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        ASSERT(buffer == nullptr);
        deviceContext1->CSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        ASSERT(buffer == nullptr);
    }

    // Check resource slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++slot) {
        ID3D11ShaderResourceView* srv = nullptr;
        deviceContext1->VSGetShaderResources(slot, 1, &srv);
        ASSERT(srv == nullptr);
        deviceContext1->PSGetShaderResources(slot, 1, &srv);
        ASSERT(srv == nullptr);
        deviceContext1->CSGetShaderResources(slot, 1, &srv);
        ASSERT(srv == nullptr);
    }

    // Check sampler slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++slot) {
        ID3D11SamplerState* sampler = nullptr;
        deviceContext1->VSGetSamplers(slot, 1, &sampler);
        ASSERT(sampler == nullptr);
        deviceContext1->PSGetSamplers(slot, 1, &sampler);
        ASSERT(sampler == nullptr);
        deviceContext1->CSGetSamplers(slot, 1, &sampler);
        ASSERT(sampler == nullptr);
    }

    // Check UAV slots
    for (UINT slot = 0; slot < D3D11_1_UAV_SLOT_COUNT; ++slot) {
        ID3D11UnorderedAccessView* uav = nullptr;
        deviceContext1->CSGetUnorderedAccessViews(slot, 1, &uav);
        ASSERT(uav == nullptr);
    }

    return true;
}

}  // namespace

BindGroupTracker::BindGroupTracker(CommandRecordingContext* commandContext)
    : mCommandContext(commandContext) {}

BindGroupTracker::~BindGroupTracker() {
    if (mLastAppliedPipelineLayout) {
        for (BindGroupIndex index :
             IterateBitSet(mLastAppliedPipelineLayout->GetBindGroupLayoutsMask())) {
            UnApplyBindGroup(index);
        }
    }
    // All slots should be unbound here.
    ASSERT(CheckAllSlotsAreEmpty(mCommandContext));
}

MaybeError BindGroupTracker::Apply() {
    BeforeApply();

    // A resource cannot be bound as both input resource and UAV at the same time, so to avoid this
    // conflict, we need to unbind groups which are not used by the new pipeline. and unbind groups
    // which are not inherited by the new pipeline.
    if (mLastAppliedPipelineLayout) {
        BindGroupLayoutMask unusedGroups = mLastAppliedPipelineLayout->GetBindGroupLayoutsMask() &
                                           ~mPipelineLayout->GetBindGroupLayoutsMask();
        // Unset bind groups which are not used by the new pipeline and are not inherited.
        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroups | unusedGroups)) {
            UnApplyBindGroup(index);
        }
    }

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup(index));
    }

    AfterApply();

    return {};
}

MaybeError BindGroupTracker::ApplyBindGroup(BindGroupIndex index) {
    ID3D11DeviceContext1* deviceContext1 = mCommandContext->GetD3D11DeviceContext1();
    BindGroupBase* group = mBindGroups[index];
    const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
    const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex{0}; bindingIndex < group->GetLayout()->GetBindingCount();
         ++bindingIndex) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];

        switch (bindingInfo.bindingType) {
            case BindingInfoType::Buffer: {
                BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                auto offset = binding.offset;
                if (bindingInfo.buffer.hasDynamicOffset) {
                    // Dynamic buffers are packed at the front of BindingIndices.
                    offset += dynamicOffsets[bindingIndex];
                }

                switch (bindingInfo.buffer.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ToBackend(binding.buffer)->EnsureConstantBufferIsUpdated(mCommandContext);
                        ID3D11Buffer* d3d11Buffer =
                            ToBackend(binding.buffer)->GetD3D11ConstantBuffer();
                        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
                        // Offset and size are measured in shader constants, which are 16 bytes
                        // (4*32-bit components). And the offsets and counts must be multiples
                        // of 16.
                        // WebGPU's minUniformBufferOffsetAlignment is 256.
                        DAWN_ASSERT(IsAligned(offset, 256));
                        uint32_t firstConstant = static_cast<uint32_t>(offset / 16);
                        uint32_t size = static_cast<uint32_t>(Align(binding.size, 16) / 16);
                        uint32_t numConstants = Align(size, 16);
                        DAWN_ASSERT(offset + numConstants * 16 <=
                                    binding.buffer->GetAllocatedSize());

                        if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                            deviceContext1->VSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                  &firstConstant, &numConstants);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->PSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                  &firstConstant, &numConstants);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                  &firstConstant, &numConstants);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        ASSERT(IsSubset(bindingInfo.visibility,
                                        wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute));
                        ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                        DAWN_TRY_ASSIGN(
                            d3d11UAV, ToBackend(binding.buffer)
                                          ->CreateD3D11UnorderedAccessView1(offset, binding.size));
                        ToBackend(binding.buffer)->MarkMutated();
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->OMSetRenderTargetsAndUnorderedAccessViews(
                                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
                                bindingSlot, 1, d3d11UAV.GetAddressOf(), nullptr);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetUnorderedAccessViews(
                                bindingSlot, 1, d3d11UAV.GetAddressOf(), nullptr);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ComPtr<ID3D11ShaderResourceView> d3d11SRV;
                        DAWN_TRY_ASSIGN(d3d11SRV,
                                        ToBackend(binding.buffer)
                                            ->CreateD3D11ShaderResourceView(offset, binding.size));
                        if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                            deviceContext1->VSSetShaderResources(bindingSlot, 1,
                                                                 d3d11SRV.GetAddressOf());
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->PSSetShaderResources(bindingSlot, 1,
                                                                 d3d11SRV.GetAddressOf());
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetShaderResources(bindingSlot, 1,
                                                                 d3d11SRV.GetAddressOf());
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Undefined:
                        UNREACHABLE();
                }
                break;
            }

            case BindingInfoType::Sampler: {
                Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                ID3D11SamplerState* d3d11SamplerState = sampler->GetD3D11SamplerState();
                if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                    deviceContext1->VSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->PSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                break;
            }

            case BindingInfoType::Texture: {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                ComPtr<ID3D11ShaderResourceView> srv;
                DAWN_TRY_ASSIGN(srv, view->CreateD3D11ShaderResourceView());
                if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                    deviceContext1->VSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->PSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                break;
            }

            case BindingInfoType::StorageTexture: {
                ASSERT(bindingInfo.storageTexture.access == wgpu::StorageTextureAccess::WriteOnly);
                ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                DAWN_TRY_ASSIGN(d3d11UAV, view->CreateD3D11UnorderedAccessView());
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->OMSetRenderTargetsAndUnorderedAccessViews(
                        D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, bindingSlot,
                        1, d3d11UAV.GetAddressOf(), nullptr);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetUnorderedAccessViews(bindingSlot, 1,
                                                              d3d11UAV.GetAddressOf(), nullptr);
                }
                break;
            }

            case BindingInfoType::ExternalTexture: {
                return DAWN_UNIMPLEMENTED_ERROR("External textures are not supported");
            }
        }
    }
    return {};
}

void BindGroupTracker::UnApplyBindGroup(BindGroupIndex index) {
    ID3D11DeviceContext1* deviceContext1 = mCommandContext->GetD3D11DeviceContext1();
    BindGroupLayoutBase* groupLayout = mLastAppliedPipelineLayout->GetBindGroupLayout(index);
    const auto& indices = ToBackend(mLastAppliedPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex{0}; bindingIndex < groupLayout->GetBindingCount();
         ++bindingIndex) {
        const BindingInfo& bindingInfo = groupLayout->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];

        switch (bindingInfo.bindingType) {
            case BindingInfoType::Buffer: {
                switch (bindingInfo.buffer.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* nullBuffer = nullptr;
                        if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                            deviceContext1->VSSetConstantBuffers1(bindingSlot, 1, &nullBuffer,
                                                                  nullptr, nullptr);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->PSSetConstantBuffers1(bindingSlot, 1, &nullBuffer,
                                                                  nullptr, nullptr);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetConstantBuffers1(bindingSlot, 1, &nullBuffer,
                                                                  nullptr, nullptr);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        ASSERT(IsSubset(bindingInfo.visibility,
                                        wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute));
                        ID3D11UnorderedAccessView* nullUAV = nullptr;
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->OMSetRenderTargetsAndUnorderedAccessViews(
                                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
                                bindingSlot, 1, &nullUAV, nullptr);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV,
                                                                      nullptr);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ID3D11ShaderResourceView* nullSRV = nullptr;
                        if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                            deviceContext1->VSSetShaderResources(bindingSlot, 1, &nullSRV);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                            deviceContext1->PSSetShaderResources(bindingSlot, 1, &nullSRV);
                        }
                        if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                            deviceContext1->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Undefined:
                        UNREACHABLE();
                }
                break;
            }

            case BindingInfoType::Sampler: {
                ID3D11SamplerState* nullSampler = nullptr;
                if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                    deviceContext1->VSSetSamplers(bindingSlot, 1, &nullSampler);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->PSSetSamplers(bindingSlot, 1, &nullSampler);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetSamplers(bindingSlot, 1, &nullSampler);
                }
                break;
            }

            case BindingInfoType::Texture: {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                if (bindingInfo.visibility & wgpu::ShaderStage::Vertex) {
                    deviceContext1->VSSetShaderResources(bindingSlot, 1, &nullSRV);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->PSSetShaderResources(bindingSlot, 1, &nullSRV);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                }
                break;
            }

            case BindingInfoType::StorageTexture: {
                ASSERT(bindingInfo.storageTexture.access == wgpu::StorageTextureAccess::WriteOnly);
                ID3D11UnorderedAccessView* nullUAV = nullptr;
                if (bindingInfo.visibility & wgpu::ShaderStage::Fragment) {
                    deviceContext1->OMSetRenderTargetsAndUnorderedAccessViews(
                        D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, bindingSlot,
                        1, &nullUAV, nullptr);
                }
                if (bindingInfo.visibility & wgpu::ShaderStage::Compute) {
                    deviceContext1->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV, nullptr);
                }
                break;
            }

            case BindingInfoType::ExternalTexture: {
                UNREACHABLE();
                break;
            }
        }
    }
}

}  // namespace dawn::native::d3d11
