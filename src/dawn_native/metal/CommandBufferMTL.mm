// Copyright 2017 The Dawn Authors
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

#include "dawn_native/metal/CommandBufferMTL.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/Commands.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/DepthStencilStateMTL.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/TextureMTL.h"

namespace dawn_native { namespace metal {

    namespace {

        struct GlobalEncoders {
            id<MTLBlitCommandEncoder> blit = nil;

            void Finish() {
                if (blit != nil) {
                    [blit endEncoding];
                    blit = nil;  // This will be autoreleased.
                }
            }

            void EnsureBlit(id<MTLCommandBuffer> commandBuffer) {
                if (blit == nil) {
                    blit = [commandBuffer blitCommandEncoder];
                }
            }
        };

        // Creates an autoreleased MTLRenderPassDescriptor matching desc
        MTLRenderPassDescriptor* CreateMTLRenderPassDescriptor(RenderPassDescriptorBase* desc) {
            MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

            for (uint32_t i : IterateBitSet(desc->GetColorAttachmentMask())) {
                auto& attachmentInfo = desc->GetColorAttachment(i);

                if (attachmentInfo.loadOp == dawn::LoadOp::Clear) {
                    descriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
                    descriptor.colorAttachments[i].clearColor = MTLClearColorMake(
                        attachmentInfo.clearColor[0], attachmentInfo.clearColor[1],
                        attachmentInfo.clearColor[2], attachmentInfo.clearColor[3]);
                } else {
                    descriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
                }

                // TODO(jiawei.shao@intel.com): support rendering into a layer of a texture.
                descriptor.colorAttachments[i].texture =
                    ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                descriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
            }

            if (desc->HasDepthStencilAttachment()) {
                auto& attachmentInfo = desc->GetDepthStencilAttachment();

                // TODO(jiawei.shao@intel.com): support rendering into a layer of a texture.
                id<MTLTexture> texture =
                    ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                dawn::TextureFormat format = attachmentInfo.view->GetTexture()->GetFormat();

                if (TextureFormatHasDepth(format)) {
                    descriptor.depthAttachment.texture = texture;
                    descriptor.depthAttachment.storeAction = MTLStoreActionStore;

                    if (attachmentInfo.depthLoadOp == dawn::LoadOp::Clear) {
                        descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                        descriptor.depthAttachment.clearDepth = attachmentInfo.clearDepth;
                    } else {
                        descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
                    }
                }

                if (TextureFormatHasStencil(format)) {
                    descriptor.stencilAttachment.texture = texture;
                    descriptor.stencilAttachment.storeAction = MTLStoreActionStore;

                    if (attachmentInfo.stencilLoadOp == dawn::LoadOp::Clear) {
                        descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                        descriptor.stencilAttachment.clearStencil = attachmentInfo.clearStencil;
                    } else {
                        descriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
                    }
                }
            }

            return descriptor;
        }

