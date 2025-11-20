// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/webgpu/CommandBufferHelpers.h"

#include <vector>

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Commands.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/RenderBundleWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"

namespace dawn::native::webgpu {

// Captures commands common to a render pass and a render bundle
MaybeError CaptureRenderCommand(CaptureContext& captureContext,
                                CommandIterator& commands,
                                Command type) {
    switch (type) {
        case Command::SetRenderPipeline: {
            const auto& cmd = *commands.NextCommand<SetRenderPipelineCmd>();
            schema::CommandBufferCommandSetRenderPipelineCmd data{{
                .data = {{
                    .pipelineId = captureContext.GetId(cmd.pipeline.Get()),
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::SetBindGroup: {
            const auto& cmd = *commands.NextCommand<SetBindGroupCmd>();
            const uint32_t* dynamicOffsetsData =
                cmd.dynamicOffsetCount > 0 ? commands.NextData<uint32_t>(cmd.dynamicOffsetCount)
                                           : nullptr;
            schema::CommandBufferCommandSetBindGroupCmd data{{
                .data = {{
                    .index = uint32_t(cmd.index),
                    .bindGroupId = captureContext.GetId(cmd.group),
                    .dynamicOffsets = std::vector<uint32_t>(
                        dynamicOffsetsData, dynamicOffsetsData + cmd.dynamicOffsetCount),
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::SetVertexBuffer: {
            const auto& cmd = *commands.NextCommand<SetVertexBufferCmd>();
            schema::CommandBufferCommandSetVertexBufferCmd data{{
                .data = {{
                    .slot = uint32_t(cmd.slot),
                    .bufferId = captureContext.GetId(cmd.buffer),
                    .offset = cmd.offset,
                    .size = cmd.size,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        case Command::Draw: {
            const auto& cmd = *commands.NextCommand<DrawCmd>();
            schema::CommandBufferCommandDrawCmd data{{
                .data = {{
                    .vertexCount = cmd.vertexCount,
                    .instanceCount = cmd.instanceCount,
                    .firstVertex = cmd.firstVertex,
                    .firstInstance = cmd.firstInstance,
                }},
            }};
            Serialize(captureContext, data);
            break;
        }
        default:
            return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
    }

    return {};
}

// Commands are encoded with a command id followed by command-specific data.
// so we're required to read each command to skip over them.
#define DAWN_SKIP_COMMAND(cmdName)            \
    case Command::cmdName: {                  \
        commands.NextCommand<cmdName##Cmd>(); \
        break;                                \
    }

// Gathers resources used by commands from both render passes and render bundles.
MaybeError GatherReferencedResourcesFromRenderCommand(CaptureContext& captureContext,
                                                      CommandIterator& commands,
                                                      CommandBufferResourceUsages& usedResources,
                                                      Command type) {
    switch (type) {
        case Command::SetRenderPipeline: {
            auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
            usedResources.renderPipelines.push_back(cmd->pipeline.Get());
            break;
        }
        case Command::SetBindGroup: {
            auto cmd = commands.NextCommand<SetBindGroupCmd>();
            usedResources.bindGroups.push_back(cmd->group.Get());
            break;
        }
            DAWN_SKIP_COMMAND(BeginOcclusionQuery)
            DAWN_SKIP_COMMAND(EndOcclusionQuery)
            DAWN_SKIP_COMMAND(Draw)
            DAWN_SKIP_COMMAND(DrawIndexed)
            DAWN_SKIP_COMMAND(DrawIndirect)
            DAWN_SKIP_COMMAND(DrawIndexedIndirect)
            DAWN_SKIP_COMMAND(InsertDebugMarker)
            DAWN_SKIP_COMMAND(PopDebugGroup)
            DAWN_SKIP_COMMAND(PushDebugGroup)
            DAWN_SKIP_COMMAND(WriteTimestamp)
            DAWN_SKIP_COMMAND(SetImmediates)
            DAWN_SKIP_COMMAND(SetIndexBuffer)
            DAWN_SKIP_COMMAND(SetVertexBuffer)
        default: {
            return DAWN_UNIMPLEMENTED_ERROR("Unimplemented command");
        }
    }
    return {};
}

MaybeError AddUsedResources(CaptureContext& captureContext,
                            const CommandBufferResourceUsages& usedResources) {
    for (auto pipeline : usedResources.computePipelines) {
        DAWN_TRY(captureContext.AddResource(pipeline));
    }
    for (auto pipeline : usedResources.renderPipelines) {
        DAWN_TRY(captureContext.AddResource(pipeline));
    }
    for (auto bindGroup : usedResources.bindGroups) {
        DAWN_TRY(captureContext.AddResource(bindGroup));
    }
    for (auto bundle : usedResources.renderBundles) {
        DAWN_TRY(captureContext.AddResource(bundle));
    }
    return {};
}

}  // namespace dawn::native::webgpu
