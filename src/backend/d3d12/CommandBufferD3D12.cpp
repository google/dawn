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

#include "backend/d3d12/CommandBufferD3D12.h"

#include "backend/Commands.h"
#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/BindGroupD3D12.h"
#include "backend/d3d12/BindGroupLayoutD3D12.h"
#include "backend/d3d12/BufferD3D12.h"
#include "backend/d3d12/ComputePipelineD3D12.h"
#include "backend/d3d12/DescriptorHeapAllocator.h"
#include "backend/d3d12/FramebufferD3D12.h"
#include "backend/d3d12/InputStateD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "backend/d3d12/RenderPipelineD3D12.h"
#include "backend/d3d12/ResourceAllocator.h"
#include "backend/d3d12/SamplerD3D12.h"
#include "backend/d3d12/TextureCopySplitter.h"
#include "backend/d3d12/TextureD3D12.h"
#include "common/Assert.h"

namespace backend {
namespace d3d12 {

    namespace {
        DXGI_FORMAT DXGIIndexFormat(nxt::IndexFormat format) {
            switch (format) {
                case nxt::IndexFormat::Uint16:
                    return DXGI_FORMAT_R16_UINT;
                case nxt::IndexFormat::Uint32:
                    return DXGI_FORMAT_R32_UINT;
                default:
                    UNREACHABLE();
            }
        }

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

            void SetInComputePass(bool inCompute) {
                this->inCompute = inCompute;
            }

            void TrackSetBindGroup(BindGroup* group, uint32_t index) {
                if (bindGroups[index] != group) {
                    bindGroups[index] = group;

                    // Descriptors don't need to be recorded if they have already been recorded in the heap. Indices are only updated when descriptors are recorded
                    const uint64_t serial = device->GetSerial();
                    if (group->GetHeapSerial() != serial) {
                        group->RecordDescriptors(cbvSrvUavCPUDescriptorHeap, &cbvSrvUavDescriptorIndex, samplerCPUDescriptorHeap, &samplerDescriptorIndex, serial);
                    }
                }
            }

            void TrackInheritedGroups(PipelineLayout* oldLayout, PipelineLayout* newLayout) {
                if (oldLayout == nullptr) {
                    return;
                }

                uint32_t inheritUntil = oldLayout->GroupsInheritUpTo(newLayout);
                for (uint32_t i = 0; i < inheritUntil; ++i) {
                    TrackSetBindGroup(bindGroups[i], i);
                }
            }

