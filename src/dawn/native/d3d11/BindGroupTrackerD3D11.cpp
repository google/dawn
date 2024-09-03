// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/BindGroupTrackerD3D11.h"

#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
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

bool CheckAllSlotsAreEmpty(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* deviceContext = commandContext->GetD3D11DeviceContext4();

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;

    // Check constant buffer slots
    for (UINT slot = 0;
         slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots; ++slot) {
        ID3D11Buffer* buffer = nullptr;
        deviceContext->VSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
        deviceContext->PSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
        deviceContext->CSGetConstantBuffers1(slot, 1, &buffer, nullptr, nullptr);
        DAWN_ASSERT(buffer == nullptr);
    }

    // Check resource slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++slot) {
        ID3D11ShaderResourceView* srv = nullptr;
        deviceContext->VSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
        deviceContext->PSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
        deviceContext->CSGetShaderResources(slot, 1, &srv);
        DAWN_ASSERT(srv == nullptr);
    }

    // Check sampler slots
    for (UINT slot = 0; slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++slot) {
        ID3D11SamplerState* sampler = nullptr;
        deviceContext->VSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
        deviceContext->PSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
        deviceContext->CSGetSamplers(slot, 1, &sampler);
        DAWN_ASSERT(sampler == nullptr);
    }

    // Check UAV slots for compute
    for (UINT slot = 0; slot < D3D11_1_UAV_SLOT_COUNT; ++slot) {
        ID3D11UnorderedAccessView* uav = nullptr;
        deviceContext->CSGetUnorderedAccessViews(slot, 1, &uav);
        DAWN_ASSERT(uav == nullptr);
    }
    // Check UAV slots for render
    for (UINT slot = 0; slot < commandContext->GetDevice()->GetUAVSlotCount(); ++slot) {
        ID3D11UnorderedAccessView* uav = nullptr;
        deviceContext->OMGetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, slot, 1,
                                                                 &uav);
        DAWN_ASSERT(uav == nullptr);
    }

    return true;
}

void ResetAllRenderSlots(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* deviceContext = commandContext->GetD3D11DeviceContext4();

    // Reserve one slot for builtin constants.
    constexpr uint32_t kReservedCBVSlots = 1;

    ID3D11Buffer* d3d11Buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
    uint32_t num = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - kReservedCBVSlots;
    deviceContext->VSSetConstantBuffers1(0, num, d3d11Buffers, nullptr, nullptr);
    deviceContext->PSSetConstantBuffers1(0, num, d3d11Buffers, nullptr, nullptr);

    ID3D11ShaderResourceView* d3d11SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    num = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    deviceContext->VSSetShaderResources(0, num, d3d11SRVs);
    deviceContext->PSSetShaderResources(0, num, d3d11SRVs);

    ID3D11SamplerState* d3d11Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
    num = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    deviceContext->VSSetSamplers(0, num, d3d11Samplers);
    deviceContext->PSSetSamplers(0, num, d3d11Samplers);

    ID3D11UnorderedAccessView* d3d11UAVs[D3D11_1_UAV_SLOT_COUNT] = {};
    num = commandContext->GetDevice()->GetUAVSlotCount();
    deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
        D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, num, d3d11UAVs, nullptr);
}

}  // namespace

BindGroupTracker::BindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    bool isRenderPass,
    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs)
    : mCommandContext(commandContext),
      mIsRenderPass(isRenderPass),
      mVisibleStages(isRenderPass ? wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment
                                  : wgpu::ShaderStage::Compute),
      mPixelLocalStorageUAVs(std::move(pixelLocalStorageUAVs)) {
    mLastAppliedPipelineLayout = mCommandContext->GetDevice()->GetEmptyPipelineLayout();
}

