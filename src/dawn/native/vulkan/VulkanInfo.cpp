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

#include "dawn/native/vulkan/VulkanInfo.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>

#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {
ResultOrError<InstanceExtSet> GatherInstanceExtensions(
    const char* layerName,
    const dawn::native::vulkan::VulkanFunctions& vkFunctions,
    const std::unordered_map<std::string, InstanceExt>& knownExts) {
    uint32_t count = 0;
    VkResult vkResult = VkResult::WrapUnsafe(
        vkFunctions.EnumerateInstanceExtensionProperties(layerName, &count, nullptr));
    if (vkResult != VK_SUCCESS && vkResult != VK_INCOMPLETE) {
        return DAWN_INTERNAL_ERROR("vkEnumerateInstanceExtensionProperties");
    }

    std::vector<VkExtensionProperties> extensions(count);
    DAWN_TRY(CheckVkSuccess(
        vkFunctions.EnumerateInstanceExtensionProperties(layerName, &count, extensions.data()),
        "vkEnumerateInstanceExtensionProperties"));

    InstanceExtSet result;
    for (const VkExtensionProperties& extension : extensions) {
        auto it = knownExts.find(extension.extensionName);
        if (it != knownExts.end()) {
            result.set(it->second, true);
        }
    }

    return result;
}

}  // namespace

bool VulkanGlobalKnobs::HasExt(InstanceExt ext) const {
    return extensions[ext];
}

bool VulkanDeviceKnobs::HasExt(DeviceExt ext) const {
    return extensions[ext];
}

ResultOrError<VulkanGlobalInfo> GatherGlobalInfo(const VulkanFunctions& vkFunctions) {
    VulkanGlobalInfo info = {};
    // Gather info on available API version
    {
        info.apiVersion = VK_API_VERSION_1_0;
        if (vkFunctions.EnumerateInstanceVersion != nullptr) {
            DAWN_TRY(CheckVkSuccess(vkFunctions.EnumerateInstanceVersion(&info.apiVersion),
                                    "vkEnumerateInstanceVersion"));
        }
    }

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

        std::vector<VkLayerProperties> layersProperties(count);
        DAWN_TRY(CheckVkSuccess(
            vkFunctions.EnumerateInstanceLayerProperties(&count, layersProperties.data()),
            "vkEnumerateInstanceLayerProperties"));

        std::unordered_map<std::string, VulkanLayer> knownLayers = CreateVulkanLayerNameMap();
        for (const VkLayerProperties& layer : layersProperties) {
            auto it = knownLayers.find(layer.layerName);
            if (it != knownLayers.end()) {
                info.layers.set(it->second, true);
            }
        }
    }

    // Gather the info about the instance extensions
    {
        std::unordered_map<std::string, InstanceExt> knownExts = CreateInstanceExtNameMap();

        DAWN_TRY_ASSIGN(info.extensions, GatherInstanceExtensions(nullptr, vkFunctions, knownExts));
        MarkPromotedExtensions(&info.extensions, info.apiVersion);
        info.extensions = EnsureDependencies(info.extensions);

        for (VulkanLayer layer : IterateBitSet(info.layers)) {
            DAWN_TRY_ASSIGN(
                info.layerExtensions[layer],
                GatherInstanceExtensions(GetVulkanLayerInfo(layer).name, vkFunctions, knownExts));
            MarkPromotedExtensions(&info.layerExtensions[layer], info.apiVersion);
            info.layerExtensions[layer] = EnsureDependencies(info.layerExtensions[layer]);
        }
    }

    return std::move(info);
}

ResultOrError<std::vector<VkPhysicalDevice>> GatherPhysicalDevices(
    VkInstance instance,
    const VulkanFunctions& vkFunctions) {
    uint32_t count = 0;
    VkResult result =
        VkResult::WrapUnsafe(vkFunctions.EnumeratePhysicalDevices(instance, &count, nullptr));
    if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
        return DAWN_INTERNAL_ERROR("vkEnumeratePhysicalDevices");
    }

    std::vector<VkPhysicalDevice> vkPhysicalDevices(count);
    DAWN_TRY(CheckVkSuccess(
        vkFunctions.EnumeratePhysicalDevices(instance, &count, vkPhysicalDevices.data()),
        "vkEnumeratePhysicalDevices"));

    return std::move(vkPhysicalDevices);
}

