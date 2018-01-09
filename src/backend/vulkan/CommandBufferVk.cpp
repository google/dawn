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
#include "backend/vulkan/BufferVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {

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

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(CommandBufferBuilder* builder)
        : CommandBufferBase(builder), mCommands(builder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::RecordCommands(VkCommandBuffer commands) {
        Device* device = ToBackend(GetDevice());

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

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mCommands.NextCommand<SetIndexBufferCmd>();
                    VkBuffer indexBuffer = ToBackend(cmd->buffer)->GetHandle();

                    // TODO(cwallez@chromium.org): get the index type from the last render pipeline
                    // and rebind if needed on pipeline change
                    device->fn.CmdBindIndexBuffer(commands, indexBuffer,
                                                  static_cast<VkDeviceSize>(cmd->offset),
                                                  VK_INDEX_TYPE_UINT16);
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
