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

#include "CommandBufferD3D12.h"

#include "common/Commands.h"
#include "D3D12Backend.h"
#include "BindGroupD3D12.h"
#include "BindGroupLayoutD3D12.h"
#include "BufferD3D12.h"
#include "DescriptorHeapAllocator.h"
#include "InputStateD3D12.h"
#include "PipelineD3D12.h"
#include "PipelineLayoutD3D12.h"
#include "SamplerD3D12.h"
#include "TextureD3D12.h"

namespace backend {
namespace d3d12 {

    namespace {
        DXGI_FORMAT DXGIIndexFormat(nxt::IndexFormat format) {
            switch (format) {
                case nxt::IndexFormat::Uint16:
                    return DXGI_FORMAT_R16_UINT;
                case nxt::IndexFormat::Uint32:
                    return DXGI_FORMAT_R32_UINT;
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
                case Command::AdvanceSubpass:
                    {
                        commands.NextCommand<AdvanceSubpassCmd>();
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
                        commandList->OMSetRenderTargets(1, &device->GetCurrentRenderTargetDescriptor(), FALSE, nullptr);
                    }
                    break;

                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = commands.NextCommand<CopyBufferToBufferCmd>();
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands.NextCommand<CopyBufferToTextureCmd>();
                    }
                    break;

                case Command::CopyTextureToBuffer:
                    {
                        CopyTextureToBufferCmd* copy = commands.NextCommand<CopyTextureToBufferCmd>();
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

                case Command::EndRenderPass:
                    {
                        EndRenderPassCmd* cmd = commands.NextCommand<EndRenderPassCmd>();
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
                        SetPushConstantsCmd* cmd = commands.NextCommand<SetPushConstantsCmd>();
                    }
                    break;

                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd* cmd = commands.NextCommand<SetStencilReferenceCmd>();
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
