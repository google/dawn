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

#include "backend/d3d12/FramebufferD3D12.h"

#include "common/BitSetIterator.h"
#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/TextureD3D12.h"

namespace backend {
namespace d3d12 {

    Framebuffer::Framebuffer(Device* device, FramebufferBuilder* builder)
        : FramebufferBase(builder), device(device) {
        RenderPass* renderPass = ToBackend(GetRenderPass());

        uint32_t rtvCount = 0, dsvCount = 0;
        attachmentHeapIndices.resize(renderPass->GetAttachmentCount());
        for (uint32_t attachment = 0; attachment < renderPass->GetAttachmentCount(); ++attachment) {
            auto* textureView = GetTextureView(attachment);
            if (!textureView) {
                // TODO(kainino@chromium.org): null=backbuffer hack
                attachmentHeapIndices[attachment] = rtvCount++;
                continue;
            }
            auto format = textureView->GetTexture()->GetFormat();
            if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                attachmentHeapIndices[attachment] = dsvCount++;
            } else {
                attachmentHeapIndices[attachment] = rtvCount++;
            }
        }

        if (rtvCount) {
            rtvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvCount);
        }
        if (dsvCount) {
            dsvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvCount);
        }

        for (uint32_t attachment = 0; attachment < renderPass->GetAttachmentCount(); ++attachment) {
            uint32_t heapIndex = attachmentHeapIndices[attachment];
            auto* textureView = GetTextureView(attachment);
            if (!textureView) {
                continue;
            }

            ComPtr<ID3D12Resource> texture = ToBackend(textureView->GetTexture())->GetD3D12Resource();
            auto format = textureView->GetTexture()->GetFormat();
            if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.GetCPUHandle(heapIndex);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = ToBackend(textureView)->GetDSVDescriptor();
                device->GetD3D12Device()->CreateDepthStencilView(texture.Get(), &dsvDesc, dsvHandle);
            } else {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(heapIndex);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = ToBackend(textureView)->GetRTVDescriptor();
                device->GetD3D12Device()->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
            }
        }
    }

    Framebuffer::OMSetRenderTargetArgs Framebuffer::GetSubpassOMSetRenderTargetArgs(uint32_t subpassIndex) {
        const auto& subpassInfo = GetRenderPass()->GetSubpassInfo(subpassIndex);
        OMSetRenderTargetArgs args = {};

        for (uint32_t index : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            uint32_t heapIndex = attachmentHeapIndices[subpassInfo.colorAttachments[index]];
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(heapIndex);

            uint32_t attachment = subpassInfo.colorAttachments[index];
            if (!GetTextureView(attachment)) {
                // TODO(kainino@chromium.org): null=backbuffer hack
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                rtvDesc.Texture2D.PlaneSlice = 0;
                device->GetD3D12Device()->CreateRenderTargetView(device->GetCurrentTexture().Get(), &rtvDesc, rtvHandle);
            }

            args.RTVs[args.numRTVs++] = rtvHandle;
        }
        if (subpassInfo.depthStencilAttachmentSet) {
            uint32_t heapIndex = attachmentHeapIndices[subpassInfo.depthStencilAttachment];
            args.dsv = dsvHeap.GetCPUHandle(heapIndex);
        }

        return args;
    }

}
}
