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

#include "dawn_native/vulkan/BackendVk.h"

#include "common/Log.h"
#include "common/SystemUtils.h"
#include "dawn_native/Instance.h"
#include "dawn_native/VulkanBackend.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/VulkanError.h"

// TODO(crbug.com/dawn/283): Link against the Vulkan Loader and remove this.
#if defined(DAWN_ENABLE_SWIFTSHADER)
#    if defined(DAWN_PLATFORM_LINUX) || defined(DAWN_PLATFORM_FUSCHIA)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.so";
#    elif defined(DAWN_PLATFORM_WINDOWS)
constexpr char kSwiftshaderLibName[] = "vk_swiftshader.dll";
#    elif defined(DAWN_PLATFORM_MACOS)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.dylib";
#    else
#        error "Unimplemented Swiftshader Vulkan backend platform"
#    endif
#endif

#if defined(DAWN_PLATFORM_LINUX)
#    if defined(DAWN_PLATFORM_ANDROID)
constexpr char kVulkanLibName[] = "libvulkan.so";
#    else
constexpr char kVulkanLibName[] = "libvulkan.so.1";
#    endif
#elif defined(DAWN_PLATFORM_WINDOWS)
constexpr char kVulkanLibName[] = "vulkan-1.dll";
#elif defined(DAWN_PLATFORM_FUCHSIA)
constexpr char kVulkanLibName[] = "libvulkan.so";
#elif defined(DAWN_ENABLE_SWIFTSHADER)
const char* kVulkanLibName = kSwiftshaderLibName;
#else
#    error "Unimplemented Vulkan backend platform"
#endif

namespace dawn_native { namespace vulkan {

    Backend::Backend(InstanceBase* instance)
        : BackendConnection(instance, wgpu::BackendType::Vulkan) {
    }

    Backend::~Backend() {
        if (mDebugReportCallback != VK_NULL_HANDLE) {
            mFunctions.DestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, nullptr);
            mDebugReportCallback = VK_NULL_HANDLE;
        }

        // VkPhysicalDevices are destroyed when the VkInstance is destroyed
        if (mInstance != VK_NULL_HANDLE) {
            mFunctions.DestroyInstance(mInstance, nullptr);
            mInstance = VK_NULL_HANDLE;
        }
    }

    const VulkanFunctions& Backend::GetFunctions() const {
        return mFunctions;
    }

    VkInstance Backend::GetVkInstance() const {
        return mInstance;
    }

    const VulkanGlobalInfo& Backend::GetGlobalInfo() const {
        return mGlobalInfo;
    }

    MaybeError Backend::LoadVulkan() {
#if defined(DAWN_ENABLE_VULKAN_VALIDATION_LAYERS)
        if (GetInstance()->IsBackendValidationEnabled()) {
            std::string vkDataDir = GetExecutableDirectory() + DAWN_VK_DATA_DIR;
            if (!SetEnvironmentVar("VK_LAYER_PATH", vkDataDir.c_str())) {
                return DAWN_INTERNAL_ERROR("Couldn't set VK_LAYER_PATH");
            }
        }
#endif
#if defined(DAWN_SWIFTSHADER_VK_ICD_JSON)
        std::string fullSwiftshaderICDPath =
            GetExecutableDirectory() + DAWN_SWIFTSHADER_VK_ICD_JSON;
        if (!SetEnvironmentVar("VK_ICD_FILENAMES", fullSwiftshaderICDPath.c_str())) {
            return DAWN_INTERNAL_ERROR("Couldn't set VK_ICD_FILENAMES");
        }
#endif

        if (mVulkanLib.Open(kVulkanLibName)) {
            return {};
        }
        dawn::WarningLog() << std::string("Couldn't open ") + kVulkanLibName;

#if defined(DAWN_ENABLE_SWIFTSHADER)
        if (strcmp(kVulkanLibName, kSwiftshaderLibName) != 0) {
            if (mVulkanLib.Open(kSwiftshaderLibName)) {
                return {};
            }
            dawn::WarningLog() << std::string("Couldn't open ") + kSwiftshaderLibName;
        }
#endif

        return DAWN_INTERNAL_ERROR("Couldn't load Vulkan");
    }

