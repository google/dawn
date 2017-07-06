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

namespace backend {
namespace d3d12 {

    namespace {
        D3D12_RESOURCE_STATES D3D12TextureUsage(nxt::TextureUsageBit usage) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            if (usage & nxt::TextureUsageBit::TransferSrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & nxt::TextureUsageBit::TransferDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & nxt::TextureUsageBit::Sampled) {
                resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
            if (usage & nxt::TextureUsageBit::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & nxt::TextureUsageBit::ColorAttachment) {
                resourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            }
            if (usage & nxt::TextureUsageBit::DepthStencilAttachment) {
                resourceState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
            }

            return resourceState;
        }

        D3D12_RESOURCE_FLAGS D3D12ResourceFlags(nxt::TextureUsageBit usage) {
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

            if (usage & nxt::TextureUsageBit::Storage) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            if (usage & nxt::TextureUsageBit::ColorAttachment) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
            if (usage & nxt::TextureUsageBit::DepthStencilAttachment) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            }

            return flags;
        }

        D3D12_RESOURCE_DIMENSION D3D12TextureDimension(nxt::TextureDimension dimension) {
            switch (dimension) {
                case nxt::TextureDimension::e2D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
        }

        DXGI_FORMAT D3D12TextureFormat(nxt::TextureFormat format) {
            switch (format) {
                case nxt::TextureFormat::R8G8B8A8Unorm:
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }
    }

    Texture::Texture(Device* device, TextureBuilder* builder)
        : TextureBase(builder), device(device) {

        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12TextureDimension(GetDimension());
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = GetWidth();
        resourceDescriptor.Height = GetHeight();
        resourceDescriptor.DepthOrArraySize = GetDepth();
        resourceDescriptor.MipLevels = GetNumMipLevels();
        resourceDescriptor.Format = D3D12TextureFormat(GetFormat());
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescriptor.Flags = D3D12ResourceFlags(GetUsage());

        resource = device->GetResourceAllocator()->Allocate(D3D12_HEAP_TYPE_DEFAULT, resourceDescriptor, D3D12TextureUsage(GetUsage()));
    }

    Texture::~Texture() {
        device->GetResourceAllocator()->Release(resource);
    }

    DXGI_FORMAT Texture::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat());
    }

    ComPtr<ID3D12Resource> Texture::GetD3D12Resource() {
        return resource;
    }

    bool Texture::GetResourceTransitionBarrier(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage, D3D12_RESOURCE_BARRIER* barrier) {
        D3D12_RESOURCE_STATES stateBefore = D3D12TextureUsage(currentUsage);
        D3D12_RESOURCE_STATES stateAfter = D3D12TextureUsage(targetUsage);

        if (stateBefore == stateAfter) {
            return false;
        }

        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->Transition.pResource = resource.Get();
        barrier->Transition.StateBefore = stateBefore;
        barrier->Transition.StateAfter = stateAfter;
        barrier->Transition.Subresource = 0;

        return true;
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) {
        D3D12_RESOURCE_BARRIER barrier;
        if (GetResourceTransitionBarrier(currentUsage, targetUsage, &barrier)) {
            device->GetPendingCommandList()->ResourceBarrier(1, &barrier);
        }
    }

    TextureView::TextureView(TextureViewBuilder* builder)
        : TextureViewBase(builder) {

        srvDesc.Format = D3D12TextureFormat(GetTexture()->GetFormat());
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        switch (GetTexture()->GetDimension()) {
            case nxt::TextureDimension::e2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = GetTexture()->GetNumMipLevels();
                srvDesc.Texture2D.PlaneSlice = 0;
                srvDesc.Texture2D.ResourceMinLODClamp = 0;
                break;
        }
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& TextureView::GetSRVDescriptor() const {
        return srvDesc;
    }

}
}
