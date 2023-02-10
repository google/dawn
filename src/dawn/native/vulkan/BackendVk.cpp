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

#include "dawn/native/vulkan/BackendVk.h"

#include <algorithm>
#include <string>
#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

// TODO(crbug.com/dawn/283): Link against the Vulkan Loader and remove this.
#if defined(DAWN_ENABLE_SWIFTSHADER)
#if DAWN_PLATFORM_IS(LINUX) || DAWN_PLATFORM_IS(FUSCHIA)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.so";
#elif DAWN_PLATFORM_IS(WINDOWS)
constexpr char kSwiftshaderLibName[] = "vk_swiftshader.dll";
#elif DAWN_PLATFORM_IS(MACOS)
constexpr char kSwiftshaderLibName[] = "libvk_swiftshader.dylib";
#else
#error "Unimplemented Swiftshader Vulkan backend platform"
#endif
#endif

#if DAWN_PLATFORM_IS(LINUX)
#if DAWN_PLATFORM_IS(ANDROID)
constexpr char kVulkanLibName[] = "libvulkan.so";
#else
constexpr char kVulkanLibName[] = "libvulkan.so.1";
#endif
#elif DAWN_PLATFORM_IS(WINDOWS)
constexpr char kVulkanLibName[] = "vulkan-1.dll";
#elif DAWN_PLATFORM_IS(MACOS)
constexpr char kVulkanLibName[] = "libvulkan.dylib";
#elif DAWN_PLATFORM_IS(FUCHSIA)
constexpr char kVulkanLibName[] = "libvulkan.so";
#else
#error "Unimplemented Vulkan backend platform"
#endif

struct SkippedMessage {
    const char* messageId;
    const char* messageContents;
};

// Array of Validation error/warning messages that will be ignored, should include bugID
constexpr SkippedMessage kSkippedMessages[] = {
    // These errors are generated when simultaneously using a read-only depth/stencil attachment as
    // a texture binding. This is valid Vulkan.
    // The substring matching matches both
    // VK_PIPELINE_STAGE_2_NONE and VK_PIPELINE_STAGE_2_NONE_KHR.
    //
    // When storeOp=NONE is not present, Dawn uses storeOp=STORE, but Vulkan validation layer
    // considers the image read-only and produces a hazard. Dawn can't rely on storeOp=NONE and
    // so this is not expected to be worked around.
    // See http://crbug.com/dawn/1225 for more details.
    //
    // Depth used as storage
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    // Depth used in sampling
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "depth aspect during store with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    // Stencil used as storage
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_STORAGE_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},
    // Stencil used in sampling (note no tests actually hit this)
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "stencil aspect during store with stencilStoreOp VK_ATTACHMENT_STORE_OP_STORE. Access info "
     "(usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_SAMPLED_READ, read_barriers: VkPipelineStageFlags2KHR(0)"},

    // http://anglebug.com/7513
    {"VUID-VkGraphicsPipelineCreateInfo-pStages-06896",
     "contains fragment shader state, but stages"},

    // A warning that's generated on valid usage of the WebGPU API where a fragment output doesn't
    // have a corresponding attachment
    {"UNASSIGNED-CoreValidation-Shader-OutputNotConsumed",
     "fragment shader writes to output location 0 with no matching attachment"},
};