    MaybeError Backend::Initialize() {
        DAWN_TRY(LoadVulkan());

        DAWN_TRY(mFunctions.LoadGlobalProcs(mVulkanLib));

        DAWN_TRY_ASSIGN(mGlobalInfo, GatherGlobalInfo(*this));

        VulkanGlobalKnobs usedGlobalKnobs = {};
        DAWN_TRY_ASSIGN(usedGlobalKnobs, CreateInstance());
        *static_cast<VulkanGlobalKnobs*>(&mGlobalInfo) = usedGlobalKnobs;

        DAWN_TRY(mFunctions.LoadInstanceProcs(mInstance, mGlobalInfo));

        if (usedGlobalKnobs.debugReport) {
            DAWN_TRY(RegisterDebugReport());
        }

        DAWN_TRY_ASSIGN(mPhysicalDevices, GetPhysicalDevices(*this));

        return {};
    }

    std::vector<std::unique_ptr<AdapterBase>> Backend::DiscoverDefaultAdapters() {
        std::vector<std::unique_ptr<AdapterBase>> adapters;

        for (VkPhysicalDevice physicalDevice : mPhysicalDevices) {
            std::unique_ptr<Adapter> adapter = std::make_unique<Adapter>(this, physicalDevice);

            if (GetInstance()->ConsumedError(adapter->Initialize())) {
                continue;
            }

            adapters.push_back(std::move(adapter));
        }

        return adapters;
    }

