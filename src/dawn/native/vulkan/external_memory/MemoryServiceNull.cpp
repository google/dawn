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

#include <memory>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"

namespace dawn::native::vulkan::external_memory {

class ServiceImplementationNull : public ServiceImplementation {
  public:
    explicit ServiceImplementationNull(Device* device)
        : ServiceImplementation(device), mSupported(CheckSupport(device->GetDeviceInfo())) {}
    ~ServiceImplementationNull() override = default;

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo) { return false; }

    bool SupportsImportMemory(VkFormat format,
                              VkImageType type,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkImageCreateFlags flags) override {
        return false;
    }

    bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                             VkFormat format,
                             VkImageUsageFlags usage,
                             bool* supportsDisjoint) override {
        *supportsDisjoint = false;
        return false;
    }

    ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image) override {
        return DAWN_UNIMPLEMENTED_ERROR("Using null memory service to interop inside Vulkan");
    }

    uint32_t GetQueueFamilyIndex() override { return VK_QUEUE_FAMILY_EXTERNAL_KHR; }

    ResultOrError<VkDeviceMemory> ImportMemory(ExternalMemoryHandle handle,
                                               const MemoryImportParams& importParams,
                                               VkImage image) override {
        return DAWN_UNIMPLEMENTED_ERROR("Using null memory service to interop inside Vulkan");
    }

    ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                       const VkImageCreateInfo& baseCreateInfo) override {
        return DAWN_UNIMPLEMENTED_ERROR("Using null memory service to interop inside Vulkan");
    }

    bool Supported() const override { return false; }

  private:
    bool mSupported = false;
};

Service::Service(Device* device) {
    mImpl = std::make_unique<ServiceImplementationNull>(device);
}

// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo) {
    return false;
}

}  // namespace dawn::native::vulkan::external_memory
