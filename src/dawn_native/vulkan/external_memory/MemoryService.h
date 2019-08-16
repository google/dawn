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

#ifndef DAWNNATIVE_VULKAN_EXTERNALMEMORY_SERVICE_H_
#define DAWNNATIVE_VULKAN_EXTERNALMEMORY_SERVICE_H_

#include "common/vulkan_platform.h"
#include "dawn_native/Error.h"
#include "dawn_native/vulkan/ExternalHandle.h"

namespace dawn_native { namespace vulkan {
    class Device;
}}  // namespace dawn_native::vulkan

namespace dawn_native { namespace vulkan { namespace external_memory {

    class Service {
      public:
        explicit Service(Device* device);
        ~Service();

        // True if the device reports it supports this feature
        bool Supported(VkFormat format,
                       VkImageType type,
                       VkImageTiling tiling,
                       VkImageUsageFlags usage,
                       VkImageCreateFlags flags);

        // Given an external handle pointing to memory, import it into a VkDeviceMemory
        ResultOrError<VkDeviceMemory> ImportMemory(ExternalMemoryHandle handle,
                                                   VkDeviceSize allocationSize,
                                                   uint32_t memoryTypeIndex);

      private:
        Device* mDevice = nullptr;

        // True if early checks pass that determine if the service is supported
        bool mSupported = false;
    };

}}}  // namespace dawn_native::vulkan::external_memory

#endif  // DAWNNATIVE_VULKAN_EXTERNALMEMORY_SERVICE_H_
