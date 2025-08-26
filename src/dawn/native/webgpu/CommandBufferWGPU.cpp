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
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

namespace {

WGPUExtent3D ToWGPU(const Extent3D& extent) {
    return {
        .width = extent.width,
        .height = extent.height,
        .depthOrArrayLayers = extent.depthOrArrayLayers,
    };
}

WGPUOrigin3D ToWGPU(const Origin3D& origin) {
    return {
        .x = origin.x,
        .y = origin.y,
        .z = origin.z,
    };
}

WGPUColor ToWGPU(const dawn::native::Color& color) {
    return {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
}

WGPUTexelCopyBufferInfo ToWGPU(const BufferCopy& copy) {
    return {
        .layout =
            {
                .offset = copy.offset,
                .bytesPerRow = copy.bytesPerRow,
                .rowsPerImage = copy.rowsPerImage,
            },
        .buffer = ToBackend(copy.buffer)->GetInnerHandle(),
    };
}

WGPUTextureAspect ToWGPU(const Aspect aspect) {
    switch (aspect) {
        case Aspect::Depth:
            return WGPUTextureAspect_DepthOnly;
        case Aspect::Stencil:
            return WGPUTextureAspect_StencilOnly;
        case Aspect::Plane0:
            return WGPUTextureAspect_Plane0Only;
        case Aspect::Plane1:
            return WGPUTextureAspect_Plane1Only;
        case Aspect::Plane2:
            return WGPUTextureAspect_Plane2Only;
        default:
            return WGPUTextureAspect_All;
    }
}

WGPULoadOp ToWGPU(const wgpu::LoadOp op) {
    switch (op) {
        case wgpu::LoadOp::Load:
            return WGPULoadOp_Load;
        case wgpu::LoadOp::Clear:
            return WGPULoadOp_Clear;
        case wgpu::LoadOp::ExpandResolveTexture:
            DAWN_UNREACHABLE();  // TODO(crbug.com/440123094): Fix when test that uses this is
                                 // enabled.
        default:
            return WGPULoadOp_Undefined;
    }
}

WGPUStoreOp ToWGPU(const wgpu::StoreOp op) {
    switch (op) {
        case wgpu::StoreOp::Store:
            return WGPUStoreOp_Store;
        case wgpu::StoreOp::Discard:
            return WGPUStoreOp_Discard;
        default:
            return WGPUStoreOp_Undefined;
    }
}

WGPUPassTimestampWrites ToWGPU(const TimestampWrites& writes) {
    return {
        .nextInChain = nullptr,
        // TODO(crbug.com/440123094): Do this when GetInnerHandle is implemented for QuerySetWGPU
        .querySet = nullptr /*ToBackend(writes.querySet)->GetInnerHandle()*/,
        .beginningOfPassWriteIndex = writes.beginningOfPassWriteIndex,
        .endOfPassWriteIndex = writes.endOfPassWriteIndex,
    };
}

WGPUTexelCopyTextureInfo ToWGPU(const TextureCopy& copy) {
    return {
        .texture = ToBackend(copy.texture)->GetInnerHandle(),
        .mipLevel = copy.mipLevel,
        .origin = ToWGPU(copy.origin),
        .aspect = ToWGPU(copy.aspect),
    };
}

WGPURenderPassColorAttachment ToWGPU(const RenderPassColorAttachmentInfo& info) {
    return {
        .nextInChain = nullptr,
        // TODO(crbug.com/440123094): Do this when GetInnerHandle is implemented for TextureViewWGPU
        .view = nullptr /* ToBackend(info.view)->GetInnerHandle()*/,
        .depthSlice = info.depthSlice,
        // TODO(crbug.com/440123094): Do this when GetInnerHandle is implemented for TextureViewWGPU
        .resolveTarget = nullptr /* ToBackend(info.resolveTarget)->GetInnerHandle()*/,
        .loadOp = ToWGPU(info.loadOp),
        .storeOp = ToWGPU(info.storeOp),
        .clearValue = ToWGPU(info.clearColor),
    };
}

WGPURenderPassDepthStencilAttachment ToWGPU(const RenderPassDepthStencilAttachmentInfo& info) {
    return {
        .nextInChain = nullptr,
        // TODO(crbug.com/440123094): Do this when GetInnerHandle is implemented for TextureViewWGPU
        .view = nullptr /* ToBackend(info.view)->GetInnerHandle()*/,
        .depthLoadOp = ToWGPU(info.depthLoadOp),
        .depthStoreOp = ToWGPU(info.depthStoreOp),
        .depthClearValue = info.clearDepth,
        .depthReadOnly = info.depthReadOnly,
        .stencilLoadOp = ToWGPU(info.stencilLoadOp),
        .stencilStoreOp = ToWGPU(info.stencilStoreOp),
        .stencilClearValue = info.clearStencil,
        .stencilReadOnly = info.stencilReadOnly,
    };
}

void EncodeComputePass(const DawnProcTable& wgpu,
                       WGPUCommandEncoder innerEncoder,
                       CommandIterator& commands,
                       BeginComputePassCmd* computePassCmd) {
    WGPUPassTimestampWrites timestampWrites = ToWGPU(computePassCmd->timestampWrites);
    WGPUComputePassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(computePassCmd->label),
        .timestampWrites = &timestampWrites,
    };
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
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // ComputePipelineWGPU
                /*auto cmd = */ commands.NextCommand<SetComputePipelineCmd>();
                wgpu.computePassEncoderSetPipeline(
                    passEncoder, nullptr /*ToBackend(cmd->pipeline)->GetInnerHandle()*/);
                break;
            }