namespace dawn::native::vulkan {

namespace {

static constexpr ICD kICDs[] = {
// Other drivers should not be loaded with MSAN because they don't have MSAN instrumentation.
// MSAN will produce false positives since it cannot detect changes to memory that the driver
// has made.
#if !defined(MEMORY_SANITIZER)
    ICD::None,
#endif
#if defined(DAWN_ENABLE_SWIFTSHADER)
    ICD::SwiftShader,
#endif  // defined(DAWN_ENABLE_SWIFTSHADER)
};

// Suppress validation errors that are known. Returns false in that case.
bool ShouldReportDebugMessage(const char* messageId, const char* message) {
    // pMessageIdName may be NULL
    if (messageId == nullptr) {
        return true;
    }

    for (const SkippedMessage& msg : kSkippedMessages) {
        if (strstr(messageId, msg.messageId) != nullptr &&
            strstr(message, msg.messageContents) != nullptr) {
            return false;
        }
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
OnDebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT /* messageTypes */,
                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                     void* pUserData) {
    if (!ShouldReportDebugMessage(pCallbackData->pMessageIdName, pCallbackData->pMessage)) {
        return VK_FALSE;
    }

    if (!(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
        dawn::WarningLog() << pCallbackData->pMessage;
        return VK_FALSE;
    }

    if (pUserData == nullptr) {
        return VK_FALSE;
    }
    VulkanInstance* instance = reinterpret_cast<VulkanInstance*>(pUserData);

    // Look through all the object labels attached to the debug message and try to parse
    // a device debug prefix out of one of them. If a debug prefix is found and matches
    // a registered device, forward the message on to it.
    for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
        const VkDebugUtilsObjectNameInfoEXT& object = pCallbackData->pObjects[i];
        std::string deviceDebugPrefix = GetDeviceDebugPrefixFromDebugName(object.pObjectName);
        if (deviceDebugPrefix.empty()) {
            continue;
        }

        if (instance->HandleDeviceMessage(std::move(deviceDebugPrefix), pCallbackData->pMessage)) {
            return VK_FALSE;
        }
    }

    // We get to this line if no device was associated with the message. Crash so that the failure
    // is loud and makes tests fail in Debug.
    dawn::ErrorLog() << pCallbackData->pMessage;
    ASSERT(false);

    return VK_FALSE;
}

// A debug callback specifically for instance creation so that we don't fire an ASSERT when
// the instance fails creation in an expected manner (for example the system not having
// Vulkan drivers).
VKAPI_ATTR VkBool32 VKAPI_CALL
OnInstanceCreationDebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                     VkDebugUtilsMessageTypeFlagsEXT /* messageTypes */,
                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                     void* /* pUserData */) {
    dawn::WarningLog() << pCallbackData->pMessage;
    return VK_FALSE;
}

}  // anonymous namespace

VulkanInstance::VulkanInstance() = default;

VulkanInstance::~VulkanInstance() {
    ASSERT(mMessageListenerDevices.empty());

    if (mDebugUtilsMessenger != VK_NULL_HANDLE) {
        mFunctions.DestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);
        mDebugUtilsMessenger = VK_NULL_HANDLE;
    }

    // VkPhysicalDevices are destroyed when the VkInstance is destroyed
    if (mInstance != VK_NULL_HANDLE) {
        mFunctions.DestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
    }
}

const VulkanFunctions& VulkanInstance::GetFunctions() const {
    return mFunctions;
}

VkInstance VulkanInstance::GetVkInstance() const {
    return mInstance;
}

const VulkanGlobalInfo& VulkanInstance::GetGlobalInfo() const {
    return mGlobalInfo;
}

const std::vector<VkPhysicalDevice>& VulkanInstance::GetPhysicalDevices() const {
    return mPhysicalDevices;
}

// static
ResultOrError<Ref<VulkanInstance>> VulkanInstance::Create(const InstanceBase* instance, ICD icd) {
    Ref<VulkanInstance> vulkanInstance = AcquireRef(new VulkanInstance());
    DAWN_TRY(vulkanInstance->Initialize(instance, icd));
    return std::move(vulkanInstance);
}

