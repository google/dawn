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

#ifndef DAWNNATIVE_VULKAN_VULKANINFO_H_
#define DAWNNATIVE_VULKAN_VULKANINFO_H_

#include "common/vulkan_platform.h"
#include "dawn_native/Error.h"

#include <vector>

namespace dawn_native { namespace vulkan {

    class Device;

    extern const char kLayerNameLunargStandardValidation[];
    extern const char kLayerNameLunargVKTrace[];
    extern const char kLayerNameRenderDocCapture[];

    extern const char kExtensionNameExtDebugReport[];
    extern const char kExtensionNameKhrSurface[];
    extern const char kExtensionNameKhrSwapchain[];

    // Global information - gathered before the instance is created
    struct VulkanGlobalKnobs {
        // Layers
        bool standardValidation = false;
        bool vktrace = false;
        bool renderDocCapture = false;

        // Extensions
        bool debugReport = false;
        bool surface = false;
    };

    struct VulkanGlobalInfo : VulkanGlobalKnobs {
        std::vector<VkLayerProperties> layers;
        std::vector<VkExtensionProperties> extensions;
        // TODO(cwallez@chromium.org): layer instance extensions
    };

    // Device information - gathered before the device is created.
    struct VulkanDeviceKnobs {
        VkPhysicalDeviceFeatures features;

        // Extensions
        bool swapchain = false;
    };

    struct VulkanDeviceInfo : VulkanDeviceKnobs {
        VkPhysicalDeviceProperties properties;
        std::vector<VkQueueFamilyProperties> queueFamilies;

        std::vector<VkMemoryType> memoryTypes;
        std::vector<VkMemoryHeap> memoryHeaps;

        std::vector<VkLayerProperties> layers;
        std::vector<VkExtensionProperties> extensions;
        // TODO(cwallez@chromium.org): layer instance extensions
    };

    struct VulkanSurfaceInfo {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        std::vector<bool> supportedQueueFamilies;
    };

    ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const Device& device);
    ResultOrError<std::vector<VkPhysicalDevice>> GetPhysicalDevices(const Device& device);
    ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const Device& device,
                                                     VkPhysicalDevice physicalDevice);
    MaybeError GatherSurfaceInfo(const Device& device,
                                 VkSurfaceKHR surface,
                                 VulkanSurfaceInfo* info);
}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_VULKANINFO_H_