            case Command::SetBindGroup: {
                auto cmd = commands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = commands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
                // BindGroupWGPU
                wgpu.computePassEncoderSetBindGroup(
                    passEncoder, static_cast<uint32_t>(cmd->index),
                    nullptr /*ToBackend(cmd->group)->GetInnerHandle()*/, cmd->dynamicOffsetCount,
                    dynamicOffsets);
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

void EncodeRenderPass(const DawnProcTable& wgpu,
                      WGPUCommandEncoder innerEncoder,
                      CommandIterator& commands,
                      BeginRenderPassCmd* renderPassCmd) {
    std::vector<WGPURenderPassColorAttachment> colorAttachments;

    for (auto i : renderPassCmd->attachmentState->GetColorAttachmentsMask()) {
        colorAttachments.push_back(ToWGPU(renderPassCmd->colorAttachments[i]));
    }

    colorAttachments.reserve(colorAttachments.size());
    WGPURenderPassDepthStencilAttachment depthStencilAttachment =
        ToWGPU(renderPassCmd->depthStencilAttachment);
    WGPUPassTimestampWrites timestampWrites = ToWGPU(renderPassCmd->timestampWrites);

    WGPURenderPassDescriptor passDescriptor{
        .nextInChain = nullptr,
        .label = ToOutputStringView(renderPassCmd->label),
        .colorAttachmentCount = colorAttachments.size(),
        .colorAttachments = colorAttachments.data(),
        .depthStencilAttachment = &depthStencilAttachment,
        // TODO(crbug.com/440123094): remove nullptr when GetInnerHandle is implemented for
        // QuerySetWGPU
        .occlusionQuerySet = nullptr /*ToBackend(cmd->occlusionQuerySet)->GetInnerHandle()*/,
        .timestampWrites = &timestampWrites,
    };
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

WGPUCommandBuffer CommandBuffer::Encode() {
    auto& wgpu = ToBackend(GetDevice())->wgpu;

    // TODO(crbug.com/413053623): Use stored command encoder descriptor
    WGPUCommandEncoder innerEncoder =
        wgpu.deviceCreateCommandEncoder(ToBackend(GetDevice())->GetInnerHandle(), nullptr);

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();
                EncodeComputePass(wgpu, innerEncoder, mCommands, cmd);
                break;
            }
            case Command::BeginRenderPass: {
                auto cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                EncodeRenderPass(wgpu, innerEncoder, mCommands, cmd);
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