        // Handles a call to SetBindGroup, directing the commands to the correct encoder.
        // There is a single function that takes both encoders to factor code. Other approaches like
        // templates wouldn't work because the name of methods are different between the two encoder
        // types.
        void ApplyBindGroup(uint32_t index,
                            BindGroup* group,
                            PipelineLayout* pipelineLayout,
                            id<MTLRenderCommandEncoder> render,
                            id<MTLComputeCommandEncoder> compute) {
            const auto& layout = group->GetLayout()->GetBindingInfo();

            // TODO(kainino@chromium.org): Maintain buffers and offsets arrays in BindGroup
            // so that we only have to do one setVertexBuffers and one setFragmentBuffers
            // call here.
            for (size_t binding = 0; binding < layout.mask.size(); ++binding) {
                if (!layout.mask[binding]) {
                    continue;
                }

                auto stage = layout.visibilities[binding];
                bool hasVertStage = stage & dawn::ShaderStageBit::Vertex && render != nil;
                bool hasFragStage = stage & dawn::ShaderStageBit::Fragment && render != nil;
                bool hasComputeStage = stage & dawn::ShaderStageBit::Compute && compute != nil;

                uint32_t vertIndex = 0;
                uint32_t fragIndex = 0;
                uint32_t computeIndex = 0;

                if (hasVertStage) {
                    vertIndex = pipelineLayout->GetBindingIndexInfo(
                        dawn::ShaderStage::Vertex)[index][binding];
                }
                if (hasFragStage) {
                    fragIndex = pipelineLayout->GetBindingIndexInfo(
                        dawn::ShaderStage::Fragment)[index][binding];
                }
                if (hasComputeStage) {
                    computeIndex = pipelineLayout->GetBindingIndexInfo(
                        dawn::ShaderStage::Compute)[index][binding];
                }

                switch (layout.types[binding]) {
                    case dawn::BindingType::UniformBuffer:
                    case dawn::BindingType::StorageBuffer: {
                        BufferView* view = ToBackend(group->GetBindingAsBufferView(binding));
                        auto b = ToBackend(view->GetBuffer());
                        const id<MTLBuffer> buffer = b->GetMTLBuffer();
                        const NSUInteger offset = view->GetOffset();

                        if (hasVertStage) {
                            [render setVertexBuffers:&buffer
                                             offsets:&offset
                                           withRange:NSMakeRange(vertIndex, 1)];
                        }
                        if (hasFragStage) {
                            [render setFragmentBuffers:&buffer
                                               offsets:&offset
                                             withRange:NSMakeRange(fragIndex, 1)];
                        }
                        if (hasComputeStage) {
                            [compute setBuffers:&buffer
                                        offsets:&offset
                                      withRange:NSMakeRange(computeIndex, 1)];
                        }

                    } break;

                    case dawn::BindingType::Sampler: {
                        auto sampler = ToBackend(group->GetBindingAsSampler(binding));
                        if (hasVertStage) {
                            [render setVertexSamplerState:sampler->GetMTLSamplerState()
                                                  atIndex:vertIndex];
                        }
                        if (hasFragStage) {
                            [render setFragmentSamplerState:sampler->GetMTLSamplerState()
                                                    atIndex:fragIndex];
                        }
                        if (hasComputeStage) {
                            [compute setSamplerState:sampler->GetMTLSamplerState()
                                             atIndex:computeIndex];
                        }
                    } break;

                    case dawn::BindingType::SampledTexture: {
                        auto textureView = ToBackend(group->GetBindingAsTextureView(binding));
                        if (hasVertStage) {
                            [render setVertexTexture:textureView->GetMTLTexture()
                                             atIndex:vertIndex];
                        }
                        if (hasFragStage) {
                            [render setFragmentTexture:textureView->GetMTLTexture()
                                               atIndex:fragIndex];
                        }
                        if (hasComputeStage) {
                            [compute setTexture:textureView->GetMTLTexture() atIndex:computeIndex];
                        }
                    } break;
                }
            }
        }

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder),
          mDevice(ToBackend(builder->GetDevice())),
          mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::FillCommands(id<MTLCommandBuffer> commandBuffer) {
        GlobalEncoders encoders;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();
                    encoders.Finish();
                    EncodeComputePass(commandBuffer);
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                    encoders.Finish();
                    EncodeRenderPass(commandBuffer, ToBackend(cmd->info.Get()));
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
                    auto& copySize = copy->copySize;
                    Buffer* buffer = ToBackend(src.buffer.Get());
                    Texture* texture = ToBackend(dst.texture.Get());

                    MTLOrigin origin;
                    origin.x = dst.origin.x;
                    origin.y = dst.origin.y;
                    origin.z = dst.origin.z;

                    MTLSize size;
                    size.width = copySize.width;
                    size.height = copySize.height;
                    size.depth = copySize.depth;

                    encoders.EnsureBlit(commandBuffer);
                    [encoders.blit copyFromBuffer:buffer->GetMTLBuffer()
                                     sourceOffset:src.offset
                                sourceBytesPerRow:src.rowPitch
                              sourceBytesPerImage:(src.rowPitch * copySize.height)
                                       sourceSize:size
                                        toTexture:texture->GetMTLTexture()
                                 destinationSlice:dst.slice
                                 destinationLevel:dst.level
                                destinationOrigin:origin];
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Texture* texture = ToBackend(src.texture.Get());
                    Buffer* buffer = ToBackend(dst.buffer.Get());

                    MTLOrigin origin;
                    origin.x = src.origin.x;
                    origin.y = src.origin.y;
                    origin.z = src.origin.z;

                    MTLSize size;
                    size.width = copySize.width;
                    size.height = copySize.height;
                    size.depth = copySize.depth;

                    encoders.EnsureBlit(commandBuffer);
                    [encoders.blit copyFromTexture:texture->GetMTLTexture()
                                       sourceSlice:src.slice
                                       sourceLevel:src.level
                                      sourceOrigin:origin
                                        sourceSize:size
                                          toBuffer:buffer->GetMTLBuffer()
                                 destinationOffset:dst.offset
                            destinationBytesPerRow:dst.rowPitch
                          destinationBytesPerImage:dst.rowPitch * copySize.height];
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        encoders.Finish();
    }

    void CommandBuffer::EncodeComputePass(id<MTLCommandBuffer> commandBuffer) {
        ComputePipeline* lastPipeline = nullptr;
        std::array<uint32_t, kMaxPushConstants> pushConstants;

        // Will be autoreleased
        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];

        // Set default values for push constants
        pushConstants.fill(0);
        [encoder setBytes:&pushConstants length:sizeof(uint32_t) * kMaxPushConstants atIndex:0];

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    [encoder endEncoding];
                    return;
                } break;

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    [encoder dispatchThreadgroups:MTLSizeMake(dispatch->x, dispatch->y, dispatch->z)
                            threadsPerThreadgroup:lastPipeline->GetLocalWorkGroupSize()];
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();

