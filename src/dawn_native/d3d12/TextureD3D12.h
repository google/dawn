// Copyright 2017 The Dawn Authors
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

#ifndef DAWNNATIVE_D3D12_TEXTURED3D12_H_
#define DAWNNATIVE_D3D12_TEXTURED3D12_H_

#include "common/Serial.h"
#include "dawn_native/Texture.h"

#include "dawn_native/DawnNative.h"
#include "dawn_native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class CommandRecordingContext;
    class Device;

    DXGI_FORMAT D3D12TextureFormat(wgpu::TextureFormat format);
    MaybeError ValidateD3D12TextureCanBeWrapped(ID3D12Resource* d3d12Resource,
                                                const TextureDescriptor* descriptor);
    MaybeError ValidateTextureDescriptorCanBeWrapped(const TextureDescriptor* descriptor);

    class Texture : public TextureBase {
      public:
        static ResultOrError<TextureBase*> Create(Device* device,
                                                  const TextureDescriptor* descriptor);
        static ResultOrError<TextureBase*> Create(Device* device,
                                                  const ExternalImageDescriptor* descriptor,
                                                  HANDLE sharedHandle,
                                                  uint64_t acquireMutexKey,
                                                  bool isSwapChainTexture);
        Texture(Device* device,
                const TextureDescriptor* descriptor,
                ComPtr<ID3D12Resource> d3d12Texture);

        ~Texture();

        DXGI_FORMAT GetD3D12Format() const;
        ID3D12Resource* GetD3D12Resource() const;

        D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor(uint32_t mipLevel,
                                                       uint32_t baseArrayLayer,
                                                       uint32_t layerCount) const;
        D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor(uint32_t mipLevel,
                                                       uint32_t baseArrayLayer,
                                                       uint32_t layerCount) const;
        void EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                 uint32_t baseMipLevel,
                                                 uint32_t levelCount,
                                                 uint32_t baseArrayLayer,
                                                 uint32_t layerCount);

        bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                             D3D12_RESOURCE_BARRIER* barrier,
                                             wgpu::TextureUsage newUsage);
        void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                        wgpu::TextureUsage usage);
        void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                        D3D12_RESOURCE_STATES newState);

      private:
        using TextureBase::TextureBase;

        MaybeError InitializeAsInternalTexture();
        MaybeError InitializeAsExternalTexture(const TextureDescriptor* descriptor,
                                               HANDLE sharedHandle,
                                               uint64_t acquireMutexKey,
                                               bool isSwapChainTexture);

        // Dawn API
        void DestroyImpl() override;
        MaybeError ClearTexture(CommandRecordingContext* commandContext,
                                uint32_t baseMipLevel,
                                uint32_t levelCount,
                                uint32_t baseArrayLayer,
                                uint32_t layerCount,
                                TextureBase::ClearValue clearValue);

        UINT16 GetDepthOrArraySize();

        bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                             D3D12_RESOURCE_BARRIER* barrier,
                                             D3D12_RESOURCE_STATES newState);
        bool TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                                  D3D12_RESOURCE_BARRIER* barrier,
                                                  D3D12_RESOURCE_STATES newState);

        ResourceHeapAllocation mResourceAllocation;
        D3D12_RESOURCE_STATES mLastState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

        Serial mLastUsedSerial = UINT64_MAX;
        bool mValidToDecay = false;
        bool mSwapChainTexture = false;

        Serial mAcquireMutexKey = 0;
        ComPtr<IDXGIKeyedMutex> mDxgiKeyedMutex;
    };

    class TextureView : public TextureViewBase {
      public:
        TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor);

        DXGI_FORMAT GetD3D12Format() const;

        const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDescriptor() const;
        D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor() const;
        D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor() const;

      private:
        D3D12_SHADER_RESOURCE_VIEW_DESC mSrvDesc;
    };
}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_TEXTURED3D12_H_