ResultOrError<VulkanDeviceInfo> GatherDeviceInfo(const PhysicalDevice& device) {
    VulkanDeviceInfo info = {};
    VkPhysicalDevice vkPhysicalDevice = device.GetVkPhysicalDevice();
    const VulkanGlobalInfo& globalInfo = device.GetVulkanInstance()->GetGlobalInfo();
    const VulkanFunctions& vkFunctions = device.GetVulkanInstance()->GetFunctions();

    // Query the device properties first to get the ICD's `apiVersion`
    vkFunctions.GetPhysicalDeviceProperties(vkPhysicalDevice, &info.properties);

    // Gather info about device memory.
    {
        VkPhysicalDeviceMemoryProperties memory;
        vkFunctions.GetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memory);

        info.memoryTypes.assign(memory.memoryTypes, memory.memoryTypes + memory.memoryTypeCount);
        info.memoryHeaps.assign(memory.memoryHeaps, memory.memoryHeaps + memory.memoryHeapCount);
    }

    // Gather info about device queue families
    {
        uint32_t count = 0;
        vkFunctions.GetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);

        info.queueFamilies.resize(count);
        vkFunctions.GetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count,
                                                           info.queueFamilies.data());
    }

    // Gather the info about the device layers
    {
        uint32_t count = 0;
        VkResult result = VkResult::WrapUnsafe(
            vkFunctions.EnumerateDeviceLayerProperties(vkPhysicalDevice, &count, nullptr));
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_INTERNAL_ERROR("vkEnumerateDeviceLayerProperties");
        }

        info.layers.resize(count);
        DAWN_TRY(CheckVkSuccess(vkFunctions.EnumerateDeviceLayerProperties(vkPhysicalDevice, &count,
                                                                           info.layers.data()),
                                "vkEnumerateDeviceLayerProperties"));
    }

    // Gather the info about the device extensions
    {
        uint32_t count = 0;
        VkResult result = VkResult::WrapUnsafe(vkFunctions.EnumerateDeviceExtensionProperties(
            vkPhysicalDevice, nullptr, &count, nullptr));
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_INTERNAL_ERROR("vkEnumerateDeviceExtensionProperties");
        }

        std::vector<VkExtensionProperties> extensionsProperties;
        extensionsProperties.resize(count);
        DAWN_TRY(CheckVkSuccess(vkFunctions.EnumerateDeviceExtensionProperties(
                                    vkPhysicalDevice, nullptr, &count, extensionsProperties.data()),
                                "vkEnumerateDeviceExtensionProperties"));

        std::unordered_map<std::string, DeviceExt> knownExts = CreateDeviceExtNameMap();

        for (const VkExtensionProperties& extension : extensionsProperties) {
            auto it = knownExts.find(extension.extensionName);
            if (it != knownExts.end()) {
                info.extensions.set(it->second, true);
            }
        }

        MarkPromotedExtensions(&info.extensions, info.properties.apiVersion);
        info.extensions = EnsureDependencies(info.extensions, globalInfo.extensions);
    }

    // Gather general and extension features and properties
    //
    // If we have DeviceExt::GetPhysicalDeviceProperties2, use features2 and properties2 so
    // that features not covered by VkPhysicalDevice{Features,Properties} can be queried.
    //
    // Note that info.properties has already been filled at the start of this function to get
    // `apiVersion`.
    ASSERT(info.properties.apiVersion != 0);
    if (info.extensions[DeviceExt::GetPhysicalDeviceProperties2]) {
        VkPhysicalDeviceFeatures2 features2 = {};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = nullptr;
        PNextChainBuilder featuresChain(&features2);

        VkPhysicalDeviceProperties2 properties2 = {};
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        PNextChainBuilder propertiesChain(&properties2);

        propertiesChain.Add(&info.propertiesMaintenance3,
                            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES);

        if (info.extensions[DeviceExt::ShaderFloat16Int8]) {
            featuresChain.Add(&info.shaderFloat16Int8Features,
                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR);
        }

        if (info.extensions[DeviceExt::_16BitStorage]) {
            featuresChain.Add(&info._16BitStorageFeatures,
                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
        }

        if (info.extensions[DeviceExt::SubgroupSizeControl]) {
            featuresChain.Add(&info.subgroupSizeControlFeatures,
                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT);
            propertiesChain.Add(
                &info.subgroupSizeControlProperties,
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT);
        }

        if (info.extensions[DeviceExt::DriverProperties]) {
            propertiesChain.Add(&info.driverProperties,
                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES);
        }

        if (info.extensions[DeviceExt::ShaderIntegerDotProduct]) {
            featuresChain.Add(
                &info.shaderIntegerDotProductFeatures,
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES_KHR);

            propertiesChain.Add(
                &info.shaderIntegerDotProductProperties,
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES_KHR);
        }

        if (info.extensions[DeviceExt::DepthClipEnable]) {
            featuresChain.Add(&info.depthClipEnableFeatures,
                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT);
        }

        if (info.extensions[DeviceExt::Maintenance4]) {
            propertiesChain.Add(&info.propertiesMaintenance4,
                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES);
        }

        if (info.extensions[DeviceExt::ZeroInitializeWorkgroupMemory]) {
            featuresChain.Add(
                &info.zeroInitializeWorkgroupMemoryFeatures,
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES);
        }

        if (info.extensions[DeviceExt::Robustness2]) {
            featuresChain.Add(&info.robustness2Features,
                              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT);
        }

        // Check subgroup features and properties
        propertiesChain.Add(&info.subgroupProperties,
                            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES);
        if (info.extensions[DeviceExt::ShaderSubgroupUniformControlFlow]) {
            featuresChain.Add(
                &info.shaderSubgroupUniformControlFlowFeatures,
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR);
        }

        // Use vkGetPhysicalDevice{Features,Properties}2 if required to gather information about
        // the extensions. DeviceExt::GetPhysicalDeviceProperties2 is guaranteed to be available
        // because these extensions (transitively) depend on it in `EnsureDependencies`
        vkFunctions.GetPhysicalDeviceProperties2(vkPhysicalDevice, &properties2);
        vkFunctions.GetPhysicalDeviceFeatures2(vkPhysicalDevice, &features2);
        info.features = features2.features;
    } else {
        vkFunctions.GetPhysicalDeviceFeatures(vkPhysicalDevice, &info.features);
    }

    // TODO(cwallez@chromium.org): gather info about formats

    return std::move(info);
}

