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
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Commands.h"
#include "dawn_native/PassResourceUsageTracker.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderPipeline.h"

namespace dawn_native {

    namespace {

        inline MaybeError PushDebugMarkerStack(unsigned int* counter) {
            *counter += 1;
            return {};
        }

        inline MaybeError PopDebugMarkerStack(unsigned int* counter) {
            if (*counter == 0) {
                return DAWN_VALIDATION_ERROR("Pop must be balanced by a corresponding Push.");
            } else {
                *counter -= 1;
            }

            return {};
        }

        inline MaybeError ValidateDebugGroups(const unsigned int counter) {
            if (counter != 0) {
                return DAWN_VALIDATION_ERROR("Each Push must be balanced by a corresponding Pop.");
            }

            return {};
        }

        void TrackBindGroupResourceUsage(BindGroupBase* group,
                                         PassResourceUsageTracker* usageTracker) {
            const auto& layoutInfo = group->GetLayout()->GetBindingInfo();

            for (uint32_t i : IterateBitSet(layoutInfo.mask)) {
                dawn::BindingType type = layoutInfo.types[i];

                switch (type) {
                    case dawn::BindingType::UniformBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(i).buffer;
                        usageTracker->BufferUsedAs(buffer, dawn::BufferUsageBit::Uniform);
                    } break;

                    case dawn::BindingType::StorageBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(i).buffer;
                        usageTracker->BufferUsedAs(buffer, dawn::BufferUsageBit::Storage);
                    } break;

                    case dawn::BindingType::SampledTexture: {
                        TextureBase* texture = group->GetBindingAsTextureView(i)->GetTexture();
                        usageTracker->TextureUsedAs(texture, dawn::TextureUsageBit::Sampled);
                    } break;

                    case dawn::BindingType::Sampler:
                        break;

                    case dawn::BindingType::StorageTexture:
                    case dawn::BindingType::ReadonlyStorageBuffer:
                        UNREACHABLE();
                        break;
                }
            }
        }

        inline MaybeError ValidateRenderBundleCommand(CommandIterator* commands,
                                                      Command type,
                                                      PassResourceUsageTracker* usageTracker,
                                                      CommandBufferStateTracker* commandBufferState,
                                                      const AttachmentState* attachmentState,
                                                      unsigned int* debugGroupStackSize,
                                                      const char* disallowedMessage) {
            switch (type) {
                case Command::Draw: {
                    commands->NextCommand<DrawCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                } break;

                case Command::DrawIndexed: {
                    commands->NextCommand<DrawIndexedCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                } break;

                case Command::DrawIndirect: {
                    DrawIndirectCmd* cmd = commands->NextCommand<DrawIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                    usageTracker->BufferUsedAs(cmd->indirectBuffer.Get(),
                                               dawn::BufferUsageBit::Indirect);
                } break;

                case Command::DrawIndexedIndirect: {
                    DrawIndexedIndirectCmd* cmd = commands->NextCommand<DrawIndexedIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                    usageTracker->BufferUsedAs(cmd->indirectBuffer.Get(),
                                               dawn::BufferUsageBit::Indirect);
                } break;

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                } break;

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    DAWN_TRY(PopDebugMarkerStack(debugGroupStackSize));
                } break;

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    DAWN_TRY(PushDebugMarkerStack(debugGroupStackSize));
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = commands->NextCommand<SetRenderPipelineCmd>();
                    RenderPipelineBase* pipeline = cmd->pipeline.Get();

                    if (DAWN_UNLIKELY(pipeline->GetAttachmentState() != attachmentState)) {
                        return DAWN_VALIDATION_ERROR("Pipeline attachment state is not compatible");
                    }
                    commandBufferState->SetRenderPipeline(pipeline);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }

                    TrackBindGroupResourceUsage(cmd->group.Get(), usageTracker);
                    commandBufferState->SetBindGroup(cmd->index, cmd->group.Get());
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = commands->NextCommand<SetIndexBufferCmd>();

                    usageTracker->BufferUsedAs(cmd->buffer.Get(), dawn::BufferUsageBit::Index);
                    commandBufferState->SetIndexBuffer();
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = commands->NextCommand<SetVertexBuffersCmd>();
                    auto buffers = commands->NextData<Ref<BufferBase>>(cmd->count);
                    commands->NextData<uint64_t>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        usageTracker->BufferUsedAs(buffers[i].Get(), dawn::BufferUsageBit::Vertex);
                    }
                    commandBufferState->SetVertexBuffer(cmd->startSlot, cmd->count);
                } break;

                default:
                    return DAWN_VALIDATION_ERROR(disallowedMessage);
            }

            return {};
        }

    }  // namespace

    MaybeError ValidateRenderBundle(CommandIterator* commands,
                                    const AttachmentState* attachmentState,
                                    PassResourceUsage* resourceUsage) {
        PassResourceUsageTracker usageTracker;
        CommandBufferStateTracker commandBufferState;
        unsigned int debugGroupStackSize = 0;

        Command type;
        while (commands->NextCommandId(&type)) {
            DAWN_TRY(ValidateRenderBundleCommand(commands, type, &usageTracker, &commandBufferState,
                                                 attachmentState, &debugGroupStackSize,
                                                 "Command disallowed inside a render bundle"));
        }

        DAWN_TRY(ValidateDebugGroups(debugGroupStackSize));
        DAWN_TRY(usageTracker.ValidateRenderPassUsages());
        ASSERT(resourceUsage != nullptr);
        *resourceUsage = usageTracker.AcquireResourceUsage();

        return {};
    }

    MaybeError ValidateRenderPass(CommandIterator* commands,
                                  BeginRenderPassCmd* renderPass,
                                  std::vector<PassResourceUsage>* perPassResourceUsages) {
        PassResourceUsageTracker usageTracker;
        CommandBufferStateTracker commandBufferState;
        unsigned int debugGroupStackSize = 0;

        // Track usage of the render pass attachments
        for (uint32_t i : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
            RenderPassColorAttachmentInfo* colorAttachment = &renderPass->colorAttachments[i];
            TextureBase* texture = colorAttachment->view->GetTexture();
            usageTracker.TextureUsedAs(texture, dawn::TextureUsageBit::OutputAttachment);

            TextureViewBase* resolveTarget = colorAttachment->resolveTarget.Get();
            if (resolveTarget != nullptr) {
                usageTracker.TextureUsedAs(resolveTarget->GetTexture(),
                                           dawn::TextureUsageBit::OutputAttachment);
            }
        }

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            TextureBase* texture = renderPass->depthStencilAttachment.view->GetTexture();
            usageTracker.TextureUsedAs(texture, dawn::TextureUsageBit::OutputAttachment);
        }

        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    commands->NextCommand<EndRenderPassCmd>();

                    DAWN_TRY(ValidateDebugGroups(debugGroupStackSize));

                    DAWN_TRY(usageTracker.ValidateRenderPassUsages());
                    ASSERT(perPassResourceUsages != nullptr);
                    perPassResourceUsages->push_back(usageTracker.AcquireResourceUsage());

                    return {};
                } break;

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = commands->NextCommand<ExecuteBundlesCmd>();
                    auto bundles = commands->NextData<Ref<RenderBundleBase>>(cmd->count);
                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        if (DAWN_UNLIKELY(renderPass->attachmentState.Get() !=
                                          bundles[i]->GetAttachmentState())) {
                            return DAWN_VALIDATION_ERROR(
                                "Render bundle is not compatible with render pass");
                        }

                        const PassResourceUsage& usages = bundles[i]->GetResourceUsage();
                        for (uint32_t i = 0; i < usages.buffers.size(); ++i) {
                            usageTracker.BufferUsedAs(usages.buffers[i], usages.bufferUsages[i]);
                        }

                        for (uint32_t i = 0; i < usages.textures.size(); ++i) {
                            usageTracker.TextureUsedAs(usages.textures[i], usages.textureUsages[i]);
                        }
                    }

                    if (cmd->count > 0) {
                        // Reset state. It is invalidated after render bundle execution.
                        commandBufferState = CommandBufferStateTracker{};
                    }

                } break;

                case Command::SetStencilReference: {
                    commands->NextCommand<SetStencilReferenceCmd>();
                } break;

                case Command::SetBlendColor: {
                    commands->NextCommand<SetBlendColorCmd>();
                } break;

                case Command::SetViewport: {
                    commands->NextCommand<SetViewportCmd>();
                } break;

                case Command::SetScissorRect: {
                    commands->NextCommand<SetScissorRectCmd>();
                } break;

                default:
                    DAWN_TRY(ValidateRenderBundleCommand(
                        commands, type, &usageTracker, &commandBufferState,
                        renderPass->attachmentState.Get(), &debugGroupStackSize,
                        "Command disallowed inside a render pass"));
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished render pass");
    }

    MaybeError ValidateComputePass(CommandIterator* commands,
                                   std::vector<PassResourceUsage>* perPassResourceUsages) {
        PassResourceUsageTracker usageTracker;
        CommandBufferStateTracker commandBufferState;
        unsigned int debugGroupStackSize = 0;

        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    commands->NextCommand<EndComputePassCmd>();

                    DAWN_TRY(ValidateDebugGroups(debugGroupStackSize));

                    DAWN_TRY(usageTracker.ValidateComputePassUsages());
                    ASSERT(perPassResourceUsages != nullptr);
                    perPassResourceUsages->push_back(usageTracker.AcquireResourceUsage());
                    return {};
                } break;

                case Command::Dispatch: {
                    commands->NextCommand<DispatchCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                } break;

                case Command::DispatchIndirect: {
                    DispatchIndirectCmd* cmd = commands->NextCommand<DispatchIndirectCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                    usageTracker.BufferUsedAs(cmd->indirectBuffer.Get(),
                                              dawn::BufferUsageBit::Indirect);
                } break;

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                } break;

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    DAWN_TRY(PopDebugMarkerStack(&debugGroupStackSize));
                } break;

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    DAWN_TRY(PushDebugMarkerStack(&debugGroupStackSize));
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = commands->NextCommand<SetComputePipelineCmd>();
                    ComputePipelineBase* pipeline = cmd->pipeline.Get();
                    commandBufferState.SetComputePipeline(pipeline);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint64_t>(cmd->dynamicOffsetCount);
                    }

                    TrackBindGroupResourceUsage(cmd->group.Get(), &usageTracker);
                    commandBufferState.SetBindGroup(cmd->index, cmd->group.Get());
                } break;

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed inside a compute pass");
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished compute pass");
    }

}  // namespace dawn_native