MaybeError VulkanInstance::Initialize(const InstanceBase* instance, ICD icd) {
    // These environment variables need only be set while loading procs and gathering device
    // info.
    ScopedEnvironmentVar vkICDFilenames;
    ScopedEnvironmentVar vkLayerPath;

    const std::vector<std::string>& searchPaths = instance->GetRuntimeSearchPaths();

    auto CommaSeparatedResolvedSearchPaths = [&](const char* name) {
        std::string list;
        bool first = true;
        for (const std::string& path : searchPaths) {
            if (!first) {
                list += ", ";
            }
            first = false;
            list += (path + name);
        }
        return list;
    };

    auto LoadVulkan = [&](const char* libName) -> MaybeError {
        for (const std::string& path : searchPaths) {
            std::string resolvedPath = path + libName;
            if (mVulkanLib.Open(resolvedPath)) {
                return {};
            }
        }
        return DAWN_FORMAT_INTERNAL_ERROR("Couldn't load Vulkan. Searched %s.",
                                          CommaSeparatedResolvedSearchPaths(libName));
    };

    switch (icd) {
        case ICD::None: {
            DAWN_TRY(LoadVulkan(kVulkanLibName));
            // Succesfully loaded driver; break.
            break;
        }
        case ICD::SwiftShader: {
#if defined(DAWN_ENABLE_SWIFTSHADER)
            DAWN_TRY(LoadVulkan(kSwiftshaderLibName));
            break;
#endif  // defined(DAWN_ENABLE_SWIFTSHADER)
        // ICD::SwiftShader should not be passed if SwiftShader is not enabled.
            UNREACHABLE();
        }
    }

    if (instance->IsBackendValidationEnabled()) {
#if defined(DAWN_ENABLE_VULKAN_VALIDATION_LAYERS)
        auto execDir = GetExecutableDirectory();
        std::string vkDataDir = execDir.value_or("") + DAWN_VK_DATA_DIR;
        if (!vkLayerPath.Set("VK_LAYER_PATH", vkDataDir.c_str())) {
            return DAWN_INTERNAL_ERROR("Couldn't set VK_LAYER_PATH");
        }
#else
        dawn::WarningLog() << "Backend validation enabled but Dawn was not built with "
                              "DAWN_ENABLE_VULKAN_VALIDATION_LAYERS.";
#endif
    }

    DAWN_TRY(mFunctions.LoadGlobalProcs(mVulkanLib));

    DAWN_TRY_ASSIGN(mGlobalInfo, GatherGlobalInfo(mFunctions));

    VulkanGlobalKnobs usedGlobalKnobs = {};
    DAWN_TRY_ASSIGN(usedGlobalKnobs, CreateVkInstance(instance));
    *static_cast<VulkanGlobalKnobs*>(&mGlobalInfo) = usedGlobalKnobs;

    DAWN_TRY(mFunctions.LoadInstanceProcs(mInstance, mGlobalInfo));

    if (usedGlobalKnobs.HasExt(InstanceExt::DebugUtils)) {
        DAWN_TRY(RegisterDebugUtils());
    }

    DAWN_TRY_ASSIGN(mPhysicalDevices, GatherPhysicalDevices(mInstance, mFunctions));

    return {};
}

ResultOrError<VulkanGlobalKnobs> VulkanInstance::CreateVkInstance(const InstanceBase* instance) {
    VulkanGlobalKnobs usedKnobs = {};
    std::vector<const char*> layerNames;
    InstanceExtSet extensionsToRequest = mGlobalInfo.extensions;

    auto UseLayerIfAvailable = [&](VulkanLayer layer) {
        if (mGlobalInfo.layers[layer]) {
            layerNames.push_back(GetVulkanLayerInfo(layer).name);
            usedKnobs.layers.set(layer, true);
            extensionsToRequest |= mGlobalInfo.layerExtensions[layer];
        }
    };

    // vktrace works by instering a layer, but we hide it behind a macro because the vktrace
    // layer crashes when used without vktrace server started. See this vktrace issue:
    // https://github.com/LunarG/VulkanTools/issues/254
    // Also it is good to put it in first position so that it doesn't see Vulkan calls inserted
    // by other layers.
#if defined(DAWN_USE_VKTRACE)
    UseLayerIfAvailable(VulkanLayer::LunargVkTrace);
#endif
    // RenderDoc installs a layer at the system level for its capture but we don't want to use
    // it unless we are debugging in RenderDoc so we hide it behind a macro.
#if defined(DAWN_USE_RENDERDOC)
    UseLayerIfAvailable(VulkanLayer::RenderDocCapture);
#endif

    if (instance->IsBackendValidationEnabled()) {
        UseLayerIfAvailable(VulkanLayer::Validation);
    }

    // Always use the Fuchsia swapchain layer if available.
    UseLayerIfAvailable(VulkanLayer::FuchsiaImagePipeSwapchain);

    // Available and known instance extensions default to being requested, but some special
    // cases are removed.
    usedKnobs.extensions = extensionsToRequest;

    std::vector<const char*> extensionNames;
    for (InstanceExt ext : IterateBitSet(extensionsToRequest)) {
        const InstanceExtInfo& info = GetInstanceExtInfo(ext);

        if (info.versionPromoted > mGlobalInfo.apiVersion) {
            extensionNames.push_back(info.name);
        }
    }

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = nullptr;
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = 0;
    appInfo.apiVersion = std::min(mGlobalInfo.apiVersion, VK_API_VERSION_1_3);

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    PNextChainBuilder createInfoChain(&createInfo);

    // Register the debug callback for instance creation so we receive message for any errors
    // (validation or other).
    VkDebugUtilsMessengerCreateInfoEXT utilsMessengerCreateInfo;
    if (usedKnobs.HasExt(InstanceExt::DebugUtils)) {
        utilsMessengerCreateInfo.flags = 0;
        utilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        utilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        utilsMessengerCreateInfo.pfnUserCallback = OnInstanceCreationDebugUtilsCallback;
        utilsMessengerCreateInfo.pUserData = nullptr;

        createInfoChain.Add(&utilsMessengerCreateInfo,
                            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
    }

    // Try to turn on synchronization validation if the instance was created with backend
    // validation enabled.
    VkValidationFeaturesEXT validationFeatures;
    VkValidationFeatureEnableEXT kEnableSynchronizationValidation =
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
    if (instance->IsBackendValidationEnabled() &&
        usedKnobs.HasExt(InstanceExt::ValidationFeatures)) {
        validationFeatures.enabledValidationFeatureCount = 1;
        validationFeatures.pEnabledValidationFeatures = &kEnableSynchronizationValidation;
        validationFeatures.disabledValidationFeatureCount = 0;
        validationFeatures.pDisabledValidationFeatures = nullptr;

        createInfoChain.Add(&validationFeatures, VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT);
    }

    DAWN_TRY(CheckVkSuccess(mFunctions.CreateInstance(&createInfo, nullptr, &mInstance),
                            "vkCreateInstance"));

    return usedKnobs;
}

MaybeError VulkanInstance::RegisterDebugUtils() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = OnDebugUtilsCallback;
    createInfo.pUserData = this;

    return CheckVkSuccess(mFunctions.CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr,
                                                                  &*mDebugUtilsMessenger),
                          "vkCreateDebugUtilsMessengerEXT");
}

