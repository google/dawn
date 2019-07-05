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

#include "dawn_native/d3d12/TextureD3D12.h"

#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_RESOURCE_STATES D3D12TextureUsage(dawn::TextureUsageBit usage, const Format& format) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            // Present is an exclusive flag.
            if (usage & dawn::TextureUsageBit::Present) {
                return D3D12_RESOURCE_STATE_PRESENT;
            }

            if (usage & dawn::TextureUsageBit::TransferSrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & dawn::TextureUsageBit::TransferDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & dawn::TextureUsageBit::Sampled) {
                resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
            if (usage & dawn::TextureUsageBit::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & dawn::TextureUsageBit::OutputAttachment) {
                if (format.HasDepthOrStencil()) {
                    resourceState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
                } else {
                    resourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }

            return resourceState;
        }

        D3D12_RESOURCE_FLAGS D3D12ResourceFlags(dawn::TextureUsageBit usage,
                                                const Format& format,
                                                bool isMultisampledTexture) {
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

            if (usage & dawn::TextureUsageBit::Storage) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            // A multisampled resource must have either D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET or
            // D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL set in D3D12_RESOURCE_DESC::Flags.
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_resource_desc
            // Currently all textures are zero-initialized via the render-target path so always add
            // the render target flag, except for compressed textures for which the render-target
            // flag is invalid.
            // TODO(natlee@microsoft.com, jiawei.shao@intel.com): do not require render target for
            // lazy clearing.
            if ((usage & dawn::TextureUsageBit::OutputAttachment) || isMultisampledTexture ||
                !format.isCompressed) {
                if (format.HasDepthOrStencil()) {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                } else {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }
            }

            ASSERT(!(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ||
                   flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            return flags;
        }

        D3D12_RESOURCE_DIMENSION D3D12TextureDimension(dawn::TextureDimension dimension) {
            switch (dimension) {
                case dawn::TextureDimension::e2D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                default:
                    UNREACHABLE();
            }
        }

    }  // namespace

    DXGI_FORMAT D3D12TextureFormat(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::RGBA8Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case dawn::TextureFormat::RG8Unorm:
                return DXGI_FORMAT_R8G8_UNORM;
            case dawn::TextureFormat::R8Unorm:
                return DXGI_FORMAT_R8_UNORM;
            case dawn::TextureFormat::RGBA8Uint:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case dawn::TextureFormat::RG8Uint:
                return DXGI_FORMAT_R8G8_UINT;
            case dawn::TextureFormat::R8Uint:
                return DXGI_FORMAT_R8_UINT;
            case dawn::TextureFormat::BGRA8Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case dawn::TextureFormat::Depth24PlusStencil8:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            // TODO(jiawei.shao@intel.com): support all BC formats
            case dawn::TextureFormat::BC5RGSnorm:
                return DXGI_FORMAT_BC5_SNORM;
            case dawn::TextureFormat::BC5RGUnorm:
                return DXGI_FORMAT_BC5_UNORM;

            default:
                UNREACHABLE();
        }
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor)
        : TextureBase(device, descriptor, TextureState::OwnedInternal) {
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12TextureDimension(GetDimension());
        resourceDescriptor.Alignment = 0;

        const Extent3D& size = GetSize();
        resourceDescriptor.Width = size.width;
        resourceDescriptor.Height = size.height;

        resourceDescriptor.DepthOrArraySize = GetDepthOrArraySize();
        resourceDescriptor.MipLevels = static_cast<UINT16>(GetNumMipLevels());
        resourceDescriptor.Format = D3D12TextureFormat(GetFormat().format);
        resourceDescriptor.SampleDesc.Count = descriptor->sampleCount;
        // TODO(bryan.bernhart@intel.com): investigate how to specify standard MSAA sample pattern.
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescriptor.Flags =
            D3D12ResourceFlags(GetUsage(), GetFormat(), IsMultisampledTexture());

        mResource = ToBackend(GetDevice())
                        ->GetResourceAllocator()
                        ->Allocate(D3D12_HEAP_TYPE_DEFAULT, resourceDescriptor,
                                   D3D12_RESOURCE_STATE_COMMON);

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            DescriptorHeapAllocator* descriptorHeapAllocator = device->GetDescriptorHeapAllocator();
            if (GetFormat().HasDepthOrStencil()) {
                TransitionUsageNow(device->GetPendingCommandList(),
                                   D3D12_RESOURCE_STATE_DEPTH_WRITE);
                DescriptorHeapHandle dsvHeap =
                    descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.GetCPUHandle(0);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = GetDSVDescriptor(0);
                device->GetD3D12Device()->CreateDepthStencilView(mResource.Get(), &dsvDesc,
                                                                 dsvHandle);

                D3D12_CLEAR_FLAGS clearFlags = {};
                if (GetFormat().HasDepth()) {
                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                }
                if (GetFormat().HasStencil()) {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }

                device->GetPendingCommandList()->ClearDepthStencilView(dsvHandle, clearFlags, 1.0f,
                                                                       1u, 0, nullptr);
            } else {
                TransitionUsageNow(device->GetPendingCommandList(),
                                   D3D12_RESOURCE_STATE_RENDER_TARGET);
                DescriptorHeapHandle rtvHeap =
                    descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(0);

                const float clearColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                // TODO(natlee@microsoft.com): clear all array layers for 2D array textures
                for (int i = 0; i < resourceDescriptor.MipLevels; i++) {
                    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
                        GetRTVDescriptor(i, 0, GetArrayLayers());
                    device->GetD3D12Device()->CreateRenderTargetView(mResource.Get(), &rtvDesc,
                                                                     rtvHandle);
                    device->GetPendingCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0,
                                                                           nullptr);
                }
            }
        }
    }

    // With this constructor, the lifetime of the ID3D12Resource is externally managed.
    Texture::Texture(Device* device,
                     const TextureDescriptor* descriptor,
                     ID3D12Resource* nativeTexture)
        : TextureBase(device, descriptor, TextureState::OwnedExternal), mResource(nativeTexture) {
    }

    Texture::~Texture() {
        DestroyInternal();
    }

    bool Texture::CreateD3D12ResourceBarrierIfNeeded(D3D12_RESOURCE_BARRIER* barrier,
                                                     dawn::TextureUsageBit newUsage) const {
        return CreateD3D12ResourceBarrierIfNeeded(barrier,
                                                  D3D12TextureUsage(newUsage, GetFormat()));
    }

    bool Texture::CreateD3D12ResourceBarrierIfNeeded(D3D12_RESOURCE_BARRIER* barrier,
                                                     D3D12_RESOURCE_STATES newState) const {
        // Avoid transitioning the texture when it isn't needed.
        // TODO(cwallez@chromium.org): Need some form of UAV barriers at some point.
        if (mLastState == newState) {
            return false;
        }

        // The COMMON state represents a state where no write operations can be pending, and where
        // all pixels are uncompressed. This makes it possible to transition to some states without
        // synchronization (i.e. without an explicit ResourceBarrier call). This can be to 1) a
        // single write state, or 2) multiple read states.
        //
        // Destination states that qualify for an implicit transition for a non-simulataneous-access
        // texture: NON_PIXEL_SHADER_RESOURCE, PIXEL_SHADER_RESOURCE, COPY_SRC, COPY_DEST
        // https://docs.microsoft.com/en-us/windows/desktop/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions
        {
            static constexpr D3D12_RESOURCE_STATES kD3D12TextureReadOnlyStates =
                D3D12_RESOURCE_STATE_COPY_SOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            if (mLastState == D3D12_RESOURCE_STATE_COMMON) {
                bool singleWriteState = (newState == D3D12_RESOURCE_STATE_COPY_DEST);
                bool readOnlyState = newState == (newState & kD3D12TextureReadOnlyStates);
                if (singleWriteState ^ readOnlyState) {
                    return false;
                }
            }
        }

        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->Transition.pResource = mResource.Get();
        barrier->Transition.StateBefore = mLastState;
        barrier->Transition.StateAfter = newState;
        barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        return true;
    }

    void Texture::DestroyImpl() {
        // If we own the resource, release it.
        ToBackend(GetDevice())->GetResourceAllocator()->Release(mResource);
        mResource = nullptr;
    }

    DXGI_FORMAT Texture::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat().format);
    }

    ID3D12Resource* Texture::GetD3D12Resource() const {
        return mResource.Get();
    }

    UINT16 Texture::GetDepthOrArraySize() {
        switch (GetDimension()) {
            case dawn::TextureDimension::e2D:
                return static_cast<UINT16>(GetArrayLayers());
            default:
                UNREACHABLE();
        }
    }

    void Texture::SetUsage(dawn::TextureUsageBit newUsage) {
        mLastState = D3D12TextureUsage(newUsage, GetFormat());
    }

    void Texture::TransitionUsageNow(ComPtr<ID3D12GraphicsCommandList> commandList,
                                     dawn::TextureUsageBit usage) {
        TransitionUsageNow(commandList, D3D12TextureUsage(usage, GetFormat()));
    }

    void Texture::TransitionUsageNow(ComPtr<ID3D12GraphicsCommandList> commandList,
                                     D3D12_RESOURCE_STATES newState) {
        D3D12_RESOURCE_BARRIER barrier;
        if (CreateD3D12ResourceBarrierIfNeeded(&barrier, newState)) {
            commandList->ResourceBarrier(1, &barrier);
        }

        mLastState = newState;
    }

    D3D12_RENDER_TARGET_VIEW_DESC Texture::GetRTVDescriptor(uint32_t baseMipLevel,
                                                            uint32_t baseArrayLayer,
                                                            uint32_t layerCount) const {
        ASSERT(GetDimension() == dawn::TextureDimension::e2D);
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = GetD3D12Format();
        if (IsMultisampledTexture()) {
            ASSERT(GetNumMipLevels() == 1);
            ASSERT(layerCount == 1);
            ASSERT(baseArrayLayer == 0);
            ASSERT(baseMipLevel == 0);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else {
            // Currently we always use D3D12_TEX2D_ARRAY_RTV because we cannot specify base array
            // layer and layer count in D3D12_TEX2D_RTV. For 2D texture views, we treat them as
            // 1-layer 2D array textures. (Just like how we treat SRVs)
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_rtv
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_array
            // _rtv
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.FirstArraySlice = baseArrayLayer;
            rtvDesc.Texture2DArray.ArraySize = layerCount;
            rtvDesc.Texture2DArray.MipSlice = baseMipLevel;
            rtvDesc.Texture2DArray.PlaneSlice = 0;
        }
        return rtvDesc;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC Texture::GetDSVDescriptor(uint32_t baseMipLevel) const {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = GetD3D12Format();
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        ASSERT(baseMipLevel == 0);

        if (IsMultisampledTexture()) {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = baseMipLevel;
        }

        return dsvDesc;
    }

    void Texture::ClearTexture(ComPtr<ID3D12GraphicsCommandList> commandList,
                               uint32_t baseMipLevel,
                               uint32_t levelCount,
                               uint32_t baseArrayLayer,
                               uint32_t layerCount) {
        // TODO(jiawei.shao@intel.com): initialize the textures in compressed formats with copies.
        if (GetFormat().isCompressed) {
            SetIsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer,
                                               layerCount);
            return;
        }

        Device* device = ToBackend(GetDevice());
        DescriptorHeapAllocator* descriptorHeapAllocator = device->GetDescriptorHeapAllocator();

        if (GetFormat().HasDepthOrStencil()) {
            TransitionUsageNow(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            DescriptorHeapHandle dsvHeap =
                descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.GetCPUHandle(0);
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = GetDSVDescriptor(baseMipLevel);
            device->GetD3D12Device()->CreateDepthStencilView(mResource.Get(), &dsvDesc, dsvHandle);

            D3D12_CLEAR_FLAGS clearFlags = {};
            if (GetFormat().HasDepth()) {
                clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
            }
            if (GetFormat().HasStencil()) {
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
            }

            commandList->ClearDepthStencilView(dsvHandle, clearFlags, 0.0f, 0u, 0, nullptr);
        } else {
            TransitionUsageNow(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
            DescriptorHeapHandle rtvHeap =
                descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(0);
            const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

            // TODO(natlee@microsoft.com): clear all array layers for 2D array textures
            for (uint32_t i = baseMipLevel; i < baseMipLevel + levelCount; i++) {
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
                    GetRTVDescriptor(i, baseArrayLayer, layerCount);
                device->GetD3D12Device()->CreateRenderTargetView(mResource.Get(), &rtvDesc,
                                                                 rtvHandle);
                commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
            }
        }
        SetIsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer, layerCount);
    }

    void Texture::EnsureSubresourceContentInitialized(ComPtr<ID3D12GraphicsCommandList> commandList,
                                                      uint32_t baseMipLevel,
                                                      uint32_t levelCount,
                                                      uint32_t baseArrayLayer,
                                                      uint32_t layerCount) {
        if (!ToBackend(GetDevice())->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer,
                                             layerCount)) {
            // If subresource has not been initialized, clear it to black as it could contain
            // dirty bits from recycled memory
            ClearTexture(commandList, baseMipLevel, levelCount, baseArrayLayer, layerCount);
        }
    }

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor) {
        mSrvDesc.Format = D3D12TextureFormat(descriptor->format);
        mSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        // Currently we always use D3D12_TEX2D_ARRAY_SRV because we cannot specify base array layer
        // and layer count in D3D12_TEX2D_SRV. For 2D texture views, we treat them as 1-layer 2D
        // array textures.
        // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_srv
        // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_array_srv
        // TODO(jiawei.shao@intel.com): support more texture view dimensions.
        // TODO(jiawei.shao@intel.com): support creating SRV on multisampled textures.
        switch (descriptor->dimension) {
            case dawn::TextureViewDimension::e2D:
            case dawn::TextureViewDimension::e2DArray:
                ASSERT(texture->GetDimension() == dawn::TextureDimension::e2D);
                mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                mSrvDesc.Texture2DArray.ArraySize = descriptor->arrayLayerCount;
                mSrvDesc.Texture2DArray.FirstArraySlice = descriptor->baseArrayLayer;
                mSrvDesc.Texture2DArray.MipLevels = descriptor->mipLevelCount;
                mSrvDesc.Texture2DArray.MostDetailedMip = descriptor->baseMipLevel;
                mSrvDesc.Texture2DArray.PlaneSlice = 0;
                mSrvDesc.Texture2DArray.ResourceMinLODClamp = 0;
                break;
            case dawn::TextureViewDimension::Cube:
            case dawn::TextureViewDimension::CubeArray:
                ASSERT(texture->GetDimension() == dawn::TextureDimension::e2D);
                ASSERT(descriptor->arrayLayerCount % 6 == 0);
                mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                mSrvDesc.TextureCubeArray.First2DArrayFace = descriptor->baseArrayLayer;
                mSrvDesc.TextureCubeArray.NumCubes = descriptor->arrayLayerCount / 6;
                mSrvDesc.TextureCubeArray.MostDetailedMip = descriptor->baseMipLevel;
                mSrvDesc.TextureCubeArray.MipLevels = descriptor->mipLevelCount;
                mSrvDesc.TextureCubeArray.ResourceMinLODClamp = 0;
                break;
            default:
                UNREACHABLE();
        }
    }

    DXGI_FORMAT TextureView::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat().format);
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& TextureView::GetSRVDescriptor() const {
        return mSrvDesc;
    }

    D3D12_RENDER_TARGET_VIEW_DESC TextureView::GetRTVDescriptor() const {
        return ToBackend(GetTexture())
            ->GetRTVDescriptor(GetBaseMipLevel(), GetBaseArrayLayer(), GetLayerCount());
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC TextureView::GetDSVDescriptor() const {
        // TODO(jiawei.shao@intel.com): support rendering into a layer of a texture.
        ASSERT(GetLayerCount() == 1);
        ASSERT(GetLevelCount() == 1);
        ASSERT(GetBaseMipLevel() == 0);
        ASSERT(GetBaseArrayLayer() == 0);
        return ToBackend(GetTexture())->GetDSVDescriptor(GetBaseMipLevel());
    }

}}  // namespace dawn_native::d3d12
