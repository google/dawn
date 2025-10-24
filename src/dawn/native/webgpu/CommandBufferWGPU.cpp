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

#include "dawn/native/webgpu/CommandBufferWGPU.h"

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/ComputePipelineWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/RenderPipelineWGPU.h"
#include "dawn/native/webgpu/Serialization.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor), RecordableObject(schema::ObjectType::CommandBuffer) {}

namespace {

void EncodeComputePass(const DawnProcTable& wgpu,
                       WGPUCommandEncoder innerEncoder,
                       CommandIterator& commands,
                       BeginComputePassCmd* computePassCmd,
                       const ComputePassResourceUsage& resourceUsages) {
    WGPUComputePassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(computePassCmd->label),
        .timestampWrites = nullptr,
    };
    WGPUPassTimestampWrites timestampWrites;
    if (computePassCmd->timestampWrites.querySet) {
        timestampWrites = ToWGPU(computePassCmd->timestampWrites);
        passDescriptor.timestampWrites = &timestampWrites;
    }

    for (auto texture : resourceUsages.referencedTextures) {
        texture->SetInitialized(true);
    }

    WGPUComputePassEncoder passEncoder =
        wgpu.commandEncoderBeginComputePass(innerEncoder, &passDescriptor);

    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                commands.NextCommand<EndComputePassCmd>();
                wgpu.computePassEncoderEnd(passEncoder);
                return;
            }

            case Command::Dispatch: {
                auto cmd = commands.NextCommand<DispatchCmd>();
                wgpu.computePassEncoderDispatchWorkgroups(passEncoder, cmd->x, cmd->y, cmd->z);
                break;
            }

            case Command::DispatchIndirect: {
                auto cmd = commands.NextCommand<DispatchIndirectCmd>();
                wgpu.computePassEncoderDispatchWorkgroupsIndirect(
                    passEncoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                    cmd->indirectOffset);
                break;
            }

            case Command::SetComputePipeline: {
                auto cmd = commands.NextCommand<SetComputePipelineCmd>();
                wgpu.computePassEncoderSetPipeline(passEncoder,
                                                   ToBackend(cmd->pipeline)->GetInnerHandle());
                break;
            }

            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                wgpu.computePassEncoderSetBindGroup(passEncoder, static_cast<uint32_t>(cmd->index),
                                                    ToBackend(cmd->group)->GetInnerHandle(),
                                                    cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }
            case Command::InsertDebugMarker: {
                auto cmd = commands.NextCommand<InsertDebugMarkerCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.computePassEncoderInsertDebugMarker(passEncoder, {label, cmd->length});
                break;
            }

            case Command::PopDebugGroup: {
                commands.NextCommand<PopDebugGroupCmd>();
                wgpu.computePassEncoderPopDebugGroup(passEncoder);
                break;
            }

            case Command::PushDebugGroup: {
                auto cmd = commands.NextCommand<PushDebugGroupCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.computePassEncoderPushDebugGroup(passEncoder, {label, cmd->length});
                break;
            }

            case Command::WriteTimestamp: {
                auto cmd = commands.NextCommand<WriteTimestampCmd>();
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // QuerySetWGPU
                wgpu.computePassEncoderWriteTimestamp(
                    passEncoder, nullptr /*ToBackend(cmd->querySet)->GetInnerHandle()*/,
                    cmd->queryIndex);
                break;
            }

            case Command::SetImmediateData: {
                auto cmd = commands.NextCommand<SetImmediateDataCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = commands.NextData<uint8_t>(cmd->size);
                wgpu.computePassEncoderSetImmediateData(passEncoder, cmd->offset, value, cmd->size);
                break;
            }

            default: {
                DAWN_UNREACHABLE();
                break;
            }
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

