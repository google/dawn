// Copyright 2021 The Dawn Authors
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

#include <unistd.h>

#include <memory>
#include <utility>
#include <vector>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/tests/white_box/VulkanImageWrappingTests.h"
#include "gtest/gtest.h"

namespace dawn::native::vulkan {

ExternalImageDescriptorVkForTesting::ExternalImageDescriptorVkForTesting()
    : ExternalImageDescriptorVk(ExternalImageType::OpaqueFD) {}
ExternalImageExportInfoVkForTesting::ExternalImageExportInfoVkForTesting()
    : ExternalImageExportInfoVk(ExternalImageType::OpaqueFD) {}

class ExternalSemaphoreOpaqueFD : public VulkanImageWrappingTestBackend::ExternalSemaphore {
  public:
    explicit ExternalSemaphoreOpaqueFD(int handle) : mHandle(handle) {}
    ~ExternalSemaphoreOpaqueFD() override {
        if (mHandle != -1) {
            close(mHandle);
        }
    }
    int AcquireHandle() {
        int handle = mHandle;
        mHandle = -1;
        return handle;
    }

  private:
    int mHandle = -1;
};

class ExternalTextureOpaqueFD : public VulkanImageWrappingTestBackend::ExternalTexture {
  public:
    ExternalTextureOpaqueFD(dawn::native::vulkan::Device* device,
                            int fd,
                            VkDeviceMemory allocation,
                            VkImage handle,
                            VkDeviceSize allocationSize,
                            uint32_t memoryTypeIndex)
        : mDevice(device),
          mFd(fd),
          mAllocation(allocation),
          mHandle(handle),
          allocationSize(allocationSize),
          memoryTypeIndex(memoryTypeIndex) {}

    ~ExternalTextureOpaqueFD() override {
        if (mFd != -1) {
            close(mFd);
        }
        if (mAllocation != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mAllocation);
        }
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        }
    }

    int Dup() const { return dup(mFd); }

  private:
    dawn::native::vulkan::Device* mDevice;
    int mFd = -1;
    VkDeviceMemory mAllocation = VK_NULL_HANDLE;
    VkImage mHandle = VK_NULL_HANDLE;

  public:
    const VkDeviceSize allocationSize;
    const uint32_t memoryTypeIndex;
};

class VulkanImageWrappingTestBackendOpaqueFD : public VulkanImageWrappingTestBackend {
  public:
    explicit VulkanImageWrappingTestBackendOpaqueFD(const wgpu::Device& device) : mDevice(device) {
        mDeviceVk = dawn::native::vulkan::ToBackend(dawn::native::FromAPI(device.Get()));
    }

    std::unique_ptr<ExternalTexture> CreateTexture(uint32_t width,
                                                   uint32_t height,
                                                   wgpu::TextureFormat format,
                                                   wgpu::TextureUsage usage) override {
        EXPECT_EQ(format, wgpu::TextureFormat::RGBA8Unorm);
        VkFormat vulkanFormat = VK_FORMAT_R8G8B8A8_UNORM;

        VkImage handle;
        ::VkResult result = CreateImage(mDeviceVk, width, height, vulkanFormat, &handle);
        EXPECT_EQ(result, VK_SUCCESS) << "Failed to create external image";

        VkDeviceMemory allocation;
        VkDeviceSize allocationSize;
        uint32_t memoryTypeIndex;
        ::VkResult resultBool =
            AllocateMemory(mDeviceVk, handle, &allocation, &allocationSize, &memoryTypeIndex);
        EXPECT_EQ(resultBool, VK_SUCCESS) << "Failed to allocate external memory";

        result = BindMemory(mDeviceVk, handle, allocation);
        EXPECT_EQ(result, VK_SUCCESS) << "Failed to bind image memory";

        int fd = GetMemoryFd(mDeviceVk, allocation);

        return std::make_unique<ExternalTextureOpaqueFD>(mDeviceVk, fd, allocation, handle,
                                                         allocationSize, memoryTypeIndex);
    }

    wgpu::Texture WrapImage(const wgpu::Device& device,
                            const ExternalTexture* texture,
                            const ExternalImageDescriptorVkForTesting& descriptor,
                            std::vector<std::unique_ptr<ExternalSemaphore>> semaphores) override {
        const ExternalTextureOpaqueFD* textureOpaqueFD =
            static_cast<const ExternalTextureOpaqueFD*>(texture);
        std::vector<int> waitFDs;
        for (auto& semaphore : semaphores) {
            waitFDs.push_back(
                static_cast<ExternalSemaphoreOpaqueFD*>(semaphore.get())->AcquireHandle());
        }

        ExternalImageDescriptorOpaqueFD descriptorOpaqueFD;
        *static_cast<ExternalImageDescriptorVk*>(&descriptorOpaqueFD) = descriptor;
        descriptorOpaqueFD.memoryFD = textureOpaqueFD->Dup();
        descriptorOpaqueFD.allocationSize = textureOpaqueFD->allocationSize;
        descriptorOpaqueFD.memoryTypeIndex = textureOpaqueFD->memoryTypeIndex;
        descriptorOpaqueFD.waitFDs = std::move(waitFDs);

        return dawn::native::vulkan::WrapVulkanImage(device.Get(), &descriptorOpaqueFD);
    }

