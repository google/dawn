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

#ifndef BACKEND_VULKAN_BINDGROUPLAYOUTVK_H_
#define BACKEND_VULKAN_BINDGROUPLAYOUTVK_H_

#include "backend/BindGroupLayout.h"

#include "common/vulkan_platform.h"

namespace backend { namespace vulkan {

    class Device;

    VkDescriptorType VulkanDescriptorType(nxt::BindingType type);

    class BindGroupLayout : public BindGroupLayoutBase {
      public:
        BindGroupLayout(Device* device, const nxt::BindGroupLayoutDescriptor* descriptor);
        ~BindGroupLayout();

        VkDescriptorSetLayout GetHandle() const;

        static constexpr size_t kMaxPoolSizesNeeded = 4;
        using PoolSizeSpec = std::array<VkDescriptorPoolSize, kMaxPoolSizesNeeded>;
        PoolSizeSpec ComputePoolSizes(uint32_t* numPoolSizes) const;

      private:
        VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;
    };

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_BINDGROUPLAYOUTVK_H_
