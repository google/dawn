// Copyright 2017 The NXT Authors
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

#include "backend/vulkan/TextureVk.h"

#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    namespace {
        // Converts an NXT texture dimension to a Vulkan image type.
        // Note that in Vulkan dimensionality is only 1D, 2D, 3D. Arrays and cube maps are expressed
        // via the array size and a "cubemap compatible" flag.
        VkImageType VulkanImageType(nxt::TextureDimension dimension) {
            switch (dimension) {
                case nxt::TextureDimension::e2D:
                    return VK_IMAGE_TYPE_2D;
                default:
                    UNREACHABLE();
            }
        }

        // Converts NXT texture format to Vulkan formats.
        VkFormat VulkanImageFormat(nxt::TextureFormat format) {
            switch (format) {
                case nxt::TextureFormat::R8G8B8A8Unorm:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                case nxt::TextureFormat::R8G8B8A8Uint:
                    return VK_FORMAT_R8G8B8A8_UINT;
                case nxt::TextureFormat::B8G8R8A8Unorm:
                    return VK_FORMAT_B8G8R8A8_UNORM;
                case nxt::TextureFormat::D32FloatS8Uint:
                    return VK_FORMAT_D32_SFLOAT_S8_UINT;
                default:
                    UNREACHABLE();
            }
        }

        // Converts the NXT usage flags to Vulkan usage flags. Also needs the format to choose
        // between color and depth attachment usages.
        VkImageUsageFlags VulkanImageUsage(nxt::TextureUsageBit usage, nxt::TextureFormat format) {
            VkImageUsageFlags flags = 0;

            if (usage & nxt::TextureUsageBit::TransferSrc) {
                flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
            if (usage & nxt::TextureUsageBit::TransferDst) {
                flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            if (usage & nxt::TextureUsageBit::Sampled) {
                flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            if (usage & nxt::TextureUsageBit::Storage) {
                flags |= VK_IMAGE_USAGE_STORAGE_BIT;
            }
            if (usage & nxt::TextureUsageBit::OutputAttachment) {
                if (TextureFormatHasDepthOrStencil(format)) {
                    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                } else {
                    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                }
            }

            return flags;
        }

    }  // namespace

    Texture::Texture(TextureBuilder* builder) : TextureBase(builder) {
        Device* device = ToBackend(GetDevice());

        // Create the Vulkan image "container". We don't need to check that the format supports the
        // combination of sample, usage etc. because validation should have been done in the NXT
        // frontend already based on the minimum supported formats in the Vulkan spec
        VkImageCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.imageType = VulkanImageType(GetDimension());
        createInfo.format = VulkanImageFormat(GetFormat());
        createInfo.extent = VkExtent3D{GetWidth(), GetHeight(), GetDepth()};
        createInfo.mipLevels = GetNumMipLevels();
        createInfo.arrayLayers = 1;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = VulkanImageUsage(GetAllowedUsage(), GetFormat());
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &mHandle) !=
            VK_SUCCESS) {
            ASSERT(false);
        }

        // Create the image memory and associate it with the container
        VkMemoryRequirements requirements;
        device->fn.GetImageMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

        if (!device->GetMemoryAllocator()->Allocate(requirements, false, &mMemoryAllocation)) {
            ASSERT(false);
        }

        if (device->fn.BindImageMemory(device->GetVkDevice(), mHandle,
                                       mMemoryAllocation.GetMemory(),
                                       mMemoryAllocation.GetMemoryOffset()) != VK_SUCCESS) {
            ASSERT(false);
        }
    }

    Texture::~Texture() {
        Device* device = ToBackend(GetDevice());

        // We need to free both the memory allocation and the container. Memory should be freed
        // after the VkImage is destroyed and this is taken care of by the FencedDeleter.
        device->GetMemoryAllocator()->Free(&mMemoryAllocation);

        if (mHandle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkImage Texture::GetHandle() const {
        return mHandle;
    }

    void Texture::RecordBarrier(VkCommandBuffer,
                                nxt::TextureUsageBit,
                                nxt::TextureUsageBit) const {
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit,
                                      nxt::TextureUsageBit) {
    }

}}  // namespace backend::vulkan
