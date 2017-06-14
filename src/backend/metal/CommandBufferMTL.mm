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

#include "CommandBufferMTL.h"

#include "common/Commands.h"
#include "BufferMTL.h"
#include "DepthStencilStateMTL.h"
#include "InputStateMTL.h"
#include "MetalBackend.h"
#include "PipelineMTL.h"
#include "PipelineLayoutMTL.h"
#include "SamplerMTL.h"
#include "TextureMTL.h"

namespace backend {
namespace metal {

    namespace {
        MTLIndexType IndexFormatType(nxt::IndexFormat format) {
            switch (format) {
                case nxt::IndexFormat::Uint16:
                    return MTLIndexTypeUInt16;
                case nxt::IndexFormat::Uint32:
                    return MTLIndexTypeUInt32;
            }
        }

        struct CurrentEncoders {
            Device* device;

            id<MTLBlitCommandEncoder> blit = nil;
            id<MTLComputeCommandEncoder> compute = nil;
            id<MTLRenderCommandEncoder> render = nil;

            RenderPass* currentRenderPass = nullptr;
            Framebuffer* currentFramebuffer = nullptr;

            void FinishEncoders() {
                ASSERT(render == nil);
                if (blit != nil) {
                    [blit endEncoding];
                    blit = nil;
                }
                if (compute != nil) {
                    [compute endEncoding];
                    compute = nil;
                }
            }

            void EnsureBlit(id<MTLCommandBuffer> commandBuffer) {
                if (blit == nil) {
                    FinishEncoders();
                    blit = [commandBuffer blitCommandEncoder];
                }
            }
            void EnsureCompute(id<MTLCommandBuffer> commandBuffer) {
                if (compute == nil) {
                    FinishEncoders();
                    compute = [commandBuffer computeCommandEncoder];
                    // TODO(cwallez@chromium.org): does any state need to be reset?
                }
            }
            void BeginSubpass(id<MTLCommandBuffer> commandBuffer, uint32_t subpass) {
                ASSERT(currentRenderPass);
                if (render != nil) {
                    [render endEncoding];
                    render = nil;
                }

                const auto& info = currentRenderPass->GetSubpassInfo(subpass);

                MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
                bool usingBackbuffer = false; // HACK(kainino@chromium.org): workaround for not having depth attachments
                for (uint32_t index = 0; index < info.colorAttachments.size(); ++index) {
                    uint32_t attachment = info.colorAttachments[index];

                    // TODO(kainino@chromium.org): currently a 'null' texture view
                    // falls back to the 'back buffer' but this should go away
                    // when we have WSI.
                    id<MTLTexture> texture = nil;
                    if (auto textureView = currentFramebuffer->GetTextureView(attachment)) {
                        texture = ToBackend(textureView->GetTexture())->GetMTLTexture();
                    } else {
                        texture = device->GetCurrentTexture();
                        usingBackbuffer = true;
                    }
                    descriptor.colorAttachments[index].texture = texture;
                    descriptor.colorAttachments[index].loadAction = MTLLoadActionLoad;
                    descriptor.colorAttachments[index].storeAction = MTLStoreActionStore;
                }
                // TODO(kainino@chromium.org): load depth attachment from subpass
                if (usingBackbuffer) {
                    descriptor.depthAttachment.texture = device->GetCurrentDepthTexture();
                    descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
                    descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                }

                render = [commandBuffer renderCommandEncoderWithDescriptor:descriptor];
                // TODO(cwallez@chromium.org): does any state need to be reset?
            }
            void EndRenderPass() {
                ASSERT(render != nil);
                [render endEncoding];
                render = nil;
            }
        };
    }

