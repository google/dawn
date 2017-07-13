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
        uint32_t attachmentCount = renderPass->GetAttachmentCount();
        rtvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, attachmentCount);

        bool usingBackbuffer = false; // HACK(enga@google.com): workaround for not having depth attachments
        uint32_t rtvIndex = 0;

        for (uint32_t attachment = 0; attachment < attachmentCount; ++attachment) {
            auto* textureView = GetTextureView(attachment);
            if (textureView) {
                ComPtr<ID3D12Resource> texture = ToBackend(textureView->GetTexture())->GetD3D12Resource();
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(rtvIndex++);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = ToBackend(textureView)->GetRTVDescriptor();

                device->GetD3D12Device()->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
            } else {
                // TODO(enga@google.com) no attachment. This will use the backbuffer. Remove when this hack is removed
                usingBackbuffer = true;
                rtvIndex++;
            }
        }

        // TODO(enga@google.com): load depth attachment from subpass
        if (usingBackbuffer) {
            dsvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap.GetCPUHandle(0);

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

            device->GetD3D12Device()->CreateDepthStencilView(device->GetCurrentDepthTexture().Get(), &dsvDesc, dsvHandle);
        }
    }

    Framebuffer::OMSetRenderTargetArgs Framebuffer::GetSubpassOMSetRenderTargetArgs(uint32_t subpassIndex) {
        const auto& subpassInfo = GetRenderPass()->GetSubpassInfo(subpassIndex);

        bool usingBackbuffer = false; // HACK(enga@google.com): workaround for not having depth attachments

        OMSetRenderTargetArgs args;
        args.numRTVs = 0;

        for (uint32_t index : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            uint32_t attachment = subpassInfo.colorAttachments[index];
            const auto& attachmentInfo = GetRenderPass()->GetAttachmentInfo(attachment);

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetCPUHandle(attachment);
            args.RTVs[args.numRTVs++] = rtvHandle;
            if (!GetTextureView(attachment)) {
                usingBackbuffer = true;

                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format = D3D12TextureFormat(attachmentInfo.format);
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                rtvDesc.Texture2D.PlaneSlice = 0;

                device->GetD3D12Device()->CreateRenderTargetView(device->GetCurrentTexture().Get(), &rtvDesc, rtvHandle);
            }
        }

        if (usingBackbuffer) {
            args.dsv = dsvHeap.GetCPUHandle(0);
        }

        return args;
    }

}
}
