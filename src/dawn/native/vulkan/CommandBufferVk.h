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

#ifndef SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_

#include <set>

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Error.h"

#include "dawn/common/vulkan_platform.h"

namespace dawn::native {
struct BeginComputePassCmd;
struct BeginRenderPassCmd;
struct TextureCopy;
}  // namespace dawn::native

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);

    MaybeError RecordCommands(CommandRecordingContext* recordingContext);

  private:
    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    MaybeError RecordComputePass(CommandRecordingContext* recordingContext,
                                 BeginComputePassCmd* computePass,
                                 const ComputePassResourceUsage& resourceUsages);
    MaybeError RecordRenderPass(CommandRecordingContext* recordingContext,
                                BeginRenderPassCmd* renderPass);
    MaybeError RecordCopyImageWithTemporaryBuffer(CommandRecordingContext* recordingContext,
                                                  const TextureCopy& srcCopy,
                                                  const TextureCopy& dstCopy,
                                                  const Extent3D& copySize);

    // Need to track depth/stencil textures used by render passes if the
    // VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass toggle is enabled.
    std::set<TextureBase*> mRenderPassDepthStencilAttachments;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_
