// Copyright 2019 The Dawn Authors
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

#include "dawn_native/CommandValidation.h"

#include "common/BitSetIterator.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Commands.h"
#include "dawn_native/PassResourceUsage.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderPipeline.h"

namespace dawn_native {

    namespace {

        inline MaybeError ValidateRenderBundleCommand(CommandIterator* commands,
                                                      Command type,
                                                      CommandBufferStateTracker* commandBufferState,
                                                      const AttachmentState* attachmentState,
                                                      uint64_t* debugGroupStackSize,
                                                      const char* disallowedMessage) {
            switch (type) {
                case Command::Draw: {
                    commands->NextCommand<DrawCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                    break;
                }

                case Command::DrawIndexed: {
                    commands->NextCommand<DrawIndexedCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                    break;
                }

                case Command::DrawIndirect: {
                    commands->NextCommand<DrawIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                    break;
                }

                case Command::DrawIndexedIndirect: {
                    commands->NextCommand<DrawIndexedIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    DAWN_TRY(ValidateCanPopDebugGroup(*debugGroupStackSize));
                    *debugGroupStackSize -= 1;
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    *debugGroupStackSize += 1;
                    break;
                }

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = commands->NextCommand<SetRenderPipelineCmd>();
                    RenderPipelineBase* pipeline = cmd->pipeline.Get();

                    if (DAWN_UNLIKELY(pipeline->GetAttachmentState() != attachmentState)) {
                        return DAWN_VALIDATION_ERROR("Pipeline attachment state is not compatible");
                    }
                    commandBufferState->SetRenderPipeline(pipeline);
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }

                    commandBufferState->SetBindGroup(cmd->index, cmd->group.Get());
                    break;
                }

                case Command::SetIndexBuffer: {
                    commands->NextCommand<SetIndexBufferCmd>();
                    commandBufferState->SetIndexBuffer();
                    break;
                }

                case Command::SetVertexBuffer: {
                    SetVertexBufferCmd* cmd = commands->NextCommand<SetVertexBufferCmd>();
                    commandBufferState->SetVertexBuffer(cmd->slot);
                    break;
                }

                default:
                    return DAWN_VALIDATION_ERROR(disallowedMessage);
            }

