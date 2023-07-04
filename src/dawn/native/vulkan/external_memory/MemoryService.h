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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_

#include <memory>

#include "dawn/common/ityp_array.h"
#include "dawn/native/Error.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/external_memory/MemoryImportParams.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"

namespace dawn::native::vulkan {
class Device;
struct VulkanDeviceInfo;
}  // namespace dawn::native::vulkan

namespace dawn::native::vulkan::external_memory {

class Service {
  public:
    explicit Service(Device* device);
    ~Service();

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo);

    // True if the device reports it supports importing external memory.
    bool SupportsImportMemory(ExternalImageType externalImageType,
                              VkFormat format,
                              VkImageType type,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkImageCreateFlags flags);

    // True if the device reports it supports creating VkImages from external memory.
    bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                             VkFormat format,
                             VkImageUsageFlags usage,
                             bool* supportsDisjoint);

    // Returns the parameters required for importing memory
    ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image);

    // Returns the index of the queue memory from this services should be exported with.
    uint32_t GetQueueFamilyIndex(ExternalImageType externalImageType);

    // Given an external handle pointing to memory, import it into a VkDeviceMemory
    ResultOrError<VkDeviceMemory> ImportMemory(ExternalImageType externalImageType,
                                               ExternalMemoryHandle handle,
                                               const MemoryImportParams& importParams,
                                               VkImage image);

    // Create a VkImage for the given handle type
    ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                       const VkImageCreateInfo& baseCreateInfo);

  private:
    ServiceImplementation* GetServiceImplementation(ExternalImageType externalImageType);

    // ExternalImageType has 7 types. Vulkan backend uses 3 of them.
    ityp::array<ExternalImageType,
                std::unique_ptr<ServiceImplementation>,
                ExternalImageType::Last + 1>
        mServiceImpls = {};
};

}  // namespace dawn::native::vulkan::external_memory

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_
