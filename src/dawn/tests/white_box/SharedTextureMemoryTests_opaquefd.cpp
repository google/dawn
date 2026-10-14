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
#include <webgpu/webgpu_cpp.h>

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/native/vulkan/DeviceVk.h"
#include "src/dawn/native/vulkan/FencedDeleter.h"
#include "src/dawn/native/vulkan/PhysicalDeviceVk.h"
#include "src/dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "src/dawn/native/vulkan/UtilsVulkan.h"
#include "src/dawn/tests/white_box/SharedTextureMemoryTests.h"
#include "src/utils/compiler.h"

namespace dawn::native::vulkan {
namespace {

template <typename CreateFn, typename... AdditionalChains>
auto WithSTMOpaqueFDDescriptorFor(native::vulkan::Device* deviceVk,
                                  uint32_t size,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  VkImageCreateFlagBits createFlags,
                                  bool dedicatedAllocation,
                                  CreateFn createFn,
                                  AdditionalChains*... additionalChains) {
    VkExternalMemoryImageCreateInfo externalInfo{};
    externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = &externalInfo;
    createInfo.flags = createFlags;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent = {size, size, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    PNextChainBuilder createInfoChain(&createInfo);
    (createInfoChain.Add(additionalChains), ...);

    VkImage vkImage;
    EXPECT_EQ(deviceVk->fn.CreateImage(deviceVk->GetVkDevice(), &createInfo, nullptr, &*vkImage),
              VK_SUCCESS);

    // Create the image memory and associate it with the container
    VkMemoryRequirements requirements;
    deviceVk->fn.GetImageMemoryRequirements(deviceVk->GetVkDevice(), vkImage, &requirements);

    auto result = deviceVk->GetResourceMemoryAllocator()->FindBestTypeIndex(
        requirements, native::vulkan::MemoryKind::DeviceLocal);
    EXPECT_TRUE(result.has_value());
    uint32_t bestType = result.value();

    VkMemoryDedicatedAllocateInfo dedicatedInfo;
    dedicatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedInfo.pNext = nullptr;
    dedicatedInfo.image = vkImage;
    dedicatedInfo.buffer = VkBuffer{};

    VkExportMemoryAllocateInfoKHR externalAllocateInfo;
    externalAllocateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    externalAllocateInfo.pNext = dedicatedAllocation ? &dedicatedInfo : nullptr;
    externalAllocateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = &externalAllocateInfo;
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = bestType;

    VkDeviceMemory vkDeviceMemory;
    EXPECT_EQ(deviceVk->fn.AllocateMemory(deviceVk->GetVkDevice(), &allocateInfo, nullptr,
                                          &*vkDeviceMemory),
              VK_SUCCESS);

    EXPECT_EQ(deviceVk->fn.BindImageMemory(deviceVk->GetVkDevice(), vkImage, vkDeviceMemory, 0),
              VK_SUCCESS);

    VkMemoryGetFdInfoKHR getFdInfo;
    getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFdInfo.pNext = nullptr;
    getFdInfo.memory = vkDeviceMemory;
    getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    int memoryFD = -1;
    deviceVk->fn.GetMemoryFdKHR(deviceVk->GetVkDevice(), &getFdInfo, &memoryFD);
    EXPECT_GE(memoryFD, 0) << "Failed to get file descriptor for external memory";

    wgpu::SharedTextureMemoryOpaqueFDDescriptor opaqueFDDesc;
    opaqueFDDesc.vkImageCreateInfo = &createInfo;
    opaqueFDDesc.memoryFD = memoryFD;
    opaqueFDDesc.memoryTypeIndex = allocateInfo.memoryTypeIndex;
    opaqueFDDesc.allocationSize = allocateInfo.allocationSize;
    opaqueFDDesc.dedicatedAllocation = dedicatedAllocation;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &opaqueFDDesc;

    std::string label;
    label += "size: " + std::to_string(size);
    label += " format: " + std::to_string(format);
    label += " usage: " + std::to_string(usage);
    label += " createFlags: " + std::to_string(createFlags);
    label += " dedicatedAllocation: " + std::to_string(dedicatedAllocation);

    auto ret = createFn(&desc);

    close(memoryFD);
    deviceVk->GetFencedDeleter()->DeleteWhenUnused(vkDeviceMemory);
    deviceVk->GetFencedDeleter()->DeleteWhenUnused(vkImage);

    return ret;
}

template <typename CreateFn, typename... AdditionalChains>
auto WithSTMOpaqueFDDescriptorFor(native::vulkan::Device* deviceVk,
                                  uint32_t size,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  CreateFn createFn,
                                  AdditionalChains*... additionalChains) {
    return WithSTMOpaqueFDDescriptorFor(deviceVk, size, format, usage, VkImageCreateFlagBits(0),
                                        false, createFn, additionalChains...);
}

bool CheckFormatSupport(native::vulkan::Device* deviceVk,
                        VkFormat format,
                        VkImageUsageFlags usage,
                        VkImageCreateFlagBits createFlags = VkImageCreateFlagBits(0)) {
    VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo;
    externalFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR;
    externalFormatInfo.pNext = nullptr;
    externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkPhysicalDeviceImageFormatInfo2 formatInfo;
    formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
    formatInfo.pNext = &externalFormatInfo;
    formatInfo.format = format;
    formatInfo.type = VK_IMAGE_TYPE_2D;
    formatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    formatInfo.usage = usage;
    formatInfo.flags = createFlags;

    VkExternalImageFormatProperties externalFormatProperties;
    externalFormatProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR;
    externalFormatProperties.pNext = nullptr;

    VkImageFormatProperties2 formatProperties;
    formatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
    formatProperties.pNext = &externalFormatProperties;

    return VkResult::WrapUnsafe(deviceVk->fn.GetPhysicalDeviceImageFormatProperties2(
               ToBackend(deviceVk->GetPhysicalDevice())->GetVkPhysicalDevice(), &formatInfo,
               &formatProperties)) == VK_SUCCESS;
}

template <wgpu::FeatureName FenceFeature, bool DedicatedAllocation>
class Backend : public SharedTextureMemoryTestVulkanBackend {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::string Name() const override {
        std::string name = "OpaqueFD";
        if (DedicatedAllocation) {
            name += ", DedicatedAlloc";
        }
        switch (FenceFeature) {
            case wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD:
                name += ", OpaqueFDFence";
                break;
            case wgpu::FeatureName::SharedFenceSyncFD:
                name += ", SyncFDFence";
                break;
            default:
                DAWN_UNREACHABLE();
        }
        return name;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryOpaqueFD,
                wgpu::FeatureName::DawnMultiPlanarFormats, FenceFeature};
    }

    bool SupportsConcurrentRead() const override {
        return FenceFeature != wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD;
    }

    template <typename CreateFn>
    auto WithSTMOpaqueFDDescriptor(native::vulkan::Device* deviceVk,
                                   uint32_t size,
                                   VkFormat format,
                                   VkImageUsageFlags usage,
                                   CreateFn createFn) {
        VkImageCreateFlagBits flags{};
        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM) {
            // Needed for view format reinterpretation.
            flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }
        return WithSTMOpaqueFDDescriptorFor(deviceVk, size, format, usage, flags,
                                            DedicatedAllocation, createFn);
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        return WithSTMOpaqueFDDescriptor(
            native::vulkan::ToBackend(native::FromAPI(device.Get())), 16, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_STORAGE_BIT,
            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                return device.ImportSharedTextureMemory(desc);
            });
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        DAWN_ASSERT(!devices.empty());

        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;
        for (VkFormat format : {
                 VK_FORMAT_R8_UNORM,
                 VK_FORMAT_R8G8_UNORM,
                 VK_FORMAT_R8G8B8A8_UNORM,
                 VK_FORMAT_B8G8R8A8_UNORM,
                 VK_FORMAT_A2B10G10R10_UNORM_PACK32,
             }) {
            for (VkImageUsageFlags usage : {
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                         VK_IMAGE_USAGE_STORAGE_BIT,
                 }) {
                auto* deviceVk = native::vulkan::ToBackend(native::FromAPI(devices[0].Get()));
                if (!CheckFormatSupport(deviceVk, format, usage)) {
                    // Skip this format if it is not supported.
                    continue;
                }

                for (uint32_t size : {4, 64}) {
                    WithSTMOpaqueFDDescriptor(
                        deviceVk, size, format, usage,
                        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                            std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                            for (auto& device : devices) {
                                perDeviceMemories.push_back(device.ImportSharedTextureMemory(desc));
                            }
                            memories.push_back(std::move(perDeviceMemories));
                            return true;
                        });
                }
            }
        }
        return memories;
    }
};

