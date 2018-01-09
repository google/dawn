// Copyright 2018 The NXT Authors
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

#ifndef BACKEND_VULKAN_RENDERPASSVK_H_
#define BACKEND_VULKAN_RENDERPASSVK_H_

#include "backend/RenderPass.h"

#include "backend/vulkan/vulkan_platform.h"

namespace backend { namespace vulkan {

    class Device;

    class RenderPass : public RenderPassBase {
      public:
        RenderPass(RenderPassBuilder* builder);
        ~RenderPass();

        // TODO(cwallez@chromium.org): We need a way to ask for a compatible VkRenderPass with the
        // given load an store operations. Also they should be cached. For now this is hardcoded to
        // have Load = Clear and Store = Write
        VkRenderPass GetHandle() const;

      private:
        VkRenderPass mHandle = VK_NULL_HANDLE;
        Device* mDevice = nullptr;
    };

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_PIPELINELAYOUTVK_H_
