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

#include "dawn/native/vulkan/DeviceVk.h"

namespace dawn::native::vulkan::external_memory {

bool Service::RequiresDedicatedAllocation(const ExternalImageDescriptorVk* descriptor,
                                          VkImage image) {
    switch (descriptor->dedicatedAllocation) {
        case NeedsDedicatedAllocation::Yes:
            return true;

        case NeedsDedicatedAllocation::No:
            return false;

        case NeedsDedicatedAllocation::Detect:
            if (!mDevice->GetDeviceInfo().HasExt(DeviceExt::DedicatedAllocation)) {
                return false;
            }

            VkMemoryDedicatedRequirements dedicatedRequirements;
            dedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
            dedicatedRequirements.pNext = nullptr;

            VkMemoryRequirements2 baseRequirements;
            baseRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
            baseRequirements.pNext = &dedicatedRequirements;

            VkImageMemoryRequirementsInfo2 imageInfo;
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
            imageInfo.pNext = nullptr;
            imageInfo.image = image;

            mDevice->fn.GetImageMemoryRequirements2(mDevice->GetVkDevice(), &imageInfo,
                                                    &baseRequirements);

            // The Vulkan spec requires that prefersDA is set if requiresDA is, so we can just check
            // for prefersDA.
            return dedicatedRequirements.prefersDedicatedAllocation;
    }
    DAWN_UNREACHABLE();
}

}  // namespace dawn::native::vulkan::external_memory
