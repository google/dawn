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

#ifndef SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_

#include <optional>
#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/d3d/TextureD3D.h"

#include "dawn/native/DawnNative.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/d3d12/FenceD3D12.h"
#include "dawn/native/d3d12/IntegerTypes.h"
#include "dawn/native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class SharedTextureMemory;
class CommandRecordingContext;
class Device;

MaybeError ValidateTextureCanBeWrapped(ID3D12Resource* d3d12Resource,
                                       const TextureDescriptor* descriptor);
MaybeError ValidateVideoTextureCanBeShared(Device* device, DXGI_FORMAT textureFormat);

class Texture final : public d3d::Texture {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device, const TextureDescriptor* descriptor);
    static ResultOrError<Ref<Texture>> CreateExternalImage(Device* device,
                                                           const TextureDescriptor* descriptor,
                                                           ComPtr<IUnknown> d3dTexture,
                                                           std::vector<Ref<d3d::Fence>> waitFences,
                                                           bool isSwapChainTexture,
                                                           bool isInitialized);
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const TextureDescriptor* descriptor,
                                              ComPtr<ID3D12Resource> d3d12Texture);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const TextureDescriptor* descriptor);

    // For external textures, returns the Device internal fence's value associated with the last
    // ExecuteCommandLists that used this texture. If nullopt is returned, the texture wasn't used.
    ResultOrError<ExecutionSerial> EndAccess() override;

    DXGI_FORMAT GetD3D12Format() const;
    ID3D12Resource* GetD3D12Resource() const;
    DXGI_FORMAT GetD3D12CopyableSubresourceFormat(Aspect aspect) const;
    D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags() const;

    D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor(const Format& format,
                                                   uint32_t mipLevel,
                                                   uint32_t baseSlice,
                                                   uint32_t sliceCount) const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor(uint32_t mipLevel,
                                                   uint32_t baseArrayLayer,
                                                   uint32_t layerCount,
                                                   Aspect aspects,
                                                   bool depthReadOnly,
                                                   bool stencilReadOnly) const;

    MaybeError EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                   const SubresourceRange& range);

    MaybeError SynchronizeImportedTextureBeforeUse();
    MaybeError SynchronizeImportedTextureAfterUse();

    void TrackUsageAndGetResourceBarrierForPass(CommandRecordingContext* commandContext,
                                                std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                                const TextureSubresourceUsage& textureUsages);
    void TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                              std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                              wgpu::TextureUsage usage,
                                              const SubresourceRange& range);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    wgpu::TextureUsage usage,
                                    const SubresourceRange& range);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    D3D12_RESOURCE_STATES newState,
                                    const SubresourceRange& range);
    // Reset the D3D12_RESOURCE_STATES and decay tracking to indicate that
    // all subresources are now in the COMMON state.
    void ResetSubresourceStateAndDecayToCommon();

  private:
    using Base = d3d::Texture;

    Texture(Device* device, const TextureDescriptor* descriptor);
    ~Texture() override;

    MaybeError InitializeAsInternalTexture();
    MaybeError InitializeAsExternalTexture(ComPtr<IUnknown> d3dTexture,
                                           std::vector<Ref<d3d::Fence>> waitFences,
                                           bool isSwapChainTexture);
    MaybeError InitializeAsSwapChainTexture(ComPtr<ID3D12Resource> d3d12Texture);

    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;
    void DestroyImpl() override;

    MaybeError ClearTexture(CommandRecordingContext* commandContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue clearValue);

    // Barriers implementation details.
    struct StateAndDecay {
        D3D12_RESOURCE_STATES lastState;
        ExecutionSerial lastDecaySerial;
        bool isValidToDecay;

        bool operator==(const StateAndDecay& other) const;
    };

    SubresourceStorage<StateAndDecay> InitialSubresourceStateAndDecay() const;

    void TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                              std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                              D3D12_RESOURCE_STATES newState,
                                              const SubresourceRange& range);
    void TransitionSubresourceRange(std::vector<D3D12_RESOURCE_BARRIER>* barriers,
                                    const SubresourceRange& range,
                                    StateAndDecay* state,
                                    D3D12_RESOURCE_STATES subresourceNewState,
                                    ExecutionSerial pendingCommandSerial) const;
    void HandleTransitionSpecialCases(CommandRecordingContext* commandContext);

    D3D12_RESOURCE_FLAGS mD3D12ResourceFlags;
    ResourceHeapAllocation mResourceAllocation;

    // TODO(dawn:1460): Encapsulate imported image fields e.g. std::unique_ptr<ExternalImportInfo>.
    std::vector<Ref<d3d::Fence>> mWaitFences;
    std::optional<ExecutionSerial> mSignalFenceValue;
    bool mSwapChainTexture = false;

    SubresourceStorage<StateAndDecay> mSubresourceStateAndDecay;
};

class TextureView final : public TextureViewBase {
  public:
    static Ref<TextureView> Create(TextureBase* texture, const TextureViewDescriptor* descriptor);

    DXGI_FORMAT GetD3D12Format() const;

    const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDescriptor() const;
    D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor() const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor(bool depthReadOnly, bool stencilReadOnly) const;
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDescriptor() const;

  private:
    TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor);

    D3D12_SHADER_RESOURCE_VIEW_DESC mSrvDesc;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_