void EncodeRenderBundleCommand(const DawnProcTable& wgpu,
                               WGPURenderBundleEncoder encoder,
                               CommandIterator& commands,
                               Command type) {
    switch (type) {
        case Command::Draw: {
            auto cmd = commands.NextCommand<DrawCmd>();
            wgpu.renderBundleEncoderDraw(encoder, cmd->vertexCount, cmd->instanceCount,
                                         cmd->firstVertex, cmd->firstInstance);
            break;
        }

        case Command::DrawIndexed: {
            auto cmd = commands.NextCommand<DrawIndexedCmd>();
            wgpu.renderBundleEncoderDrawIndexed(encoder, cmd->indexCount, cmd->instanceCount,
                                                cmd->firstIndex, cmd->baseVertex,
                                                cmd->firstInstance);
            break;
        }

        case Command::DrawIndirect: {
            auto cmd = commands.NextCommand<DrawIndirectCmd>();
            wgpu.renderBundleEncoderDrawIndirect(
                encoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(), cmd->indirectOffset);
            break;
        }

        case Command::DrawIndexedIndirect: {
            auto cmd = commands.NextCommand<DrawIndexedIndirectCmd>();
            wgpu.renderBundleEncoderDrawIndexedIndirect(
                encoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(), cmd->indirectOffset);
            break;
        }

        case Command::MultiDrawIndirect: {
            DAWN_UNREACHABLE();
            break;
        }

        case Command::MultiDrawIndexedIndirect: {
            DAWN_UNREACHABLE();
            break;
        }

        case Command::InsertDebugMarker: {
            auto cmd = commands.NextCommand<InsertDebugMarkerCmd>();
            char* label = commands.NextData<char>(cmd->length + 1);
            wgpu.renderBundleEncoderInsertDebugMarker(encoder, {label, cmd->length});
            break;
        }

        case Command::PopDebugGroup: {
            commands.NextCommand<PopDebugGroupCmd>();
            wgpu.renderBundleEncoderPopDebugGroup(encoder);
            break;
        }

        case Command::PushDebugGroup: {
            auto cmd = commands.NextCommand<PushDebugGroupCmd>();
            char* label = commands.NextData<char>(cmd->length + 1);
            wgpu.renderBundleEncoderPushDebugGroup(encoder, {label, cmd->length});
            break;
        }

        case Command::SetBindGroup: {
            auto cmd = commands.NextCommand<SetBindGroupCmd>();
            uint32_t* dynamicOffsets = nullptr;
            if (cmd->dynamicOffsetCount > 0) {
                dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
            }
            wgpu.renderBundleEncoderSetBindGroup(encoder, static_cast<uint32_t>(cmd->index),
                                                 ToBackend(cmd->group)->GetInnerHandle(),
                                                 cmd->dynamicOffsetCount, dynamicOffsets);
            break;
        }

        case Command::SetIndexBuffer: {
            auto cmd = commands.NextCommand<SetIndexBufferCmd>();
            wgpu.renderBundleEncoderSetIndexBuffer(encoder,
                                                   ToBackend(cmd->buffer)->GetInnerHandle(),
                                                   ToWGPU(cmd->format), cmd->offset, cmd->size);
            break;
        }

        case Command::SetRenderPipeline: {
            auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
            wgpu.renderBundleEncoderSetPipeline(encoder,
                                                ToBackend(cmd->pipeline)->GetInnerHandle());
            break;
        }

        case Command::SetVertexBuffer: {
            auto cmd = commands.NextCommand<SetVertexBufferCmd>();
            wgpu.renderBundleEncoderSetVertexBuffer(encoder, static_cast<uint8_t>(cmd->slot),
                                                    ToBackend(cmd->buffer)->GetInnerHandle(),
                                                    cmd->offset, cmd->size);
            break;
        }

        case Command::SetImmediateData: {
            auto cmd = commands.NextCommand<SetImmediateDataCmd>();
            DAWN_ASSERT(cmd->size > 0);
            uint8_t* value = nullptr;
            value = commands.NextData<uint8_t>(cmd->size);
            wgpu.renderBundleEncoderSetImmediateData(encoder, cmd->offset, value, cmd->size);
            break;
        }

        default:
            DAWN_UNREACHABLE();
            break;
    }
}

