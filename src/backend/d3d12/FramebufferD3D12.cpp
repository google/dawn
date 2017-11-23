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
        : FramebufferBase(builder), mDevice(device) {
        RenderPass* renderPass = ToBackend(GetRenderPass());

        uint32_t rtvCount = 0, dsvCount = 0;
        mAttachmentHeapIndices.resize(renderPass->GetAttachmentCount());
        for (uint32_t attachment = 0; attachment < renderPass->GetAttachmentCount(); ++attachment) {
            auto* textureView = GetTextureView(attachment);
            auto format = textureView->GetTexture()->GetFormat();
            if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                mAttachmentHeapIndices[attachment] = dsvCount++;
            } else {
                mAttachmentHeapIndices[attachment] = rtvCount++;
            }
        }

        if (rtvCount) {
            mRtvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_RTV, rtvCount);
        }
        if (dsvCount) {
            mDsvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvCount);
        }

        for (uint32_t attachment = 0; attachment < renderPass->GetAttachmentCount(); ++attachment) {
            uint32_t heapIndex = mAttachmentHeapIndices[attachment];
            auto* textureView = GetTextureView(attachment);

            ComPtr<ID3D12Resource> texture = ToBackend(textureView->GetTexture())->GetD3D12Resource();
            auto format = textureView->GetTexture()->GetFormat();
            if (TextureFormatHasDepth(format) || TextureFormatHasStencil(format)) {
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap.GetCPUHandle(heapIndex);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = ToBackend(textureView)->GetDSVDescriptor();
                device->GetD3D12Device()->CreateDepthStencilView(texture.Get(), &dsvDesc, dsvHandle);
            } else {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap.GetCPUHandle(heapIndex);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = ToBackend(textureView)->GetRTVDescriptor();
                device->GetD3D12Device()->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
            }
        }
    }

    Framebuffer::OMSetRenderTargetArgs Framebuffer::GetSubpassOMSetRenderTargetArgs(uint32_t subpassIndex) {
        const auto& subpassInfo = GetRenderPass()->GetSubpassInfo(subpassIndex);
        OMSetRenderTargetArgs args = {};

        for (uint32_t location : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            uint32_t slot = subpassInfo.colorAttachments[location];
            args.RTVs[args.numRTVs] = GetRTVDescriptor(slot);
            args.numRTVs++;
        }
        if (subpassInfo.depthStencilAttachmentSet) {
            uint32_t slot = subpassInfo.depthStencilAttachment;
            args.dsv = GetDSVDescriptor(slot);
        }

        return args;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Framebuffer::GetRTVDescriptor(uint32_t attachmentSlot) {
        return mRtvHeap.GetCPUHandle(mAttachmentHeapIndices[attachmentSlot]);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Framebuffer::GetDSVDescriptor(uint32_t attachmentSlot) {
        return mDsvHeap.GetCPUHandle(mAttachmentHeapIndices[attachmentSlot]);
    }

}
}
