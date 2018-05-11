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

#include "backend/metal/CommandBufferMTL.h"

#include "backend/Commands.h"
#include "backend/metal/BufferMTL.h"
#include "backend/metal/ComputePipelineMTL.h"
#include "backend/metal/DepthStencilStateMTL.h"
#include "backend/metal/InputStateMTL.h"
#include "backend/metal/MetalBackend.h"
#include "backend/metal/PipelineLayoutMTL.h"
#include "backend/metal/RenderPipelineMTL.h"
#include "backend/metal/SamplerMTL.h"
#include "backend/metal/TextureMTL.h"

namespace backend { namespace metal {

    namespace {
        struct CurrentEncoders {
            Device* device;

            id<MTLBlitCommandEncoder> blit = nil;
            id<MTLComputeCommandEncoder> compute = nil;
            id<MTLRenderCommandEncoder> render = nil;

            void EnsureNoBlitEncoder() {
                ASSERT(render == nil);
                ASSERT(compute == nil);
                if (blit != nil) {
                    [blit endEncoding];
                    blit = nil;  // This will be autoreleased.
                }
            }

            void EnsureBlit(id<MTLCommandBuffer> commandBuffer) {
                ASSERT(render == nil);
                ASSERT(compute == nil);
                if (blit == nil) {
                    blit = [commandBuffer blitCommandEncoder];
                }
            }

            void BeginCompute(id<MTLCommandBuffer> commandBuffer) {
                EnsureNoBlitEncoder();
                compute = [commandBuffer computeCommandEncoder];
                // TODO(cwallez@chromium.org): does any state need to be reset?
            }

            void EndCompute() {
                ASSERT(compute != nil);
                [compute endEncoding];
                compute = nil;  // This will be autoreleased.
            }

            void BeginRenderPass(id<MTLCommandBuffer> commandBuffer, RenderPassDescriptor* info) {
                if (render != nil) {
                    [render endEncoding];
                    render = nil;  // This will be autoreleased.
                }

                MTLRenderPassDescriptor* descriptor =
                    [MTLRenderPassDescriptor renderPassDescriptor];

                for (uint32_t i : IterateBitSet(info->GetColorAttachmentMask())) {
                    auto& attachmentInfo = info->GetColorAttachment(i);

                    if (attachmentInfo.loadOp == nxt::LoadOp::Clear) {
                        descriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
                        descriptor.colorAttachments[i].clearColor = MTLClearColorMake(
                            attachmentInfo.clearColor[0], attachmentInfo.clearColor[1],
                            attachmentInfo.clearColor[2], attachmentInfo.clearColor[3]);
                    } else {
                        descriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
                    }

                    descriptor.colorAttachments[i].texture =
                        ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                    descriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
                }

                if (info->HasDepthStencilAttachment()) {
                    auto& attachmentInfo = info->GetDepthStencilAttachment();

                    id<MTLTexture> texture =
                        ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                    nxt::TextureFormat format = attachmentInfo.view->GetTexture()->GetFormat();

                    if (TextureFormatHasDepth(format)) {
                        descriptor.depthAttachment.texture = texture;
                        descriptor.depthAttachment.storeAction = MTLStoreActionStore;

                        if (attachmentInfo.depthLoadOp == nxt::LoadOp::Clear) {
                            descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                            descriptor.depthAttachment.clearDepth = attachmentInfo.clearDepth;
                        } else {
                            descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
                        }
                    }

                    if (TextureFormatHasStencil(format)) {
                        descriptor.stencilAttachment.texture = texture;
                        descriptor.stencilAttachment.storeAction = MTLStoreActionStore;

                        if (attachmentInfo.stencilLoadOp == nxt::LoadOp::Clear) {
                            descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                            descriptor.stencilAttachment.clearStencil = attachmentInfo.clearStencil;
                        } else {
                            descriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
                        }
                    }
                }

                render = [commandBuffer renderCommandEncoderWithDescriptor:descriptor];
                // TODO(cwallez@chromium.org): does any state need to be reset?
            }

