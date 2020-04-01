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

#include "common/Log.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/BackendVk.h"
#include "dawn_native/vulkan/VulkanError.h"

#include <cstring>

namespace dawn_native { namespace vulkan {

    namespace {
        bool IsLayerName(const VkLayerProperties& layer, const char* name) {
            return strncmp(layer.layerName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
        }

        bool IsExtensionName(const VkExtensionProperties& extension, const char* name) {
            return strncmp(extension.extensionName, name, VK_MAX_EXTENSION_NAME_SIZE) == 0;
        }

        bool EnumerateInstanceExtensions(const char* layerName,
                                         const dawn_native::vulkan::VulkanFunctions& vkFunctions,
                                         std::vector<VkExtensionProperties>* extensions) {
            uint32_t count = 0;
            VkResult result = VkResult::WrapUnsafe(
                vkFunctions.EnumerateInstanceExtensionProperties(layerName, &count, nullptr));
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return false;
            }
            extensions->resize(count);
            result = VkResult::WrapUnsafe(vkFunctions.EnumerateInstanceExtensionProperties(
                layerName, &count, extensions->data()));
            return (result == VK_SUCCESS);
        }

    }  // namespace

    const char kLayerNameLunargStandardValidation[] = "VK_LAYER_LUNARG_standard_validation";
    const char kLayerNameLunargVKTrace[] = "VK_LAYER_LUNARG_vktrace";
    const char kLayerNameRenderDocCapture[] = "VK_LAYER_RENDERDOC_Capture";
    const char kLayerNameFuchsiaImagePipeSwapchain[] = "VK_LAYER_FUCHSIA_imagepipe_swapchain";

