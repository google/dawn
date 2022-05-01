// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"

namespace dawn::native::vulkan {

class Device;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipeline> CreateUninitialized(Device* device,
                                                   const RenderPipelineDescriptor* descriptor);
    static void InitializeAsync(Ref<RenderPipelineBase> renderPipeline,
                                WGPUCreateRenderPipelineAsyncCallback callback,
                                void* userdata);

    VkPipeline GetHandle() const;

    MaybeError Initialize() override;

    // Dawn API
    void SetLabelImpl() override;

  private:
    ~RenderPipeline() override;
    void DestroyImpl() override;
    using RenderPipelineBase::RenderPipelineBase;

    struct PipelineVertexInputStateCreateInfoTemporaryAllocations {
        std::array<VkVertexInputBindingDescription, kMaxVertexBuffers> bindings;
        std::array<VkVertexInputAttributeDescription, kMaxVertexAttributes> attributes;
    };
    VkPipelineVertexInputStateCreateInfo ComputeVertexInputDesc(
        PipelineVertexInputStateCreateInfoTemporaryAllocations* temporaryAllocations);

    VkPipeline mHandle = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_
