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
#include "PipelineD3D12.h"
#include "PipelineLayoutD3D12.h"

namespace backend {
namespace d3d12 {

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

                          // TODO(enga@google.com): Set the back buffer as the render target only when a new render target is set
                          D3D12_RESOURCE_BARRIER resourceBarrier;
                          resourceBarrier.Transition.pResource = device->GetNextRenderTarget().Get();
                          resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                          resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                          resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                          resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                          resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                          commandList->ResourceBarrier(1, &resourceBarrier);
                          commandList->OMSetRenderTargets(1, &device->GetNextRenderTargetDescriptor(), FALSE, nullptr);
                      }
                      break;

                  case Command::CopyBufferToTexture:
                      {
                          CopyBufferToTextureCmd* copy = commands.NextCommand<CopyBufferToTextureCmd>();
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
                      }
                      break;

                  case Command::EndRenderPass:
                      {
                          EndRenderPassCmd* cmd = commands.NextCommand<EndRenderPassCmd>();

                          // TODO(enga@google.com): Present the back buffer only before swap
                          D3D12_RESOURCE_BARRIER resourceBarrier;
                          resourceBarrier.Transition.pResource = device->GetNextRenderTarget().Get();
                          resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                          resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                          resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                          resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                          resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                          commandList->ResourceBarrier(1, &resourceBarrier);
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
                      }
                      break;

                  case Command::SetVertexBuffers:
                      {
                          SetVertexBuffersCmd* cmd = commands.NextCommand<SetVertexBuffersCmd>();
                      }
                      break;

                  case Command::TransitionBufferUsage:
                      {
                          TransitionBufferUsageCmd* cmd = commands.NextCommand<TransitionBufferUsageCmd>();
                      }
                      break;

                  case Command::TransitionTextureUsage:
                      {
                          TransitionTextureUsageCmd* cmd = commands.NextCommand<TransitionTextureUsageCmd>();
                      }
                      break;
              }
          }
      }
}
}
