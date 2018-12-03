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

#include "dawn_native/vulkan/VulkanInfo.h"

#include "dawn_native/vulkan/DeviceVk.h"

#include <cstring>

namespace {
    bool IsLayerName(const VkLayerProperties& layer, const char* name) {
        return strncmp(layer.layerName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
    }

    bool IsExtensionName(const VkExtensionProperties& extension, const char* name) {
        return strncmp(extension.extensionName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
    }
}  // namespace

namespace dawn_native { namespace vulkan {

    const char kLayerNameLunargStandardValidation[] = "VK_LAYER_LUNARG_standard_validation";
    const char kLayerNameLunargVKTrace[] = "VK_LAYER_LUNARG_vktrace";
    const char kLayerNameRenderDocCapture[] = "VK_LAYER_RENDERDOC_Capture";

    const char kExtensionNameExtDebugReport[] = "VK_EXT_debug_report";
    const char kExtensionNameKhrSurface[] = "VK_KHR_surface";
    const char kExtensionNameKhrSwapchain[] = "VK_KHR_swapchain";

    ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const Device& device) {
        VulkanGlobalInfo info = {};

        // Gather the info about the instance layers
        {
            uint32_t count = 0;
            VkResult result = device.fn.EnumerateInstanceLayerProperties(&count, nullptr);
            // From the Vulkan spec result should be success if there are 0 layers,
            // incomplete otherwise. This means that both values represent a success.
            // This is the same for all Enumarte functions
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateInstanceLayerProperties");
            }

            info.layers.resize(count);
            result = device.fn.EnumerateInstanceLayerProperties(&count, info.layers.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateInstanceLayerProperties");
            }

            for (const auto& layer : info.layers) {
                if (IsLayerName(layer, kLayerNameLunargStandardValidation)) {
                    info.standardValidation = true;
                }
                if (IsLayerName(layer, kLayerNameLunargVKTrace)) {
                    info.vktrace = true;
                }
                if (IsLayerName(layer, kLayerNameRenderDocCapture)) {
                    info.renderDocCapture = true;
                }
            }
        }

        // Gather the info about the instance extensions
        {
            uint32_t count = 0;
            VkResult result =
                device.fn.EnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateInstanceExtensionProperties");
            }

            info.extensions.resize(count);
            result = device.fn.EnumerateInstanceExtensionProperties(nullptr, &count,
                                                                    info.extensions.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateInstanceExtensionProperties");
            }

            for (const auto& extension : info.extensions) {
                if (IsExtensionName(extension, kExtensionNameExtDebugReport)) {
                    info.debugReport = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrSurface)) {
                    info.surface = true;
                }
            }
        }

        // TODO(cwallez@chromium:org): Each layer can expose additional extensions, query them?

        return info;
    }

    ResultOrError<std::vector<VkPhysicalDevice>> GetPhysicalDevices(const Device& device) {
        VkInstance instance = device.GetInstance();

        uint32_t count = 0;
        VkResult result = device.fn.EnumeratePhysicalDevices(instance, &count, nullptr);
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_CONTEXT_LOST_ERROR("vkEnumeratePhysicalDevices");
        }

        std::vector<VkPhysicalDevice> physicalDevices(count);
        result = device.fn.EnumeratePhysicalDevices(instance, &count, physicalDevices.data());
        if (result != VK_SUCCESS) {
            return DAWN_CONTEXT_LOST_ERROR("vkEnumeratePhysicalDevices");
        }

        return physicalDevices;
    }

    ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const Device& device,
                                                     VkPhysicalDevice physicalDevice) {
        VulkanDeviceInfo info = {};

        // Gather general info about the device
        device.fn.GetPhysicalDeviceProperties(physicalDevice, &info.properties);
        device.fn.GetPhysicalDeviceFeatures(physicalDevice, &info.features);

        // Gather info about device memory.
        {
            VkPhysicalDeviceMemoryProperties memory;
            device.fn.GetPhysicalDeviceMemoryProperties(physicalDevice, &memory);

            info.memoryTypes.assign(memory.memoryTypes,
                                    memory.memoryTypes + memory.memoryTypeCount);
            info.memoryHeaps.assign(memory.memoryHeaps,
                                    memory.memoryHeaps + memory.memoryHeapCount);
        }

        // Gather info about device queue families
        {
            uint32_t count = 0;
            device.fn.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

            info.queueFamilies.resize(count);
            device.fn.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                                             info.queueFamilies.data());
        }

        // Gather the info about the device layers
        {
            uint32_t count = 0;
            VkResult result =
                device.fn.EnumerateDeviceLayerProperties(physicalDevice, &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateDeviceLayerProperties");
            }

            info.layers.resize(count);
            result = device.fn.EnumerateDeviceLayerProperties(physicalDevice, &count,
                                                              info.layers.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateDeviceLayerProperties");
            }
        }

        // Gather the info about the device extensions
        {
            uint32_t count = 0;
            VkResult result = device.fn.EnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                                           &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateDeviceExtensionProperties");
            }

            info.extensions.resize(count);
            result = device.fn.EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count,
                                                                  info.extensions.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkEnumerateDeviceExtensionProperties");
            }

            for (const auto& extension : info.extensions) {
                if (IsExtensionName(extension, kExtensionNameKhrSwapchain)) {
                    info.swapchain = true;
                }
            }
        }

        // TODO(cwallez@chromium.org): gather info about formats

        return info;
    }

    MaybeError GatherSurfaceInfo(const Device& device,
                                 VkSurfaceKHR surface,
                                 VulkanSurfaceInfo* info) {
        VkPhysicalDevice physicalDevice = device.GetPhysicalDevice();

        // Get the surface capabilities
        {
            VkResult result = device.fn.GetPhysicalDeviceSurfaceCapabilitiesKHR(
                physicalDevice, surface, &info->capabilities);
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
            }
        }

        // Query which queue families support presenting this surface
        {
            size_t nQueueFamilies = device.GetDeviceInfo().queueFamilies.size();
            info->supportedQueueFamilies.resize(nQueueFamilies, false);

            for (uint32_t i = 0; i < nQueueFamilies; ++i) {
                VkBool32 supported = VK_FALSE;
                VkResult result = device.fn.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i,
                                                                               surface, &supported);

                if (result != VK_SUCCESS) {
                    return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfaceSupportKHR");
                }

                info->supportedQueueFamilies[i] = (supported == VK_TRUE);
            }
        }

        // Gather supported formats
        {
            uint32_t count = 0;
            VkResult result = device.fn.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                                           &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR");
            }

            info->formats.resize(count);
            result = device.fn.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count,
                                                                  info->formats.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR");
            }
        }

        // Gather supported presents modes
        {
            uint32_t count = 0;
            VkResult result = device.fn.GetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice, surface, &count, nullptr);
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR");
            }

            info->presentModes.resize(count);
            result = device.fn.GetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice, surface, &count, info->presentModes.data());
            if (result != VK_SUCCESS) {
                return DAWN_CONTEXT_LOST_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR");
            }
        }

        return {};
    }

}}  // namespace dawn_native::vulkan