ResultOrError<VulkanSurfaceInfo> GatherSurfaceInfo(const PhysicalDevice& device,
                                                   VkSurfaceKHR surface) {
    VulkanSurfaceInfo info = {};

    VkPhysicalDevice vkPhysicalDevice = device.GetVkPhysicalDevice();
    const VulkanFunctions& vkFunctions = device.GetVulkanInstance()->GetFunctions();

    // Get the surface capabilities
    DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceCapabilitiesKHR(
                                vkPhysicalDevice, surface, &info.capabilities),
                            "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));

    // Query which queue families support presenting this surface
    {
        size_t nQueueFamilies = device.GetDeviceInfo().queueFamilies.size();
        info.supportedQueueFamilies.resize(nQueueFamilies, false);

        for (uint32_t i = 0; i < nQueueFamilies; ++i) {
            VkBool32 supported = VK_FALSE;
            DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceSupportKHR(
                                        vkPhysicalDevice, i, surface, &supported),
                                    "vkGetPhysicalDeviceSurfaceSupportKHR"));

            info.supportedQueueFamilies[i] = (supported == VK_TRUE);
        }
    }

    // Gather supported formats
    {
        uint32_t count = 0;
        VkResult result = VkResult::WrapUnsafe(vkFunctions.GetPhysicalDeviceSurfaceFormatsKHR(
            vkPhysicalDevice, surface, &count, nullptr));
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_INTERNAL_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR");
        }

        info.formats.resize(count);
        DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfaceFormatsKHR(
                                    vkPhysicalDevice, surface, &count, info.formats.data()),
                                "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    }

    // Gather supported presents modes
    {
        uint32_t count = 0;
        VkResult result = VkResult::WrapUnsafe(vkFunctions.GetPhysicalDeviceSurfacePresentModesKHR(
            vkPhysicalDevice, surface, &count, nullptr));
        if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
            return DAWN_INTERNAL_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR");
        }

        info.presentModes.resize(count);
        DAWN_TRY(CheckVkSuccess(vkFunctions.GetPhysicalDeviceSurfacePresentModesKHR(
                                    vkPhysicalDevice, surface, &count, info.presentModes.data()),
                                "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    }

    return std::move(info);
}

}  // namespace dawn::native::vulkan