BindGroupTracker::~BindGroupTracker() {
    if (mIsRenderPass) {
        ResetAllRenderSlots(mCommandContext);
    } else {
        for (BindGroupIndex index :
             IterateBitSet(mLastAppliedPipelineLayout->GetBindGroupLayoutsMask())) {
            UnapplyComputeBindings(index);
        }
    }
    // All slots should be unbound here.
    DAWN_ASSERT(CheckAllSlotsAreEmpty(mCommandContext));
}

MaybeError BindGroupTracker::Apply() {
    BeforeApply();

    if (mIsRenderPass) {
        // As D3d11 requires to bind all UAVs slots at the same time for pixel shaders, we record
        // all UAV slot assignments in the bind groups, and then bind them all together.
        const BindGroupMask uavBindGroups =
            ToBackend(mPipelineLayout)->GetUAVBindGroupLayoutsMask();
        std::vector<ComPtr<ID3D11UnorderedAccessView>> uavsInBindGroup;
        for (BindGroupIndex index : IterateBitSet(uavBindGroups)) {
            BindGroupBase* group = mBindGroups[index];
            const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];

            for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
                const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

                DAWN_TRY(MatchVariant(
                    bindingInfo.bindingLayout,
                    [&](const BufferBindingInfo& layout) -> MaybeError {
                        BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                        auto offset = binding.offset;
                        if (layout.hasDynamicOffset) {
                            // Dynamic buffers are packed at the front of BindingIndices.
                            offset += dynamicOffsets[bindingIndex];
                        }

                        switch (layout.type) {
                            case wgpu::BufferBindingType::Storage:
                            case kInternalStorageBufferBinding: {
                                DAWN_ASSERT(IsSubset(
                                    bindingInfo.visibility,
                                    wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute));
                                ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                                DAWN_TRY_ASSIGN(d3d11UAV, ToGPUUsableBuffer(binding.buffer)
                                                              ->UseAsUAV(mCommandContext, offset,
                                                                         binding.size));
                                uavsInBindGroup.insert(uavsInBindGroup.begin(),
                                                       std::move(d3d11UAV));
                                break;
                            }
                            case wgpu::BufferBindingType::Uniform:
                            case wgpu::BufferBindingType::ReadOnlyStorage:
                            case wgpu::BufferBindingType::Undefined: {
                                break;
                            }
                        }
                        return {};
                    },
                    [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                        switch (layout.access) {
                            case wgpu::StorageTextureAccess::WriteOnly:
                            case wgpu::StorageTextureAccess::ReadWrite: {
                                ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                                TextureView* view =
                                    ToBackend(group->GetBindingAsTextureView(bindingIndex));
                                DAWN_TRY_ASSIGN(d3d11UAV,
                                                view->GetOrCreateD3D11UnorderedAccessView());
                                uavsInBindGroup.insert(uavsInBindGroup.begin(),
                                                       std::move(d3d11UAV));
                                break;
                            }
                            case wgpu::StorageTextureAccess::ReadOnly:
                                break;
                            default:
                                DAWN_UNREACHABLE();
                                break;
                        }
                        return {};
                    },
                    [](const TextureBindingInfo&) -> MaybeError { return {}; },
                    [](const SamplerBindingInfo&) -> MaybeError { return {}; },
                    [](const StaticSamplerBindingInfo&) -> MaybeError {
                        // Static samplers are implemented in the frontend on
                        // D3D11.
                        DAWN_UNREACHABLE();
                        return {};
                    },
                    [](const InputAttachmentBindingInfo&) -> MaybeError {
                        DAWN_UNREACHABLE();
                        return {};
                    }));
            }
        }

        uint32_t uavSlotCount = ToBackend(mPipelineLayout->GetDevice())->GetUAVSlotCount();
        std::vector<ID3D11UnorderedAccessView*> views;
        views.reserve(uavsInBindGroup.size() + mPixelLocalStorageUAVs.size());
        for (auto& uav : uavsInBindGroup) {
            views.push_back(uav.Get());
        }
        for (auto& uav : mPixelLocalStorageUAVs) {
            views.push_back(uav.Get());
        }
        if (!views.empty()) {
            DAWN_ASSERT(uavSlotCount >= views.size());
            mCommandContext->GetD3D11DeviceContext4()->OMSetRenderTargetsAndUnorderedAccessViews(
                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
                uavSlotCount - views.size(), views.size(), views.data(), nullptr);
        }
    } else {
        BindGroupMask inheritedGroups =
            mPipelineLayout->InheritedGroupsMask(mLastAppliedPipelineLayout);
        BindGroupMask previousGroups = mLastAppliedPipelineLayout->GetBindGroupLayoutsMask();

        // To avoid UAV / SRV conflicts with bindings in previously bind groups, we unset the bind
        // groups that aren't reused by the current pipeline.
        // We also need to unset the inherited bind groups which are dirty as the group may have
        // both SRV and UAV, and the same resource may change its binding from UAV to SRV next
        // dispatch in the same group.
        //
        // Note: WebGPU API guarantees that resources are not used both as UAV and SRV in the same
        // render pass. So we don't need to do this inside render passes.
        BindGroupMask groupsToUnset = previousGroups & (~inheritedGroups | mDirtyBindGroups);
        for (BindGroupIndex index : IterateBitSet(groupsToUnset)) {
            UnapplyComputeBindings(index);
        }
    }

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup(index));
    }

    AfterApply();

    return {};
}

