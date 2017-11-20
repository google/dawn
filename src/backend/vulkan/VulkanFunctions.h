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

#ifndef BACKEND_VULKAN_VULKANFUNCTIONS_H_
#define BACKEND_VULKAN_VULKANFUNCTIONS_H_

#include "backend/vulkan/vulkan_platform.h"

class DynamicLib;

namespace backend {
namespace vulkan {

    struct KnownGlobalVulkanExtensions;

    // Stores the Vulkan entry points. Also loads them from the dynamic library
    // and the vkGet*ProcAddress entry points.
    struct VulkanFunctions {
        bool LoadGlobalProcs(const DynamicLib& vulkanLib);
        bool LoadInstanceProcs(VkInstance instance, const KnownGlobalVulkanExtensions& usedGlobals);

        // Initial proc from which we can get all the others
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;

        // Global procs, can be used without an instance
        PFN_vkCreateInstance CreateInstance = nullptr;
        PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties = nullptr;
        PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties = nullptr;
        // DestroyInstance isn't technically a global proc but we want to be able to use it
        // before querying the instance procs in case we need to error out during initialization.
        PFN_vkDestroyInstance DestroyInstance = nullptr;

        // Instance procs
        PFN_vkCreateDevice CreateDevice = nullptr;
        PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties = nullptr;
        PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties = nullptr;
        PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices = nullptr;
        PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures = nullptr;
        PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties = nullptr;
        PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties = nullptr;
        PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties = nullptr;
        PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties = nullptr;
        PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties = nullptr;
        PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties = nullptr;
        // Not technically an instance proc but we want to be able to use it as soon as the
        // device is created.
        PFN_vkDestroyDevice DestroyDevice = nullptr;

        // VK_EXT_debug_report
        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT = nullptr;
        PFN_vkDebugReportMessageEXT DebugReportMessageEXT = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT = nullptr;
    };


}
}

#endif // BACKEND_VULKAN_VULKANFUNCTIONS_H_
