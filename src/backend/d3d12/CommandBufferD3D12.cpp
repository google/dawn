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
#include "backend/d3d12/DescriptorHeapAllocator.h"
#include "backend/d3d12/InputStateD3D12.h"
#include "backend/d3d12/PipelineD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "backend/d3d12/ResourceAllocator.h"
#include "backend/d3d12/SamplerD3D12.h"
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
            DescriptorHeapHandle cbvSrvUavCPUDescriptorHeap;
            DescriptorHeapHandle samplerCPUDescriptorHeap;
            DescriptorHeapHandle cbvSrvUavGPUDescriptorHeap;
            DescriptorHeapHandle samplerGPUDescriptorHeap;
            std::array<BindGroup*, kMaxBindGroups> bindGroups = {};

            Device* device;

            BindGroupStateTracker(Device* device) : device(device) {
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

            void TrackSetBindInheritedGroup(uint32_t index) {
                BindGroup* group = bindGroups[index];
                if (group != nullptr) {
                    TrackSetBindGroup(group, index);
                }
            }

            void SetBindGroup(ComPtr<ID3D12GraphicsCommandList> commandList, Pipeline* pipeline, BindGroup* group, uint32_t index, bool force = false) {
                if (bindGroups[index] != group || force) {
                    bindGroups[index] = group;

                    PipelineLayout* pipelineLayout = ToBackend(pipeline->GetLayout());
                    uint32_t cbvUavSrvCount = ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount();
                    uint32_t samplerCount = ToBackend(group->GetLayout())->GetSamplerDescriptorCount();

                    if (cbvUavSrvCount > 0) {
                        uint32_t parameterIndex = pipelineLayout->GetCbvUavSrvRootParameterIndex(index);

                        if (pipeline->IsCompute()) {
                            commandList->SetComputeRootDescriptorTable(parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                        } else {
                            commandList->SetGraphicsRootDescriptorTable(parameterIndex, cbvSrvUavGPUDescriptorHeap.GetGPUHandle(group->GetCbvUavSrvHeapOffset()));
                        }
                    }

                    if (samplerCount > 0) {
                        uint32_t parameterIndex = pipelineLayout->GetSamplerRootParameterIndex(index);

                        if (pipeline->IsCompute()) {
                            commandList->SetComputeRootDescriptorTable(parameterIndex, samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                        } else {
                            commandList->SetGraphicsRootDescriptorTable(parameterIndex, samplerGPUDescriptorHeap.GetGPUHandle(group->GetSamplerHeapOffset()));
                        }
                    }
                }
            }

            void SetInheritedBindGroup(ComPtr<ID3D12GraphicsCommandList> commandList, Pipeline* pipeline, uint32_t index) {
                BindGroup* group = bindGroups[index];
                if (group != nullptr) {
                    SetBindGroup(commandList, pipeline, group, index, true);
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
                Pipeline* lastPipeline = nullptr;
                PipelineLayout* lastLayout = nullptr;

                while (commands->NextCommandId(&type)) {
                    switch (type) {
                        case Command::SetPipeline:
                        {
                            SetPipelineCmd* cmd = commands->NextCommand<SetPipelineCmd>();
                            Pipeline* pipeline = ToBackend(cmd->pipeline).Get();
                            PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                            if (lastLayout) {
                                auto mask = layout->GetBindGroupsLayoutMask();
                                for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                                    // matching bind groups are inherited until they differ
                                    if (mask[i] && lastLayout->GetBindGroupLayout(i) == layout->GetBindGroupLayout(i)) {
                                        bindingTracker->TrackSetBindInheritedGroup(i);
                                    } else {
                                        break;
                                    }
                                }
                            }

                            lastPipeline = pipeline;
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
    
        D3D12_TEXTURE_COPY_LOCATION D3D12PlacedTextureCopyLocation(BufferCopyLocation& bufferLocation, Texture* texture, const TextureCopyLocation& textureLocation) {
            D3D12_TEXTURE_COPY_LOCATION d3d12Location;
            d3d12Location.pResource = ToBackend(bufferLocation.buffer.Get())->GetD3D12Resource().Get();
            d3d12Location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            d3d12Location.PlacedFootprint.Offset = bufferLocation.offset;
            d3d12Location.PlacedFootprint.Footprint.Format = texture->GetD3D12Format();
            d3d12Location.PlacedFootprint.Footprint.Width = textureLocation.width;
            d3d12Location.PlacedFootprint.Footprint.Height = textureLocation.height;
            d3d12Location.PlacedFootprint.Footprint.Depth = textureLocation.depth;

            uint32_t texelSize = 0;
            switch (texture->GetFormat()) {
				case nxt::TextureFormat::R8G8B8A8Unorm:
					texelSize = 4;
					break;
            }
            uint32_t rowSize = textureLocation.width * texelSize;
            d3d12Location.PlacedFootprint.Footprint.RowPitch = ((rowSize - 1) / D3D12_TEXTURE_DATA_PITCH_ALIGNMENT + 1) * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

            return d3d12Location;
        }

        D3D12_TEXTURE_COPY_LOCATION D3D12TextureCopyLocation(TextureCopyLocation& textureLocation) {
            D3D12_TEXTURE_COPY_LOCATION d3d12Location;
            d3d12Location.pResource = ToBackend(textureLocation.texture.Get())->GetD3D12Resource().Get();
            d3d12Location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            d3d12Location.SubresourceIndex = textureLocation.level;
            return d3d12Location;
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
        Pipeline* lastPipeline = nullptr;
        PipelineLayout* lastLayout = nullptr;

        RenderPass* currentRenderPass = nullptr;
        Framebuffer* currentFramebuffer = nullptr;

        while(commands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass:
                    {
                        commands.NextCommand<BeginComputePassCmd>();
                    }
                    break;

                case Command::BeginRenderPass:
                    {
                        BeginRenderPassCmd* beginRenderPassCmd = commands.NextCommand<BeginRenderPassCmd>();
                        currentRenderPass = ToBackend(beginRenderPassCmd->renderPass.Get());
                        currentFramebuffer = ToBackend(beginRenderPassCmd->framebuffer.Get());

                        uint32_t width = currentFramebuffer->GetWidth();
                        uint32_t height = currentFramebuffer->GetHeight();
                        D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f };
                        D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
                        commandList->RSSetViewports(1, &viewport);
                        commandList->RSSetScissorRects(1, &scissorRect);
                        D3D12_CPU_DESCRIPTOR_HANDLE rtv = device->GetCurrentRenderTargetDescriptor();
                        commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
                    }
                    break;

                case Command::BeginRenderSubpass:
                    {
                        commands.NextCommand<BeginRenderSubpassCmd>();
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

                        D3D12_TEXTURE_COPY_LOCATION srcLocation = D3D12PlacedTextureCopyLocation(copy->source, texture, copy->destination);
                        D3D12_TEXTURE_COPY_LOCATION dstLocation = D3D12TextureCopyLocation(copy->destination);

                        // TODO(enga@google.com): This assertion will not be true if the number of bytes in each row of the texture is not a multiple of 256
                        // To resolve this we would need to create an intermediate resource or force all textures to be 256-byte aligned
                        uint64_t totalBytes = srcLocation.PlacedFootprint.Footprint.RowPitch * copy->destination.height * copy->destination.depth;
                        ASSERT(totalBytes <= buffer->GetD3D12Size());

                        commandList->CopyTextureRegion(&dstLocation, copy->destination.x, copy->destination.y, copy->destination.z, &srcLocation, nullptr);
                    }
                    break;

                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = commands.NextCommand<CopyTextureToBufferCmd>();
                        Texture* texture = ToBackend(copy->source.texture.Get());
                        Buffer* buffer = ToBackend(copy->destination.buffer.Get());

                        D3D12_TEXTURE_COPY_LOCATION srcLocation = D3D12TextureCopyLocation(copy->source);
                        D3D12_TEXTURE_COPY_LOCATION dstLocation = D3D12PlacedTextureCopyLocation(copy->destination, texture, copy->source);

                        // TODO(enga@google.com): This assertion will not be true if the number of bytes in each row of the texture is not a multiple of 256
                        // To resolve this we would need to create an intermediate resource or force all textures to be 256-byte aligned
                        uint64_t totalBytes = dstLocation.PlacedFootprint.Footprint.RowPitch * copy->source.height * copy->source.depth;
                        ASSERT(totalBytes <= buffer->GetD3D12Size());

                        D3D12_BOX sourceRegion;
                        sourceRegion.left = copy->source.x;
                        sourceRegion.top = copy->source.y;
                        sourceRegion.front = copy->source.z;
                        sourceRegion.right = copy->source.x + copy->source.width;
                        sourceRegion.bottom = copy->source.y + copy->source.height;
                        sourceRegion.back = copy->source.z + copy->source.depth;
                        commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &sourceRegion);
                    }
                    break;

                case Command::Dispatch:
                    {
                        DispatchCmd* dispatch = commands.NextCommand<DispatchCmd>();

                        ASSERT(lastPipeline->IsCompute());
                        commandList->Dispatch(dispatch->x, dispatch->y, dispatch->z);
                    }
                    break;

                case Command::DrawArrays:
                    {
                        DrawArraysCmd* draw = commands.NextCommand<DrawArraysCmd>();

                        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

                        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
                    }
                    break;

                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = commands.NextCommand<SetPipelineCmd>();

                        Pipeline* pipeline = ToBackend(cmd->pipeline).Get();
                        PipelineLayout* layout = ToBackend(pipeline->GetLayout());

                        // TODO
                        if (pipeline->IsCompute()) {
                        }
                        else {
                            commandList->SetGraphicsRootSignature(layout->GetRootSignature().Get());
                            commandList->SetPipelineState(pipeline->GetRenderPipelineState().Get());
                        }

                        if (lastLayout) {
                            auto mask = layout->GetBindGroupsLayoutMask();
                            for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
                                // matching bind groups are inherited until they differ
                                if (mask[i] && lastLayout->GetBindGroupLayout(i) == layout->GetBindGroupLayout(i)) {
                                    bindingTracker.SetInheritedBindGroup(commandList, pipeline, i);
                                } else {
                                    break;
                                }
                            }
                        }


                        lastPipeline = pipeline;
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
                        commands.NextCommand<SetStencilReferenceCmd>();
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = commands.NextCommand<SetBindGroupCmd>();
                        BindGroup* group = ToBackend(cmd->group.Get());
                        bindingTracker.SetBindGroup(commandList, lastPipeline, group, cmd->index);
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

                        auto inputState = ToBackend(lastPipeline->GetInputState());

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
