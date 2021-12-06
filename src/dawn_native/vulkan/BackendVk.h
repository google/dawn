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

#ifndef DAWNNATIVE_VULKAN_BACKENDVK_H_
#define DAWNNATIVE_VULKAN_BACKENDVK_H_

#include "dawn_native/BackendConnection.h"

#include "common/DynamicLib.h"
#include "common/RefCounted.h"
#include "common/ityp_array.h"
#include "dawn_native/vulkan/VulkanFunctions.h"
#include "dawn_native/vulkan/VulkanInfo.h"

namespace dawn_native { namespace vulkan {

    enum class ICD {
        None,
        SwiftShader,
    };

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
        ~VulkanInstance();

        const VulkanFunctions& GetFunctions() const;
        VkInstance GetVkInstance() const;
        const VulkanGlobalInfo& GetGlobalInfo() const;
        const std::vector<VkPhysicalDevice>& GetPhysicalDevices() const;

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
    };

    class Backend : public BackendConnection {
      public:
        Backend(InstanceBase* instance);
        ~Backend() override;

        MaybeError Initialize();

        std::vector<std::unique_ptr<AdapterBase>> DiscoverDefaultAdapters() override;
        ResultOrError<std::vector<std::unique_ptr<AdapterBase>>> DiscoverAdapters(
            const AdapterDiscoveryOptionsBase* optionsBase) override;

      private:
        ityp::array<ICD, Ref<VulkanInstance>, 2> mVulkanInstances = {};
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_BACKENDVK_H_
