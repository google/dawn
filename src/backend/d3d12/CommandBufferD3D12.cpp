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
#include "BufferD3D12.h"
#include "InputStateD3D12.h"
#include "PipelineD3D12.h"
#include "PipelineLayoutD3D12.h"

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
    }

    CommandBuffer::CommandBuffer(Device* device, CommandBufferBuilder* builder)
        : CommandBufferBase(builder), device(device), commands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&commands);
    }

    void CommandBuffer::FillCommands(ComPtr<ID3D12GraphicsCommandList> commandList) {
        Command type;
        Pipeline* lastPipeline = nullptr;

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

                        float width = (float) currentFramebuffer->GetWidth();
                        float height = (float) currentFramebuffer->GetHeight();
                        D3D12_VIEWPORT viewport = { 0.f, 0.f, width, height, 0.f, 1.f };
                        D3D12_RECT scissorRect = { 0.f, 0.f, width, height };
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
                        lastPipeline = ToBackend(cmd->pipeline).Get();
                        PipelineLayout* pipelineLayout = ToBackend(lastPipeline->GetLayout());

                        // TODO
                        if (lastPipeline->IsCompute()) {
                        } else {
                            commandList->SetGraphicsRootSignature(pipelineLayout->GetRootSignature().Get());
                            commandList->SetPipelineState(lastPipeline->GetRenderPipelineState().Get());
                        }
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
                        texture->UpdateUsageInternal(cmd->usage);
                    }
                    break;
            }
        }
    }
}
}