void EncodeRenderPass(const Device* device,
                      WGPUCommandEncoder innerEncoder,
                      CommandIterator& commands,
                      BeginRenderPassCmd* renderPassCmd) {
    const DawnProcTable& wgpu = device->wgpu;

    PerColorAttachment<WGPURenderPassColorAttachment> colorAttachments = {};

    size_t colorAttachmentCount = 0;
    for (auto i : renderPassCmd->attachmentState->GetColorAttachmentsMask()) {
        auto& colorAttachment = renderPassCmd->colorAttachments[i];
        colorAttachment.view->GetTexture()->SetInitialized(true);
        if (colorAttachment.resolveTarget != nullptr) {
            colorAttachment.resolveTarget->GetTexture()->SetInitialized(true);
        }
        colorAttachments[i] = ToWGPU(colorAttachment);
        colorAttachmentCount = static_cast<size_t>(i) + 1;
    }

    WGPURenderPassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(renderPassCmd->label),
        .colorAttachmentCount = colorAttachmentCount,
        .colorAttachments = colorAttachments.data(),
        .depthStencilAttachment = nullptr,
        // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
        // QuerySetWGPU
        .occlusionQuerySet = nullptr /*ToBackend(cmd->occlusionQuerySet)->GetInnerHandle()*/,
        .timestampWrites = nullptr,
    };
    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
    if (renderPassCmd->attachmentState->HasDepthStencilAttachment()) {
        renderPassCmd->depthStencilAttachment.view->GetTexture()->SetInitialized(true);
        depthStencilAttachment = ToWGPU(renderPassCmd->depthStencilAttachment);
        passDescriptor.depthStencilAttachment = &depthStencilAttachment;
    }
    WGPUPassTimestampWrites timestampWrites;
    if (renderPassCmd->timestampWrites.querySet) {
        timestampWrites = ToWGPU(renderPassCmd->timestampWrites);
        passDescriptor.timestampWrites = &timestampWrites;
    }
    WGPURenderPassDescriptorResolveRect resolveRect;
    if (renderPassCmd->resolveRect.HasValue()) {
        resolveRect = WGPU_RENDER_PASS_DESCRIPTOR_RESOLVE_RECT_INIT;
        resolveRect.colorOffsetX = renderPassCmd->resolveRect.colorOffsetX;
        resolveRect.colorOffsetY = renderPassCmd->resolveRect.colorOffsetY;
        resolveRect.resolveOffsetX = renderPassCmd->resolveRect.resolveOffsetX;
        resolveRect.resolveOffsetY = renderPassCmd->resolveRect.resolveOffsetY;
        resolveRect.width = renderPassCmd->resolveRect.updateWidth;
        resolveRect.height = renderPassCmd->resolveRect.updateHeight;
        passDescriptor.nextInChain = &(resolveRect.chain);
    }
    WGPURenderPassEncoder passEncoder =
        wgpu.commandEncoderBeginRenderPass(innerEncoder, &passDescriptor);

    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                commands.NextCommand<EndRenderPassCmd>();
                wgpu.renderPassEncoderEnd(passEncoder);
                return;
            }

            case Command::SetBlendConstant: {
                auto cmd = commands.NextCommand<SetBlendConstantCmd>();
                WGPUColor color = ToWGPU(cmd->color);
                wgpu.renderPassEncoderSetBlendConstant(passEncoder, &color);
                break;
            }

            case Command::SetStencilReference: {
                auto cmd = commands.NextCommand<SetStencilReferenceCmd>();
                wgpu.renderPassEncoderSetStencilReference(passEncoder, cmd->reference);
                break;
            }

            case Command::SetViewport: {
                auto cmd = commands.NextCommand<SetViewportCmd>();
                wgpu.renderPassEncoderSetViewport(passEncoder, cmd->x, cmd->y, cmd->width,
                                                  cmd->height, cmd->minDepth, cmd->maxDepth);
                break;
            }

            case Command::SetScissorRect: {
                auto cmd = commands.NextCommand<SetScissorRectCmd>();
                wgpu.renderPassEncoderSetScissorRect(passEncoder, cmd->x, cmd->y, cmd->width,
                                                     cmd->height);
                break;
            }

            case Command::ExecuteBundles: {
                auto* cmd = commands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = commands.NextData<Ref<RenderBundleBase>>(cmd->count);
                std::vector<WGPURenderBundle> wgpuBundles;
                wgpuBundles.reserve(cmd->count);

                // Frontend validation guarantees that the render pass layout of all the render
                // bundles here match that of the current render pass. So we get the render pass
                // layout information from the current render pass.
                PerColorAttachment<WGPUTextureFormat> colorFormats = {};
                for (auto i : renderPassCmd->attachmentState->GetColorAttachmentsMask()) {
                    colorFormats[i] = ToAPI(
                        renderPassCmd->colorAttachments[i].view->GetTexture()->GetFormat().format);
                }

                WGPURenderBundleEncoderDescriptor bundleEncoderDescriptorBase =
                    WGPU_RENDER_BUNDLE_ENCODER_DESCRIPTOR_INIT;
                bundleEncoderDescriptorBase.colorFormatCount = colorAttachmentCount;
                bundleEncoderDescriptorBase.colorFormats = colorFormats.data();
                if (renderPassCmd->attachmentState->HasDepthStencilAttachment()) {
                    bundleEncoderDescriptorBase.depthStencilFormat =
                        ToAPI(renderPassCmd->depthStencilAttachment.view->GetTexture()
                                  ->GetFormat()
                                  .format);
                }

                for (uint32_t i = 0; i < cmd->count; ++i) {
                    WGPURenderBundleEncoderDescriptor bundleEncoderDescriptor =
                        bundleEncoderDescriptorBase;
                    bundleEncoderDescriptor.depthReadOnly = bundles[i]->IsDepthReadOnly();
                    bundleEncoderDescriptor.stencilReadOnly = bundles[i]->IsStencilReadOnly();

                    WGPURenderBundleEncoder bundleEncoder = wgpu.deviceCreateRenderBundleEncoder(
                        device->GetInnerHandle(), &bundleEncoderDescriptor);

                    CommandIterator* iter = bundles[i]->GetCommands();
                    Command bundleCommandType;
                    while (iter->NextCommandId(&bundleCommandType)) {
                        EncodeRenderBundleCommand(wgpu, bundleEncoder, *iter, bundleCommandType);
                    }

                    wgpuBundles.emplace_back(
                        wgpu.renderBundleEncoderFinish(bundleEncoder, nullptr));
                }
                wgpu.renderPassEncoderExecuteBundles(passEncoder, wgpuBundles.size(),
                                                     wgpuBundles.data());
                break;
            }

            case Command::BeginOcclusionQuery: {
                auto cmd = commands.NextCommand<BeginOcclusionQueryCmd>();
                wgpu.renderPassEncoderBeginOcclusionQuery(passEncoder, cmd->queryIndex);
                break;
            }

            case Command::EndOcclusionQuery: {
                commands.NextCommand<EndOcclusionQueryCmd>();
                wgpu.renderPassEncoderEndOcclusionQuery(passEncoder);
                break;
            }

            case Command::WriteTimestamp: {
                auto cmd = commands.NextCommand<WriteTimestampCmd>();
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // QuerySetWGPU
                wgpu.renderPassEncoderWriteTimestamp(
                    passEncoder, nullptr /*ToBackend(cmd->querySet)->GetInnerHandle()*/,
                    cmd->queryIndex);
                break;
            }

            // The followings are commands shared with RenderBundleEncoder, but it's a bit hard to
            // share code with EncodeRenderBundleCommand since we are using webgpu C header.
            case Command::Draw: {
                auto cmd = commands.NextCommand<DrawCmd>();
                wgpu.renderPassEncoderDraw(passEncoder, cmd->vertexCount, cmd->instanceCount,
                                           cmd->firstVertex, cmd->firstInstance);
                break;
            }

            case Command::DrawIndexed: {
                auto cmd = commands.NextCommand<DrawIndexedCmd>();
                wgpu.renderPassEncoderDrawIndexed(passEncoder, cmd->indexCount, cmd->instanceCount,
                                                  cmd->firstIndex, cmd->baseVertex,
                                                  cmd->firstInstance);
                break;
            }

            case Command::DrawIndirect: {
                auto cmd = commands.NextCommand<DrawIndirectCmd>();
                wgpu.renderPassEncoderDrawIndirect(passEncoder,
                                                   ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                                                   cmd->indirectOffset);
                break;
            }

            case Command::DrawIndexedIndirect: {
                auto cmd = commands.NextCommand<DrawIndexedIndirectCmd>();
                wgpu.renderPassEncoderDrawIndexedIndirect(
                    passEncoder, ToBackend(cmd->indirectBuffer)->GetInnerHandle(),
                    cmd->indirectOffset);
                break;
            }

            case Command::MultiDrawIndirect: {
                DAWN_UNREACHABLE();
                break;
            }

            case Command::MultiDrawIndexedIndirect: {
                DAWN_UNREACHABLE();
                break;
            }

            case Command::InsertDebugMarker: {
                auto cmd = commands.NextCommand<InsertDebugMarkerCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.renderPassEncoderInsertDebugMarker(passEncoder, {label, cmd->length});
                break;
            }

            case Command::PopDebugGroup: {
                commands.NextCommand<PopDebugGroupCmd>();
                wgpu.renderPassEncoderPopDebugGroup(passEncoder);
                break;
            }

            case Command::PushDebugGroup: {
                auto cmd = commands.NextCommand<PushDebugGroupCmd>();
                char* label = commands.NextData<char>(cmd->length + 1);
                wgpu.renderPassEncoderPushDebugGroup(passEncoder, {label, cmd->length});
                break;
            }

            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                wgpu.renderPassEncoderSetBindGroup(passEncoder, static_cast<uint32_t>(cmd->index),
                                                   ToBackend(cmd->group)->GetInnerHandle(),
                                                   cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                auto cmd = commands.NextCommand<SetIndexBufferCmd>();
                wgpu.renderPassEncoderSetIndexBuffer(passEncoder,
                                                     ToBackend(cmd->buffer)->GetInnerHandle(),
                                                     ToWGPU(cmd->format), cmd->offset, cmd->size);
                break;
            }

            case Command::SetRenderPipeline: {
                auto cmd = commands.NextCommand<SetRenderPipelineCmd>();
                wgpu.renderPassEncoderSetPipeline(passEncoder,
                                                  ToBackend(cmd->pipeline)->GetInnerHandle());
                break;
            }

            case Command::SetVertexBuffer: {
                auto cmd = commands.NextCommand<SetVertexBufferCmd>();
                wgpu.renderPassEncoderSetVertexBuffer(passEncoder, static_cast<uint8_t>(cmd->slot),
                                                      ToBackend(cmd->buffer)->GetInnerHandle(),
                                                      cmd->offset, cmd->size);
                break;
            }

            case Command::SetImmediateData: {
                auto cmd = commands.NextCommand<SetImmediateDataCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = commands.NextData<uint8_t>(cmd->size);
                wgpu.renderPassEncoderSetImmediateData(passEncoder, cmd->offset, value, cmd->size);
                break;
            }

            default: {
                DAWN_UNREACHABLE();
                break;
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

}  // anonymous namespace

