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

}}  // namespace dawn_native::vulkan