    ResultOrError<VulkanGlobalKnobs> Backend::CreateInstance() {
        VulkanGlobalKnobs usedKnobs = {};

        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;

        // vktrace works by instering a layer, but we hide it behind a macro due to the vktrace
        // layer crashes when used without vktrace server started. See this vktrace issue:
        // https://github.com/LunarG/VulkanTools/issues/254
        // Also it is good to put it in first position so that it doesn't see Vulkan calls inserted
        // by other layers.
#if defined(DAWN_USE_VKTRACE)
        if (mGlobalInfo.vktrace) {
            layersToRequest.push_back(kLayerNameLunargVKTrace);
            usedKnobs.vktrace = true;
        }
#endif
        // RenderDoc installs a layer at the system level for its capture but we don't want to use
        // it unless we are debugging in RenderDoc so we hide it behind a macro.
#if defined(DAWN_USE_RENDERDOC)
        if (mGlobalInfo.renderDocCapture) {
            layersToRequest.push_back(kLayerNameRenderDocCapture);
            usedKnobs.renderDocCapture = true;
        }
#endif

        if (GetInstance()->IsBackendValidationEnabled()) {
            if (mGlobalInfo.standardValidation) {
                layersToRequest.push_back(kLayerNameLunargStandardValidation);
                usedKnobs.standardValidation = true;
            }
            if (mGlobalInfo.debugReport) {
                extensionsToRequest.push_back(kExtensionNameExtDebugReport);
                usedKnobs.debugReport = true;
            }
        }

        // Always request all extensions used to create VkSurfaceKHR objects so that they are
        // always available for embedders looking to create VkSurfaceKHR on our VkInstance.
        if (mGlobalInfo.fuchsiaImagePipeSwapchain) {
            layersToRequest.push_back(kLayerNameFuchsiaImagePipeSwapchain);
            usedKnobs.fuchsiaImagePipeSwapchain = true;
        }
        if (mGlobalInfo.metalSurface) {
            extensionsToRequest.push_back(kExtensionNameExtMetalSurface);
            usedKnobs.metalSurface = true;
        }
        if (mGlobalInfo.surface) {
            extensionsToRequest.push_back(kExtensionNameKhrSurface);
            usedKnobs.surface = true;
        }
        if (mGlobalInfo.waylandSurface) {
            extensionsToRequest.push_back(kExtensionNameKhrWaylandSurface);
            usedKnobs.waylandSurface = true;
        }
        if (mGlobalInfo.win32Surface) {
            extensionsToRequest.push_back(kExtensionNameKhrWin32Surface);
            usedKnobs.win32Surface = true;
        }
        if (mGlobalInfo.xcbSurface) {
            extensionsToRequest.push_back(kExtensionNameKhrXcbSurface);
            usedKnobs.xcbSurface = true;
        }
        if (mGlobalInfo.xlibSurface) {
            extensionsToRequest.push_back(kExtensionNameKhrXlibSurface);
            usedKnobs.xlibSurface = true;
        }
        if (mGlobalInfo.fuchsiaImagePipeSurface) {
            extensionsToRequest.push_back(kExtensionNameFuchsiaImagePipeSurface);
            usedKnobs.fuchsiaImagePipeSurface = true;
        }

        // Mark the promoted extensions as present if the core version in which they were promoted
        // is used. This allows having a single boolean that checks if the functionality from that
        // extension is available (instead of checking extension || coreVersion).
        if (mGlobalInfo.apiVersion >= VK_MAKE_VERSION(1, 1, 0)) {
            usedKnobs.getPhysicalDeviceProperties2 = true;
            usedKnobs.externalMemoryCapabilities = true;
            usedKnobs.externalSemaphoreCapabilities = true;
        } else {
            if (mGlobalInfo.externalMemoryCapabilities) {
                extensionsToRequest.push_back(kExtensionNameKhrExternalMemoryCapabilities);
                usedKnobs.externalMemoryCapabilities = true;
            }
            if (mGlobalInfo.externalSemaphoreCapabilities) {
                extensionsToRequest.push_back(kExtensionNameKhrExternalSemaphoreCapabilities);
                usedKnobs.externalSemaphoreCapabilities = true;
            }
            if (mGlobalInfo.getPhysicalDeviceProperties2) {
                extensionsToRequest.push_back(kExtensionNameKhrGetPhysicalDeviceProperties2);
                usedKnobs.getPhysicalDeviceProperties2 = true;
            }
        }

        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = nullptr;
        appInfo.applicationVersion = 0;
        appInfo.pEngineName = nullptr;
        appInfo.engineVersion = 0;
        appInfo.apiVersion = mGlobalInfo.apiVersion;

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
        createInfo.ppEnabledLayerNames = layersToRequest.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsToRequest.size());
        createInfo.ppEnabledExtensionNames = extensionsToRequest.data();

        DAWN_TRY(CheckVkSuccess(mFunctions.CreateInstance(&createInfo, nullptr, &mInstance),
                                "vkCreateInstance"));

        return usedKnobs;
    }

    MaybeError Backend::RegisterDebugReport() {
        VkDebugReportCallbackCreateInfoEXT createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = Backend::OnDebugReportCallback;
        createInfo.pUserData = this;

        return CheckVkSuccess(mFunctions.CreateDebugReportCallbackEXT(
                                  mInstance, &createInfo, nullptr, &*mDebugReportCallback),
                              "vkCreateDebugReportcallback");
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    Backend::OnDebugReportCallback(VkDebugReportFlagsEXT flags,
                                   VkDebugReportObjectTypeEXT /*objectType*/,
                                   uint64_t /*object*/,
                                   size_t /*location*/,
                                   int32_t /*messageCode*/,
                                   const char* /*pLayerPrefix*/,
                                   const char* pMessage,
                                   void* /*pUserdata*/) {
        dawn::WarningLog() << pMessage;
        ASSERT((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);

        return VK_FALSE;
    }

    BackendConnection* Connect(InstanceBase* instance) {
        Backend* backend = new Backend(instance);

        if (instance->ConsumedError(backend->Initialize())) {
            delete backend;
            return nullptr;
        }

        return backend;
    }

}}  // namespace dawn_native::vulkan
