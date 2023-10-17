// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <unistd.h>
#include <utility>

#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementation.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"

static constexpr VkExternalSemaphoreHandleTypeFlagBits kHandleType =
#if DAWN_PLATFORM_IS(ANDROID) || DAWN_PLATFORM_IS(CHROMEOS)
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
#else
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

namespace dawn::native::vulkan::external_semaphore {

class ServiceImplementationFD : public ServiceImplementation {
  public:
    explicit ServiceImplementationFD(Device* device)
        : ServiceImplementation(device),
          mSupported(CheckSupport(device->GetDeviceInfo(),
                                  ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice(),
                                  device->fn)) {}

    ~ServiceImplementationFD() override = default;

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo,
                             VkPhysicalDevice physicalDevice,
                             const VulkanFunctions& fn) {
        if (!deviceInfo.HasExt(DeviceExt::ExternalSemaphoreFD)) {
            return false;
        }

        VkPhysicalDeviceExternalSemaphoreInfoKHR semaphoreInfo;
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.handleType = kHandleType;

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
        DAWN_INVALID_IF(handle < 0, "Importing a semaphore with an invalid handle.");

        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &info, nullptr, &*semaphore),
            "vkCreateSemaphore"));

        VkImportSemaphoreFdInfoKHR importSemaphoreFdInfo;
        importSemaphoreFdInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
        importSemaphoreFdInfo.pNext = nullptr;
        importSemaphoreFdInfo.semaphore = semaphore;
        importSemaphoreFdInfo.flags = 0;
        importSemaphoreFdInfo.handleType = kHandleType;
        importSemaphoreFdInfo.fd = handle;

        MaybeError status = CheckVkSuccess(
            mDevice->fn.ImportSemaphoreFdKHR(mDevice->GetVkDevice(), &importSemaphoreFdInfo),
            "vkImportSemaphoreFdKHR");

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
        exportSemaphoreInfo.handleTypes = kHandleType;

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
        VkSemaphoreGetFdInfoKHR semaphoreGetFdInfo;
        semaphoreGetFdInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
        semaphoreGetFdInfo.pNext = nullptr;
        semaphoreGetFdInfo.semaphore = semaphore;
        semaphoreGetFdInfo.handleType = kHandleType;

        int fd = -1;
        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.GetSemaphoreFdKHR(mDevice->GetVkDevice(), &semaphoreGetFdInfo, &fd),
            "vkGetSemaphoreFdKHR"));

        DAWN_ASSERT(fd >= 0);
        return fd;
    }

    // Duplicate a new external handle from the given one.
    ExternalSemaphoreHandle DuplicateHandle(ExternalSemaphoreHandle handle) override {
        int fd = dup(handle);
        DAWN_ASSERT(fd >= 0);
        return fd;
    }

    void CloseHandle(ExternalSemaphoreHandle handle) override {
        int ret = close(handle);
        DAWN_ASSERT(ret == 0);
    }

  private:
    bool mSupported = false;
};

std::unique_ptr<ServiceImplementation> CreateFDService(Device* device) {
    return std::make_unique<ServiceImplementationFD>(device);
}

bool CheckFDSupport(const VulkanDeviceInfo& deviceInfo,
                    VkPhysicalDevice physicalDevice,
                    const VulkanFunctions& fn) {
    return ServiceImplementationFD::CheckSupport(deviceInfo, physicalDevice, fn);
}

}  // namespace dawn::native::vulkan::external_semaphore
