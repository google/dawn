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
#include "dawn_native/Commands.h"
#include "dawn_native/d3d12/BindGroupD3D12.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/ComputePipelineD3D12.h"
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/InputStateD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/RenderPassDescriptorD3D12.h"
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
                const uint64_t serial = device->GetSerial();
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

    namespace {

        void AllocateAndSetDescriptorHeaps(Device* device,
                                           BindGroupStateTracker* bindingTracker,
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
                        default:
                            SkipCommand(commands, type);
                    }
                }

                commands->Reset();
            }

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

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::RecordCommands(ComPtr<ID3D12GraphicsCommandList> commandList,
                                       uint32_t indexInSubmit) {
        Device* device = ToBackend(GetDevice());
        BindGroupStateTracker bindingTracker(device);

        // Precompute the allocation of bindgroups in descriptor heaps
        // TODO(cwallez@chromium.org): Iterating over all the commands here is inefficient. We
        // should have a system where commands and descriptors are recorded in parallel then the
        // heaps set using a small CommandList inserted just before the main CommandList.
        {
            AllocateAndSetDescriptorHeaps(device, &bindingTracker, &mCommands, indexInSubmit);
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
                    RecordRenderPass(commandList, &bindingTracker,
                                     ToBackend(beginRenderPassCmd->info.Get()));

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
                        copy->destination.x, copy->destination.y, copy->destination.z,
                        copy->destination.width, copy->destination.height, copy->destination.depth,
                        static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                        copy->source.offset, copy->rowPitch);

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
                        bufferLocation.PlacedFootprint.Footprint.RowPitch = copy->rowPitch;

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
                        copy->source.x, copy->source.y, copy->source.z, copy->source.width,
                        copy->source.height, copy->source.depth,
                        static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                        copy->destination.offset, copy->rowPitch);

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
                        bufferLocation.PlacedFootprint.Footprint.RowPitch = copy->rowPitch;

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
                                         RenderPassDescriptor* renderPass) {
        // Clear framebuffer attachments as needed and transition to render target
        {
            for (uint32_t i : IterateBitSet(renderPass->GetColorAttachmentMask())) {
                auto& attachmentInfo = renderPass->GetColorAttachment(i);

                // Load op - color
                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = renderPass->GetRTVDescriptor(i);
                    commandList->ClearRenderTargetView(handle, attachmentInfo.clearColor.data(), 0,
                                                       nullptr);
                }
            }

            if (renderPass->HasDepthStencilAttachment()) {
                auto& attachmentInfo = renderPass->GetDepthStencilAttachment();
                Texture* texture = ToBackend(attachmentInfo.view->GetTexture());

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
                    auto handle = renderPass->GetDSVDescriptor();
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
            RenderPassDescriptor::OMSetRenderTargetArgs args =
                renderPass->GetSubpassOMSetRenderTargetArgs();
            if (args.dsv.ptr) {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, &args.dsv);
            } else {
                commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, nullptr);
            }
        }

        // Set up default dynamic state
        {
            uint32_t width = renderPass->GetWidth();
            uint32_t height = renderPass->GetHeight();
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

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    return;
                } break;

                case Command::DrawArrays: {
                    DrawArraysCmd* draw = mCommands.NextCommand<DrawArraysCmd>();
                    commandList->DrawInstanced(draw->vertexCount, draw->instanceCount,
                                               draw->firstVertex, draw->firstInstance);
                } break;

                case Command::DrawElements: {
                    DrawElementsCmd* draw = mCommands.NextCommand<DrawElementsCmd>();
                    commandList->DrawIndexedInstanced(draw->indexCount, draw->instanceCount,
                                                      draw->firstIndex, 0, draw->firstInstance);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();
                    PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                    commandList->SetGraphicsRootSignature(layout->GetRootSignature().Get());
                    commandList->SetPipelineState(pipeline->GetPipelineState().Get());
                    commandList->IASetPrimitiveTopology(pipeline->GetD3D12PrimitiveTopology());

                    bindingTracker->SetInheritedBindGroups(commandList, lastLayout, layout);

                    lastPipeline = pipeline;
                    lastLayout = layout;
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
                    commandList->OMSetBlendFactor(static_cast<const FLOAT*>(&cmd->r));
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

                    auto inputState = ToBackend(lastPipeline->GetInputState());

                    std::array<D3D12_VERTEX_BUFFER_VIEW, kMaxVertexInputs> d3d12BufferViews;
                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        auto input = inputState->GetInput(cmd->startSlot + i);
                        Buffer* buffer = ToBackend(buffers[i].Get());
                        d3d12BufferViews[i].BufferLocation = buffer->GetVA() + offsets[i];
                        d3d12BufferViews[i].StrideInBytes = input.stride;
                        d3d12BufferViews[i].SizeInBytes = buffer->GetSize() - offsets[i];
                    }

                    commandList->IASetVertexBuffers(cmd->startSlot, cmd->count,
                                                    d3d12BufferViews.data());
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

}}  // namespace dawn_native::d3d12