            void SetBindGroup(ComPtr<ID3D12GraphicsCommandList> commandList, PipelineLayout* pipelineLayout, BindGroup* group,
                              uint32_t index, bool force = false) {
                if (bindGroups[index] != group || force) {
                    bindGroups[index] = group;

                    uint32_t cbvUavSrvCount = ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount();
                    uint32_t samplerCount = ToBackend(group->GetLayout())->GetSamplerDescriptorCount();

                    if (cbvUavSrvCount > 0) {
                        uint32_t parameterIndex = pipelineLayout->GetCbvUavSrvRootParameterIndex(index);

                        if (inCompute) {
                            commandList->SetComputeRootDescriptorTable(parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                        } else {
                            commandList->SetGraphicsRootDescriptorTable(parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                        }
                    }

                    if (samplerCount > 0) {
                        uint32_t parameterIndex = pipelineLayout->GetSamplerRootParameterIndex(index);

                        if (inCompute) {
                            commandList->SetComputeRootDescriptorTable(parameterIndex, samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                        } else {
                            commandList->SetGraphicsRootDescriptorTable(parameterIndex, samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                        }
                    }
                }
            }

            void SetInheritedBindGroups(ComPtr<ID3D12GraphicsCommandList> commandList, PipelineLayout* oldLayout, PipelineLayout* newLayout) {
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

        void AllocateAndSetDescriptorHeaps(Device* device, BindGroupStateTracker* bindingTracker, CommandIterator* commands) {
            auto* descriptorHeapAllocator = device->GetDescriptorHeapAllocator();

            // TODO(enga@google.com): This currently allocates CPU heaps of arbitrarily chosen sizes
            // This will not work if there are too many descriptors
            bindingTracker->cbvSrvUavCPUDescriptorHeap = descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192);
            bindingTracker->samplerCPUDescriptorHeap = descriptorHeapAllocator->AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

            {
                Command type;
                PipelineLayout* lastLayout = nullptr;

                while (commands->NextCommandId(&type)) {
                    switch (type) {
                        case Command::SetComputePipeline:
                        {
                            SetComputePipelineCmd* cmd = commands->NextCommand<SetComputePipelineCmd>();
                            PipelineLayout* layout = ToBackend(cmd->pipeline->GetLayout());
                            bindingTracker->TrackInheritedGroups(lastLayout, layout);
                            lastLayout = layout;
                        }
                        break;

                    case Command::SetRenderPipeline:
                        {
                            SetRenderPipelineCmd* cmd = commands->NextCommand<SetRenderPipelineCmd>();
                            PipelineLayout* layout = ToBackend(cmd->pipeline->GetLayout());
                            bindingTracker->TrackInheritedGroups(lastLayout, layout);
                            lastLayout = layout;
                        }
                        break;

                        case Command::SetBindGroup:
                        {
                            SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                            BindGroup* group = ToBackend(cmd->group.Get());
                            bindingTracker->TrackSetBindGroup(group, cmd->index);
                        }
                        break;
                        default:
                            SkipCommand(commands, type);
                    }
                }

                commands->Reset();
            }

            if (bindingTracker->cbvSrvUavDescriptorIndex > 0) {
                // Allocate a GPU-visible heap and copy from the CPU-only heap to the GPU-visible heap
                bindingTracker->cbvSrvUavGPUDescriptorHeap = descriptorHeapAllocator->AllocateGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, bindingTracker->cbvSrvUavDescriptorIndex);
                device->GetD3D12Device()->CopyDescriptorsSimple(
                    bindingTracker->cbvSrvUavDescriptorIndex,
                    bindingTracker->cbvSrvUavGPUDescriptorHeap.GetCPUHandle(0),
                    bindingTracker->cbvSrvUavCPUDescriptorHeap.GetCPUHandle(0),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            if (bindingTracker->samplerDescriptorIndex > 0) {
                bindingTracker->samplerGPUDescriptorHeap = descriptorHeapAllocator->AllocateGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, bindingTracker->samplerDescriptorIndex);
                device->GetD3D12Device()->CopyDescriptorsSimple(
                    bindingTracker->samplerDescriptorIndex,
                    bindingTracker->samplerGPUDescriptorHeap.GetCPUHandle(0),
                    bindingTracker->samplerCPUDescriptorHeap.GetCPUHandle(0),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            }
        }
    }

    CommandBuffer::CommandBuffer(Device* device, CommandBufferBuilder* builder)
        : CommandBufferBase(builder), device(device), commands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&commands);
    }

    void CommandBuffer::FillCommands(ComPtr<ID3D12GraphicsCommandList> commandList) {
        BindGroupStateTracker bindingTracker(device);
        AllocateAndSetDescriptorHeaps(device, &bindingTracker, &commands);
        bindingTracker.Reset();

        ID3D12DescriptorHeap* descriptorHeaps[2] = { bindingTracker.cbvSrvUavGPUDescriptorHeap.Get(), bindingTracker.samplerGPUDescriptorHeap.Get() };
        if (descriptorHeaps[0] && descriptorHeaps[1]) {
            commandList->SetDescriptorHeaps(2, descriptorHeaps);
        } else if (descriptorHeaps[0]) {
            commandList->SetDescriptorHeaps(1, descriptorHeaps);
        } else if (descriptorHeaps[1]) {
            commandList->SetDescriptorHeaps(2, &descriptorHeaps[1]);
        }

        Command type;
        RenderPipeline* lastRenderPipeline = nullptr;
        PipelineLayout* lastLayout = nullptr;

        RenderPass* currentRenderPass = nullptr;
        Framebuffer* currentFramebuffer = nullptr;
        uint32_t currentSubpass = 0;

        while(commands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass:
                    {
                        commands.NextCommand<BeginComputePassCmd>();
                        bindingTracker.SetInComputePass(true);
                    }
                    break;

                case Command::BeginRenderPass:
                    {
                        BeginRenderPassCmd* beginRenderPassCmd = commands.NextCommand<BeginRenderPassCmd>();
                        currentRenderPass = ToBackend(beginRenderPassCmd->renderPass.Get());
                        currentFramebuffer = ToBackend(beginRenderPassCmd->framebuffer.Get());
                        currentSubpass = 0;

                        uint32_t width = currentFramebuffer->GetWidth();
                        uint32_t height = currentFramebuffer->GetHeight();
                        D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f };
                        D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
                        commandList->RSSetViewports(1, &viewport);
                        commandList->RSSetScissorRects(1, &scissorRect);
                    }
                    break;

                case Command::BeginRenderSubpass:
                    {
                        commands.NextCommand<BeginRenderSubpassCmd>();
                        const auto& subpass = currentRenderPass->GetSubpassInfo(currentSubpass);

                        Framebuffer::OMSetRenderTargetArgs args = currentFramebuffer->GetSubpassOMSetRenderTargetArgs(currentSubpass);
                        if (args.dsv.ptr) {
                            commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, &args.dsv);
                        } else {
                            commandList->OMSetRenderTargets(args.numRTVs, args.RTVs.data(), FALSE, nullptr);
                        }

                        // Clear framebuffer attachments as needed

                        for (unsigned int location : IterateBitSet(subpass.colorAttachmentsSet)) {
                            uint32_t attachmentSlot = subpass.colorAttachments[location];
                            const auto& attachmentInfo = currentRenderPass->GetAttachmentInfo(attachmentSlot);

                            Texture* texture = ToBackend(currentFramebuffer->GetTextureView(attachmentSlot)->GetTexture());
                            constexpr auto usage = nxt::TextureUsageBit::OutputAttachment;
                            // It's already validated that this texture is either frozen to the correct usage, or not frozen.
                            if (!texture->IsFrozen()) {
                                texture->TransitionUsageImpl(texture->GetUsage(), usage);
                                texture->UpdateUsageInternal(usage);
                            }

                            // Only perform load op on first use
                            if (attachmentInfo.firstSubpass == currentSubpass) {
                                // Load op - color
                                if (attachmentInfo.colorLoadOp == nxt::LoadOp::Clear) {
                                    auto handle = currentFramebuffer->GetRTVDescriptor(attachmentSlot);
                                    const auto& clear = currentFramebuffer->GetClearColor(attachmentSlot);
                                    commandList->ClearRenderTargetView(handle, clear.color, 0, nullptr);
                                }
                            }
                        }

                        if (subpass.depthStencilAttachmentSet) {
                            uint32_t attachmentSlot = subpass.depthStencilAttachment;
                            const auto& attachmentInfo = currentRenderPass->GetAttachmentInfo(attachmentSlot);

                            // Only perform load op on first use
                            if (attachmentInfo.firstSubpass == currentSubpass) {
                                // Load op - depth/stencil
                                bool doDepthClear = TextureFormatHasDepth(attachmentInfo.format) &&
                                    (attachmentInfo.depthLoadOp == nxt::LoadOp::Clear);
                                bool doStencilClear = TextureFormatHasStencil(attachmentInfo.format) &&
                                    (attachmentInfo.stencilLoadOp == nxt::LoadOp::Clear);

                                D3D12_CLEAR_FLAGS clearFlags = {};
                                if (doDepthClear) {
                                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                                }
                                if (doStencilClear) {
                                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                                }
                                if (clearFlags) {
                                    auto handle = currentFramebuffer->GetDSVDescriptor(attachmentSlot);
                                    const auto& clear = currentFramebuffer->GetClearDepthStencil(attachmentSlot);
                                    // TODO(kainino@chromium.org): investigate: should the NXT clear stencil type be uint8_t?
                                    uint8_t clearStencil = static_cast<uint8_t>(clear.stencil);
                                    commandList->ClearDepthStencilView(handle, clearFlags, clear.depth, clearStencil, 0, nullptr);
                                }
                            }
                        }

                        static constexpr std::array<float, 4> defaultBlendFactor = { 0, 0, 0, 0 };
                        commandList->OMSetBlendFactor(&defaultBlendFactor[0]);
                    }
                    break;

                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = commands.NextCommand<CopyBufferToBufferCmd>();
                        auto src = ToBackend(copy->source.buffer.Get())->GetD3D12Resource();
                        auto dst = ToBackend(copy->destination.buffer.Get())->GetD3D12Resource();
                        commandList->CopyBufferRegion(dst.Get(), copy->destination.offset, src.Get(), copy->source.offset, copy->size);
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands.NextCommand<CopyBufferToTextureCmd>();
                        Buffer* buffer = ToBackend(copy->source.buffer.Get());
                        Texture* texture = ToBackend(copy->destination.texture.Get());

                        auto copySplit = ComputeTextureCopySplit(
                            copy->destination.x,
                            copy->destination.y,
                            copy->destination.z,
                            copy->destination.width,
                            copy->destination.height,
                            copy->destination.depth,
                            static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                            copy->source.offset,
                            copy->rowPitch
                        );

                        D3D12_TEXTURE_COPY_LOCATION textureLocation;
                        textureLocation.pResource = texture->GetD3D12Resource();
                        textureLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                        textureLocation.SubresourceIndex = copy->destination.level;

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

                            commandList->CopyTextureRegion(&textureLocation, info.textureOffset.x, info.textureOffset.y, info.textureOffset.z, &bufferLocation, &sourceRegion);
                        }
                    }
                    break;

                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = commands.NextCommand<CopyTextureToBufferCmd>();
                        Texture* texture = ToBackend(copy->source.texture.Get());
                        Buffer* buffer = ToBackend(copy->destination.buffer.Get());

