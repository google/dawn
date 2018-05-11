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

#include "backend/vulkan/CommandBufferVk.h"

#include "backend/Commands.h"
#include "backend/vulkan/BindGroupVk.h"
#include "backend/vulkan/BufferVk.h"
#include "backend/vulkan/ComputePipelineVk.h"
#include "backend/vulkan/PipelineLayoutVk.h"
#include "backend/vulkan/RenderPassDescriptorVk.h"
#include "backend/vulkan/RenderPipelineVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {

        VkIndexType VulkanIndexType(nxt::IndexFormat format) {
            switch (format) {
                case nxt::IndexFormat::Uint16:
                    return VK_INDEX_TYPE_UINT16;
                case nxt::IndexFormat::Uint32:
                    return VK_INDEX_TYPE_UINT32;
                default:
                    UNREACHABLE();
            }
        }

        VkBufferImageCopy ComputeBufferImageCopyRegion(uint32_t rowPitch,
                                                       const BufferCopyLocation& bufferLocation,
                                                       const TextureCopyLocation& textureLocation) {
            const Texture* texture = ToBackend(textureLocation.texture).Get();

            VkBufferImageCopy region;

            region.bufferOffset = bufferLocation.offset;
            // In Vulkan the row length is in texels while it is in bytes for NXT
            region.bufferRowLength = rowPitch / TextureFormatPixelSize(texture->GetFormat());
            region.bufferImageHeight = rowPitch * textureLocation.height;

            region.imageSubresource.aspectMask = texture->GetVkAspectMask();
            region.imageSubresource.mipLevel = textureLocation.level;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset.x = textureLocation.x;
            region.imageOffset.y = textureLocation.y;
            region.imageOffset.z = textureLocation.z;

            region.imageExtent.width = textureLocation.width;
            region.imageExtent.height = textureLocation.height;
            region.imageExtent.depth = textureLocation.depth;

            return region;
        }

        class DescriptorSetTracker {
          public:
            void OnSetBindGroup(uint32_t index, VkDescriptorSet set) {
                mDirtySets.set(index);
                mSets[index] = set;
            }

            void OnBeginPass() {
                // All bindgroups will have to be bound in the pass before any draw / dispatch.
                // Resetting the layout and ensures nothing gets propagated from an earlier pass
                // to this pass.
                mCurrentLayout = nullptr;
            }

            void OnPipelineLayoutChange(PipelineLayout* layout) {
                if (layout == mCurrentLayout) {
                    return;
                }

                if (mCurrentLayout == nullptr) {
                    // We're at the beginning of a pass so all bind groups will be set before any
                    // draw / dispatch. Still clear the dirty sets to avoid leftover dirty sets
                    // from previous passes.
                    mDirtySets.reset();
                } else {
                    // Bindgroups that are not inherited will be set again before any draw or
                    // dispatch. Resetting the bits also makes sure we don't have leftover dirty
                    // bindgroups that don't exist in the pipeline layout.
                    mDirtySets &= ~layout->InheritedGroupsMask(mCurrentLayout);
                }
                mCurrentLayout = layout;
            }

            void Flush(Device* device, VkCommandBuffer commands, VkPipelineBindPoint bindPoint) {
                for (uint32_t dirtyIndex : IterateBitSet(mDirtySets)) {
                    device->fn.CmdBindDescriptorSets(commands, bindPoint,
                                                     mCurrentLayout->GetHandle(), dirtyIndex, 1,
                                                     &mSets[dirtyIndex], 0, nullptr);
                }
                mDirtySets.reset();
            }

          private:
            PipelineLayout* mCurrentLayout = nullptr;
            std::array<VkDescriptorSet, kMaxBindGroups> mSets;
            std::bitset<kMaxBindGroups> mDirtySets;
        };

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::RecordCommands(VkCommandBuffer commands) {
        Device* device = ToBackend(GetDevice());

        DescriptorSetTracker descriptorSets;
        RenderPipeline* lastRenderPipeline = nullptr;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    VkBufferCopy region;
                    region.srcOffset = src.offset;
                    region.dstOffset = dst.offset;
                    region.size = copy->size;

                    VkBuffer srcHandle = ToBackend(src.buffer)->GetHandle();
                    VkBuffer dstHandle = ToBackend(dst.buffer)->GetHandle();
                    device->fn.CmdCopyBuffer(commands, srcHandle, dstHandle, 1, &region);
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    VkBuffer srcBuffer = ToBackend(src.buffer)->GetHandle();
                    VkImage dstImage = ToBackend(dst.texture)->GetHandle();
                    VkBufferImageCopy region =
                        ComputeBufferImageCopyRegion(copy->rowPitch, src, dst);

                    // The image is written to so the NXT guarantees make sure it is in the
                    // TRANSFER_DST_OPTIMAL layout
                    device->fn.CmdCopyBufferToImage(commands, srcBuffer, dstImage,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                    &region);
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    VkImage srcImage = ToBackend(src.texture)->GetHandle();
                    VkBuffer dstBuffer = ToBackend(dst.buffer)->GetHandle();
                    VkBufferImageCopy region =
                        ComputeBufferImageCopyRegion(copy->rowPitch, dst, src);

                    // The NXT TransferSrc usage is always mapped to GENERAL
                    device->fn.CmdCopyImageToBuffer(commands, srcImage, VK_IMAGE_LAYOUT_GENERAL,
                                                    dstBuffer, 1, &region);
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                    RenderPassDescriptor* info = ToBackend(cmd->info.Get());

                    // NXT has an implicit transition to color attachment on render passes.
                    // Transition the attachments now before we start the render pass.
                    for (uint32_t i : IterateBitSet(info->GetColorAttachmentMask())) {
                        Texture* attachment =
                            ToBackend(info->GetColorAttachment(i).view->GetTexture());

                        if (!(attachment->GetUsage() & nxt::TextureUsageBit::OutputAttachment)) {
                            attachment->RecordBarrier(commands, attachment->GetUsage(),
                                                      nxt::TextureUsageBit::OutputAttachment);
                            attachment->UpdateUsageInternal(nxt::TextureUsageBit::OutputAttachment);
                        }
                    }
                    if (info->HasDepthStencilAttachment()) {
                        Texture* attachment =
                            ToBackend(info->GetDepthStencilAttachment().view->GetTexture());

                        if (!(attachment->GetUsage() & nxt::TextureUsageBit::OutputAttachment)) {
                            attachment->RecordBarrier(commands, attachment->GetUsage(),
                                                      nxt::TextureUsageBit::OutputAttachment);
                            attachment->UpdateUsageInternal(nxt::TextureUsageBit::OutputAttachment);
                        }
                    }

                    info->RecordBeginRenderPass(commands);

                    // Set all the dynamic state just in case.
                    device->fn.CmdSetLineWidth(commands, 1.0f);
                    device->fn.CmdSetDepthBounds(commands, 0.0f, 1.0f);

                    device->fn.CmdSetStencilReference(commands, VK_STENCIL_FRONT_AND_BACK, 0);

                    float blendConstants[4] = {
                        0.0f,
                        0.0f,
                        0.0f,
                        0.0f,
                    };
                    device->fn.CmdSetBlendConstants(commands, blendConstants);

                    // The viewport and scissor default to cover all of the attachments
                    VkViewport viewport;
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = static_cast<float>(info->GetWidth());
                    viewport.height = static_cast<float>(info->GetHeight());
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    device->fn.CmdSetViewport(commands, 0, 1, &viewport);

                    VkRect2D scissorRect;
                    scissorRect.offset.x = 0;
                    scissorRect.offset.y = 0;
                    scissorRect.extent.width = info->GetWidth();
                    scissorRect.extent.height = info->GetHeight();
                    device->fn.CmdSetScissor(commands, 0, 1, &scissorRect);

                    descriptorSets.OnBeginPass();
                } break;

                case Command::DrawArrays: {
                    DrawArraysCmd* draw = mCommands.NextCommand<DrawArraysCmd>();

                    descriptorSets.Flush(device, commands, VK_PIPELINE_BIND_POINT_GRAPHICS);
                    device->fn.CmdDraw(commands, draw->vertexCount, draw->instanceCount,
                                       draw->firstVertex, draw->firstInstance);
                } break;

                case Command::DrawElements: {
                    DrawElementsCmd* draw = mCommands.NextCommand<DrawElementsCmd>();

                    descriptorSets.Flush(device, commands, VK_PIPELINE_BIND_POINT_GRAPHICS);
                    uint32_t vertexOffset = 0;
                    device->fn.CmdDrawIndexed(commands, draw->indexCount, draw->instanceCount,
                                              draw->firstIndex, vertexOffset, draw->firstInstance);
                } break;

                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    device->fn.CmdEndRenderPass(commands);
                } break;

                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();
                    descriptorSets.OnBeginPass();
                } break;

                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                } break;

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                    descriptorSets.Flush(device, commands, VK_PIPELINE_BIND_POINT_COMPUTE);
                    device->fn.CmdDispatch(commands, dispatch->x, dispatch->y, dispatch->z);
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    VkDescriptorSet set = ToBackend(cmd->group.Get())->GetHandle();

                    descriptorSets.OnSetBindGroup(cmd->index, set);
                } break;

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    float blendConstants[4] = {
                        cmd->r,
                        cmd->g,
                        cmd->b,
                        cmd->a,
                    };
                    device->fn.CmdSetBlendConstants(commands, blendConstants);
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mCommands.NextCommand<SetIndexBufferCmd>();
                    VkBuffer indexBuffer = ToBackend(cmd->buffer)->GetHandle();

                    // TODO(cwallez@chromium.org): get the index type from the last render pipeline
                    // and rebind if needed on pipeline change
                    ASSERT(lastRenderPipeline != nullptr);
                    VkIndexType indexType = VulkanIndexType(lastRenderPipeline->GetIndexFormat());
                    device->fn.CmdBindIndexBuffer(
                        commands, indexBuffer, static_cast<VkDeviceSize>(cmd->offset), indexType);
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();

                    device->fn.CmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
                                               pipeline->GetHandle());
                    descriptorSets.OnPipelineLayoutChange(ToBackend(pipeline->GetLayout()));
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();

                    device->fn.CmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               pipeline->GetHandle());
                    lastRenderPipeline = pipeline;

                    descriptorSets.OnPipelineLayoutChange(ToBackend(pipeline->GetLayout()));
                } break;

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                    device->fn.CmdSetStencilReference(commands, VK_STENCIL_FRONT_AND_BACK,
                                                      cmd->reference);
                } break;

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    VkRect2D rect;
                    rect.offset.x = cmd->x;
                    rect.offset.y = cmd->y;
                    rect.extent.width = cmd->width;
                    rect.extent.height = cmd->height;

                    device->fn.CmdSetScissor(commands, 0, 1, &rect);
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mCommands.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mCommands.NextData<Ref<BufferBase>>(cmd->count);
                    auto offsets = mCommands.NextData<uint32_t>(cmd->count);

                    std::array<VkBuffer, kMaxVertexInputs> vkBuffers;
                    std::array<VkDeviceSize, kMaxVertexInputs> vkOffsets;

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        Buffer* buffer = ToBackend(buffers[i].Get());
                        vkBuffers[i] = buffer->GetHandle();
                        vkOffsets[i] = static_cast<VkDeviceSize>(offsets[i]);
                    }

                    device->fn.CmdBindVertexBuffers(commands, cmd->startSlot, cmd->count,
                                                    vkBuffers.data(), vkOffsets.data());
                } break;

                case Command::TransitionBufferUsage: {
                    TransitionBufferUsageCmd* cmd =
                        mCommands.NextCommand<TransitionBufferUsageCmd>();

                    Buffer* buffer = ToBackend(cmd->buffer.Get());
                    buffer->RecordBarrier(commands, buffer->GetUsage(), cmd->usage);
                    buffer->UpdateUsageInternal(cmd->usage);
                } break;

                case Command::TransitionTextureUsage: {
                    TransitionTextureUsageCmd* cmd =
                        mCommands.NextCommand<TransitionTextureUsageCmd>();

                    Texture* texture = ToBackend(cmd->texture.Get());
                    texture->RecordBarrier(commands, texture->GetUsage(), cmd->usage);
                    texture->UpdateUsageInternal(cmd->usage);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

}}  // namespace backend::vulkan
