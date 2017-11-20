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

#include <cstring>

namespace {
    bool IsLayerName(const VkLayerProperties& layer, const char* name) {
        return strncmp(layer.layerName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
    }

    bool IsExtensionName(const VkExtensionProperties& extension, const char* name) {
        return strncmp(extension.extensionName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
    }
}

namespace backend {
namespace vulkan {

    const char kLayerNameLunargStandardValidation[] = "VK_LAYER_LUNARG_standard_validation";

    const char kExtensionNameExtDebugReport[] = "VK_EXT_debug_report";

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

            for (const auto& layer : global.layers) {
                if (IsLayerName(layer, kLayerNameLunargStandardValidation)) {
                    global.standardValidation = true;
                }
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

            for (const auto& extension : global.extensions) {
                if (IsExtensionName(extension, kExtensionNameExtDebugReport)) {
                    global.debugReport = true;
                }
            }
        }

        // TODO(cwallez@chromium:org): Each layer can expose additional extensions, query them?

        return true;
    }

    void VulkanInfo::SetUsedGlobals(const KnownGlobalVulkanExtensions& usedGlobals) {
        *static_cast<KnownGlobalVulkanExtensions*>(&global) = usedGlobals;
    }
}
}