class SharedTextureMemoryOpaqueFDValidationTest : public SharedTextureMemoryTests {};
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryOpaqueFDValidationTest);

// Test that the Vulkan image must be created with VK_IMAGE_USAGE_TRANSFER_DST_BIT.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, RequiresCopyDst) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    // Test that including TRANSFER_DST is not an error.
    WithSTMOpaqueFDDescriptorFor(deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                     device.ImportSharedTextureMemory(desc);
                                     return true;
                                 });

    // Test that excluding TRANSFER_DST is an error.
    WithSTMOpaqueFDDescriptorFor(deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                     ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                                             testing::HasSubstr("TRANSFER_DST"));
                                     return true;
                                 });
}

// Check that the OpaqueFD import path doesn't modify chained descriptor (such as the very common
// VkImageFormatListCreateInfo)
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, PreservesImageCreateChain) {
    auto* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    DAWN_TEST_UNSUPPORTED_IF(!deviceVk->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList));

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageFormatListCreateInfo sourceFormats = {};
    sourceFormats.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
    sourceFormats.viewFormatCount = 1;
    sourceFormats.pViewFormats = &format;

    WithSTMOpaqueFDDescriptorFor(
        deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        [&](const wgpu::SharedTextureMemoryDescriptor* originalDesc) {
            auto opaqueDesc = *static_cast<const wgpu::SharedTextureMemoryOpaqueFDDescriptor*>(
                originalDesc->nextInChain);
            auto createInfo = *static_cast<const VkImageCreateInfo*>(opaqueDesc.vkImageCreateInfo);
            auto externalInfo =
                *static_cast<const VkExternalMemoryImageCreateInfo*>(createInfo.pNext);
            externalInfo.pNext = nullptr;
            VkImageFormatListCreateInfo formats = {};
            formats.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
            formats.pNext = &externalInfo;
            formats.viewFormatCount = 1;
            formats.pViewFormats = &createInfo.format;
            createInfo.pNext = &formats;
            opaqueDesc.vkImageCreateInfo = &createInfo;
            wgpu::SharedTextureMemoryDescriptor desc;
            desc.nextInChain = &opaqueDesc;

            // Put the format list first, with a non-null pNext. Appending it directly to
            // a query would truncate the caller's chain and break the second import.
            for (int i = 0; i < 2; ++i) {
                device.ImportSharedTextureMemory(&desc);
                EXPECT_EQ(createInfo.pNext, &formats);
                EXPECT_EQ(formats.pNext, &externalInfo);
                EXPECT_EQ(formats.pViewFormats, &createInfo.format);
                EXPECT_EQ(formats.viewFormatCount, 1u);
                EXPECT_EQ(externalInfo.pNext, nullptr);
                EXPECT_EQ(externalInfo.handleTypes, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
            }

            return true;
        },
        &sourceFormats);
}