MaybeError CommandBuffer::AddReferenced(CaptureContext& captureContext) const {
    const auto& resourceUsages = GetResourceUsages();
    for (auto buffer : resourceUsages.topLevelBuffers) {
        DAWN_TRY(captureContext.AddResource(buffer));
    }
    for (auto texture : resourceUsages.topLevelTextures) {
        DAWN_TRY(captureContext.AddResource(texture));
    }
    for (const auto& pass : resourceUsages.renderPasses) {
        for (auto buffer : pass.buffers) {
            DAWN_TRY(captureContext.AddResource(buffer));
        }
    }
    for (const auto& pass : resourceUsages.computePasses) {
        for (auto buffer : pass.referencedBuffers) {
            DAWN_TRY(captureContext.AddResource(buffer));
        }
    }
    return {};
}

MaybeError CommandBuffer::CaptureCreationParameters(CaptureContext& captureContext) {
    CommandIterator& commands = *GetCommandIteratorForTesting();

    Command type;
    while (commands.NextCommandId(&type)) {
        switch (type) {
            case Command::CopyBufferToBuffer: {
                const auto& cmd = *commands.NextCommand<CopyBufferToBufferCmd>();
                schema::EncoderCommandCopyBufferToBufferCmd data{{
                    .data = {{
                        .srcBufferId = captureContext.GetId(cmd.source.Get()),
                        .srcOffset = cmd.sourceOffset,
                        .dstBufferId = captureContext.GetId(cmd.destination.Get()),
                        .dstOffset = cmd.destinationOffset,
                        .size = cmd.size,
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyBufferToTexture: {
                const auto& cmd = *commands.NextCommand<CopyBufferToTextureCmd>();
                schema::EncoderCommandCopyBufferToTextureCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source),
                        .destination = ToSchema(captureContext, cmd.destination),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyTextureToBuffer: {
                const auto& cmd = *commands.NextCommand<CopyTextureToBufferCmd>();
                schema::EncoderCommandCopyTextureToBufferCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source),
                        .destination = ToSchema(captureContext, cmd.destination),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            case Command::CopyTextureToTexture: {
                const auto& cmd = *commands.NextCommand<CopyTextureToTextureCmd>();
                schema::EncoderCommandCopyTextureToTextureCmd data{{
                    .data = {{
                        .source = ToSchema(captureContext, cmd.source),
                        .destination = ToSchema(captureContext, cmd.destination),
                        .copySize = ToSchema(cmd.copySize),
                    }},
                }};
                Serialize(captureContext, data);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }
    Serialize(captureContext, schema::EncoderCommand::End);
    return {};
}

WGPUCommandBuffer CommandBuffer::Encode() {
    auto& wgpu = ToBackend(GetDevice())->wgpu;

    // TODO(crbug.com/413053623): Use stored command encoder descriptor
    WGPUCommandEncoder innerEncoder =
        wgpu.deviceCreateCommandEncoder(ToBackend(GetDevice())->GetInnerHandle(), nullptr);

    size_t nextComputePassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();
                EncodeComputePass(wgpu, innerEncoder, mCommands, cmd,
                                  GetResourceUsages().computePasses[nextComputePassNumber]);
                ++nextComputePassNumber;
                break;
            }
            case Command::BeginRenderPass: {
                auto cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                EncodeRenderPass(ToBackend(GetDevice()), innerEncoder, mCommands, cmd);
                break;
            }
            case Command::CopyBufferToBuffer: {
                auto copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                wgpu.commandEncoderCopyBufferToBuffer(
                    innerEncoder, ToBackend(copy->source)->GetInnerHandle(), copy->sourceOffset,
                    ToBackend(copy->destination)->GetInnerHandle(), copy->destinationOffset,
                    copy->size);
                break;
            }
            case Command::CopyBufferToTexture: {
                auto cmd = mCommands.NextCommand<CopyBufferToTextureCmd>();
                WGPUTexelCopyBufferInfo source = ToWGPU(cmd->source);
                WGPUTexelCopyTextureInfo destination = ToWGPU(cmd->destination);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                wgpu.commandEncoderCopyBufferToTexture(innerEncoder, &source, &destination, &size);
                cmd->destination.texture.Get()->SetInitialized(true);
                break;
            }
            case Command::CopyTextureToBuffer: {
                auto cmd = mCommands.NextCommand<CopyTextureToBufferCmd>();
                WGPUTexelCopyTextureInfo source = ToWGPU(cmd->source);
                WGPUTexelCopyBufferInfo destination = ToWGPU(cmd->destination);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                wgpu.commandEncoderCopyTextureToBuffer(innerEncoder, &source, &destination, &size);
                break;
            }
            case Command::CopyTextureToTexture: {
                auto cmd = mCommands.NextCommand<CopyTextureToTextureCmd>();
                WGPUTexelCopyTextureInfo source = ToWGPU(cmd->source);
                WGPUTexelCopyTextureInfo destination = ToWGPU(cmd->destination);
                WGPUExtent3D size = ToWGPU(cmd->copySize);
                wgpu.commandEncoderCopyTextureToTexture(innerEncoder, &source, &destination, &size);
                cmd->destination.texture.Get()->SetInitialized(true);
                break;
            }
            case Command::ClearBuffer: {
                auto cmd = mCommands.NextCommand<ClearBufferCmd>();
                wgpu.commandEncoderClearBuffer(
                    innerEncoder, ToBackend(cmd->buffer)->GetInnerHandle(), cmd->offset, cmd->size);
                break;
            }
            case Command::ResolveQuerySet: {
                auto cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // QuerySetWGPU
                wgpu.commandEncoderResolveQuerySet(
                    innerEncoder, nullptr /*ToBackend(cmd->querySet)->GetInnerHandle()*/,
                    cmd->firstQuery, cmd->queryCount, ToBackend(cmd->destination)->GetInnerHandle(),
                    cmd->destinationOffset);
                break;
            }
            case Command::WriteTimestamp: {
                auto cmd = mCommands.NextCommand<WriteTimestampCmd>();
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // QuerySetWGPU
                wgpu.commandEncoderWriteTimestamp(
                    innerEncoder, nullptr /*ToBackend(cmd->querySet)->GetInnerHandle()*/,
                    cmd->queryIndex);
                break;
            }
            case Command::InsertDebugMarker: {
                auto cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                wgpu.commandEncoderInsertDebugMarker(innerEncoder, {label, cmd->length});
                break;
            }
            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();
                wgpu.commandEncoderPopDebugGroup(innerEncoder);
                break;
            }
            case Command::PushDebugGroup: {
                auto cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                wgpu.commandEncoderPushDebugGroup(innerEncoder, {label, cmd->length});
                break;
            }
            case Command::WriteBuffer: {
                auto cmd = mCommands.NextCommand<WriteBufferCmd>();
                auto data = mCommands.NextData<uint8_t>(cmd->size);
                wgpu.commandEncoderWriteBuffer(innerEncoder,
                                               ToBackend(cmd->buffer)->GetInnerHandle(),
                                               cmd->offset, data, cmd->size);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    // TODO(crbug.com/413053623): Store WGPUCommandBufferDescriptor and assign here.
    WGPUCommandBuffer result = wgpu.commandEncoderFinish(innerEncoder, nullptr);
    wgpu.commandEncoderRelease(innerEncoder);
    return result;
}

}  // namespace dawn::native::webgpu