MaybeError BindGroupTracker::ApplyBindGroup(BindGroupIndex index) {
    auto* deviceContext = mCommandContext->GetD3D11DeviceContext4();
    BindGroupBase* group = mBindGroups[index];
    const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
    const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        const auto bindingVisibility = bindingInfo.visibility & mVisibleStages;

        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) -> MaybeError {
                BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                auto offset = binding.offset;
                if (layout.hasDynamicOffset) {
                    // Dynamic buffers are packed at the front of BindingIndices.
                    offset += dynamicOffsets[bindingIndex];
                }

                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* d3d11Buffer;
                        DAWN_TRY_ASSIGN(d3d11Buffer, ToGPUUsableBuffer(binding.buffer)
                                                         ->GetD3D11ConstantBuffer(mCommandContext));
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

                        if (bindingVisibility & wgpu::ShaderStage::Vertex) {
                            deviceContext->VSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Fragment) {
                            deviceContext->PSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            deviceContext->CSSetConstantBuffers1(bindingSlot, 1, &d3d11Buffer,
                                                                 &firstConstant, &numConstants);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        DAWN_ASSERT(
                            IsSubset(bindingInfo.visibility,
                                     wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute));
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV,
                                            ToGPUUsableBuffer(binding.buffer)
                                                ->UseAsUAV(mCommandContext, offset, binding.size));
                            deviceContext->CSSetUnorderedAccessViews(
                                bindingSlot, 1, d3d11UAV.GetAddressOf(), nullptr);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ComPtr<ID3D11ShaderResourceView> d3d11SRV;
                        DAWN_TRY_ASSIGN(d3d11SRV,
                                        ToGPUUsableBuffer(binding.buffer)
                                            ->UseAsSRV(mCommandContext, offset, binding.size));
                        if (bindingVisibility & wgpu::ShaderStage::Vertex) {
                            deviceContext->VSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Fragment) {
                            deviceContext->PSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            deviceContext->CSSetShaderResources(bindingSlot, 1,
                                                                d3d11SRV.GetAddressOf());
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
                return {};
            },
            [&](const StaticSamplerBindingInfo&) -> MaybeError {
                // Static samplers are implemented in the frontend on
                // D3D11.
                DAWN_UNREACHABLE();
                return {};
            },
            [&](const SamplerBindingInfo&) -> MaybeError {
                Sampler* sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                ID3D11SamplerState* d3d11SamplerState = sampler->GetD3D11SamplerState();
                if (bindingVisibility & wgpu::ShaderStage::Vertex) {
                    deviceContext->VSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingVisibility & wgpu::ShaderStage::Fragment) {
                    deviceContext->PSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                if (bindingVisibility & wgpu::ShaderStage::Compute) {
                    deviceContext->CSSetSamplers(bindingSlot, 1, &d3d11SamplerState);
                }
                return {};
            },
            [&](const TextureBindingInfo&) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                ComPtr<ID3D11ShaderResourceView> srv;
                // For sampling from stencil, we have to use an internal mirror 'R8Uint' texture.
                if (view->GetAspects() == Aspect::Stencil) {
                    DAWN_TRY_ASSIGN(
                        srv, ToBackend(view->GetTexture())->GetStencilSRV(mCommandContext, view));
                } else {
                    DAWN_TRY_ASSIGN(srv, view->GetOrCreateD3D11ShaderResourceView());
                }
                if (bindingVisibility & wgpu::ShaderStage::Vertex) {
                    deviceContext->VSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingVisibility & wgpu::ShaderStage::Fragment) {
                    deviceContext->PSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                if (bindingVisibility & wgpu::ShaderStage::Compute) {
                    deviceContext->CSSetShaderResources(bindingSlot, 1, srv.GetAddressOf());
                }
                return {};
            },
            [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        ID3D11UnorderedAccessView* d3d11UAV = nullptr;
                        DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &d3d11UAV,
                                                                     nullptr);
                        }
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* d3d11SRV = nullptr;
                        DAWN_TRY_ASSIGN(d3d11SRV, view->GetOrCreateD3D11ShaderResourceView());
                        if (bindingVisibility & wgpu::ShaderStage::Vertex) {
                            deviceContext->VSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Fragment) {
                            deviceContext->PSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            deviceContext->CSSetShaderResources(bindingSlot, 1, &d3d11SRV);
                        }
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
                return {};
            },
            [](const InputAttachmentBindingInfo&) -> MaybeError {
                DAWN_UNREACHABLE();
                return {};
            }));
    }
    return {};
}

