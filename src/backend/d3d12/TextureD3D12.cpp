// Copyright 2017 The NXT Authors
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

#include "backend/d3d12/TextureD3D12.h"

#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/ResourceAllocator.h"

namespace backend { namespace d3d12 {

    namespace {
        D3D12_RESOURCE_STATES D3D12TextureUsage(nxt::TextureUsageBit usage,
                                                nxt::TextureFormat format) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            // Present is an exclusive flag.
            if (usage & nxt::TextureUsageBit::Present) {
                return D3D12_RESOURCE_STATE_PRESENT;
            }

            if (usage & nxt::TextureUsageBit::TransferSrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & nxt::TextureUsageBit::TransferDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & nxt::TextureUsageBit::Sampled) {
                resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
            if (usage & nxt::TextureUsageBit::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & nxt::TextureUsageBit::OutputAttachment) {
                if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                    resourceState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
                } else {
                    resourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }

            return resourceState;
        }

        D3D12_RESOURCE_FLAGS D3D12ResourceFlags(nxt::TextureUsageBit usage,
                                                nxt::TextureFormat format) {
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

            if (usage & nxt::TextureUsageBit::Storage) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            if (usage & nxt::TextureUsageBit::OutputAttachment) {
                if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                } else {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }
            }

            ASSERT(!(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ||
                   flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            return flags;
        }

        D3D12_RESOURCE_DIMENSION D3D12TextureDimension(nxt::TextureDimension dimension) {
            switch (dimension) {
                case nxt::TextureDimension::e2D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                default:
                    UNREACHABLE();
            }
        }

    }  // namespace

    DXGI_FORMAT D3D12TextureFormat(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case nxt::TextureFormat::R8G8B8A8Uint:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case nxt::TextureFormat::D32FloatS8Uint:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            default:
                UNREACHABLE();
        }
    }

    Texture::Texture(TextureBuilder* builder)
        : TextureBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12TextureDimension(GetDimension());
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = GetWidth();
        resourceDescriptor.Height = GetHeight();
        resourceDescriptor.DepthOrArraySize = static_cast<UINT16>(GetDepth());
        resourceDescriptor.MipLevels = static_cast<UINT16>(GetNumMipLevels());
        resourceDescriptor.Format = D3D12TextureFormat(GetFormat());
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescriptor.Flags = D3D12ResourceFlags(GetAllowedUsage(), GetFormat());

        mResource =
            mDevice->GetResourceAllocator()->Allocate(D3D12_HEAP_TYPE_DEFAULT, resourceDescriptor,
                                                      D3D12TextureUsage(GetUsage(), GetFormat()));
        mResourcePtr = mResource.Get();
    }

    // With this constructor, the lifetime of the ID3D12Resource is externally managed.
    Texture::Texture(TextureBuilder* builder, ID3D12Resource* nativeTexture)
        : TextureBase(builder),
          mDevice(ToBackend(builder->GetDevice())),
          mResourcePtr(nativeTexture) {
    }

    Texture::~Texture() {
        if (mResource) {
            // If we own the resource, release it.
            mDevice->GetResourceAllocator()->Release(mResource);
        }
    }

    DXGI_FORMAT Texture::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat());
    }

    ID3D12Resource* Texture::GetD3D12Resource() {
        return mResourcePtr;
    }

    bool Texture::GetResourceTransitionBarrier(nxt::TextureUsageBit currentUsage,
                                               nxt::TextureUsageBit targetUsage,
                                               D3D12_RESOURCE_BARRIER* barrier) {
        D3D12_RESOURCE_STATES stateBefore = D3D12TextureUsage(currentUsage, GetFormat());
        D3D12_RESOURCE_STATES stateAfter = D3D12TextureUsage(targetUsage, GetFormat());

        if (stateBefore == stateAfter) {
            return false;
        }

        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->Transition.pResource = mResourcePtr;
        barrier->Transition.StateBefore = stateBefore;
        barrier->Transition.StateAfter = stateAfter;
        barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        return true;
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit currentUsage,
                                      nxt::TextureUsageBit targetUsage) {
        D3D12_RESOURCE_BARRIER barrier;
        if (GetResourceTransitionBarrier(currentUsage, targetUsage, &barrier)) {
            mDevice->GetPendingCommandList()->ResourceBarrier(1, &barrier);
        }
    }

    TextureView::TextureView(TextureViewBuilder* builder) : TextureViewBase(builder) {
        mSrvDesc.Format = D3D12TextureFormat(GetTexture()->GetFormat());
        mSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        switch (GetTexture()->GetDimension()) {
            case nxt::TextureDimension::e2D:
                mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                mSrvDesc.Texture2D.MostDetailedMip = 0;
                mSrvDesc.Texture2D.MipLevels = GetTexture()->GetNumMipLevels();
                mSrvDesc.Texture2D.PlaneSlice = 0;
                mSrvDesc.Texture2D.ResourceMinLODClamp = 0;
                break;
        }
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& TextureView::GetSRVDescriptor() const {
        return mSrvDesc;
    }

    D3D12_RENDER_TARGET_VIEW_DESC TextureView::GetRTVDescriptor() {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = ToBackend(GetTexture())->GetD3D12Format();
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        return rtvDesc;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC TextureView::GetDSVDescriptor() {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = ToBackend(GetTexture())->GetD3D12Format();
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        return dsvDesc;
    }

}}  // namespace backend::d3d12