#if defined(DAWN_ENABLE_ERROR_INJECTION)
// Exercise every error injection point in the import, with and without a dedicated allocation.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, ImportErrors) {
    auto* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    for (bool dedicated : {false, true}) {
        SCOPED_TRACE(dedicated);
        WithSTMOpaqueFDDescriptorFor(
            deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VkImageCreateFlagBits(0), dedicated,
            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                // Measure the injection points in a successful import.
                ClearErrorInjector();
                EnableErrorInjector();
                device.ImportSharedTextureMemory(desc);
                DisableErrorInjector();
                const uint64_t failureCount = AcquireErrorInjectorCallCount();
                EXPECT_GT(failureCount, 0u);
                WaitForAllOperations();

                for (uint64_t failureIndex = 0; failureIndex < failureCount; ++failureIndex) {
                    SCOPED_TRACE(failureIndex);
                    // ImportSharedTextureMemory turns unexpected backend errors into device loss.
                    // Use a fresh device so every iteration reaches the intended failure point.
                    wgpu::Device importDevice = CreateDevice();
                    EXPECT_DEVICE_LOSS_ON(importDevice, {
                        ClearErrorInjector();
                        EnableErrorInjector();
                        InjectErrorAt(failureIndex);
                        importDevice.ImportSharedTextureMemory(desc);
                        DisableErrorInjector();
                    });
                    EXPECT_EQ(AcquireErrorInjectorCallCount(), failureIndex + 1);
                }
                return true;
            });
    }
}
#endif  // defined(DAWN_ENABLE_ERROR_INJECTION)

