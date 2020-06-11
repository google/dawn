// Copyright 2020 The Dawn Authors
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

#include "dawn_native/vulkan/VulkanExtensions.h"

#include "common/Assert.h"
#include "common/vulkan_platform.h"

#include <array>
#include <limits>

namespace dawn_native { namespace vulkan {

    static constexpr uint32_t VulkanVersion_1_1 = VK_MAKE_VERSION(1, 1, 0);
    static constexpr uint32_t VulkanVersion_1_2 = VK_MAKE_VERSION(1, 2, 0);
    static constexpr uint32_t NeverPromoted = std::numeric_limits<uint32_t>::max();

    // A static array for InstanceExtInfo that can be indexed with InstanceExts.
    // GetInstanceExtInfo checks that "index" matches the index used to access this array so an
    // assert will fire if it isn't in the correct order.
    static constexpr size_t kInstanceExtCount = static_cast<size_t>(InstanceExt::EnumCount);
    static constexpr std::array<InstanceExtInfo, kInstanceExtCount> sInstanceExtInfos{{
        //
        {InstanceExt::GetPhysicalDeviceProperties2, "VK_KHR_get_physical_device_properties2",
         VulkanVersion_1_1},
        {InstanceExt::ExternalMemoryCapabilities, "VK_KHR_external_memory_capabilities",
         VulkanVersion_1_1},
        {InstanceExt::ExternalSemaphoreCapabilities, "VK_KHR_external_semaphore_capabilities",
         VulkanVersion_1_1},

        {InstanceExt::Surface, "VK_KHR_surface", NeverPromoted},
        {InstanceExt::FuchsiaImagePipeSurface, "VK_FUCHSIA_imagepipe_surface", NeverPromoted},
        {InstanceExt::MetalSurface, "VK_EXT_metal_surface", NeverPromoted},
        {InstanceExt::WaylandSurface, "VK_KHR_wayland_surface", NeverPromoted},
        {InstanceExt::Win32Surface, "VK_KHR_win32_surface", NeverPromoted},
        {InstanceExt::XcbSurface, "VK_KHR_xcb_surface", NeverPromoted},
        {InstanceExt::XlibSurface, "VK_KHR_xlib_surface", NeverPromoted},

        {InstanceExt::DebugReport, "VK_EXT_debug_report", NeverPromoted}
        //
    }};

    void InstanceExtSet::Set(InstanceExt extension, bool enabled) {
        extensionBitSet.set(static_cast<uint32_t>(extension), enabled);
    }

    bool InstanceExtSet::Has(InstanceExt extension) const {
        return extensionBitSet[static_cast<uint32_t>(extension)];
    }

    const InstanceExtInfo& GetInstanceExtInfo(InstanceExt ext) {
        uint32_t index = static_cast<uint32_t>(ext);
        ASSERT(index < sInstanceExtInfos.size());
        ASSERT(sInstanceExtInfos[index].index == ext);
        return sInstanceExtInfos[index];
    }

    std::unordered_map<std::string, InstanceExt> CreateInstanceExtNameMap() {
        std::unordered_map<std::string, InstanceExt> result;
        for (const InstanceExtInfo& info : sInstanceExtInfos) {
            result[info.name] = info.index;
        }
        return result;
    }