    CommandBuffer::CommandBuffer(Device* device, CommandBufferBuilder* builder)
        : CommandBufferBase(builder), device(device), commands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&commands);
    }

    void CommandBuffer::FillCommands(id<MTLCommandBuffer> commandBuffer, std::unordered_set<std::mutex*>* mutexes) {
        Command type;
        Pipeline* lastPipeline = nullptr;
        id<MTLBuffer> indexBuffer = nil;
        uint32_t indexBufferOffset = 0;
        MTLIndexType indexType = MTLIndexTypeUInt32;

        CurrentEncoders encoders;
        encoders.device = device;

        uint32_t currentSubpass = 0;
        id<MTLRenderCommandEncoder> renderEncoder = nil;

        while (commands.NextCommandId(&type)) {
            switch (type) {
                case Command::AdvanceSubpass:
                    {
                        commands.NextCommand<AdvanceSubpassCmd>();
                        currentSubpass += 1;
                        encoders.BeginSubpass(commandBuffer, currentSubpass);
                    }
                    break;

                case Command::BeginRenderPass:
                    {
                        BeginRenderPassCmd* beginRenderPassCmd = commands.NextCommand<BeginRenderPassCmd>();
                        encoders.currentRenderPass = ToBackend(beginRenderPassCmd->renderPass.Get());
                        encoders.currentFramebuffer = ToBackend(beginRenderPassCmd->framebuffer.Get());
                        encoders.FinishEncoders();
                        currentSubpass = 0;
                        encoders.BeginSubpass(commandBuffer, currentSubpass);
                    }
                    break;

                case Command::CopyBufferToBuffer:
                    {
                        CopyBufferToBufferCmd* copy = commands.NextCommand<CopyBufferToBufferCmd>();

                        encoders.EnsureBlit(commandBuffer);
                        [encoders.blit
                            copyFromBuffer:ToBackend(copy->source)->GetMTLBuffer()
                            sourceOffset:copy->sourceOffset
                            toBuffer:ToBackend(copy->destination)->GetMTLBuffer()
                            destinationOffset:copy->destinationOffset
                            size:copy->size];
                    }
                    break;

                case Command::CopyBufferToTexture:
                    {
                        CopyBufferToTextureCmd* copy = commands.NextCommand<CopyBufferToTextureCmd>();
                        Buffer* buffer = ToBackend(copy->buffer.Get());
                        Texture* texture = ToBackend(copy->texture.Get());

                        unsigned rowSize = copy->width * TextureFormatPixelSize(texture->GetFormat());
                        MTLOrigin origin;
                        origin.x = copy->x;
                        origin.y = copy->y;
                        origin.z = copy->z;

                        MTLSize size;
                        size.width = copy->width;
                        size.height = copy->height;
                        size.depth = copy->depth;

                        encoders.EnsureBlit(commandBuffer);
                        [encoders.blit
                            copyFromBuffer:buffer->GetMTLBuffer()
                            sourceOffset:copy->bufferOffset
                            sourceBytesPerRow:rowSize
                            sourceBytesPerImage:(rowSize * copy->height)
                            sourceSize:size
                            toTexture:texture->GetMTLTexture()
                            destinationSlice:0
                            destinationLevel:copy->level
                            destinationOrigin:origin];
                    }
                    break;

                case Command::Dispatch:
                    {
                        DispatchCmd* dispatch = commands.NextCommand<DispatchCmd>();
                        encoders.EnsureCompute(commandBuffer);
                        ASSERT(lastPipeline->IsCompute());

                        [encoders.compute dispatchThreadgroups:MTLSizeMake(dispatch->x, dispatch->y, dispatch->z)
                            threadsPerThreadgroup: lastPipeline->GetLocalWorkGroupSize()];
                    }
                    break;

                case Command::DrawArrays:
                    {
                        DrawArraysCmd* draw = commands.NextCommand<DrawArraysCmd>();

                        ASSERT(encoders.render);
                        [encoders.render
                            drawPrimitives:MTLPrimitiveTypeTriangle
                            vertexStart:draw->firstVertex
                            vertexCount:draw->vertexCount
                            instanceCount:draw->instanceCount
                            baseInstance:draw->firstInstance];
                    }
                    break;

                case Command::DrawElements:
                    {
                        DrawElementsCmd* draw = commands.NextCommand<DrawElementsCmd>();

                        ASSERT(encoders.render);
                        [encoders.render
                            drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:draw->indexCount
                            indexType:indexType
                            indexBuffer:indexBuffer
                            indexBufferOffset:indexBufferOffset
                            instanceCount:draw->instanceCount
                            baseVertex:0
                            baseInstance:draw->firstInstance];
                    }
                    break;

                case Command::EndRenderPass:
                    {
                        commands.NextCommand<EndRenderPassCmd>();
                        encoders.EndRenderPass();
                    }
                    break;

                case Command::SetPipeline:
                    {
                        SetPipelineCmd* cmd = commands.NextCommand<SetPipelineCmd>();
                        lastPipeline = ToBackend(cmd->pipeline).Get();

                        if (lastPipeline->IsCompute()) {
                            encoders.EnsureCompute(commandBuffer);
                            lastPipeline->Encode(encoders.compute);
                        } else {
                            ASSERT(encoders.render);
                            DepthStencilState* depthStencilState = ToBackend(lastPipeline->GetDepthStencilState());
                            [encoders.render setDepthStencilState:depthStencilState->GetMTLDepthStencilState()];
                            lastPipeline->Encode(encoders.render);
                        }
                    }
                    break;

                case Command::SetPushConstants:
                    {
                        SetPushConstantsCmd* cmd = commands.NextCommand<SetPushConstantsCmd>();
                        uint32_t* valuesUInt = commands.NextData<uint32_t>(cmd->count);
                        int32_t* valuesInt = reinterpret_cast<int32_t*>(valuesUInt);
                        float* valuesFloat = reinterpret_cast<float*>(valuesUInt);

                        // TODO(kainino@chromium.org): implement SetPushConstants
                    }
                    break;

                case Command::SetStencilReference:
                    {
                        SetStencilReferenceCmd* cmd = commands.NextCommand<SetStencilReferenceCmd>();

                        ASSERT(encoders.render);

                        [encoders.render setStencilReferenceValue:cmd->reference];
                    }
                    break;

                case Command::SetBindGroup:
                    {
                        SetBindGroupCmd* cmd = commands.NextCommand<SetBindGroupCmd>();
                        BindGroup* group = ToBackend(cmd->group.Get());
                        uint32_t groupIndex = cmd->index;

                        const auto& layout = group->GetLayout()->GetBindingInfo();

                        if (lastPipeline->IsCompute()) {
                            encoders.EnsureCompute(commandBuffer);
                        } else {
                            ASSERT(encoders.render);
                        }

                        // TODO(kainino@chromium.org): Maintain buffers and offsets arrays in BindGroup so that we
                        // only have to do one setVertexBuffers and one setFragmentBuffers call here.
                        for (size_t binding = 0; binding < layout.mask.size(); ++binding) {
                            if (!layout.mask[binding]) {
                                continue;
                            }

                            auto stage = layout.visibilities[binding];
                            bool vertStage = stage & nxt::ShaderStageBit::Vertex;
                            bool fragStage = stage & nxt::ShaderStageBit::Fragment;
                            bool computeStage = stage & nxt::ShaderStageBit::Compute;
                            uint32_t vertIndex = 0;
                            uint32_t fragIndex = 0;
                            uint32_t computeIndex = 0;
                            if (vertStage) {
                                vertIndex = ToBackend(lastPipeline->GetLayout())->
                                    GetBindingIndexInfo(nxt::ShaderStage::Vertex)[groupIndex][binding];
                            }
                            if (fragStage) {
                                fragIndex = ToBackend(lastPipeline->GetLayout())->
                                    GetBindingIndexInfo(nxt::ShaderStage::Fragment)[groupIndex][binding];
                            }
                            if (computeStage) {
                                computeIndex = ToBackend(lastPipeline->GetLayout())->
                                    GetBindingIndexInfo(nxt::ShaderStage::Compute)[groupIndex][binding];
                            }

                            switch (layout.types[binding]) {
                                case nxt::BindingType::UniformBuffer:
                                case nxt::BindingType::StorageBuffer:
                                    {
                                        BufferView* view = ToBackend(group->GetBindingAsBufferView(binding));
                                        auto b = ToBackend(view->GetBuffer());
                                        mutexes->insert(&b->GetMutex());
                                        const id<MTLBuffer> buffer = b->GetMTLBuffer();
                                        const NSUInteger offset = view->GetOffset();
                                        if (vertStage) {
                                            [encoders.render
                                                setVertexBuffers:&buffer
                                                offsets:&offset
                                                withRange:NSMakeRange(vertIndex, 1)];
                                        }
                                        if (fragStage) {
                                            [encoders.render
                                                setFragmentBuffers:&buffer
                                                offsets:&offset
                                                withRange:NSMakeRange(fragIndex, 1)];
                                        }
                                        if (computeStage) {
                                            [encoders.compute
                                                setBuffers:&buffer
                                                offsets:&offset
                                                withRange:NSMakeRange(computeIndex, 1)];
                                        }

                                    }
                                    break;

                                case nxt::BindingType::Sampler:
                                    {
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
                                            [encoders.compute
                                                setSamplerState:sampler->GetMTLSamplerState()
                                                atIndex:computeIndex];
                                        }
                                    }
                                    break;

                                case nxt::BindingType::SampledTexture:
                                    {
                                        auto texture = ToBackend(group->GetBindingAsTextureView(binding)->GetTexture());
                                        if (vertStage) {
                                            [encoders.render
                                                setVertexTexture:texture->GetMTLTexture()
                                                atIndex:vertIndex];
                                        }
                                        if (fragStage) {
                                            [encoders.render
                                                setFragmentTexture:texture->GetMTLTexture()
                                                atIndex:fragIndex];
                                        }
                                        if (computeStage) {
                                            [encoders.compute
                                                setTexture:texture->GetMTLTexture()
                                                atIndex:computeIndex];
                                        }
                                    }
                                    break;
                            }
                        }
                    }
                    break;

                case Command::SetIndexBuffer:
                    {
                        SetIndexBufferCmd* cmd = commands.NextCommand<SetIndexBufferCmd>();
                        auto b = ToBackend(cmd->buffer.Get());
                        mutexes->insert(&b->GetMutex());
                        indexBuffer = b->GetMTLBuffer();
                        indexBufferOffset = cmd->offset;
                        indexType = IndexFormatType(cmd->format);
                    }
                    break;

                case Command::SetVertexBuffers:
                    {
                        SetVertexBuffersCmd* cmd = commands.NextCommand<SetVertexBuffersCmd>();
                        auto buffers = commands.NextData<Ref<BufferBase>>(cmd->count);
                        auto offsets = commands.NextData<uint32_t>(cmd->count);

                        auto inputState = lastPipeline->GetInputState();

                        std::array<id<MTLBuffer>, kMaxVertexInputs> mtlBuffers;
                        std::array<NSUInteger, kMaxVertexInputs> mtlOffsets;

                        // Perhaps an "array of vertex buffers(+offsets?)" should be
                        // a NXT API primitive to avoid reconstructing this array?
                        for (uint32_t i = 0; i < cmd->count; ++i) {
                            Buffer* buffer = ToBackend(buffers[i].Get());
                            mutexes->insert(&buffer->GetMutex());
                            mtlBuffers[i] = buffer->GetMTLBuffer();
                            mtlOffsets[i] = offsets[i];
                        }

                        ASSERT(encoders.render);
                        [encoders.render
                            setVertexBuffers:mtlBuffers.data()
                            offsets:mtlOffsets.data()
                            withRange:NSMakeRange(kMaxBindingsPerGroup + cmd->startSlot, cmd->count)];
                    }
                    break;

                case Command::TransitionBufferUsage:
                    {
                        TransitionBufferUsageCmd* cmd = commands.NextCommand<TransitionBufferUsageCmd>();

                        cmd->buffer->UpdateUsageInternal(cmd->usage);
                    }
                    break;

                case Command::TransitionTextureUsage:
                    {
                        TransitionTextureUsageCmd* cmd = commands.NextCommand<TransitionTextureUsageCmd>();

                        cmd->texture->UpdateUsageInternal(cmd->usage);
                    }
                    break;
            }
        }

        encoders.FinishEncoders();
    }

}
}
