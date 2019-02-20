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

#include "dawn_native/d3d12/CommandBufferD3D12.h"

#include "common/Assert.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/d3d12/BindGroupD3D12.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/ComputePipelineD3D12.h"
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/InputStateD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/TextureCopySplitter.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        DXGI_FORMAT DXGIIndexFormat(dawn::IndexFormat format) {
            switch (format) {
                case dawn::IndexFormat::Uint16:
                    return DXGI_FORMAT_R16_UINT;
                case dawn::IndexFormat::Uint32:
                    return DXGI_FORMAT_R32_UINT;
                default:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    struct BindGroupStateTracker {
        uint32_t cbvSrvUavDescriptorIndex = 0;
        uint32_t samplerDescriptorIndex = 0;
        DescriptorHeapHandle cbvSrvUavCPUDescriptorHeap = {};
        DescriptorHeapHandle samplerCPUDescriptorHeap = {};
        DescriptorHeapHandle cbvSrvUavGPUDescriptorHeap = {};
        DescriptorHeapHandle samplerGPUDescriptorHeap = {};
        std::array<BindGroup*, kMaxBindGroups> bindGroups = {};
        bool inCompute = false;

        Device* device;

        BindGroupStateTracker(Device* device) : device(device) {
        }

        void SetInComputePass(bool inCompute_) {
            inCompute = inCompute_;
        }

        void TrackSetBindGroup(BindGroup* group, uint32_t index, uint32_t indexInSubmit) {
            if (bindGroups[index] != group) {
                bindGroups[index] = group;

                // Descriptors don't need to be recorded if they have already been recorded in
                // the heap. Indices are only updated when descriptors are recorded
                const uint64_t serial = device->GetPendingCommandSerial();
                if (group->GetHeapSerial() != serial ||
                    group->GetIndexInSubmit() != indexInSubmit) {
                    group->RecordDescriptors(cbvSrvUavCPUDescriptorHeap, &cbvSrvUavDescriptorIndex,
                                             samplerCPUDescriptorHeap, &samplerDescriptorIndex,
                                             serial, indexInSubmit);
                }
            }
        }

        void TrackInheritedGroups(PipelineLayout* oldLayout,
                                  PipelineLayout* newLayout,
                                  uint32_t indexInSubmit) {
            if (oldLayout == nullptr) {
                return;
            }

            uint32_t inheritUntil = oldLayout->GroupsInheritUpTo(newLayout);
            for (uint32_t i = 0; i < inheritUntil; ++i) {
                TrackSetBindGroup(bindGroups[i], i, indexInSubmit);
            }
        }

        void SetBindGroup(ComPtr<ID3D12GraphicsCommandList> commandList,
                          PipelineLayout* pipelineLayout,
                          BindGroup* group,
                          uint32_t index,
                          bool force = false) {
            if (bindGroups[index] != group || force) {
                bindGroups[index] = group;

                uint32_t cbvUavSrvCount =
                    ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount();
                uint32_t samplerCount = ToBackend(group->GetLayout())->GetSamplerDescriptorCount();

                if (cbvUavSrvCount > 0) {
                    uint32_t parameterIndex = pipelineLayout->GetCbvUavSrvRootParameterIndex(index);

                    if (inCompute) {
                        commandList->SetComputeRootDescriptorTable(
                            parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(
                                                group->GetCbvUavSrvHeapOffset()));
                    } else {
                        commandList->SetGraphicsRootDescriptorTable(
                            parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(
                                                group->GetCbvUavSrvHeapOffset()));
                    }
                }

                if (samplerCount > 0) {
                    uint32_t parameterIndex = pipelineLayout->GetSamplerRootParameterIndex(index);

                    if (inCompute) {
                        commandList->SetComputeRootDescriptorTable(
                            parameterIndex,
                            samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                    } else {
                        commandList->SetGraphicsRootDescriptorTable(
                            parameterIndex,
                            samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                    }
                }
            }
        }

        void SetInheritedBindGroups(ComPtr<ID3D12GraphicsCommandList> commandList,
                                    PipelineLayout* oldLayout,
                                    PipelineLayout* newLayout) {
            if (oldLayout == nullptr) {
                return;
            }

            uint32_t inheritUntil = oldLayout->GroupsInheritUpTo(newLayout);
            for (uint32_t i = 0; i < inheritUntil; ++i) {
                SetBindGroup(commandList, newLayout, bindGroups[i], i, true);
            }
        }

        void Reset() {
            for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                bindGroups[i] = nullptr;
            }
        }
    };

    struct OMSetRenderTargetArgs {
        unsigned int numRTVs = 0;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxColorAttachments> RTVs = {};
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
    };

    class RenderPassDescriptorHeapTracker {
      public:
        RenderPassDescriptorHeapTracker(Device* device) : mDevice(device) {
        }

        // This function must only be called before calling AllocateRTVAndDSVHeaps().
        void TrackRenderPass(const BeginRenderPassCmd* renderPass) {
            DAWN_ASSERT(mRTVHeap.Get() == nullptr && mDSVHeap.Get() == nullptr);
            mNumRTVs += static_cast<uint32_t>(renderPass->colorAttachmentsSet.count());
            if (renderPass->hasDepthStencilAttachment) {
                ++mNumDSVs;
            }
        }

        void AllocateRTVAndDSVHeaps() {
            // This function should only be called once.
            DAWN_ASSERT(mRTVHeap.Get() == nullptr && mDSVHeap.Get() == nullptr);
            DescriptorHeapAllocator* allocator = mDevice->GetDescriptorHeapAllocator();
            if (mNumRTVs > 0) {
                mRTVHeap = allocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, mNumRTVs);
            }
            if (mNumDSVs > 0) {
                mDSVHeap = allocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, mNumDSVs);
            }
        }

        // TODO(jiawei.shao@intel.com): use hash map <RenderPass, OMSetRenderTargetArgs> as cache to
        // avoid redundant RTV and DSV memory allocations.
        OMSetRenderTargetArgs GetSubpassOMSetRenderTargetArgs(BeginRenderPassCmd* renderPass) {
            OMSetRenderTargetArgs args = {};

            unsigned int rtvIndex = 0;
            uint32_t rtvCount = static_cast<uint32_t>(renderPass->colorAttachmentsSet.count());
            DAWN_ASSERT(mAllocatedRTVs + rtvCount <= mNumRTVs);
            for (uint32_t i : IterateBitSet(renderPass->colorAttachmentsSet)) {
                TextureView* view = ToBackend(renderPass->colorAttachments[i].view).Get();
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVHeap.GetCPUHandle(mAllocatedRTVs);
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = view->GetRTVDescriptor();
                mDevice->GetD3D12Device()->CreateRenderTargetView(
                    ToBackend(view->GetTexture())->GetD3D12Resource(), &rtvDesc, rtvHandle);
                args.RTVs[i] = rtvHandle;

                ++rtvIndex;
                ++mAllocatedRTVs;
            }
            args.numRTVs = rtvIndex;

            if (renderPass->hasDepthStencilAttachment) {
                DAWN_ASSERT(mAllocatedDSVs < mNumDSVs);
                TextureView* view = ToBackend(renderPass->depthStencilAttachment.view).Get();
                D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = mDSVHeap.GetCPUHandle(mAllocatedDSVs);
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = view->GetDSVDescriptor();
                mDevice->GetD3D12Device()->CreateDepthStencilView(
                    ToBackend(view->GetTexture())->GetD3D12Resource(), &dsvDesc, dsvHandle);
                args.dsv = dsvHandle;

                ++mAllocatedDSVs;
            }

            return args;
        }

        bool IsHeapAllocationCompleted() const {
            return mNumRTVs == mAllocatedRTVs && mNumDSVs == mAllocatedDSVs;
        }

      private:
        Device* mDevice;
        DescriptorHeapHandle mRTVHeap = {};
        DescriptorHeapHandle mDSVHeap = {};
        uint32_t mNumRTVs = 0;
        uint32_t mNumDSVs = 0;

        uint32_t mAllocatedRTVs = 0;
        uint32_t mAllocatedDSVs = 0;
    };

    namespace {

        void AllocateAndSetDescriptorHeaps(Device* device,
                                           BindGroupStateTracker* bindingTracker,
                                           RenderPassDescriptorHeapTracker* renderPassTracker,
                                           CommandIterator* commands,
                                           int indexInSubmit) {
            auto* descriptorHeapAllocator = device->GetDescriptorHeapAllocator();

            // TODO(enga@google.com): This currently allocates CPU heaps of arbitrarily chosen sizes
            // This will not work if there are too many descriptors
            bindingTracker->cbvSrvUavCPUDescriptorHeap = descriptorHeapAllocator->AllocateCPUHeap(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192);
            bindingTracker->samplerCPUDescriptorHeap =
                descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

            {
                Command type;
                PipelineLayout* lastLayout = nullptr;

                while (commands->NextCommandId(&type)) {
                    switch (type) {
                        case Command::SetComputePipeline: {
                            SetComputePipelineCmd* cmd =
                                commands->NextCommand<SetComputePipelineCmd>();
                            PipelineLayout* layout = ToBackend(cmd->pipeline->GetLayout());
                            bindingTracker->TrackInheritedGroups(lastLayout, layout, indexInSubmit);
                            lastLayout = layout;
                        } break;

                        case Command::SetRenderPipeline: {
                            SetRenderPipelineCmd* cmd =
                                commands->NextCommand<SetRenderPipelineCmd>();
                            PipelineLayout* layout = ToBackend(cmd->pipeline->GetLayout());
                            bindingTracker->TrackInheritedGroups(lastLayout, layout, indexInSubmit);
                            lastLayout = layout;
                        } break;

                        case Command::SetBindGroup: {
                            SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                            BindGroup* group = ToBackend(cmd->group.Get());
                            bindingTracker->TrackSetBindGroup(group, cmd->index, indexInSubmit);
                        } break;
                        case Command::BeginRenderPass: {
                            BeginRenderPassCmd* cmd = commands->NextCommand<BeginRenderPassCmd>();
                            renderPassTracker->TrackRenderPass(cmd);
                        } break;
                        default:
                            SkipCommand(commands, type);
                    }
                }

                commands->Reset();
            }

            renderPassTracker->AllocateRTVAndDSVHeaps();

            if (bindingTracker->cbvSrvUavDescriptorIndex > 0) {
                // Allocate a GPU-visible heap and copy from the CPU-only heap to the GPU-visible
                // heap
                bindingTracker->cbvSrvUavGPUDescriptorHeap =
                    descriptorHeapAllocator->AllocateGPUHeap(
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                        bindingTracker->cbvSrvUavDescriptorIndex);
                device->GetD3D12Device()->CopyDescriptorsSimple(
                    bindingTracker->cbvSrvUavDescriptorIndex,
                    bindingTracker->cbvSrvUavGPUDescriptorHeap.GetCPUHandle(0),
                    bindingTracker->cbvSrvUavCPUDescriptorHeap.GetCPUHandle(0),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            if (bindingTracker->samplerDescriptorIndex > 0) {
                bindingTracker->samplerGPUDescriptorHeap = descriptorHeapAllocator->AllocateGPUHeap(
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, bindingTracker->samplerDescriptorIndex);
                device->GetD3D12Device()->CopyDescriptorsSimple(
                    bindingTracker->samplerDescriptorIndex,
                    bindingTracker->samplerGPUDescriptorHeap.GetCPUHandle(0),
                    bindingTracker->samplerCPUDescriptorHeap.GetCPUHandle(0),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            }
        }

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(Device* device, CommandEncoderBase* encoder)
        : CommandBufferBase(device, encoder), mCommands(encoder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::RecordCommands(ComPtr<ID3D12GraphicsCommandList> commandList,
                                       uint32_t indexInSubmit) {
        Device* device = ToBackend(GetDevice());
        BindGroupStateTracker bindingTracker(device);
        RenderPassDescriptorHeapTracker renderPassTracker(device);

        // Precompute the allocation of bindgroups in descriptor heaps
        // TODO(cwallez@chromium.org): Iterating over all the commands here is inefficient. We
        // should have a system where commands and descriptors are recorded in parallel then the
        // heaps set using a small CommandList inserted just before the main CommandList.
        {
            AllocateAndSetDescriptorHeaps(device, &bindingTracker, &renderPassTracker, &mCommands,
                                          indexInSubmit);
            bindingTracker.Reset();

            ID3D12DescriptorHeap* descriptorHeaps[2] = {
                bindingTracker.cbvSrvUavGPUDescriptorHeap.Get(),
                bindingTracker.samplerGPUDescriptorHeap.Get()};
            if (descriptorHeaps[0] && descriptorHeaps[1]) {
                commandList->SetDescriptorHeaps(2, descriptorHeaps);
            } else if (descriptorHeaps[0]) {
                commandList->SetDescriptorHeaps(1, descriptorHeaps);
            } else if (descriptorHeaps[1]) {
                commandList->SetDescriptorHeaps(1, &descriptorHeaps[1]);
            }
        }

        // Records the necessary barriers for the resource usage pre-computed by the frontend
        auto TransitionForPass = [](ComPtr<ID3D12GraphicsCommandList> commandList,
                                    const PassResourceUsage& usages) {
            for (size_t i = 0; i < usages.buffers.size(); ++i) {
                Buffer* buffer = ToBackend(usages.buffers[i]);
                buffer->TransitionUsageNow(commandList, usages.bufferUsages[i]);
            }
            for (size_t i = 0; i < usages.textures.size(); ++i) {
                Texture* texture = ToBackend(usages.textures[i]);
                texture->TransitionUsageNow(commandList, usages.textureUsages[i]);
            }
        };

        const std::vector<PassResourceUsage>& passResourceUsages = GetResourceUsages().perPass;
        uint32_t nextPassNumber = 0;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();

                    TransitionForPass(commandList, passResourceUsages[nextPassNumber]);
                    bindingTracker.SetInComputePass(true);
                    RecordComputePass(commandList, &bindingTracker);

                    nextPassNumber++;
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* beginRenderPassCmd =
                        mCommands.NextCommand<BeginRenderPassCmd>();

                    TransitionForPass(commandList, passResourceUsages[nextPassNumber]);
                    bindingTracker.SetInComputePass(false);
                    RecordRenderPass(commandList, &bindingTracker, &renderPassTracker,
                                     beginRenderPassCmd);

                    nextPassNumber++;
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    Buffer* srcBuffer = ToBackend(copy->source.buffer.Get());
                    Buffer* dstBuffer = ToBackend(copy->destination.buffer.Get());

                    srcBuffer->TransitionUsageNow(commandList, dawn::BufferUsageBit::TransferSrc);
                    dstBuffer->TransitionUsageNow(commandList, dawn::BufferUsageBit::TransferDst);

                    commandList->CopyBufferRegion(
                        dstBuffer->GetD3D12Resource().Get(), copy->destination.offset,
                        srcBuffer->GetD3D12Resource().Get(), copy->source.offset, copy->size);
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    Buffer* buffer = ToBackend(copy->source.buffer.Get());
                    Texture* texture = ToBackend(copy->destination.texture.Get());

                    buffer->TransitionUsageNow(commandList, dawn::BufferUsageBit::TransferSrc);
                    texture->TransitionUsageNow(commandList, dawn::TextureUsageBit::TransferDst);

                    auto copySplit = ComputeTextureCopySplit(
                        copy->destination.origin, copy->copySize,
                        static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                        copy->source.offset, copy->source.rowPitch, copy->source.imageHeight);

                    D3D12_TEXTURE_COPY_LOCATION textureLocation;
                    textureLocation.pResource = texture->GetD3D12Resource();
                    textureLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    textureLocation.SubresourceIndex =
                        texture->GetNumMipLevels() * copy->destination.slice +
                        copy->destination.level;

                    for (uint32_t i = 0; i < copySplit.count; ++i) {
                        auto& info = copySplit.copies[i];

                        D3D12_TEXTURE_COPY_LOCATION bufferLocation;
                        bufferLocation.pResource = buffer->GetD3D12Resource().Get();
                        bufferLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                        bufferLocation.PlacedFootprint.Offset = copySplit.offset;
                        bufferLocation.PlacedFootprint.Footprint.Format = texture->GetD3D12Format();
                        bufferLocation.PlacedFootprint.Footprint.Width = info.bufferSize.width;
                        bufferLocation.PlacedFootprint.Footprint.Height = info.bufferSize.height;
                        bufferLocation.PlacedFootprint.Footprint.Depth = info.bufferSize.depth;
                        bufferLocation.PlacedFootprint.Footprint.RowPitch = copy->source.rowPitch;

                        D3D12_BOX sourceRegion;
                        sourceRegion.left = info.bufferOffset.x;
                        sourceRegion.top = info.bufferOffset.y;
                        sourceRegion.front = info.bufferOffset.z;
                        sourceRegion.right = info.bufferOffset.x + info.copySize.width;
                        sourceRegion.bottom = info.bufferOffset.y + info.copySize.height;
                        sourceRegion.back = info.bufferOffset.z + info.copySize.depth;

                        commandList->CopyTextureRegion(&textureLocation, info.textureOffset.x,
                                                       info.textureOffset.y, info.textureOffset.z,
                                                       &bufferLocation, &sourceRegion);
                    }
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    Texture* texture = ToBackend(copy->source.texture.Get());
                    Buffer* buffer = ToBackend(copy->destination.buffer.Get());

                    texture->TransitionUsageNow(commandList, dawn::TextureUsageBit::TransferSrc);
                    buffer->TransitionUsageNow(commandList, dawn::BufferUsageBit::TransferDst);

                    auto copySplit = ComputeTextureCopySplit(
                        copy->source.origin, copy->copySize,
                        static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                        copy->destination.offset, copy->destination.rowPitch,
                        copy->destination.imageHeight);

                    D3D12_TEXTURE_COPY_LOCATION textureLocation;
                    textureLocation.pResource = texture->GetD3D12Resource();
                    textureLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    textureLocation.SubresourceIndex =
                        texture->GetNumMipLevels() * copy->source.slice + copy->source.level;

                    for (uint32_t i = 0; i < copySplit.count; ++i) {
                        auto& info = copySplit.copies[i];

                        D3D12_TEXTURE_COPY_LOCATION bufferLocation;
                        bufferLocation.pResource = buffer->GetD3D12Resource().Get();
                        bufferLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                        bufferLocation.PlacedFootprint.Offset = copySplit.offset;
                        bufferLocation.PlacedFootprint.Footprint.Format = texture->GetD3D12Format();
                        bufferLocation.PlacedFootprint.Footprint.Width = info.bufferSize.width;
                        bufferLocation.PlacedFootprint.Footprint.Height = info.bufferSize.height;
                        bufferLocation.PlacedFootprint.Footprint.Depth = info.bufferSize.depth;
                        bufferLocation.PlacedFootprint.Footprint.RowPitch =
                            copy->destination.rowPitch;

                        D3D12_BOX sourceRegion;
                        sourceRegion.left = info.textureOffset.x;
                        sourceRegion.top = info.textureOffset.y;
                        sourceRegion.front = info.textureOffset.z;
                        sourceRegion.right = info.textureOffset.x + info.copySize.width;
                        sourceRegion.bottom = info.textureOffset.y + info.copySize.height;
                        sourceRegion.back = info.textureOffset.z + info.copySize.depth;

                        commandList->CopyTextureRegion(&bufferLocation, info.bufferOffset.x,
                                                       info.bufferOffset.y, info.bufferOffset.z,
                                                       &textureLocation, &sourceRegion);
                    }
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        DAWN_ASSERT(renderPassTracker.IsHeapAllocationCompleted());
    }

    void CommandBuffer::FlushSetVertexBuffers(ComPtr<ID3D12GraphicsCommandList> commandList,
                                              VertexBuffersInfo* vertexBuffersInfo,
                                              const InputState* inputState) {
        DAWN_ASSERT(vertexBuffersInfo != nullptr);
        DAWN_ASSERT(inputState != nullptr);

        auto inputsMask = inputState->GetInputsSetMask();

        uint32_t startSlot = vertexBuffersInfo->startSlot;
        uint32_t endSlot = vertexBuffersInfo->endSlot;

        // If the input state has changed, we need to update the StrideInBytes
        // for the D3D12 buffer views. We also need to extend the dirty range to
        // touch all these slots because the stride may have changed.
        if (vertexBuffersInfo->lastInputState != inputState) {
            vertexBuffersInfo->lastInputState = inputState;

            for (uint32_t slot : IterateBitSet(inputsMask)) {
                startSlot = std::min(startSlot, slot);
                endSlot = std::max(endSlot, slot + 1);
                vertexBuffersInfo->d3d12BufferViews[slot].StrideInBytes =
                    inputState->GetInput(slot).stride;
            }
        }

        if (endSlot <= startSlot) {
            return;
        }

        // d3d12BufferViews is kept up to date with the most recent data passed
        // to SetVertexBuffers. This makes it correct to only track the start
        // and end of the dirty range. When FlushSetVertexBuffers is called,
        // we will at worst set non-dirty vertex buffers in duplicate.
        uint32_t count = endSlot - startSlot;
        commandList->IASetVertexBuffers(startSlot, count,
                                        &vertexBuffersInfo->d3d12BufferViews[startSlot]);

        vertexBuffersInfo->startSlot = kMaxVertexInputs;
        vertexBuffersInfo->endSlot = 0;
    }

    void CommandBuffer::RecordComputePass(ComPtr<ID3D12GraphicsCommandList> commandList,
                                          BindGroupStateTracker* bindingTracker) {
        PipelineLayout* lastLayout = nullptr;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    commandList->Dispatch(dispatch->x, dispatch->y, dispatch->z);
                } break;

                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    return;
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();
                    PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                    commandList->SetComputeRootSignature(layout->GetRootSignature().Get());
                    commandList->SetPipelineState(pipeline->GetPipelineState().Get());

                    bindingTracker->SetInheritedBindGroups(commandList, lastLayout, layout);
                    lastLayout = layout;
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    BindGroup* group = ToBackend(cmd->group.Get());
                    bindingTracker->SetBindGroup(commandList, lastLayout, group, cmd->index);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::RecordRenderPass(ComPtr<ID3D12GraphicsCommandList> commandList,
                                         BindGroupStateTracker* bindingTracker,
                                         RenderPassDescriptorHeapTracker* renderPassTracker,
                                         BeginRenderPassCmd* renderPass) {
        OMSetRenderTargetArgs args = renderPassTracker->GetSubpassOMSetRenderTargetArgs(renderPass);

        // Clear framebuffer attachments as needed and transition to render target
        {
            for (uint32_t i : IterateBitSet(renderPass->colorAttachmentsSet)) {
                auto& attachmentInfo = renderPass->colorAttachments[i];

                // Load op - color
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = args.RTVs[i];
                    commandList->ClearRenderTargetView(handle, attachmentInfo.clearColor.data(), 0,
                                                       nullptr);
                }
            }

            if (renderPass->hasDepthStencilAttachment) {
                auto& attachmentInfo = renderPass->depthStencilAttachment;
                Texture* texture = ToBackend(renderPass->depthStencilAttachment.view->GetTexture());

                // Load op - depth/stencil
                bool doDepthClear = TextureFormatHasDepth(texture->GetFormat()) &&
                                    (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear);
                bool doStencilClear = TextureFormatHasStencil(texture->GetFormat()) &&
                                      (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear);

                D3D12_CLEAR_FLAGS clearFlags = {};
                if (doDepthClear) {
                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                }
                if (doStencilClear) {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }

                if (clearFlags) {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = args.dsv;
                    // TODO(kainino@chromium.org): investigate: should the Dawn clear
                    // stencil type be uint8_t?
                    uint8_t clearStencil = static_cast<uint8_t>(attachmentInfo.clearStencil);
                    commandList->ClearDepthStencilView(
                        handle, clearFlags, attachmentInfo.clearDepth, clearStencil, 0, nullptr);
                }
            }
        }

        // Set up render targets
        {
            if (args.dsv.ptr) {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, &args.dsv);
            } else {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, nullptr);
            }
        }

        // Set up default dynamic state
        {
            uint32_t width = renderPass->width;
            uint32_t height = renderPass->height;
            D3D12_VIEWPORT viewport = {
                0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f};
            D3D12_RECT scissorRect = {0, 0, static_cast<long>(width), static_cast<long>(height)};
            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);

            static constexpr std::array<float, 4> defaultBlendFactor = {0, 0, 0, 0};
            commandList->OMSetBlendFactor(&defaultBlendFactor[0]);
        }

        RenderPipeline* lastPipeline = nullptr;
        PipelineLayout* lastLayout = nullptr;
        InputState* lastInputState = nullptr;
        VertexBuffersInfo vertexBuffersInfo = {};

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    return;
                } break;

                case Command::Draw: {
                    DrawCmd* draw = mCommands.NextCommand<DrawCmd>();

                    FlushSetVertexBuffers(commandList, &vertexBuffersInfo, lastInputState);
                    commandList->DrawInstanced(draw->vertexCount, draw->instanceCount,
                                               draw->firstVertex, draw->firstInstance);
                } break;

                case Command::DrawIndexed: {
                    DrawIndexedCmd* draw = mCommands.NextCommand<DrawIndexedCmd>();

                    FlushSetVertexBuffers(commandList, &vertexBuffersInfo, lastInputState);
                    commandList->DrawIndexedInstanced(draw->indexCount, draw->instanceCount,
                                                      draw->firstIndex, draw->baseVertex,
                                                      draw->firstInstance);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();
                    PipelineLayout* layout = ToBackend(pipeline->GetLayout());
                    InputState* inputState = ToBackend(pipeline->GetInputState());

                    commandList->SetGraphicsRootSignature(layout->GetRootSignature().Get());
                    commandList->SetPipelineState(pipeline->GetPipelineState().Get());
                    commandList->IASetPrimitiveTopology(pipeline->GetD3D12PrimitiveTopology());

                    bindingTracker->SetInheritedBindGroups(commandList, lastLayout, layout);

                    lastPipeline = pipeline;
                    lastLayout = layout;
                    lastInputState = inputState;
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();

                    commandList->OMSetStencilRef(cmd->reference);
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    D3D12_RECT rect;
                    rect.left = cmd->x;
                    rect.top = cmd->y;
                    rect.right = cmd->x + cmd->width;
                    rect.bottom = cmd->y + cmd->height;

                    commandList->RSSetScissorRects(1, &rect);
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    commandList->OMSetBlendFactor(static_cast<const FLOAT*>(&cmd->color.r));
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    BindGroup* group = ToBackend(cmd->group.Get());
                    bindingTracker->SetBindGroup(commandList, lastLayout, group, cmd->index);
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mCommands.NextCommand<SetIndexBufferCmd>();

                    Buffer* buffer = ToBackend(cmd->buffer.Get());
                    D3D12_INDEX_BUFFER_VIEW bufferView;
                    bufferView.BufferLocation = buffer->GetVA() + cmd->offset;
                    bufferView.SizeInBytes = buffer->GetSize() - cmd->offset;
                    // TODO(cwallez@chromium.org): Make index buffers lazily applied, right now
                    // this will break if the pipeline is changed for one with a different index
                    // format after SetIndexBuffer
                    bufferView.Format = DXGIIndexFormat(lastPipeline->GetIndexFormat());

                    commandList->IASetIndexBuffer(&bufferView);
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mCommands.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mCommands.NextData<Ref<BufferBase>>(cmd->count);
                    auto offsets = mCommands.NextData<uint32_t>(cmd->count);

                    vertexBuffersInfo.startSlot =
                        std::min(vertexBuffersInfo.startSlot, cmd->startSlot);
                    vertexBuffersInfo.endSlot =
                        std::max(vertexBuffersInfo.endSlot, cmd->startSlot + cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        Buffer* buffer = ToBackend(buffers[i].Get());
                        auto* d3d12BufferView =
                            &vertexBuffersInfo.d3d12BufferViews[cmd->startSlot + i];
                        d3d12BufferView->BufferLocation = buffer->GetVA() + offsets[i];
                        d3d12BufferView->SizeInBytes = buffer->GetSize() - offsets[i];
                        // The bufferView stride is set based on the input state before a draw.
                    }
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

}}  // namespace dawn_native::d3d12