            void EndRenderPass() {
                ASSERT(render != nil);
                [render endEncoding];
                render = nil;  // This will be autoreleased.
            }
        };
    }

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder),
          mDevice(ToBackend(builder->GetDevice())),
          mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::FillCommands(id<MTLCommandBuffer> commandBuffer) {
        Command type;
        ComputePipeline* lastComputePipeline = nullptr;
        RenderPipeline* lastRenderPipeline = nullptr;
        id<MTLBuffer> indexBuffer = nil;
        uint32_t indexBufferBaseOffset = 0;

        CurrentEncoders encoders;
        encoders.device = mDevice;

        PerStage<std::array<uint32_t, kMaxPushConstants>> pushConstants;

        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();
                    encoders.BeginCompute(commandBuffer);

                    pushConstants[nxt::ShaderStage::Compute].fill(0);
                    [encoders.compute setBytes:&pushConstants[nxt::ShaderStage::Compute]
                                        length:sizeof(uint32_t) * kMaxPushConstants
                                       atIndex:0];
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* beginRenderPassCmd =
                        mCommands.NextCommand<BeginRenderPassCmd>();

                    RenderPassDescriptor* info = ToBackend(beginRenderPassCmd->info.Get());

                    encoders.EnsureNoBlitEncoder();
                    encoders.BeginRenderPass(commandBuffer, info);

                    pushConstants[nxt::ShaderStage::Vertex].fill(0);
                    pushConstants[nxt::ShaderStage::Fragment].fill(0);

                    [encoders.render setVertexBytes:&pushConstants[nxt::ShaderStage::Vertex]
                                             length:sizeof(uint32_t) * kMaxPushConstants
                                            atIndex:0];
                    [encoders.render setFragmentBytes:&pushConstants[nxt::ShaderStage::Fragment]
                                               length:sizeof(uint32_t) * kMaxPushConstants
                                              atIndex:0];
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    encoders.EnsureBlit(commandBuffer);
                    [encoders.blit copyFromBuffer:ToBackend(src.buffer)->GetMTLBuffer()
                                     sourceOffset:src.offset
                                         toBuffer:ToBackend(dst.buffer)->GetMTLBuffer()
                                destinationOffset:dst.offset
                                             size:copy->size];
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    Buffer* buffer = ToBackend(src.buffer.Get());
                    Texture* texture = ToBackend(dst.texture.Get());

                    MTLOrigin origin;
                    origin.x = dst.x;
                    origin.y = dst.y;
                    origin.z = dst.z;

                    MTLSize size;
                    size.width = dst.width;
                    size.height = dst.height;
                    size.depth = dst.depth;

                    encoders.EnsureBlit(commandBuffer);
                    [encoders.blit copyFromBuffer:buffer->GetMTLBuffer()
                                     sourceOffset:src.offset
                                sourceBytesPerRow:copy->rowPitch
                              sourceBytesPerImage:(copy->rowPitch * dst.height)
                                       sourceSize:size
                                        toTexture:texture->GetMTLTexture()
                                 destinationSlice:0
                                 destinationLevel:dst.level
                                destinationOrigin:origin];
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    Texture* texture = ToBackend(src.texture.Get());
                    Buffer* buffer = ToBackend(dst.buffer.Get());

                    MTLOrigin origin;
                    origin.x = src.x;
                    origin.y = src.y;
                    origin.z = src.z;

                    MTLSize size;
                    size.width = src.width;
                    size.height = src.height;
                    size.depth = src.depth;

                    encoders.EnsureBlit(commandBuffer);
                    [encoders.blit copyFromTexture:texture->GetMTLTexture()
                                       sourceSlice:0
                                       sourceLevel:src.level
                                      sourceOrigin:origin
                                        sourceSize:size
                                          toBuffer:buffer->GetMTLBuffer()
                                 destinationOffset:dst.offset
                            destinationBytesPerRow:copy->rowPitch
                          destinationBytesPerImage:copy->rowPitch * src.height];
                } break;

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    ASSERT(encoders.compute);