void BindGroupTracker::UnapplyComputeBindings(BindGroupIndex index) {
    DAWN_ASSERT(!mIsRenderPass);
    auto* deviceContext = mCommandContext->GetD3D11DeviceContext4();
    BindGroupLayoutInternalBase* groupLayout =
        mLastAppliedPipelineLayout->GetBindGroupLayout(index);
    const auto& indices = ToBackend(mLastAppliedPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(groupLayout->GetBindingCount())) {
        const BindingInfo& bindingInfo = groupLayout->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        const auto bindingVisibility = bindingInfo.visibility & mVisibleStages;
        if (!(bindingVisibility & wgpu::ShaderStage::Compute)) {
            continue;
        }

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        ID3D11Buffer* nullBuffer = nullptr;
                        deviceContext->CSSetConstantBuffers1(bindingSlot, 1, &nullBuffer, nullptr,
                                                             nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        DAWN_ASSERT(
                            IsSubset(bindingInfo.visibility,
                                     wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute));
                        ID3D11UnorderedAccessView* nullUAV = nullptr;
                        deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV, nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage: {
                        ID3D11ShaderResourceView* nullSRV = nullptr;
                        deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                        break;
                    }
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
            },
            [&](const StaticSamplerBindingInfo&) {
                // Static samplers are implemented in the frontend on
                // D3D11.
                DAWN_UNREACHABLE();
            },
            [&](const SamplerBindingInfo&) {
                ID3D11SamplerState* nullSampler = nullptr;
                deviceContext->CSSetSamplers(bindingSlot, 1, &nullSampler);
            },
            [&](const TextureBindingInfo&) {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
            },
            [&](const StorageTextureBindingInfo& layout) {
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        ID3D11UnorderedAccessView* nullUAV = nullptr;
                        deviceContext->CSSetUnorderedAccessViews(bindingSlot, 1, &nullUAV, nullptr);
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* nullSRV = nullptr;
                        deviceContext->CSSetShaderResources(bindingSlot, 1, &nullSRV);
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            },
            [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
    }
}

}  // namespace dawn::native::d3d11
