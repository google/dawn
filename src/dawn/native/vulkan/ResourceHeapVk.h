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

#ifndef SRC_DAWN_NATIVE_VULKAN_RESOURCEHEAPVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RESOURCEHEAPVK_H_

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/ResourceHeap.h"

namespace dawn::native::vulkan {

// Wrapper for physical memory used with or without a resource object.
class ResourceHeap : public ResourceHeapBase {
  public:
    ResourceHeap(VkDeviceMemory memory, size_t memoryType);
    ~ResourceHeap() override = default;

    VkDeviceMemory GetMemory() const;
    size_t GetMemoryType() const;

  private:
    VkDeviceMemory mMemory = VK_NULL_HANDLE;
    size_t mMemoryType = 0;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RESOURCEHEAPVK_H_