                    [encoders.compute
                         dispatchThreadgroups:MTLSizeMake(dispatch->x, dispatch->y, dispatch->z)
                        threadsPerThreadgroup:lastComputePipeline->GetLocalWorkGroupSize()];
                } break;

                case Command::DrawArrays: {
                    DrawArraysCmd* draw = mCommands.NextCommand<DrawArraysCmd>();

                    ASSERT(encoders.render);
                    [encoders.render drawPrimitives:lastRenderPipeline->GetMTLPrimitiveTopology()
                                        vertexStart:draw->firstVertex
                                        vertexCount:draw->vertexCount
                                      instanceCount:draw->instanceCount
                                       baseInstance:draw->firstInstance];
                } break;

                case Command::DrawElements: {
                    DrawElementsCmd* draw = mCommands.NextCommand<DrawElementsCmd>();
                    size_t formatSize = IndexFormatSize(lastRenderPipeline->GetIndexFormat());

                    ASSERT(encoders.render);
                    [encoders.render
                        drawIndexedPrimitives:lastRenderPipeline->GetMTLPrimitiveTopology()
                                   indexCount:draw->indexCount
                                    indexType:lastRenderPipeline->GetMTLIndexType()
                                  indexBuffer:indexBuffer
                            indexBufferOffset:indexBufferBaseOffset + draw->firstIndex * formatSize
                                instanceCount:draw->instanceCount
                                   baseVertex:0
                                 baseInstance:draw->firstInstance];
                } break;

                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    encoders.EndCompute();
                } break;

                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    encoders.EndRenderPass();
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastComputePipeline = ToBackend(cmd->pipeline).Get();

                    ASSERT(encoders.compute);
                    lastComputePipeline->Encode(encoders.compute);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    lastRenderPipeline = ToBackend(cmd->pipeline).Get();

                    ASSERT(encoders.render);
                    DepthStencilState* depthStencilState =
                        ToBackend(lastRenderPipeline->GetDepthStencilState());
                    [encoders.render
                        setDepthStencilState:depthStencilState->GetMTLDepthStencilState()];
                    lastRenderPipeline->Encode(encoders.render);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mCommands.NextCommand<SetPushConstantsCmd>();
                    uint32_t* values = mCommands.NextData<uint32_t>(cmd->count);

                    for (auto stage : IterateStages(cmd->stages)) {
                        memcpy(&pushConstants[stage][cmd->offset], values,
                               cmd->count * sizeof(uint32_t));

                        switch (stage) {
                            case nxt::ShaderStage::Compute:
                                ASSERT(encoders.compute);
                                [encoders.compute setBytes:&pushConstants[nxt::ShaderStage::Compute]
                                                    length:sizeof(uint32_t) * kMaxPushConstants
                                                   atIndex:0];
                                break;
                            case nxt::ShaderStage::Fragment:
                                ASSERT(encoders.render);
                                [encoders.render
                                    setFragmentBytes:&pushConstants[nxt::ShaderStage::Fragment]
                                              length:sizeof(uint32_t) * kMaxPushConstants
                                             atIndex:0];
                                break;
                            case nxt::ShaderStage::Vertex:
                                ASSERT(encoders.render);
                                [encoders.render
                                    setVertexBytes:&pushConstants[nxt::ShaderStage::Vertex]
                                            length:sizeof(uint32_t) * kMaxPushConstants
                                           atIndex:0];
                                break;
                            default:
                                UNREACHABLE();
                                break;
                        }
                    }
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();

                    ASSERT(encoders.render);
                    [encoders.render setStencilReferenceValue:cmd->reference];
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    MTLScissorRect rect;
                    rect.x = cmd->x;
                    rect.y = cmd->y;
                    rect.width = cmd->width;
                    rect.height = cmd->height;

                    ASSERT(encoders.render);
                    [encoders.render setScissorRect:rect];
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();

                    ASSERT(encoders.render);
                    [encoders.render setBlendColorRed:cmd->r green:cmd->g blue:cmd->b alpha:cmd->a];
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    BindGroup* group = ToBackend(cmd->group.Get());
                    uint32_t groupIndex = cmd->index;

                    const auto& layout = group->GetLayout()->GetBindingInfo();

                    // TODO(kainino@chromium.org): Maintain buffers and offsets arrays in BindGroup
                    // so that we only have to do one setVertexBuffers and one setFragmentBuffers
                    // call here.
                    for (size_t binding = 0; binding < layout.mask.size(); ++binding) {
                        if (!layout.mask[binding]) {
                            continue;
                        }

                        auto stage = layout.visibilities[binding];
                        bool vertStage =
                            stage & nxt::ShaderStageBit::Vertex && lastRenderPipeline != nullptr;
                        bool fragStage =
                            stage & nxt::ShaderStageBit::Fragment && lastRenderPipeline != nullptr;
                        bool computeStage =
                            stage & nxt::ShaderStageBit::Compute && lastComputePipeline != nullptr;
                        uint32_t vertIndex = 0;
                        uint32_t fragIndex = 0;
                        uint32_t computeIndex = 0;
                        if (vertStage) {
                            ASSERT(lastRenderPipeline != nullptr);
                            vertIndex = ToBackend(lastRenderPipeline->GetLayout())
                                            ->GetBindingIndexInfo(
                                                nxt::ShaderStage::Vertex)[groupIndex][binding];
                        }
                        if (fragStage) {
                            ASSERT(lastRenderPipeline != nullptr);
                            fragIndex = ToBackend(lastRenderPipeline->GetLayout())
                                            ->GetBindingIndexInfo(
                                                nxt::ShaderStage::Fragment)[groupIndex][binding];
                        }
                        if (computeStage) {
                            ASSERT(lastComputePipeline != nullptr);
                            computeIndex = ToBackend(lastComputePipeline->GetLayout())
                                               ->GetBindingIndexInfo(
                                                   nxt::ShaderStage::Compute)[groupIndex][binding];
                        }

                        switch (layout.types[binding]) {
                            case nxt::BindingType::UniformBuffer:
                            case nxt::BindingType::StorageBuffer: {
                                BufferView* view =
                                    ToBackend(group->GetBindingAsBufferView(binding));
                                auto b = ToBackend(view->GetBuffer());
                                const id<MTLBuffer> buffer = b->GetMTLBuffer();
                                const NSUInteger offset = view->GetOffset();
                                if (vertStage) {
                                    [encoders.render setVertexBuffers:&buffer
                                                              offsets:&offset
                                                            withRange:NSMakeRange(vertIndex, 1)];
                                }
                                if (fragStage) {
                                    [encoders.render setFragmentBuffers:&buffer
                                                                offsets:&offset
                                                              withRange:NSMakeRange(fragIndex, 1)];
                                }
                                if (computeStage) {
                                    [encoders.compute setBuffers:&buffer
                                                         offsets:&offset
                                                       withRange:NSMakeRange(computeIndex, 1)];
                                }

                            } break;

                            case nxt::BindingType::Sampler: {
                                auto sampler = ToBackend(group->GetBindingAsSampler(binding));
                                if (vertStage) {
                                    [encoders.render
                                        setVertexSamplerState:sampler->GetMTLSamplerState()
                                                      atIndex:vertIndex];
                                }
                                if (fragStage) {
                                    [encoders.render
                                        setFragmentSamplerState:sampler->GetMTLSamplerState()
                                                        atIndex:fragIndex];
                                }
                                if (computeStage) {
                                    [encoders.compute setSamplerState:sampler->GetMTLSamplerState()
                                                              atIndex:computeIndex];
                                }
                            } break;

                            case nxt::BindingType::SampledTexture: {
                                auto texture = ToBackend(
                                    group->GetBindingAsTextureView(binding)->GetTexture());
                                if (vertStage) {
                                    [encoders.render setVertexTexture:texture->GetMTLTexture()
                                                              atIndex:vertIndex];
                                }
                                if (fragStage) {
                                    [encoders.render setFragmentTexture:texture->GetMTLTexture()
                                                                atIndex:fragIndex];
                                }
                                if (computeStage) {
                                    [encoders.compute setTexture:texture->GetMTLTexture()
                                                         atIndex:computeIndex];
                                }
                            } break;
                        }
                    }
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mCommands.NextCommand<SetIndexBufferCmd>();
                    auto b = ToBackend(cmd->buffer.Get());
                    indexBuffer = b->GetMTLBuffer();
                    indexBufferBaseOffset = cmd->offset;
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mCommands.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mCommands.NextData<Ref<BufferBase>>(cmd->count);
                    auto offsets = mCommands.NextData<uint32_t>(cmd->count);

                    std::array<id<MTLBuffer>, kMaxVertexInputs> mtlBuffers;
                    std::array<NSUInteger, kMaxVertexInputs> mtlOffsets;

                    // Perhaps an "array of vertex buffers(+offsets?)" should be
                    // a NXT API primitive to avoid reconstructing this array?
                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        Buffer* buffer = ToBackend(buffers[i].Get());
                        mtlBuffers[i] = buffer->GetMTLBuffer();
                        mtlOffsets[i] = offsets[i];
                    }

                    ASSERT(encoders.render);
                    [encoders.render
                        setVertexBuffers:mtlBuffers.data()
                                 offsets:mtlOffsets.data()
                               withRange:NSMakeRange(kMaxBindingsPerGroup + cmd->startSlot,
                                                     cmd->count)];
                } break;

                case Command::TransitionBufferUsage: {
                    TransitionBufferUsageCmd* cmd =
                        mCommands.NextCommand<TransitionBufferUsageCmd>();

                    cmd->buffer->UpdateUsageInternal(cmd->usage);
                } break;

                case Command::TransitionTextureUsage: {
                    TransitionTextureUsageCmd* cmd =
                        mCommands.NextCommand<TransitionTextureUsageCmd>();

                    cmd->texture->UpdateUsageInternal(cmd->usage);
                } break;
            }
        }

        encoders.EnsureNoBlitEncoder();
        ASSERT(encoders.render == nil);
        ASSERT(encoders.compute == nil);
    }

}}  // namespace backend::metal
