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
#ifndef SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_
#define SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_

#include <set>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/VulkanFunctions.h"

namespace dawn::native::vulkan {

class Texture;

// Wrapping class that currently associates a command buffer to it's corresponding pool.
// TODO(dawn:1601) Revisit this structure since it is where the 1:1 mapping is implied.
//                 Also consider reusing this in CommandRecordingContext below instead of
//                 flattening the pool and command buffer again.
struct CommandPoolAndBuffer {
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

// Used to track operations that are handled after recording.
// Currently only tracks semaphores, but may be used to do barrier coalescing in the future.
struct CommandRecordingContext {
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    std::vector<VkSemaphore> waitSemaphores = {};

    // The internal buffers used in the workaround of texture-to-texture copies with compressed
    // formats.
    std::vector<Ref<Buffer>> tempBuffers;

    // External textures that will be eagerly transitioned just before VkSubmit. The textures are
    // kept alive by the CommandBuffer so they don't need to be Ref-ed.
    std::set<Texture*> externalTexturesForEagerTransition;

    // Mappable buffers which will be eagerly transitioned to usage MapRead or MapWrite after
    // VkSubmit.
    std::set<Ref<Buffer>> mappableBuffersForEagerTransition;

    // For Device state tracking only.
    VkCommandPool commandPool = VK_NULL_HANDLE;
    bool needsSubmit = false;
    bool used = false;

    // In some cases command buffer will need to be split to accomodate driver bug workarounds.
    // See the VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass toggle as an
    // example. This tracks the list of all command buffers used for this recording context,
    // with commandBuffer always being the last element.
    std::vector<VkCommandBuffer> commandBufferList;
    std::vector<VkCommandPool> commandPoolList;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_
