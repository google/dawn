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

#include "backend/d3d12/RenderPassDescriptorD3D12.h"

#include "backend/d3d12/DeviceD3D12.h"
#include "backend/d3d12/TextureD3D12.h"
#include "common/BitSetIterator.h"

namespace backend { namespace d3d12 {

    RenderPassDescriptor::RenderPassDescriptor(Device* device, RenderPassDescriptorBuilder* builder)
        : RenderPassDescriptorBase(builder), mDevice(device) {
        // Get and fill an RTV heap with the color attachments
        uint32_t colorAttachmentCount = static_cast<uint32_t>(GetColorAttachmentMask().count());
        if (colorAttachmentCount != 0) {
            mRtvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV, colorAttachmentCount);

            for (uint32_t i : IterateBitSet(GetColorAttachmentMask())) {
                TextureView* view = ToBackend(GetColorAttachment(i).view.Get());
                ComPtr<ID3D12Resource> resource = ToBackend(view->GetTexture())->GetD3D12Resource();

                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap.GetCPUHandle(i);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = view->GetRTVDescriptor();
                device->GetD3D12Device()->CreateRenderTargetView(resource.Get(), &rtvDesc,
                                                                 rtvHandle);
            }
        }

        // Get and fill a DSV heap with the depth stencil attachment
        if (HasDepthStencilAttachment()) {
            mDsvHeap = device->GetDescriptorHeapAllocator()->AllocateCPUHeap(
                D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

            TextureView* view = ToBackend(GetDepthStencilAttachment().view.Get());
            ComPtr<ID3D12Resource> resource = ToBackend(view->GetTexture())->GetD3D12Resource();

            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDsvHeap.GetCPUHandle(0);
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = view->GetDSVDescriptor();
            device->GetD3D12Device()->CreateDepthStencilView(resource.Get(), &dsvDesc, dsvHandle);
        }
    }

    RenderPassDescriptor::OMSetRenderTargetArgs
    RenderPassDescriptor::GetSubpassOMSetRenderTargetArgs() {
        OMSetRenderTargetArgs args = {};

        unsigned int rtvIndex = 0;
        for (uint32_t i : IterateBitSet(GetColorAttachmentMask())) {
            args.RTVs[rtvIndex] = GetRTVDescriptor(i);
            rtvIndex++;
        }
        args.numRTVs = rtvIndex;

        if (HasDepthStencilAttachment()) {
            args.dsv = GetDSVDescriptor();
        }

        return args;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderPassDescriptor::GetRTVDescriptor(uint32_t attachmentSlot) {
        return mRtvHeap.GetCPUHandle(attachmentSlot);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderPassDescriptor::GetDSVDescriptor() {
        return mDsvHeap.GetCPUHandle(0);
    }

}}  // namespace backend::d3d12
