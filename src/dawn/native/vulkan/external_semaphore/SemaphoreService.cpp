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

#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"
#include "dawn/native/vulkan/external_semaphore/ServiceImplementation.h"

#if DAWN_PLATFORM_IS(FUCHSIA)
#include "dawn/native/vulkan/external_semaphore/ServiceImplementationZirconHandle.h"
#endif  // DAWN_PLATFORM_IS(FUCHSIA)

// Android, ChromeOS and Linux
#if DAWN_PLATFORM_IS(LINUX)
#include "dawn/native/vulkan/external_semaphore/ServiceImplementationFD.h"
#endif  // DAWN_PLATFORM_IS(LINUX)

namespace dawn::native::vulkan::external_semaphore {
// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo,
                           VkPhysicalDevice physicalDevice,
                           const VulkanFunctions& fn) {
#if DAWN_PLATFORM_IS(FUCHSIA)
    return CheckZirconHandleSupport(deviceInfo, physicalDevice, fn);
#elif DAWN_PLATFORM_IS(LINUX)  // Android, ChromeOS and Linux
    return CheckFDSupport(deviceInfo, physicalDevice, fn);
#else
    return false;
#endif
}

Service::Service(Device* device) {
#if DAWN_PLATFORM_IS(FUCHSIA)  // Fuchsia
    mServiceImpl = CreateZirconHandleService(device);
#elif DAWN_PLATFORM_IS(LINUX) || DAWN_PLATFORM_IS(CHROMEOS)  // Android, ChromeOS and Linux
    mServiceImpl = CreateFDService(device);
#endif
}

Service::~Service() = default;

bool Service::Supported() {
    if (!mServiceImpl) {
        return false;
    }

    return mServiceImpl->Supported();
}

void Service::CloseHandle(ExternalSemaphoreHandle handle) {
    ASSERT(mServiceImpl);
    mServiceImpl->CloseHandle(handle);
}

ResultOrError<VkSemaphore> Service::ImportSemaphore(ExternalSemaphoreHandle handle) {
    ASSERT(mServiceImpl);
    return mServiceImpl->ImportSemaphore(handle);
}

ResultOrError<VkSemaphore> Service::CreateExportableSemaphore() {
    ASSERT(mServiceImpl);
    return mServiceImpl->CreateExportableSemaphore();
}

ResultOrError<ExternalSemaphoreHandle> Service::ExportSemaphore(VkSemaphore semaphore) {
    ASSERT(mServiceImpl);
    return mServiceImpl->ExportSemaphore(semaphore);
}

ExternalSemaphoreHandle Service::DuplicateHandle(ExternalSemaphoreHandle handle) {
    ASSERT(mServiceImpl);
    return mServiceImpl->DuplicateHandle(handle);
}

}  // namespace dawn::native::vulkan::external_semaphore
