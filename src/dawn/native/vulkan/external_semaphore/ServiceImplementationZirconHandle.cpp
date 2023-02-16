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

#include <zircon/syscalls.h>
#include <utility>

#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_semaphore/ServiceImplementation.h"
#include "dawn/native/vulkan/external_semaphore/ServiceImplementationZirconHandle.h"

namespace dawn::native::vulkan::external_semaphore {

class ServiceImplementationZirconHandle : public ServiceImplementation {
  public:
    explicit ServiceImplementationZirconHandle(Device* device)
        : ServiceImplementation(device),
          mSupported(CheckSupport(device->GetDeviceInfo(),
                                  ToBackend(device->GetAdapter())->GetPhysicalDevice(),
                                  device->fn)) {}

    ~ServiceImplementationZirconHandle() override = default;

    static bool CheckSupport(
        const VulkanDeviceInfo& deviceInfo,
        VkPhysicalDevice physicalDevice,
        const VulkanFunctions& fn) static void CloseHandle(ExternalSemaphoreHandle handle) {
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

    // True if the device reports it supports this feature
    bool Supported() override { return mSupported; }

    // Given an external handle, import it into a VkSemaphore
    ResultOrError<VkSemaphore> ImportSemaphore(ExternalSemaphoreHandle handle) override {
        DAWN_INVALID_IF(handle == ZX_HANDLE_INVALID,
                        "Importing a semaphore with an invalid handle.");

        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &info, nullptr, &*semaphore),
            "vkCreateSemaphore"));

        VkImportSemaphoreZirconHandleInfoFUCHSIA importSemaphoreHandleInfo;
        importSemaphoreHandleInfo.sType =
            VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA;
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

    // Create a VkSemaphore that is exportable into an external handle later
    ResultOrError<VkSemaphore> CreateExportableSemaphore() override {
        VkExportSemaphoreCreateInfoKHR exportSemaphoreInfo;
        exportSemaphoreInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR;
        exportSemaphoreInfo.pNext = nullptr;
        exportSemaphoreInfo.handleTypes =
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

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

    // Export a VkSemaphore into an external handle
    ResultOrError<ExternalSemaphoreHandle> ExportSemaphore(VkSemaphore semaphore) override {
        VkSemaphoreGetZirconHandleInfoFUCHSIA semaphoreGetHandleInfo;
        semaphoreGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA;
        semaphoreGetHandleInfo.pNext = nullptr;
        semaphoreGetHandleInfo.semaphore = semaphore;
        semaphoreGetHandleInfo.handleType =
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

        zx_handle_t handle = ZX_HANDLE_INVALID;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.GetSemaphoreZirconHandleFUCHSIA(
                                    mDevice->GetVkDevice(), &semaphoreGetHandleInfo, &handle),
                                "VkSemaphoreGetZirconHandleInfoFUCHSIA"));

        ASSERT(handle != ZX_HANDLE_INVALID);
        return handle;
    }

    // Duplicate a new external handle from the given one.
    ExternalSemaphoreHandle DuplicateHandle(ExternalSemaphoreHandle handle) override {
        zx_handle_t out_handle = ZX_HANDLE_INVALID;
        zx_status_t status = zx_handle_duplicate(handle, ZX_RIGHT_SAME_RIGHTS, &out_handle);
        ASSERT(status == ZX_OK);
        return out_handle;
    }

    void ServiceImplementationZirconHandle::CloseHandle(ExternalSemaphoreHandle handle) override {
        zx_status_t status = zx_handle_close(handle);
        ASSERT(status == ZX_OK);
    }

  private:
    bool mSupported = false;
};

std::unique_ptr<ServiceImplementation> CreateZirconHandleService(Device* device) {
    return td::make_unique<ServiceImplementationZirconHandle>(device);
}
bool CheckZirconHandleSupport(const VulkanDeviceInfo& deviceInfo,
                              VkPhysicalDevice physicalDevice,
                              const VulkanFunctions& fn) {
    return ServiceImplementationZirconHandle::CheckSupport(deviceInfo, physicalDevice, fn);
}

}  // namespace dawn::native::vulkan::external_semaphore