    const char kExtensionNameExtDebugMarker[] = "VK_EXT_debug_marker";
    const char kExtensionNameExtDebugReport[] = "VK_EXT_debug_report";
    const char kExtensionNameExtMetalSurface[] = "VK_EXT_metal_surface";
    const char kExtensionNameKhrExternalMemory[] = "VK_KHR_external_memory";
    const char kExtensionNameKhrExternalMemoryCapabilities[] =
        "VK_KHR_external_memory_capabilities";
    const char kExtensionNameKhrExternalMemoryFD[] = "VK_KHR_external_memory_fd";
    const char kExtensionNameExtExternalMemoryDmaBuf[] = "VK_EXT_external_memory_dma_buf";
    const char kExtensionNameExtImageDrmFormatModifier[] = "VK_EXT_image_drm_format_modifier";
    const char kExtensionNameFuchsiaExternalMemory[] = "VK_FUCHSIA_external_memory";
    const char kExtensionNameKhrExternalSemaphore[] = "VK_KHR_external_semaphore";
    const char kExtensionNameKhrExternalSemaphoreCapabilities[] =
        "VK_KHR_external_semaphore_capabilities";
    const char kExtensionNameKhrExternalSemaphoreFD[] = "VK_KHR_external_semaphore_fd";
    const char kExtensionNameFuchsiaExternalSemaphore[] = "VK_FUCHSIA_external_semaphore";
    const char kExtensionNameKhrGetPhysicalDeviceProperties2[] =
        "VK_KHR_get_physical_device_properties2";
    const char kExtensionNameKhrSurface[] = "VK_KHR_surface";
    const char kExtensionNameKhrSwapchain[] = "VK_KHR_swapchain";
    const char kExtensionNameKhrWaylandSurface[] = "VK_KHR_wayland_surface";
    const char kExtensionNameKhrWin32Surface[] = "VK_KHR_win32_surface";
    const char kExtensionNameKhrXcbSurface[] = "VK_KHR_xcb_surface";
    const char kExtensionNameKhrXlibSurface[] = "VK_KHR_xlib_surface";
    const char kExtensionNameFuchsiaImagePipeSurface[] = "VK_FUCHSIA_imagepipe_surface";
    const char kExtensionNameKhrMaintenance1[] = "VK_KHR_maintenance1";

    ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const Backend& backend) {
        VulkanGlobalInfo info = {};
        const VulkanFunctions& vkFunctions = backend.GetFunctions();

        // Gather the info about the instance layers
        {
            uint32_t count = 0;
            VkResult result =
                VkResult::WrapUnsafe(vkFunctions.EnumerateInstanceLayerProperties(&count, nullptr));
            // From the Vulkan spec result should be success if there are 0 layers,
            // incomplete otherwise. This means that both values represent a success.
            // This is the same for all Enumarte functions
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_INTERNAL_ERROR("vkEnumerateInstanceLayerProperties");
            }

            info.layers.resize(count);
            DAWN_TRY(CheckVkSuccess(
                vkFunctions.EnumerateInstanceLayerProperties(&count, info.layers.data()),
                "vkEnumerateInstanceLayerProperties"));

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
                // Technical note: Fuchsia implements the swapchain through
                // a layer (VK_LAYER_FUCHSIA_image_pipe_swapchain), which adds
                // an instance extensions (VK_FUCHSIA_image_surface) to all ICDs.
                if (IsLayerName(layer, kLayerNameFuchsiaImagePipeSwapchain)) {
                    info.fuchsiaImagePipeSwapchain = true;
                }
            }
        }

        // Gather the info about the instance extensions
        {
            if (!EnumerateInstanceExtensions(nullptr, vkFunctions, &info.extensions)) {
                return DAWN_INTERNAL_ERROR("vkEnumerateInstanceExtensionProperties");
            }

            for (const auto& extension : info.extensions) {
                if (IsExtensionName(extension, kExtensionNameExtDebugReport)) {
                    info.debugReport = true;
                }
                if (IsExtensionName(extension, kExtensionNameExtMetalSurface)) {
                    info.metalSurface = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalMemoryCapabilities)) {
                    info.externalMemoryCapabilities = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalSemaphoreCapabilities)) {
                    info.externalSemaphoreCapabilities = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrGetPhysicalDeviceProperties2)) {
                    info.getPhysicalDeviceProperties2 = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrSurface)) {
                    info.surface = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrWaylandSurface)) {
                    info.waylandSurface = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrWin32Surface)) {
                    info.win32Surface = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrXcbSurface)) {
                    info.xcbSurface = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrXlibSurface)) {
                    info.xlibSurface = true;
                }
                if (IsExtensionName(extension, kExtensionNameFuchsiaImagePipeSurface)) {
                    info.fuchsiaImagePipeSurface = true;
                }
            }
        }

        // Specific handling for the Fuchsia swapchain surface creation extension
        // which is normally part of the Fuchsia-specific swapchain layer.
        if (info.fuchsiaImagePipeSwapchain && !info.fuchsiaImagePipeSurface) {
            std::vector<VkExtensionProperties> layer_extensions;
            if (!EnumerateInstanceExtensions(kLayerNameFuchsiaImagePipeSwapchain, vkFunctions,
                                             &layer_extensions)) {
                return DAWN_INTERNAL_ERROR("vkEnumerateInstanceExtensionProperties");
            }

            for (const auto& extension : layer_extensions) {
                if (IsExtensionName(extension, kExtensionNameFuchsiaImagePipeSurface)) {
                    info.fuchsiaImagePipeSurface = true;
                    // For now, copy this to the global extension list.
                    info.extensions.push_back(extension);
                }
            }
        }

        // Gather info on available API version
        {
            uint32_t supportedAPIVersion = VK_MAKE_VERSION(1, 0, 0);
            if (vkFunctions.EnumerateInstanceVersion) {
                vkFunctions.EnumerateInstanceVersion(&supportedAPIVersion);
            }

            // Use Vulkan 1.1 if it's available.
            info.apiVersion = (supportedAPIVersion >= VK_MAKE_VERSION(1, 1, 0))
                                  ? VK_MAKE_VERSION(1, 1, 0)
                                  : VK_MAKE_VERSION(1, 0, 0);
        }

        // TODO(cwallez@chromium:org): Each layer can expose additional extensions, query them?

        return info;
    }

    ResultOrError<std::vector<VkPhysicalDevice>> GetPhysicalDevices(const Backend& backend) {
        VkInstance instance = backend.GetVkInstance();
        const VulkanFunctions& vkFunctions = backend.GetFunctions();

        uint32_t count = 0;
        VkResult result =
            VkResult::WrapUnsafe(vkFunctions.EnumeratePhysicalDevices(instance, &count, nullptr));
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_INTERNAL_ERROR("vkEnumeratePhysicalDevices");
        }

        std::vector<VkPhysicalDevice> physicalDevices(count);
        DAWN_TRY(CheckVkSuccess(
            vkFunctions.EnumeratePhysicalDevices(instance, &count, physicalDevices.data()),
            "vkEnumeratePhysicalDevices"));

        return physicalDevices;
    }

    ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const Adapter& adapter) {
        VulkanDeviceInfo info = {};
        VkPhysicalDevice physicalDevice = adapter.GetPhysicalDevice();
        const VulkanFunctions& vkFunctions = adapter.GetBackend()->GetFunctions();

        // Gather general info about the device
        vkFunctions.GetPhysicalDeviceProperties(physicalDevice, &info.properties);
        vkFunctions.GetPhysicalDeviceFeatures(physicalDevice, &info.features);

        // Gather info about device memory.
        {
            VkPhysicalDeviceMemoryProperties memory;
            vkFunctions.GetPhysicalDeviceMemoryProperties(physicalDevice, &memory);

            info.memoryTypes.assign(memory.memoryTypes,
                                    memory.memoryTypes + memory.memoryTypeCount);
            info.memoryHeaps.assign(memory.memoryHeaps,
                                    memory.memoryHeaps + memory.memoryHeapCount);
        }

        // Gather info about device queue families
        {
            uint32_t count = 0;
            vkFunctions.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

            info.queueFamilies.resize(count);
            vkFunctions.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
                                                               info.queueFamilies.data());
        }

        // Gather the info about the device layers
        {
            uint32_t count = 0;
            VkResult result = VkResult::WrapUnsafe(
                vkFunctions.EnumerateDeviceLayerProperties(physicalDevice, &count, nullptr));
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_INTERNAL_ERROR("vkEnumerateDeviceLayerProperties");
            }

            info.layers.resize(count);
            DAWN_TRY(CheckVkSuccess(vkFunctions.EnumerateDeviceLayerProperties(
                                        physicalDevice, &count, info.layers.data()),
                                    "vkEnumerateDeviceLayerProperties"));
        }

        // Gather the info about the device extensions
        {
            uint32_t count = 0;
            VkResult result = VkResult::WrapUnsafe(vkFunctions.EnumerateDeviceExtensionProperties(
                physicalDevice, nullptr, &count, nullptr));
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_INTERNAL_ERROR("vkEnumerateDeviceExtensionProperties");
            }

            info.extensions.resize(count);
            DAWN_TRY(CheckVkSuccess(vkFunctions.EnumerateDeviceExtensionProperties(
                                        physicalDevice, nullptr, &count, info.extensions.data()),
                                    "vkEnumerateDeviceExtensionProperties"));

            for (const auto& extension : info.extensions) {
                if (IsExtensionName(extension, kExtensionNameExtDebugMarker)) {
                    info.debugMarker = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalMemory)) {
                    info.externalMemory = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalMemoryFD)) {
                    info.externalMemoryFD = true;
                }
                if (IsExtensionName(extension, kExtensionNameExtExternalMemoryDmaBuf)) {
                    info.externalMemoryDmaBuf = true;
                }
                if (IsExtensionName(extension, kExtensionNameExtImageDrmFormatModifier)) {
                    info.imageDrmFormatModifier = true;
                }
                if (IsExtensionName(extension, kExtensionNameFuchsiaExternalMemory)) {
                    info.externalMemoryZirconHandle = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalSemaphore)) {
                    info.externalSemaphore = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrExternalSemaphoreFD)) {
                    info.externalSemaphoreFD = true;
                }
                if (IsExtensionName(extension, kExtensionNameFuchsiaExternalSemaphore)) {
                    info.externalSemaphoreZirconHandle = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrSwapchain)) {
                    info.swapchain = true;
                }
                if (IsExtensionName(extension, kExtensionNameKhrMaintenance1)) {
                    info.maintenance1 = true;
                }
            }
        }

        // Mark the extensions promoted to Vulkan 1.1 as available.
        if (info.properties.apiVersion >= VK_MAKE_VERSION(1, 1, 0)) {
            info.maintenance1 = true;
        }

        // TODO(cwallez@chromium.org): gather info about formats

        return info;
    }

    ResultOrError<VulkanSurfaceInfo> GatherSurfaceInfo(const Adapter& adapter,
                                                       VkSurfaceKHR surface) {
        VulkanSurfaceInfo info = {};

        VkPhysicalDevice physicalDevice = adapter.GetPhysicalDevice();
        const VulkanFunctions& vkFunctions = adapter.GetBackend()->GetFunctions();

        // Get the surface capabilities
        DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceCapabilitiesKHR(
                                    physicalDevice, surface, &info.capabilities),
                                "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));

        // Query which queue families support presenting this surface
        {
            size_t nQueueFamilies = adapter.GetDeviceInfo().queueFamilies.size();
            info.supportedQueueFamilies.resize(nQueueFamilies, false);

            for (uint32_t i = 0; i < nQueueFamilies; ++i) {
                VkBool32 supported = VK_FALSE;
                DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceSupportKHR(
                                            physicalDevice, i, surface, &supported),
                                        "vkGetPhysicalDeviceSurfaceSupportKHR"));

                info.supportedQueueFamilies[i] = (supported == VK_TRUE);
            }
        }

        // Gather supported formats
        {
            uint32_t count = 0;
            VkResult result = VkResult::WrapUnsafe(vkFunctions.GetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice, surface, &count, nullptr));
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_INTERNAL_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR");
            }

            info.formats.resize(count);
            DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceFormatsKHR(
                                        physicalDevice, surface, &count, info.formats.data()),
                                    "vkGetPhysicalDeviceSurfaceFormatsKHR"));
        }

        // Gather supported presents modes
        {
            uint32_t count = 0;
            VkResult result =
                VkResult::WrapUnsafe(vkFunctions.GetPhysicalDeviceSurfacePresentModesKHR(
                    physicalDevice, surface, &count, nullptr));
            if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
                return DAWN_INTERNAL_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR");
            }

            info.presentModes.resize(count);
            DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfacePresentModesKHR(
                                        physicalDevice, surface, &count, info.presentModes.data()),
                                    "vkGetPhysicalDeviceSurfacePresentModesKHR"));
        }

        return info;
    }

}}  // namespace dawn_native::vulkan
