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

#ifndef SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/native/BackendConnection.h"

#include "dawn/common/DynamicLib.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native::vulkan {

enum class ICD {
    None,
    SwiftShader,
};

class Device;

// VulkanInstance holds the reference to the Vulkan library, the VkInstance, VkPhysicalDevices
// on that instance, Vulkan functions loaded from the library, and global information
// gathered from the instance. VkPhysicalDevices bound to the VkInstance are bound to the GPU
// and GPU driver, keeping them active. It is RefCounted so that (eventually) when all adapters
// on an instance are no longer in use, the instance is deleted. This can be particuarly useful
// when we create multiple instances to selectively discover ICDs (like only
// SwiftShader/iGPU/dGPU/eGPU), and only one physical device on one instance remains in use. We
// can delete the VkInstances that are not in use to avoid holding the discrete GPU active.
class VulkanInstance : public RefCounted {
  public:
    static ResultOrError<Ref<VulkanInstance>> Create(const InstanceBase* instance, ICD icd);
    ~VulkanInstance() override;

    const VulkanFunctions& GetFunctions() const;
    VkInstance GetVkInstance() const;
    const VulkanGlobalInfo& GetGlobalInfo() const;
    const std::vector<VkPhysicalDevice>& GetPhysicalDevices() const;

    // TODO(dawn:831): This set of functions guards may need to be adjusted when Dawn is updated
    // to support multithreading.
    void StartListeningForDeviceMessages(Device* device);
    void StopListeningForDeviceMessages(Device* device);
    bool HandleDeviceMessage(std::string deviceDebugPrefix, std::string message);

  private:
    VulkanInstance();

    MaybeError Initialize(const InstanceBase* instance, ICD icd);
    ResultOrError<VulkanGlobalKnobs> CreateVkInstance(const InstanceBase* instance);

    MaybeError RegisterDebugUtils();

    DynamicLib mVulkanLib;
    VulkanGlobalInfo mGlobalInfo = {};
    VkInstance mInstance = VK_NULL_HANDLE;
    VulkanFunctions mFunctions;

    VkDebugUtilsMessengerEXT mDebugUtilsMessenger = VK_NULL_HANDLE;

    std::vector<VkPhysicalDevice> mPhysicalDevices;

    // Devices keep the VulkanInstance alive, so as long as devices remove themselves from this
    // map on destruction the pointers it contains should remain valid.
    std::unordered_map<std::string, Device*> mMessageListenerDevices;
    std::mutex mMessageListenerDevicesMutex;
};

class Backend : public BackendConnection {
  public:
    explicit Backend(InstanceBase* instance);
    ~Backend() override;

    MaybeError Initialize();

    std::vector<Ref<AdapterBase>> DiscoverDefaultAdapters() override;
    ResultOrError<std::vector<Ref<AdapterBase>>> DiscoverAdapters(
        const AdapterDiscoveryOptionsBase* optionsBase) override;

  private:
    ityp::array<ICD, Ref<VulkanInstance>, 2> mVulkanInstances = {};
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BACKENDVK_H_
