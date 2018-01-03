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
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

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

                case Command::TransitionBufferUsage: {
                    TransitionBufferUsageCmd* cmd =
                        mCommands.NextCommand<TransitionBufferUsageCmd>();

                    Buffer* buffer = ToBackend(cmd->buffer.Get());
                    buffer->RecordBarrier(commands, buffer->GetUsage(), cmd->usage);
                    buffer->UpdateUsageInternal(cmd->usage);
                } break;

                default: { UNREACHABLE(); } break;
            }
        }
    }

}}  // namespace backend::vulkan
