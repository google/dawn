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

#include <utility>

#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

namespace dawn::native::vulkan::external_semaphore {

Service::Service(Device* device)
    : mDevice(device),
      mSupported(CheckSupport(device->GetDeviceInfo(),
                              ToBackend(device->GetAdapter())->GetPhysicalDevice(),
                              device->fn)) {}

Service::~Service() = default;

// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo,
                           VkPhysicalDevice physicalDevice,
                           const VulkanFunctions& fn) {
    if (!deviceInfo.HasExt(DeviceExt::ExternalSemaphoreZirconHandle)) {
        return false;
    }

    VkPhysicalDeviceExternalSemaphoreInfoKHR semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

    VkExternalSemaphorePropertiesKHR semaphoreProperties;
    semaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR;
    semaphoreProperties.pNext = nullptr;

    fn.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, &semaphoreInfo,
                                                    &semaphoreProperties);

    VkFlags requiredFlags = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR |
                            VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR;

    return IsSubset(requiredFlags, semaphoreProperties.externalSemaphoreFeatures);
}

bool Service::Supported() {
    return mSupported;
}

ResultOrError<VkSemaphore> Service::ImportSemaphore(ExternalSemaphoreHandle handle) {
    DAWN_INVALID_IF(handle == ZX_HANDLE_INVALID, "Importing a semaphore with an invalid handle.");

    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &info, nullptr, &*semaphore),
        "vkCreateSemaphore"));

    VkImportSemaphoreZirconHandleInfoFUCHSIA importSemaphoreHandleInfo;
    importSemaphoreHandleInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA;
    importSemaphoreHandleInfo.pNext = nullptr;
    importSemaphoreHandleInfo.semaphore = semaphore;
    importSemaphoreHandleInfo.flags = 0;
    importSemaphoreHandleInfo.handleType =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;
    importSemaphoreHandleInfo.handle = handle;

    MaybeError status = CheckVkSuccess(mDevice->fn.ImportSemaphoreZirconHandleFUCHSIA(
                                           mDevice->GetVkDevice(), &importSemaphoreHandleInfo),
                                       "vkImportSemaphoreZirconHandleFUCHSIA");

    if (status.IsError()) {
        mDevice->fn.DestroySemaphore(mDevice->GetVkDevice(), semaphore, nullptr);
        DAWN_TRY(std::move(status));
    }

    return semaphore;
}

ResultOrError<VkSemaphore> Service::CreateExportableSemaphore() {
    VkExportSemaphoreCreateInfoKHR exportSemaphoreInfo;
    exportSemaphoreInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR;
    exportSemaphoreInfo.pNext = nullptr;
    exportSemaphoreInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = &exportSemaphoreInfo;
    semaphoreCreateInfo.flags = 0;

    VkSemaphore signalSemaphore;
    DAWN_TRY(
        CheckVkSuccess(mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &semaphoreCreateInfo,
                                                   nullptr, &*signalSemaphore),
                       "vkCreateSemaphore"));
    return signalSemaphore;
}

ResultOrError<ExternalSemaphoreHandle> Service::ExportSemaphore(VkSemaphore semaphore) {
    VkSemaphoreGetZirconHandleInfoFUCHSIA semaphoreGetHandleInfo;
    semaphoreGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA;
    semaphoreGetHandleInfo.pNext = nullptr;
    semaphoreGetHandleInfo.semaphore = semaphore;
    semaphoreGetHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

    zx_handle_t handle = ZX_HANDLE_INVALID;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.GetSemaphoreZirconHandleFUCHSIA(
                                mDevice->GetVkDevice(), &semaphoreGetHandleInfo, &handle),
                            "VkSemaphoreGetZirconHandleInfoFUCHSIA"));

    ASSERT(handle != ZX_HANDLE_INVALID);
    return handle;
}

}  // namespace dawn::native::vulkan::external_semaphore