                        auto copySplit = ComputeTextureCopySplit(
                            copy->source.x,
                            copy->source.y,
                            copy->source.z,
                            copy->source.width,
                            copy->source.height,
                            copy->source.depth,
                            static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat())),
                            copy->destination.offset,
                            copy->rowPitch
                        );

                        D3D12_TEXTURE_COPY_LOCATION textureLocation;
                        textureLocation.pResource = texture->GetD3D12Resource();
                        textureLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                        textureLocation.SubresourceIndex = copy->source.level;

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

                            commandList->CopyTextureRegion(&bufferLocation, info.bufferOffset.x, info.bufferOffset.y, info.bufferOffset.z, &textureLocation, &sourceRegion);
                        }
                    }
                    break;

                case Command::Dispatch:
                    {
                        DispatchCmd* dispatch = commands.NextCommand<DispatchCmd>();
                        commandList->Dispatch(dispatch->x, dispatch->y, dispatch->z);
                    }
                    break;

                case Command::DrawArrays:
                    {
                        DrawArraysCmd* draw = commands.NextCommand<DrawArraysCmd>();
                        commandList->DrawInstanced(
                            draw->vertexCount,
                            draw->instanceCount,
                            draw->firstVertex,
                            draw->firstInstance
                        );
                    }
                    break;

                case Command::DrawElements:
                    {
                        DrawElementsCmd* draw = commands.NextCommand<DrawElementsCmd>();

                        commandList->DrawIndexedInstanced(
                            draw->indexCount,
                            draw->instanceCount,
                            draw->firstIndex,
                            0,
                            draw->firstInstance
                        );
                    }
                    break;

                case Command::EndComputePass:
                    {
                        commands.NextCommand<EndComputePassCmd>();
                        bindingTracker.SetInComputePass(false);
                    }
                    break;

                case Command::EndRenderPass:
                    {
                        commands.NextCommand<EndRenderPassCmd>();
                    }
                    break;

                case Command::EndRenderSubpass:
                    {
                        commands.NextCommand<EndRenderSubpassCmd>();
                        currentSubpass += 1;
                    }
                    break;

                case Command::SetComputePipeline:
                    {
                        SetComputePipelineCmd* cmd = commands.NextCommand<SetComputePipelineCmd>();
                        ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();
                        PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                        commandList->SetComputeRootSignature(layout->GetRootSignature().Get());
                        commandList->SetPipelineState(pipeline->GetPipelineState().Get());

                        // TODO(enga@google.com): Implement compute pipelines
                        bindingTracker.SetInheritedBindGroups(commandList, lastLayout, layout);
                        lastLayout = layout;
                    }
                    break;

                case Command::SetRenderPipeline:
                    {
                        SetRenderPipelineCmd* cmd = commands.NextCommand<SetRenderPipelineCmd>();
                        RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();
                        PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                        commandList->SetGraphicsRootSignature(layout->GetRootSignature().Get());
                        commandList->SetPipelineState(pipeline->GetPipelineState().Get());
                        commandList->IASetPrimitiveTopology(pipeline->GetD3D12PrimitiveTopology());

                        bindingTracker.SetInheritedBindGroups(commandList, lastLayout, layout);

                        lastRenderPipeline = pipeline;
                        lastLayout = layout;
                    }
                    break;

                case Command::SetPushConstants:
                    {
                        commands.NextCommand<SetPushConstantsCmd>();
                    }
                    break;

                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd *cmd = commands.NextCommand<SetStencilReferenceCmd>();

                        commandList->OMSetStencilRef(cmd->reference);
                    }
                    break;

                case Command::SetBlendColor:
                    {
                        SetBlendColorCmd* cmd = commands.NextCommand<SetBlendColorCmd>();
                        ASSERT(lastRenderPipeline);
                        commandList->OMSetBlendFactor(static_cast<const FLOAT*>(&cmd->r));
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = commands.NextCommand<SetBindGroupCmd>();
                        BindGroup* group = ToBackend(cmd->group.Get());
                        bindingTracker.SetBindGroup(commandList, lastLayout, group, cmd->index);
                    }
                    break;

                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = commands.NextCommand<SetIndexBufferCmd>();

                        Buffer* buffer = ToBackend(cmd->buffer.Get());
                        D3D12_INDEX_BUFFER_VIEW bufferView;
                        bufferView.BufferLocation = buffer->GetVA() + cmd->offset;
                        bufferView.SizeInBytes = buffer->GetSize() - cmd->offset;
                        bufferView.Format = DXGIIndexFormat(cmd->format);

                        commandList->IASetIndexBuffer(&bufferView);
                    }
                    break;

                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = commands.NextCommand<SetVertexBuffersCmd>();
                        auto buffers = commands.NextData<Ref<BufferBase>>(cmd->count);
                        auto offsets = commands.NextData<uint32_t>(cmd->count);

                        auto inputState = ToBackend(lastRenderPipeline->GetInputState());

                        std::array<D3D12_VERTEX_BUFFER_VIEW, kMaxVertexInputs> d3d12BufferViews;
                        for (uint32_t i = 0; i < cmd->count; ++i) {
                            auto input = inputState->GetInput(cmd->startSlot + i);
                            Buffer* buffer = ToBackend(buffers[i].Get());
                            d3d12BufferViews[i].BufferLocation = buffer->GetVA() + offsets[i];
                            d3d12BufferViews[i].StrideInBytes = input.stride;
                            d3d12BufferViews[i].SizeInBytes = buffer->GetSize() - offsets[i];
                        }

                        commandList->IASetVertexBuffers(cmd->startSlot, cmd->count, d3d12BufferViews.data());
                    }
                    break;

                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = commands.NextCommand<TransitionBufferUsageCmd>();

                        Buffer* buffer = ToBackend(cmd->buffer.Get());

                        D3D12_RESOURCE_BARRIER barrier;
                        if (buffer->GetResourceTransitionBarrier(buffer->GetUsage(), cmd->usage, &barrier)) {
                            commandList->ResourceBarrier(1, &barrier);
                        }

                        buffer->UpdateUsageInternal(cmd->usage);
                    }
                    break;

                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = commands.NextCommand<TransitionTextureUsageCmd>();

                        Texture* texture = ToBackend(cmd->texture.Get());

                        D3D12_RESOURCE_BARRIER barrier;
                        if (texture->GetResourceTransitionBarrier(texture->GetUsage(), cmd->usage, &barrier)) {
                            commandList->ResourceBarrier(1, &barrier);
                        }

                        texture->UpdateUsageInternal(cmd->usage);
                    }
                    break;
            }
        }
    }
}
}
