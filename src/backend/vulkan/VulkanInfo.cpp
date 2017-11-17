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

#include "backend/vulkan/VulkanInfo.h"

#include "backend/vulkan/VulkanBackend.h"

namespace backend {
namespace vulkan {

    bool VulkanInfo::GatherGlobalInfo(const Device& device) {
        // Gather the info about the instance layers
        {
            uint32_t count = 0;
            VkResult result = device.fn.EnumerateInstanceLayerProperties(&count, nullptr);
            // From the Vulkan spec result should be success if there are 0 layers,
            // incomplete otherwise. This means that both values represent a success.
            // This is the same for all Enumarte functions
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return false;
            }

            global.layers.resize(count);
            result = device.fn.EnumerateInstanceLayerProperties(&count, global.layers.data());
            if (result != VK_SUCCESS) {
                return false;
            }
        }

        // Gather the info about the instance extensions
        {
            uint32_t count = 0;
            VkResult result = device.fn.EnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return false;
            }

            global.extensions.resize(count);
            result = device.fn.EnumerateInstanceExtensionProperties(nullptr, &count, global.extensions.data());
            if (result != VK_SUCCESS) {
                return false;
            }
        }

        // TODO(cwallez@chromium:org): Each layer can expose additional extensions, query them?

        return true;
    }
}
}