            return {};
        }

    }  // namespace

    MaybeError ValidateCanPopDebugGroup(uint64_t debugGroupStackSize) {
        if (debugGroupStackSize == 0) {
            return DAWN_VALIDATION_ERROR("Pop must be balanced by a corresponding Push.");
        }
        return {};
    }

    MaybeError ValidateFinalDebugGroupStackSize(uint64_t debugGroupStackSize) {
        if (debugGroupStackSize != 0) {
            return DAWN_VALIDATION_ERROR("Each Push must be balanced by a corresponding Pop.");
        }
        return {};
    }

    MaybeError ValidateRenderBundle(CommandIterator* commands,
                                    const AttachmentState* attachmentState) {
        CommandBufferStateTracker commandBufferState;
        uint64_t debugGroupStackSize = 0;

        Command type;
        while (commands->NextCommandId(&type)) {
            DAWN_TRY(ValidateRenderBundleCommand(commands, type, &commandBufferState,
                                                 attachmentState, &debugGroupStackSize,
                                                 "Command disallowed inside a render bundle"));
        }

        DAWN_TRY(ValidateFinalDebugGroupStackSize(debugGroupStackSize));
        return {};
    }

    MaybeError ValidateRenderPass(CommandIterator* commands, const BeginRenderPassCmd* renderPass) {
        CommandBufferStateTracker commandBufferState;
        uint64_t debugGroupStackSize = 0;

        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    commands->NextCommand<EndRenderPassCmd>();
                    DAWN_TRY(ValidateFinalDebugGroupStackSize(debugGroupStackSize));
                    return {};
                }

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = commands->NextCommand<ExecuteBundlesCmd>();
                    auto bundles = commands->NextData<Ref<RenderBundleBase>>(cmd->count);
                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        if (DAWN_UNLIKELY(renderPass->attachmentState.Get() !=
                                          bundles[i]->GetAttachmentState())) {
                            return DAWN_VALIDATION_ERROR(
                                "Render bundle is not compatible with render pass");
                        }
                    }

                    if (cmd->count > 0) {
                        // Reset state. It is invalidated after render bundle execution.
                        commandBufferState = CommandBufferStateTracker{};
                    }

                    break;
                }

                case Command::SetStencilReference: {
                    commands->NextCommand<SetStencilReferenceCmd>();
                    break;
                }

                case Command::SetBlendColor: {
                    commands->NextCommand<SetBlendColorCmd>();
                    break;
                }

                case Command::SetViewport: {
                    commands->NextCommand<SetViewportCmd>();
                    break;
                }

                case Command::SetScissorRect: {
                    commands->NextCommand<SetScissorRectCmd>();
                    break;
                }

                default:
                    DAWN_TRY(ValidateRenderBundleCommand(
                        commands, type, &commandBufferState, renderPass->attachmentState.Get(),
                        &debugGroupStackSize, "Command disallowed inside a render pass"));
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished render pass");
    }

    MaybeError ValidateComputePass(CommandIterator* commands) {
        CommandBufferStateTracker commandBufferState;
        uint64_t debugGroupStackSize = 0;

        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    commands->NextCommand<EndComputePassCmd>();
                    DAWN_TRY(ValidateFinalDebugGroupStackSize(debugGroupStackSize));
                    return {};
                }

                case Command::Dispatch: {
                    commands->NextCommand<DispatchCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                    break;
                }

                case Command::DispatchIndirect: {
                    commands->NextCommand<DispatchIndirectCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    DAWN_TRY(ValidateCanPopDebugGroup(debugGroupStackSize));
                    debugGroupStackSize--;
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    debugGroupStackSize++;
                    break;
                }

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = commands->NextCommand<SetComputePipelineCmd>();
                    ComputePipelineBase* pipeline = cmd->pipeline.Get();
                    commandBufferState.SetComputePipeline(pipeline);
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }
                    commandBufferState.SetBindGroup(cmd->index, cmd->group.Get());
                    break;
                }

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed inside a compute pass");
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished compute pass");
    }

    // Performs the per-pass usage validation checks
    // This will eventually need to differentiate between render and compute passes.
    // It will be valid to use a buffer both as uniform and storage in the same compute pass.
    MaybeError ValidatePassResourceUsage(const PassResourceUsage& pass) {
        // Buffers can only be used as single-write or multiple read.
        for (size_t i = 0; i < pass.buffers.size(); ++i) {
            const BufferBase* buffer = pass.buffers[i];
            wgpu::BufferUsage usage = pass.bufferUsages[i];

            if (usage & ~buffer->GetUsage()) {
                return DAWN_VALIDATION_ERROR("Buffer missing usage for the pass");
            }

            bool readOnly = (usage & kReadOnlyBufferUsages) == usage;
            bool singleUse = wgpu::HasZeroOrOneBits(usage);

            if (!readOnly && !singleUse) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer used as writable usage and another usage in pass");
            }
        }

        // Textures can only be used as single-write or multiple read.
        // TODO(cwallez@chromium.org): implement per-subresource tracking
        for (size_t i = 0; i < pass.textures.size(); ++i) {
            const TextureBase* texture = pass.textures[i];
            wgpu::TextureUsage usage = pass.textureUsages[i];

            if (usage & ~texture->GetUsage()) {
                return DAWN_VALIDATION_ERROR("Texture missing usage for the pass");
            }

            // For textures the only read-only usage in a pass is Sampled, so checking the
            // usage constraint simplifies to checking a single usage bit is set.
            if (!wgpu::HasZeroOrOneBits(usage)) {
                return DAWN_VALIDATION_ERROR("Texture used with more than one usage in pass");
            }
        }

        return {};
    }

}  // namespace dawn_native