    InstanceExtSet EnsureDependencies(const InstanceExtSet& advertisedExts) {
        // We need to check that all transitive dependencies of extensions are advertised.
        // To do that in a single pass and no data structures, the extensions are topologically
        // sorted in the definition of InstanceExt.
        // To ensure the order is correct, we mark visited extensions in `visitedSet` and each
        // dependency check will first assert all its dependents have been visited.
        InstanceExtSet visitedSet;
        InstanceExtSet trimmedSet;

        auto HasDep = [&](InstanceExt ext) -> bool {
            ASSERT(visitedSet.Has(ext));
            return trimmedSet.Has(ext);
        };

        for (uint32_t i = 0; i < sInstanceExtInfos.size(); i++) {
            InstanceExt ext = static_cast<InstanceExt>(i);

            bool hasDependencies = false;
            switch (ext) {
                case InstanceExt::GetPhysicalDeviceProperties2:
                case InstanceExt::Surface:
                case InstanceExt::DebugReport:
                    hasDependencies = true;
                    break;

                case InstanceExt::ExternalMemoryCapabilities:
                case InstanceExt::ExternalSemaphoreCapabilities:
                    hasDependencies = HasDep(InstanceExt::GetPhysicalDeviceProperties2);
                    break;

                case InstanceExt::FuchsiaImagePipeSurface:
                case InstanceExt::MetalSurface:
                case InstanceExt::WaylandSurface:
                case InstanceExt::Win32Surface:
                case InstanceExt::XcbSurface:
                case InstanceExt::XlibSurface:
                    hasDependencies = HasDep(InstanceExt::Surface);
                    break;

                default:
                    UNREACHABLE();
                    break;
            }

            trimmedSet.Set(ext, hasDependencies && advertisedExts.Has(ext));
            visitedSet.Set(ext, true);
        }

        return trimmedSet;
    }

    void MarkPromotedExtensions(InstanceExtSet* extensions, uint32_t version) {
        for (const InstanceExtInfo& info : sInstanceExtInfos) {
            if (info.versionPromoted <= version) {
                extensions->Set(info.index, true);
            }
        }
    }

    static constexpr size_t kDeviceExtCount = static_cast<size_t>(DeviceExt::EnumCount);
    static constexpr std::array<DeviceExtInfo, kDeviceExtCount> sDeviceExtInfos{{
        //
        {DeviceExt::Maintenance1, "VK_KHR_maintenance1", VulkanVersion_1_1},
        {DeviceExt::ExternalMemory, "VK_KHR_external_memory", VulkanVersion_1_1},
        {DeviceExt::ExternalSemaphore, "VK_KHR_external_semaphore", VulkanVersion_1_1},
        {DeviceExt::_16BitStorage, "VK_KHR_16bit_storage", VulkanVersion_1_1},

        {DeviceExt::ShaderFloat16Int8, "VK_KHR_shader_float16_int8", VulkanVersion_1_2},

        {DeviceExt::ExternalMemoryFD, "VK_KHR_external_memory_fd", NeverPromoted},
        {DeviceExt::ExternalMemoryDmaBuf, "VK_EXT_external_memory_dma_buf", NeverPromoted},
        {DeviceExt::ExternalMemoryZirconHandle, "VK_FUCHSIA_external_memory", NeverPromoted},
        {DeviceExt::ExternalSemaphoreFD, "VK_KHR_external_semaphore_fd", NeverPromoted},
        {DeviceExt::ExternalSemaphoreZirconHandle, "VK_FUCHSIA_external_semaphore", NeverPromoted},

        {DeviceExt::DebugMarker, "VK_EXT_debug_marker", NeverPromoted},
        {DeviceExt::ImageDrmFormatModifier, "VK_EXT_image_drm_format_modifier", NeverPromoted},
        {DeviceExt::Swapchain, "VK_KHR_swapchain", NeverPromoted},
        //
    }};

    void DeviceExtSet::Set(DeviceExt extension, bool enabled) {
        extensionBitSet.set(static_cast<uint32_t>(extension), enabled);
    }

    bool DeviceExtSet::Has(DeviceExt extension) const {
        return extensionBitSet[static_cast<uint32_t>(extension)];
    }

    const DeviceExtInfo& GetDeviceExtInfo(DeviceExt ext) {
        uint32_t index = static_cast<uint32_t>(ext);
        ASSERT(index < sDeviceExtInfos.size());
        ASSERT(sDeviceExtInfos[index].index == ext);
        return sDeviceExtInfos[index];
    }

    std::unordered_map<std::string, DeviceExt> CreateDeviceExtNameMap() {
        std::unordered_map<std::string, DeviceExt> result;
        for (const DeviceExtInfo& info : sDeviceExtInfos) {
            result[info.name] = info.index;
        }
        return result;
    }

