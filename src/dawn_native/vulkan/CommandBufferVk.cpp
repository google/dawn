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

#include "dawn_native/vulkan/CommandBufferVk.h"

#include "dawn_native/Commands.h"
#include "dawn_native/vulkan/BindGroupVk.h"
#include "dawn_native/vulkan/BufferVk.h"
#include "dawn_native/vulkan/ComputePipelineVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/PipelineLayoutVk.h"
#include "dawn_native/vulkan/RenderPassDescriptorVk.h"
#include "dawn_native/vulkan/RenderPipelineVk.h"
#include "dawn_native/vulkan/TextureVk.h"

namespace dawn_native { namespace vulkan {

    namespace {

        VkIndexType VulkanIndexType(dawn::IndexFormat format) {
            switch (format) {
                case dawn::IndexFormat::Uint16:
                    return VK_INDEX_TYPE_UINT16;
                case dawn::IndexFormat::Uint32:
                    return VK_INDEX_TYPE_UINT32;
                default:
                    UNREACHABLE();
            }
        }

        VkBufferImageCopy ComputeBufferImageCopyRegion(const BufferCopy& bufferCopy,
                                                       const TextureCopy& textureCopy,
                                                       const Extent3D& copySize) {
            const Texture* texture = ToBackend(textureCopy.texture.Get());

            VkBufferImageCopy region;

            region.bufferOffset = bufferCopy.offset;
            // In Vulkan the row length is in texels while it is in bytes for Dawn
            region.bufferRowLength =
                bufferCopy.rowPitch / TextureFormatPixelSize(texture->GetFormat());
            region.bufferImageHeight = bufferCopy.rowPitch * copySize.height;

            region.imageSubresource.aspectMask = texture->GetVkAspectMask();
            region.imageSubresource.mipLevel = textureCopy.level;
            region.imageSubresource.baseArrayLayer = textureCopy.slice;
            region.imageSubresource.layerCount = 1;

            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = textureCopy.origin.y;
            region.imageOffset.z = textureCopy.origin.z;

            region.imageExtent.width = copySize.width;
            region.imageExtent.height = copySize.height;
            region.imageExtent.depth = copySize.depth;

            return region;
        }

        class DescriptorSetTracker {
          public:
            void OnSetBindGroup(uint32_t index, VkDescriptorSet set) {
                mDirtySets.set(index);
                mSets[index] = set;
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

        // Records the necessary barriers for the resource usage pre-computed by the frontend
        auto TransitionForPass = [](VkCommandBuffer commands, const PassResourceUsage& usages) {
            for (size_t i = 0; i < usages.buffers.size(); ++i) {
                Buffer* buffer = ToBackend(usages.buffers[i]);
                buffer->TransitionUsageNow(commands, usages.bufferUsages[i]);
            }
            for (size_t i = 0; i < usages.textures.size(); ++i) {
                Texture* texture = ToBackend(usages.textures[i]);
                texture->TransitionUsageNow(commands, usages.textureUsages[i]);
            }
        };

        const std::vector<PassResourceUsage>& passResourceUsages = GetResourceUsages().perPass;
        size_t nextPassNumber = 0;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    ToBackend(src.buffer)
                        ->TransitionUsageNow(commands, dawn::BufferUsageBit::TransferSrc);
                    ToBackend(dst.buffer)
                        ->TransitionUsageNow(commands, dawn::BufferUsageBit::TransferDst);

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

                    ToBackend(src.buffer)
                        ->TransitionUsageNow(commands, dawn::BufferUsageBit::TransferSrc);
                    ToBackend(dst.texture)
                        ->TransitionUsageNow(commands, dawn::TextureUsageBit::TransferDst);

                    VkBuffer srcBuffer = ToBackend(src.buffer)->GetHandle();
                    VkImage dstImage = ToBackend(dst.texture)->GetHandle();

                    VkBufferImageCopy region =
                        ComputeBufferImageCopyRegion(src, dst, copy->copySize);

                    // The image is written to so the Dawn guarantees make sure it is in the
                    // TRANSFER_DST_OPTIMAL layout
                    device->fn.CmdCopyBufferToImage(commands, srcBuffer, dstImage,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                    &region);
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;

                    ToBackend(src.texture)
                        ->TransitionUsageNow(commands, dawn::TextureUsageBit::TransferSrc);
                    ToBackend(dst.buffer)
                        ->TransitionUsageNow(commands, dawn::BufferUsageBit::TransferDst);

                    VkImage srcImage = ToBackend(src.texture)->GetHandle();
                    VkBuffer dstBuffer = ToBackend(dst.buffer)->GetHandle();

                    VkBufferImageCopy region =
                        ComputeBufferImageCopyRegion(dst, src, copy->copySize);

                    // The Dawn TransferSrc usage is always mapped to GENERAL
                    device->fn.CmdCopyImageToBuffer(commands, srcImage, VK_IMAGE_LAYOUT_GENERAL,
                                                    dstBuffer, 1, &region);
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();

                    TransitionForPass(commands, passResourceUsages[nextPassNumber]);
                    RecordRenderPass(commands, ToBackend(cmd->info.Get()));

                    nextPassNumber++;
                } break;

                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();

                    TransitionForPass(commands, passResourceUsages[nextPassNumber]);
                    RecordComputePass(commands);

                    nextPassNumber++;
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

    void CommandBuffer::RecordComputePass(VkCommandBuffer commands) {
        Device* device = ToBackend(GetDevice());

        DescriptorSetTracker descriptorSets;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    return;
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

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();

                    device->fn.CmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
                                               pipeline->GetHandle());
                    descriptorSets.OnPipelineLayoutChange(ToBackend(pipeline->GetLayout()));
                } break;

                default: { UNREACHABLE(); } break;
            }
        }

        // EndComputePass should have been called
        UNREACHABLE();
    }
    void CommandBuffer::RecordRenderPass(VkCommandBuffer commands,
                                         RenderPassDescriptor* renderPass) {
        Device* device = ToBackend(GetDevice());

        renderPass->RecordBeginRenderPass(commands);

        // Set the default value for the dynamic state
        {
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
            viewport.width = static_cast<float>(renderPass->GetWidth());
            viewport.height = static_cast<float>(renderPass->GetHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            device->fn.CmdSetViewport(commands, 0, 1, &viewport);

            VkRect2D scissorRect;
            scissorRect.offset.x = 0;
            scissorRect.offset.y = 0;
            scissorRect.extent.width = renderPass->GetWidth();
            scissorRect.extent.height = renderPass->GetHeight();
            device->fn.CmdSetScissor(commands, 0, 1, &scissorRect);
        }

        DescriptorSetTracker descriptorSets;
        RenderPipeline* lastPipeline = nullptr;

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    device->fn.CmdEndRenderPass(commands);
                    return;
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
                    ASSERT(lastPipeline != nullptr);
                    VkIndexType indexType = VulkanIndexType(lastPipeline->GetIndexFormat());
                    device->fn.CmdBindIndexBuffer(
                        commands, indexBuffer, static_cast<VkDeviceSize>(cmd->offset), indexType);
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mCommands.NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();

                    device->fn.CmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               pipeline->GetHandle());
                    lastPipeline = pipeline;

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

                default: { UNREACHABLE(); } break;
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::vulkan
