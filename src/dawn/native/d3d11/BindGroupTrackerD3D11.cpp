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

#include <algorithm>
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
    auto* deviceContext = commandContext->GetD3D11DeviceContext3();

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

}  // namespace

template <typename T, uint32_t InitialCapacity>
template <typename Fn>
void RenderPassBindGroupTracker::BindingSlot<T, InitialCapacity>::Bind(uint32_t idx,
                                                                       T binding,
                                                                       Fn&& bindFunc) {
    if (MaxBoundSlots() <= idx) {
        mBoundSlots.resize(idx + 1);
    }
    if (mBoundSlots[idx] == binding) {
        // redundant binding, return
        return;
    }

    bindFunc(idx, binding);

    mBoundSlots[idx] = std::move(binding);
}

BindGroupTracker::BindGroupTracker(const ScopedSwapStateCommandRecordingContext* commandContext)
    : mCommandContext(commandContext) {
    mLastAppliedPipelineLayout = mCommandContext->GetDevice()->GetEmptyPipelineLayout();
}

BindGroupTracker::~BindGroupTracker() {
    auto* deviceContext = mCommandContext->GetD3D11DeviceContext3();

    // Unbind constant buffers.
    // Note: We already track max bound slots so we can precisely unbind the correct number of used
    // slots. It should be faster than unbinding everything from
    // [0 - D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT).
    static constexpr ID3D11Buffer* kNullBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] =
        {};
#define UNBIND_CONST_BUFFERS(prefix)                                                           \
    {                                                                                          \
        auto& slots = m##prefix##ConstantBufferSlots;                                          \
        if (slots.MaxBoundSlots() > 0) {                                                       \
            deviceContext->prefix##SetConstantBuffers1(0, slots.MaxBoundSlots(), kNullBuffers, \
                                                       nullptr, nullptr);                      \
        }                                                                                      \
    }
    UNBIND_CONST_BUFFERS(VS)
    UNBIND_CONST_BUFFERS(PS)
    UNBIND_CONST_BUFFERS(CS)

    // Unbind SRVs.
    static constexpr ID3D11ShaderResourceView*
        kNullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
#define UNBIND_SRVS(prefix)                                                                 \
    {                                                                                       \
        auto& slots = m##prefix##SRVSlots;                                                  \
        if (slots.MaxBoundSlots() > 0) {                                                    \
            deviceContext->prefix##SetShaderResources(0, slots.MaxBoundSlots(), kNullSRVs); \
        }                                                                                   \
    }
    UNBIND_SRVS(VS)
    UNBIND_SRVS(PS)
    UNBIND_SRVS(CS)

    // Unbind samplers.
    static constexpr ID3D11SamplerState* kNullSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
#define UNBIND_SAMPLERS(prefix)                                                          \
    {                                                                                    \
        auto& slots = m##prefix##SamplerSlots;                                           \
        if (slots.MaxBoundSlots() > 0) {                                                 \
            deviceContext->prefix##SetSamplers(0, slots.MaxBoundSlots(), kNullSamplers); \
        }                                                                                \
    }
    UNBIND_SAMPLERS(VS)
    UNBIND_SAMPLERS(PS)
    UNBIND_SAMPLERS(CS)

    // Unbind UAVs.
    static constexpr ID3D11UnorderedAccessView* kNullUAVs[D3D11_1_UAV_SLOT_COUNT] = {};
    if (mCSUAVSlots.MaxBoundSlots() > 0) {
        deviceContext->CSSetUnorderedAccessViews(0, mCSUAVSlots.MaxBoundSlots(), kNullUAVs,
                                                 nullptr);
    }
    if (mPSMaxBoundUAVs > 0) {
        deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, 0, mPSMaxBoundUAVs,
            kNullUAVs, nullptr);
    }

    DAWN_ASSERT(CheckAllSlotsAreEmpty(mCommandContext));
}