// Check the limits for the exact external-image format query, before an invalid image
// reaches vkCreateImage. Reuse a valid exported FD so each failure exercises image validation.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, ImageFormatLimits) {
    auto* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    WithSTMOpaqueFDDescriptorFor(
        deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        [&](const wgpu::SharedTextureMemoryDescriptor* originalDesc) {
            auto opaqueDesc = *static_cast<const wgpu::SharedTextureMemoryOpaqueFDDescriptor*>(
                originalDesc->nextInChain);
            const auto originalCreateInfo =
                *static_cast<const VkImageCreateInfo*>(opaqueDesc.vkImageCreateInfo);
            VkImageCreateInfo createInfo = originalCreateInfo;
            opaqueDesc.vkImageCreateInfo = &createInfo;
            wgpu::SharedTextureMemoryDescriptor desc;
            desc.nextInChain = &opaqueDesc;

            // Establish the successful baseline before exercising format-limit failures.
            device.ImportSharedTextureMemory(&desc);

            VkPhysicalDeviceExternalImageFormatInfo externalInfo = {};
            externalInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
            externalInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
            VkPhysicalDeviceImageFormatInfo2 formatInfo = {};
            formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
            formatInfo.pNext = &externalInfo;
            formatInfo.format = createInfo.format;
            formatInfo.type = createInfo.imageType;
            formatInfo.tiling = createInfo.tiling;
            formatInfo.usage = createInfo.usage;
            formatInfo.flags = createInfo.flags;
            VkImageFormatProperties2 properties = {};
            properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
            const auto result = deviceVk->fn.GetPhysicalDeviceImageFormatProperties2(
                ToBackend(deviceVk->GetPhysicalDevice())->GetVkPhysicalDevice(), &formatInfo,
                &properties);
            // The descriptor is known to be supported; this query retrieves its limits.
            EXPECT_EQ(result, VK_SUCCESS);
            const auto& limits = properties.imageFormatProperties;

            struct LimitCase {
                raw_ptr<uint32_t> value;
                uint32_t maximum;
                const char* error;
            };
            const LimitCase cases[] = {
                {&createInfo.extent.width, limits.maxExtent.width, "maxExtent"},
                {&createInfo.extent.height, limits.maxExtent.height, "maxExtent"},
                {&createInfo.extent.depth, limits.maxExtent.depth, "maxExtent"},
                {&createInfo.mipLevels, limits.maxMipLevels, "maxMipLevels"},
                {&createInfo.arrayLayers, limits.maxArrayLayers, "maxArrayLayers"},
            };
            for (const auto& test : cases) {
                SCOPED_TRACE(test.error);
                *test.value = 0;
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(&desc),
                                        testing::HasSubstr(test.error));
                createInfo = originalCreateInfo;

                // There is no representable value above UINT32_MAX.
                if (test.maximum == std::numeric_limits<uint32_t>::max()) {
                    continue;
                }
                *test.value = test.maximum + 1;
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(&desc),
                                        testing::HasSubstr(test.error));
                createInfo = originalCreateInfo;
            }
            for (VkSampleCountFlagBits samples :
                 {VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_4_BIT,
                  VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_32_BIT,
                  VK_SAMPLE_COUNT_64_BIT}) {
                if ((limits.sampleCounts & samples) != 0) {
                    continue;
                }
                SCOPED_TRACE(samples);
                createInfo.samples = samples;
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(&desc),
                                        testing::HasSubstr("sampleCounts"));
            }
            for (uint32_t samples :
                 {0u, static_cast<uint32_t>(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT)}) {
                SCOPED_TRACE(samples);
                createInfo.samples = static_cast<VkSampleCountFlagBits>(samples);
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(&desc),
                                        testing::HasSubstr("sampleCounts"));
            }
            // Validation failures must leave the descriptor usable for another import.
            createInfo = originalCreateInfo;
            device.ImportSharedTextureMemory(&desc);
            return true;
        });
}