void VulkanInstance::StartListeningForDeviceMessages(Device* device) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    mMessageListenerDevices.insert({device->GetDebugPrefix(), device});
}
void VulkanInstance::StopListeningForDeviceMessages(Device* device) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    mMessageListenerDevices.erase(device->GetDebugPrefix());
}
bool VulkanInstance::HandleDeviceMessage(std::string deviceDebugPrefix, std::string message) {
    std::lock_guard<std::mutex> lock(mMessageListenerDevicesMutex);
    auto it = mMessageListenerDevices.find(deviceDebugPrefix);
    if (it != mMessageListenerDevices.end()) {
        it->second->OnDebugMessage(std::move(message));
        return true;
    }
    return false;
}

Backend::Backend(InstanceBase* instance) : BackendConnection(instance, wgpu::BackendType::Vulkan) {}

Backend::~Backend() = default;

std::vector<Ref<AdapterBase>> Backend::DiscoverDefaultAdapters() {
    AdapterDiscoveryOptions options;
    auto result = DiscoverAdapters(&options);
    if (result.IsError()) {
        GetInstance()->ConsumedError(result.AcquireError());
        return {};
    }
    return result.AcquireSuccess();
}

ResultOrError<std::vector<Ref<AdapterBase>>> Backend::DiscoverAdapters(
    const AdapterDiscoveryOptionsBase* optionsBase) {
    ASSERT(optionsBase->backendType == WGPUBackendType_Vulkan);

    const AdapterDiscoveryOptions* options =
        static_cast<const AdapterDiscoveryOptions*>(optionsBase);

    std::vector<Ref<AdapterBase>> adapters;

    InstanceBase* instance = GetInstance();
    for (ICD icd : kICDs) {
#if DAWN_PLATFORM_IS(MACOS)
        // On Mac, we don't expect non-Swiftshader Vulkan to be available.
        if (icd == ICD::None) {
            continue;
        }
#endif  // DAWN_PLATFORM_IS(MACOS)
        if (options->forceSwiftShader && icd != ICD::SwiftShader) {
            continue;
        }
        if (mVulkanInstances[icd] == nullptr && instance->ConsumedError([&]() -> MaybeError {
                DAWN_TRY_ASSIGN(mVulkanInstances[icd], VulkanInstance::Create(instance, icd));
                return {};
            }())) {
            // Instance failed to initialize.
            continue;
        }
        const std::vector<VkPhysicalDevice>& physicalDevices =
            mVulkanInstances[icd]->GetPhysicalDevices();
        for (uint32_t i = 0; i < physicalDevices.size(); ++i) {
            Ref<Adapter> adapter =
                AcquireRef(new Adapter(instance, mVulkanInstances[icd].Get(), physicalDevices[i]));
            if (instance->ConsumedError(adapter->Initialize())) {
                continue;
            }
            adapters.push_back(std::move(adapter));
        }
    }
    return adapters;
}

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance);
}

}  // namespace dawn::native::vulkan