void BindGroupTracker::VSSetConstantBuffer(uint32_t idx,
                                           ID3D11Buffer* d3d11Buffer,
                                           uint32_t firstConstant,
                                           uint32_t numConstants) {
    mVSConstantBufferSlots.Bind(
        idx, {d3d11Buffer, firstConstant, numConstants},
        [this](size_t idx, const ConstantBufferBinding& binding) {
            mCommandContext->GetD3D11DeviceContext3()->VSSetConstantBuffers1(
                idx, 1, binding.buffer.GetAddressOf(), &binding.firstConstant,
                &binding.numConstants);
        });
}
void BindGroupTracker::PSSetConstantBuffer(uint32_t idx,
                                           ID3D11Buffer* d3d11Buffer,
                                           uint32_t firstConstant,
                                           uint32_t numConstants) {
    mPSConstantBufferSlots.Bind(
        idx, {d3d11Buffer, firstConstant, numConstants},
        [this](size_t idx, const ConstantBufferBinding& binding) {
            mCommandContext->GetD3D11DeviceContext3()->PSSetConstantBuffers1(
                idx, 1, binding.buffer.GetAddressOf(), &binding.firstConstant,
                &binding.numConstants);
        });
}
void BindGroupTracker::CSSetConstantBuffer(uint32_t idx,
                                           ID3D11Buffer* d3d11Buffer,
                                           uint32_t firstConstant,
                                           uint32_t numConstants) {
    mCSConstantBufferSlots.Bind(
        idx, {d3d11Buffer, firstConstant, numConstants},
        [this](size_t idx, const ConstantBufferBinding& binding) {
            mCommandContext->GetD3D11DeviceContext3()->CSSetConstantBuffers1(
                idx, 1, binding.buffer.GetAddressOf(), &binding.firstConstant,
                &binding.numConstants);
        });
}
void BindGroupTracker::VSSetShaderResource(uint32_t idx, ComPtr<ID3D11ShaderResourceView> srv) {
    mVSSRVSlots.Bind(idx, std::move(srv),
                     [this](size_t idx, const ComPtr<ID3D11ShaderResourceView>& binding) {
                         mCommandContext->GetD3D11DeviceContext3()->VSSetShaderResources(
                             idx, 1, binding.GetAddressOf());
                     });
}
void BindGroupTracker::PSSetShaderResource(uint32_t idx, ComPtr<ID3D11ShaderResourceView> srv) {
    mPSSRVSlots.Bind(idx, std::move(srv),
                     [this](size_t idx, const ComPtr<ID3D11ShaderResourceView>& binding) {
                         mCommandContext->GetD3D11DeviceContext3()->PSSetShaderResources(
                             idx, 1, binding.GetAddressOf());
                     });
}
void BindGroupTracker::CSSetShaderResource(uint32_t idx, ComPtr<ID3D11ShaderResourceView> srv) {
    mCSSRVSlots.Bind(idx, std::move(srv),
                     [this](size_t idx, const ComPtr<ID3D11ShaderResourceView>& binding) {
                         mCommandContext->GetD3D11DeviceContext3()->CSSetShaderResources(
                             idx, 1, binding.GetAddressOf());
                     });
}
void BindGroupTracker::VSSetSampler(uint32_t idx, ID3D11SamplerState* sampler) {
    mVSSamplerSlots.Bind(idx, sampler,
                         [this](size_t idx, const ComPtr<ID3D11SamplerState>& binding) {
                             mCommandContext->GetD3D11DeviceContext3()->VSSetSamplers(
                                 idx, 1, binding.GetAddressOf());
                         });
}
void BindGroupTracker::PSSetSampler(uint32_t idx, ID3D11SamplerState* sampler) {
    mPSSamplerSlots.Bind(idx, sampler,
                         [this](size_t idx, const ComPtr<ID3D11SamplerState>& binding) {
                             mCommandContext->GetD3D11DeviceContext3()->PSSetSamplers(
                                 idx, 1, binding.GetAddressOf());
                         });
}
void BindGroupTracker::CSSetSampler(uint32_t idx, ID3D11SamplerState* sampler) {
    mCSSamplerSlots.Bind(idx, sampler,
                         [this](size_t idx, const ComPtr<ID3D11SamplerState>& binding) {
                             mCommandContext->GetD3D11DeviceContext3()->CSSetSamplers(
                                 idx, 1, binding.GetAddressOf());
                         });
}

void BindGroupTracker::CSSetUnorderedAccessView(uint32_t idx,
                                                ComPtr<ID3D11UnorderedAccessView> uav) {
    mCSUAVSlots.Bind(idx, std::move(uav),
                     [this](size_t idx, const ComPtr<ID3D11UnorderedAccessView>& binding) {
                         mCommandContext->GetD3D11DeviceContext3()->CSSetUnorderedAccessViews(
                             idx, 1, binding.GetAddressOf(), nullptr);
                     });
}

void BindGroupTracker::OMSetUnorderedAccessViews(uint32_t startSlot,
                                                 uint32_t count,
                                                 ID3D11UnorderedAccessView* const* uavs) {
    mPSMaxBoundUAVs = std::max(mPSMaxBoundUAVs, startSlot + count);

    GetCommandContext()->GetD3D11DeviceContext3()->OMSetRenderTargetsAndUnorderedAccessViews(
        D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, startSlot, count, uavs,
        nullptr);
}