// Test requirements for the Vulkan image if it is BGRA8Unorm.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, BGRA8UnormStorageRequirements) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    DAWN_TEST_UNSUPPORTED_IF(!CheckFormatSupport(deviceVk, VK_FORMAT_B8G8R8A8_UNORM,
                                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                     VK_IMAGE_USAGE_STORAGE_BIT,
                                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT));

    // Test that including MUTABLE_FORMAT_BIT is valid.
    WithSTMOpaqueFDDescriptorFor(deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                     VK_IMAGE_USAGE_STORAGE_BIT,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                                 [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                     device.ImportSharedTextureMemory(desc);
                                     return true;
                                 });

    // Test that excluding MUTABLE_FORMAT_BIT is invalid.
    WithSTMOpaqueFDDescriptorFor(
        deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT,
        VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                    testing::HasSubstr("MUTABLE_FORMAT_BIT"));
            return true;
        });

    // Test that excluding MUTABLE_FORMAT_BIT if STORAGE_BIT is not present is valid.
    WithSTMOpaqueFDDescriptorFor(deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 VkImageCreateFlagBits(0), false,
                                 [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                     device.ImportSharedTextureMemory(desc);
                                     return true;
                                 });

    // Test that including MUTABLE_FORMAT_BIT if STORAGE_BIT is not present is valid.
    WithSTMOpaqueFDDescriptorFor(deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                                 [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                     device.ImportSharedTextureMemory(desc);
                                     return true;
                                 });
}

// Test requirements for the Vulkan image if it is may need view format reinterpretation.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, ViewFormatRequirements) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    std::array<VkFormat, 2> vkFormats = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB};
    for (VkFormat vkFormat : vkFormats) {
        // Test that including MUTABLE_FORMAT_BIT is valid.
        WithSTMOpaqueFDDescriptorFor(deviceVk, 4, vkFormat,
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                                     [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                         device.ImportSharedTextureMemory(desc);
                                         return true;
                                     });

        // Test that excluding MUTABLE_FORMAT_BIT is invalid.
        WithSTMOpaqueFDDescriptorFor(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                        testing::HasSubstr("MUTABLE_FORMAT_BIT"));
                return true;
            });

        // Test that including MUTABLE_FORMAT_BIT if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is not
        // present is valid.
        WithSTMOpaqueFDDescriptorFor(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                device.ImportSharedTextureMemory(desc);
                return true;
            });

        // Test that excluding MUTABLE_FORMAT_BIT if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is not
        // present is valid.
        WithSTMOpaqueFDDescriptorFor(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                device.ImportSharedTextureMemory(desc);
                return true;
            });

        // Test that if the image format list is provided, all of the vkFormats must be listed.
        VkImageFormatListCreateInfo imageFormatListInfo;
        imageFormatListInfo.pNext = nullptr;
        imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
        {
            // Passing all of them is valid.
            imageFormatListInfo.pViewFormats = vkFormats.data();
            imageFormatListInfo.viewFormatCount = vkFormats.size();

            WithSTMOpaqueFDDescriptorFor(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    device.ImportSharedTextureMemory(desc);
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing the first is invalid.
            imageFormatListInfo.pViewFormats = vkFormats.data();
            imageFormatListInfo.viewFormatCount = 1;

            WithSTMOpaqueFDDescriptorFor(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing the second is invalid.
            imageFormatListInfo.pViewFormats = DAWN_UNSAFE_TODO(vkFormats.data() + 1);
            imageFormatListInfo.viewFormatCount = 1;

            WithSTMOpaqueFDDescriptorFor(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing none is invalid.
            imageFormatListInfo.pViewFormats = nullptr;
            imageFormatListInfo.viewFormatCount = 0;

            WithSTMOpaqueFDDescriptorFor(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
    }
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryNoFeatureTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance()},
    {1});

// These tests construct descriptors directly rather than using the backend's
// DedicatedAllocation parameter. ImportErrors covers both allocation modes.
DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryOpaqueFDValidationTest,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, true>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, true>::GetInstance()},
    {1});

}  // anonymous namespace
}  // namespace dawn::native::vulkan
