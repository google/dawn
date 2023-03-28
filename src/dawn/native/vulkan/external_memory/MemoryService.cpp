// Copyright 2022 The Dawn Authors
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

#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"

namespace dawn::native::vulkan::external_memory {

Service::~Service() = default;

bool Service::SupportsImportMemory(VkFormat format,
                                   VkImageType type,
                                   VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkImageCreateFlags flags) {
    return mImpl->SupportsImportMemory(format, type, tiling, usage, flags);
}

bool Service::SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  bool* supportsDisjoint) {
    return mImpl->SupportsCreateImage(descriptor, format, usage, supportsDisjoint);
}

ResultOrError<MemoryImportParams> Service::GetMemoryImportParams(
    const ExternalImageDescriptor* descriptor,
    VkImage image) {
    return mImpl->GetMemoryImportParams(descriptor, image);
}

uint32_t Service::GetQueueFamilyIndex() {
    return mImpl->GetQueueFamilyIndex();
}

ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalMemoryHandle handle,
                                                    const MemoryImportParams& importParams,
                                                    VkImage image) {
    return mImpl->ImportMemory(handle, importParams, image);
}

ResultOrError<VkImage> Service::CreateImage(const ExternalImageDescriptor* descriptor,
                                            const VkImageCreateInfo& baseCreateInfo) {
    return mImpl->CreateImage(descriptor, baseCreateInfo);
}

}  // namespace dawn::native::vulkan::external_memory