template <wgpu::ShaderStage kVisibleStage>
MaybeError BindGroupTracker::ApplyBindGroup(BindGroupIndex index) {
    constexpr wgpu::ShaderStage kVisibleFragment = wgpu::ShaderStage::Fragment & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleVertex = wgpu::ShaderStage::Vertex & kVisibleStage;
    constexpr wgpu::ShaderStage kVisibleCompute = wgpu::ShaderStage::Compute & kVisibleStage;

    BindGroupBase* group = mBindGroups[index];
    const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
    const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        const auto bindingVisibility = bindingInfo.visibility & kVisibleStage;

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

                        if (bindingVisibility & kVisibleVertex) {
                            this->VSSetConstantBuffer(bindingSlot, d3d11Buffer, firstConstant,
                                                      numConstants);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            this->PSSetConstantBuffer(bindingSlot, d3d11Buffer, firstConstant,
                                                      numConstants);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            this->CSSetConstantBuffer(bindingSlot, d3d11Buffer, firstConstant,
                                                      numConstants);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        // Skip fragment on purpose because render passes requires a single
                        // OMSetRenderTargetsAndUnorderedAccessViews call to set all UAVs.
                        // Delegate to RenderPassBindGroupTracker::Apply.
                        if (bindingVisibility & wgpu::ShaderStage::Compute) {
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV,
                                            ToGPUUsableBuffer(binding.buffer)
                                                ->UseAsUAV(mCommandContext, offset, binding.size));
                            this->CSSetUnorderedAccessView(bindingSlot, d3d11UAV);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding: {
                        ComPtr<ID3D11ShaderResourceView> d3d11SRV;
                        DAWN_TRY_ASSIGN(d3d11SRV,
                                        ToGPUUsableBuffer(binding.buffer)
                                            ->UseAsSRV(mCommandContext, offset, binding.size));
                        if (bindingVisibility & kVisibleVertex) {
                            this->VSSetShaderResource(bindingSlot, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            this->PSSetShaderResource(bindingSlot, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            this->CSSetShaderResource(bindingSlot, d3d11SRV);
                        }
                        break;
                    }
                    case wgpu::BufferBindingType::BindingNotUsed:
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
                if (bindingVisibility & kVisibleVertex) {
                    this->VSSetSampler(bindingSlot, d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleFragment) {
                    this->PSSetSampler(bindingSlot, d3d11SamplerState);
                }
                if (bindingVisibility & kVisibleCompute) {
                    this->CSSetSampler(bindingSlot, d3d11SamplerState);
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
                if (bindingVisibility & kVisibleVertex) {
                    this->VSSetShaderResource(bindingSlot, srv);
                }
                if (bindingVisibility & kVisibleFragment) {
                    this->PSSetShaderResource(bindingSlot, srv);
                }
                if (bindingVisibility & kVisibleCompute) {
                    this->CSSetShaderResource(bindingSlot, srv);
                }
                return {};
            },
            [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        // Skip fragment on purpose because render passes requires a single
                        // OMSetRenderTargetsAndUnorderedAccessViews call to set all UAVs.
                        // Delegate to RenderPassBindGroupTracker::Apply.
                        if (bindingVisibility & kVisibleCompute) {
                            ID3D11UnorderedAccessView* d3d11UAV = nullptr;
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            this->CSSetUnorderedAccessView(bindingSlot, d3d11UAV);
                        }
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        ID3D11ShaderResourceView* d3d11SRV = nullptr;
                        DAWN_TRY_ASSIGN(d3d11SRV, view->GetOrCreateD3D11ShaderResourceView());
                        if (bindingVisibility & kVisibleVertex) {
                            this->VSSetShaderResource(bindingSlot, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleFragment) {
                            this->PSSetShaderResource(bindingSlot, d3d11SRV);
                        }
                        if (bindingVisibility & kVisibleCompute) {
                            this->CSSetShaderResource(bindingSlot, d3d11SRV);
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

const ScopedSwapStateCommandRecordingContext* BindGroupTracker::GetCommandContext() const {
    return mCommandContext.get();
}

ComputePassBindGroupTracker::ComputePassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext)
    : BindGroupTracker(commandContext) {}

ComputePassBindGroupTracker::~ComputePassBindGroupTracker() = default;

void ComputePassBindGroupTracker::UnapplyComputeBindings(BindGroupIndex index) {
    const BindGroupLayoutInternalBase* groupLayout =
        mLastAppliedPipelineLayout->GetBindGroupLayout(index);
    const auto& indices = ToBackend(mLastAppliedPipelineLayout)->GetBindingIndexInfo()[index];

    for (BindingIndex bindingIndex : Range(groupLayout->GetBindingCount())) {
        const BindingInfo& bindingInfo = groupLayout->GetBindingInfo(bindingIndex);
        const uint32_t bindingSlot = indices[bindingIndex];
        if (!(bindingInfo.visibility & wgpu::ShaderStage::Compute)) {
            continue;
        }

        MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                switch (layout.type) {
                    case wgpu::BufferBindingType::Uniform: {
                        this->CSSetConstantBuffer(bindingSlot, nullptr, 0, 0);
                        break;
                    }
                    case wgpu::BufferBindingType::Storage:
                    case kInternalStorageBufferBinding: {
                        this->CSSetUnorderedAccessView(bindingSlot, nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                    case kInternalReadOnlyStorageBufferBinding: {
                        this->CSSetShaderResource(bindingSlot, nullptr);
                        break;
                    }
                    case wgpu::BufferBindingType::BindingNotUsed:
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
            },
            [&](const StaticSamplerBindingInfo&) {
                // Static samplers are implemented in the frontend on
                // D3D11.
                DAWN_UNREACHABLE();
            },
            [&](const SamplerBindingInfo&) { this->CSSetSampler(bindingSlot, nullptr); },
            [&](const TextureBindingInfo&) { this->CSSetShaderResource(bindingSlot, nullptr); },
            [&](const StorageTextureBindingInfo& layout) {
                switch (layout.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                    case wgpu::StorageTextureAccess::ReadWrite: {
                        this->CSSetUnorderedAccessView(bindingSlot, nullptr);
                        break;
                    }
                    case wgpu::StorageTextureAccess::ReadOnly: {
                        this->CSSetShaderResource(bindingSlot, nullptr);
                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            },
            [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
    }
}

MaybeError ComputePassBindGroupTracker::Apply() {
    BeforeApply();

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

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Compute>(index));
    }

    AfterApply();

    return {};
}

RenderPassBindGroupTracker::RenderPassBindGroupTracker(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs)
    : BindGroupTracker(commandContext), mPixelLocalStorageUAVs(std::move(pixelLocalStorageUAVs)) {}

RenderPassBindGroupTracker::~RenderPassBindGroupTracker() = default;

MaybeError RenderPassBindGroupTracker::Apply() {
    BeforeApply();

    // As D3d11 requires to bind all UAVs slots at the same time for pixel shaders, we record
    // all UAV slot assignments in the bind groups, and then bind them all together.
    // TODO(crbug.com/366291600): Clean up D3D11 logic, replace following getters with
    // GetUAVCount() and GetUAVStartSlot(). Clean up related validations.
    const BindGroupMask uavBindGroups = ToBackend(mPipelineLayout)->GetUAVBindGroupLayoutsMask();
    const uint32_t uavSlotCount = ToBackend(mPipelineLayout)->GetTotalUAVBindingCount();
    const uint32_t plsSlotCount = ToBackend(mPipelineLayout)->GetPLSSlotCount();
    const uint32_t unusedUavCount = ToBackend(mPipelineLayout)->GetUnusedUAVBindingCount();

    DAWN_ASSERT(uavSlotCount >= unusedUavCount + plsSlotCount);
    const uint32_t usedUavCount = uavSlotCount - unusedUavCount - plsSlotCount;

    const uint32_t uavStartSlot = unusedUavCount;
    std::vector<ComPtr<ID3D11UnorderedAccessView>> uavsInBindGroup(usedUavCount);

    for (BindGroupIndex index : IterateBitSet(uavBindGroups)) {
        BindGroupBase* group = mBindGroups[index];
        const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets = mDynamicOffsets[index];
        const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[index];

        // D3D11 uav slot allocated in reverse order.
        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
            uint32_t pos = indices[bindingIndex] - uavStartSlot;
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
                            ComPtr<ID3D11UnorderedAccessView> d3d11UAV;
                            DAWN_TRY_ASSIGN(d3d11UAV, ToGPUUsableBuffer(binding.buffer)
                                                          ->UseAsUAV(GetCommandContext(), offset,
                                                                     binding.size));
                            uavsInBindGroup[pos] = std::move(d3d11UAV);
                            break;
                        }
                        case wgpu::BufferBindingType::Uniform:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding: {
                            break;
                        }
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
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
                            DAWN_TRY_ASSIGN(d3d11UAV, view->GetOrCreateD3D11UnorderedAccessView());
                            uavsInBindGroup[pos] = std::move(d3d11UAV);
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

    const uint32_t plsCount = mPixelLocalStorageUAVs.size();
    const uint32_t plsAndUavCount = plsCount + usedUavCount;
    std::vector<ID3D11UnorderedAccessView*> plsAndUavs;
    plsAndUavs.reserve(plsAndUavCount);

    for (auto& uav : uavsInBindGroup) {
        plsAndUavs.push_back(uav.Get());
    }

    for (auto& uav : mPixelLocalStorageUAVs) {
        plsAndUavs.push_back(uav.Get());
    }

    if (!plsAndUavs.empty()) {
        this->OMSetUnorderedAccessViews(uavStartSlot, plsAndUavCount, plsAndUavs.data());
    }

    for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
        DAWN_TRY(ApplyBindGroup<wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment>(index));
    }

    AfterApply();

    return {};
}

}  // namespace dawn::native::d3d11