    DeviceExtSet EnsureDependencies(const DeviceExtSet& advertisedExts,
                                    const InstanceExtSet& instanceExts) {
        // This is very similar to EnsureDependencies for instanceExtSet. See comment there for
        // an explanation of what happens.
        DeviceExtSet visitedSet;
        DeviceExtSet trimmedSet;

        auto HasDep = [&](DeviceExt ext) -> bool {
            ASSERT(visitedSet.Has(ext));
            return trimmedSet.Has(ext);
        };

        for (uint32_t i = 0; i < sDeviceExtInfos.size(); i++) {
            DeviceExt ext = static_cast<DeviceExt>(i);

            bool hasDependencies = false;
            switch (ext) {
                case DeviceExt::DebugMarker:
                    // TODO(cwallez@chromium.org): VK_KHR_debug_report is deprecated, switch to
                    // using VK_KHR_debug_utils instead.
                    hasDependencies = instanceExts.Has(InstanceExt::DebugReport);
                    break;

                case DeviceExt::ImageDrmFormatModifier:
                    // TODO(cwallez@chromium.org): ImageDrmFormatModifier actually requires:
                    //  - VK_KHR_bind_memory2
                    //  - VK_KHR_image_format_list
                    //  - VK_KHR_sampler_ycbcr_conversion
                    //
                    // Also switch to using DeviceExt::GetPhysicalDeviceProperties2 when we decouple
                    // Instance / Device physical device extensions.
                    hasDependencies = instanceExts.Has(InstanceExt::GetPhysicalDeviceProperties2);
                    break;

                case DeviceExt::Swapchain:
                    hasDependencies = instanceExts.Has(InstanceExt::Surface);
                    break;

                case DeviceExt::Maintenance1:
                    hasDependencies = true;
                    break;

                case DeviceExt::ShaderFloat16Int8:
                    // TODO(cwallez@chromium.org): switch to using
                    // DeviceExt::GetPhysicalDeviceProperties2 when we decouple Instance / Device
                    // physical device extensions.
                    hasDependencies = instanceExts.Has(InstanceExt::GetPhysicalDeviceProperties2);
                    break;

                case DeviceExt::ExternalMemory:
                    // TODO(cwallez@chromium.org): switch to using
                    // DeviceExt::ExternalMemoryCapabilities when we decouple Instance / Device
                    // physical device extensions.
                    hasDependencies = instanceExts.Has(InstanceExt::ExternalMemoryCapabilities);
                    break;

                case DeviceExt::ExternalSemaphore:
                    // TODO(cwallez@chromium.org): switch to using
                    // DeviceExt::ExternalSemaphoreCapabilities when we decouple Instance / Device
                    // physical device extensions.
                    hasDependencies = instanceExts.Has(InstanceExt::ExternalSemaphoreCapabilities);
                    break;

                case DeviceExt::_16BitStorage:
                    // TODO(cwallez@chromium.org): switch to using
                    // DeviceExt::GetPhysicalDeviceProperties2 when we decouple Instance / Device
                    // physical device extensions.
                    // Also depends on VK_KHR_storage_buffer_storage_class
                    hasDependencies = instanceExts.Has(InstanceExt::GetPhysicalDeviceProperties2);
                    break;

                case DeviceExt::ExternalMemoryFD:
                case DeviceExt::ExternalMemoryZirconHandle:
                    hasDependencies = HasDep(DeviceExt::ExternalMemory);
                    break;

                case DeviceExt::ExternalMemoryDmaBuf:
                    hasDependencies = HasDep(DeviceExt::ExternalMemoryFD);
                    break;

                case DeviceExt::ExternalSemaphoreFD:
                case DeviceExt::ExternalSemaphoreZirconHandle:
                    hasDependencies = HasDep(DeviceExt::ExternalSemaphore);
                    break;

                default:
                    UNREACHABLE();
                    break;
            }

            trimmedSet.Set(ext, hasDependencies && advertisedExts.Has(ext));
            visitedSet.Set(ext, true);
        }

        return trimmedSet;
    }

    void MarkPromotedExtensions(DeviceExtSet* extensions, uint32_t version) {
        for (const DeviceExtInfo& info : sDeviceExtInfos) {
            if (info.versionPromoted <= version) {
                extensions->Set(info.index, true);
            }
        }
    }

}}  // namespace dawn_native::vulkan
