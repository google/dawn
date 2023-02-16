// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SERVICEIMPLEMENTATION_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SERVICEIMPLEMENTATION_H_

#include "dawn/native/Error.h"
#include "dawn/native/vulkan/ExternalHandle.h"

namespace dawn::native::vulkan {
class Device;
}  // namespace dawn::native::vulkan

namespace dawn::native::vulkan::external_semaphore {

class ServiceImplementation {
  public:
    explicit ServiceImplementation(Device* device);
    virtual ~ServiceImplementation();

    // True if the device reports it supports this feature
    virtual bool Supported() = 0;

    // Given an external handle, import it into a VkSemaphore
    virtual ResultOrError<VkSemaphore> ImportSemaphore(ExternalSemaphoreHandle handle) = 0;

    // Create a VkSemaphore that is exportable into an external handle later
    virtual ResultOrError<VkSemaphore> CreateExportableSemaphore() = 0;

    // Export a VkSemaphore into an external handle
    virtual ResultOrError<ExternalSemaphoreHandle> ExportSemaphore(VkSemaphore semaphore) = 0;

    // Duplicate a new external handle from the given one.
    virtual ExternalSemaphoreHandle DuplicateHandle(ExternalSemaphoreHandle handle) = 0;

    // Close external handle to release it.
    virtual void CloseHandle(ExternalSemaphoreHandle handle) = 0;

  protected:
    Device* mDevice = nullptr;
};

}  // namespace dawn::native::vulkan::external_semaphore

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNAL_SEMAPHORE_SERVICEIMPLEMENTATION_H_