    bool ExportImage(const wgpu::Texture& texture,
                     VkImageLayout layout,
                     ExternalImageExportInfoVkForTesting* exportInfo) override {
        ExternalImageExportInfoOpaqueFD infoOpaqueFD;
        bool success = ExportVulkanImage(texture.Get(), layout, &infoOpaqueFD);

        *static_cast<ExternalImageExportInfoVk*>(exportInfo) = infoOpaqueFD;
        for (int fd : infoOpaqueFD.semaphoreHandles) {
            EXPECT_NE(fd, -1);
            exportInfo->semaphores.push_back(std::make_unique<ExternalSemaphoreOpaqueFD>(fd));
        }

        return success;
    }

  private:
    // Creates a VkImage with external memory
    ::VkResult CreateImage(dawn::native::vulkan::Device* deviceVk,
                           uint32_t width,
                           uint32_t height,
                           VkFormat format,
                           VkImage* image) {
        VkExternalMemoryImageCreateInfoKHR externalInfo;
        externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR;
        externalInfo.pNext = nullptr;
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkImageCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = &externalInfo;
        createInfo.flags = VK_IMAGE_CREATE_ALIAS_BIT_KHR;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.format = format;
        createInfo.extent = {width, height, 1};
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        return deviceVk->fn.CreateImage(deviceVk->GetVkDevice(), &createInfo, nullptr, &**image);
    }

    // Allocates memory for an image
    ::VkResult AllocateMemory(dawn::native::vulkan::Device* deviceVk,
                              VkImage handle,
                              VkDeviceMemory* allocation,
                              VkDeviceSize* allocationSize,
                              uint32_t* memoryTypeIndex) {
        // Create the image memory and associate it with the container
        VkMemoryRequirements requirements;
        deviceVk->fn.GetImageMemoryRequirements(deviceVk->GetVkDevice(), handle, &requirements);

        // Import memory from file descriptor
        VkExportMemoryAllocateInfoKHR externalInfo;
        externalInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
        externalInfo.pNext = nullptr;
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        int bestType = deviceVk->GetResourceMemoryAllocator()->FindBestTypeIndex(
            requirements, MemoryKind::Opaque);
        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = &externalInfo;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = static_cast<uint32_t>(bestType);

        *allocationSize = allocateInfo.allocationSize;
        *memoryTypeIndex = allocateInfo.memoryTypeIndex;

        return deviceVk->fn.AllocateMemory(deviceVk->GetVkDevice(), &allocateInfo, nullptr,
                                           &**allocation);
    }

    // Binds memory to an image
    ::VkResult BindMemory(dawn::native::vulkan::Device* deviceVk,
                          VkImage handle,
                          VkDeviceMemory memory) {
        return deviceVk->fn.BindImageMemory(deviceVk->GetVkDevice(), handle, memory, 0);
    }

    // Extracts a file descriptor representing memory on a device
    int GetMemoryFd(dawn::native::vulkan::Device* deviceVk, VkDeviceMemory memory) {
        VkMemoryGetFdInfoKHR getFdInfo;
        getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        getFdInfo.pNext = nullptr;
        getFdInfo.memory = memory;
        getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        int memoryFd = -1;
        deviceVk->fn.GetMemoryFdKHR(deviceVk->GetVkDevice(), &getFdInfo, &memoryFd);

        EXPECT_GE(memoryFd, 0) << "Failed to get file descriptor for external memory";
        return memoryFd;
    }

    // Prepares and exports memory for an image on a given device
    void CreateBindExportImage(dawn::native::vulkan::Device* deviceVk,
                               uint32_t width,
                               uint32_t height,
                               VkFormat format,
                               VkImage* handle,
                               VkDeviceMemory* allocation,
                               VkDeviceSize* allocationSize,
                               uint32_t* memoryTypeIndex,
                               int* memoryFd) {}

    wgpu::Device mDevice;
    dawn::native::vulkan::Device* mDeviceVk;
};

// static
std::unique_ptr<VulkanImageWrappingTestBackend> VulkanImageWrappingTestBackend::Create(
    const wgpu::Device& device) {
    return std::make_unique<VulkanImageWrappingTestBackendOpaqueFD>(device);
}

}  // namespace dawn::native::vulkan