                    lastPipeline->Encode(encoder);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mCommands.NextCommand<SetPushConstantsCmd>();
                    uint32_t* values = mCommands.NextData<uint32_t>(cmd->count);

                    if (cmd->stages & dawn::ShaderStageBit::Compute) {
                        memcpy(&pushConstants[cmd->offset], values, cmd->count * sizeof(uint32_t));

                        [encoder setBytes:&pushConstants
                                   length:sizeof(uint32_t) * kMaxPushConstants
                                  atIndex:0];
                    }
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    ApplyBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                   ToBackend(lastPipeline->GetLayout()), nil, encoder);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndComputePass should have been called
        UNREACHABLE();
    }

    void CommandBuffer::EncodeRenderPass(id<MTLCommandBuffer> commandBuffer,
                                         RenderPassDescriptorBase* renderPass) {
        RenderPipeline* lastPipeline = nullptr;
        id<MTLBuffer> indexBuffer = nil;
        uint32_t indexBufferBaseOffset = 0;

        std::array<uint32_t, kMaxPushConstants> vertexPushConstants;
        std::array<uint32_t, kMaxPushConstants> fragmentPushConstants;

        // This will be autoreleased
        id<MTLRenderCommandEncoder> encoder = [commandBuffer
            renderCommandEncoderWithDescriptor:CreateMTLRenderPassDescriptor(renderPass)];

        // Set default values for push constants
        vertexPushConstants.fill(0);
        fragmentPushConstants.fill(0);

        [encoder setVertexBytes:&vertexPushConstants
                         length:sizeof(uint32_t) * kMaxPushConstants
                        atIndex:0];
        [encoder setFragmentBytes:&fragmentPushConstants
                           length:sizeof(uint32_t) * kMaxPushConstants
                          atIndex:0];

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    [encoder endEncoding];
                    return;
                } break;

                case Command::DrawArrays: {
                    DrawArraysCmd* draw = mCommands.NextCommand<DrawArraysCmd>();

                    [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                vertexStart:draw->firstVertex
                                vertexCount:draw->vertexCount
                              instanceCount:draw->instanceCount
                               baseInstance:draw->firstInstance];
                } break;

                case Command::DrawElements: {
                    DrawElementsCmd* draw = mCommands.NextCommand<DrawElementsCmd>();
                    size_t formatSize = IndexFormatSize(lastPipeline->GetIndexFormat());

                    [encoder
                        drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                   indexCount:draw->indexCount
                                    indexType:lastPipeline->GetMTLIndexType()
                                  indexBuffer:indexBuffer
                            indexBufferOffset:indexBufferBaseOffset + draw->firstIndex * formatSize
                                instanceCount:draw->instanceCount
                                   baseVertex:0
                                 baseInstance:draw->firstInstance];
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();

                    DepthStencilState* depthStencilState =
                        ToBackend(lastPipeline->GetDepthStencilState());
                    [encoder setDepthStencilState:depthStencilState->GetMTLDepthStencilState()];
                    lastPipeline->Encode(encoder);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mCommands.NextCommand<SetPushConstantsCmd>();
                    uint32_t* values = mCommands.NextData<uint32_t>(cmd->count);

                    if (cmd->stages & dawn::ShaderStageBit::Vertex) {
                        memcpy(&vertexPushConstants[cmd->offset], values,
                               cmd->count * sizeof(uint32_t));
                        [encoder setVertexBytes:&vertexPushConstants
                                         length:sizeof(uint32_t) * kMaxPushConstants
                                        atIndex:0];
                    }

                    if (cmd->stages & dawn::ShaderStageBit::Fragment) {
                        memcpy(&fragmentPushConstants[cmd->offset], values,
                               cmd->count * sizeof(uint32_t));
                        [encoder setFragmentBytes:&fragmentPushConstants
                                           length:sizeof(uint32_t) * kMaxPushConstants
                                          atIndex:0];
                    }
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                    [encoder setStencilReferenceValue:cmd->reference];
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    MTLScissorRect rect;
                    rect.x = cmd->x;
                    rect.y = cmd->y;
                    rect.width = cmd->width;
                    rect.height = cmd->height;

                    [encoder setScissorRect:rect];
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    [encoder setBlendColorRed:cmd->r green:cmd->g blue:cmd->b alpha:cmd->a];
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    ApplyBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                   ToBackend(lastPipeline->GetLayout()), encoder, nil);
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
                    // a Dawn API primitive to avoid reconstructing this array?
                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        Buffer* buffer = ToBackend(buffers[i].Get());
                        mtlBuffers[i] = buffer->GetMTLBuffer();
                        mtlOffsets[i] = offsets[i];
                    }

                    [encoder setVertexBuffers:mtlBuffers.data()
                                      offsets:mtlOffsets.data()
                                    withRange:NSMakeRange(kMaxBindingsPerGroup + cmd->startSlot,
                                                          cmd->count)];
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::metal
