// Copyright 2018 The Dawn Authors
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

#include "dawn_native/vulkan/TextureVk.h"

#include "common/Assert.h"
#include "common/Math.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/EnumMaskIterator.h"
#include "dawn_native/Error.h"
#include "dawn_native/VulkanBackend.h"
#include "dawn_native/vulkan/AdapterVk.h"
#include "dawn_native/vulkan/BufferVk.h"
#include "dawn_native/vulkan/CommandRecordingContext.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/ResourceHeapVk.h"
#include "dawn_native/vulkan/StagingBufferVk.h"
#include "dawn_native/vulkan/UtilsVulkan.h"
#include "dawn_native/vulkan/VulkanError.h"

namespace dawn_native { namespace vulkan {

    namespace {
        // Converts an Dawn texture dimension to a Vulkan image view type.
        // Contrary to image types, image view types include arrayness and cubemapness
        VkImageViewType VulkanImageViewType(wgpu::TextureViewDimension dimension) {
            switch (dimension) {
                case wgpu::TextureViewDimension::e2D:
                    return VK_IMAGE_VIEW_TYPE_2D;
                case wgpu::TextureViewDimension::e2DArray:
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                case wgpu::TextureViewDimension::Cube:
                    return VK_IMAGE_VIEW_TYPE_CUBE;
                case wgpu::TextureViewDimension::CubeArray:
                    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e3D:
                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
            }
        }

        // Computes which vulkan access type could be required for the given Dawn usage.
        // TODO(cwallez@chromium.org): We shouldn't need any access usages for srcAccessMask when
        // the previous usage is readonly because an execution dependency is sufficient.
        VkAccessFlags VulkanAccessFlags(wgpu::TextureUsage usage, const Format& format) {
            VkAccessFlags flags = 0;

            if (usage & wgpu::TextureUsage::CopySrc) {
                flags |= VK_ACCESS_TRANSFER_READ_BIT;
            }
            if (usage & wgpu::TextureUsage::CopyDst) {
                flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            if (usage & wgpu::TextureUsage::Sampled) {
                flags |= VK_ACCESS_SHADER_READ_BIT;
            }
            if (usage & wgpu::TextureUsage::Storage) {
                flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            }
            if (usage & kReadOnlyStorageTexture) {
                flags |= VK_ACCESS_SHADER_READ_BIT;
            }
            if (usage & wgpu::TextureUsage::RenderAttachment) {
                if (format.HasDepthOrStencil()) {
                    flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                } else {
                    flags |=
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                }
            }
            if (usage & kPresentTextureUsage) {
                // The present usage is only used internally by the swapchain and is never used in
                // combination with other usages.
                ASSERT(usage == kPresentTextureUsage);
                // The Vulkan spec has the following note:
                //
                //   When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
                //   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, there is no need to delay subsequent
                //   processing, or perform any visibility operations (as vkQueuePresentKHR performs
                //   automatic visibility operations). To achieve this, the dstAccessMask member of
                //   the VkImageMemoryBarrier should be set to 0, and the dstStageMask parameter
                //   should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
                //
                // So on the transition to Present we don't need an access flag. The other
                // direction doesn't matter because swapchain textures always start a new frame
                // as uninitialized.
                flags |= 0;
            }

            return flags;
        }

        // Computes which Vulkan pipeline stage can access a texture in the given Dawn usage
        VkPipelineStageFlags VulkanPipelineStage(wgpu::TextureUsage usage, const Format& format) {
            VkPipelineStageFlags flags = 0;

            if (usage == wgpu::TextureUsage::None) {
                // This only happens when a texture is initially created (and for srcAccessMask) in
                // which case there is no need to wait on anything to stop accessing this texture.
                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }
            if (usage & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) {
                flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            if (usage & (wgpu::TextureUsage::Sampled | kReadOnlyStorageTexture)) {
                // TODO(cwallez@chromium.org): Only transition to the usage we care about to avoid
                // introducing FS -> VS dependencies that would prevent parallelization on tiler
                // GPUs
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            if (usage & wgpu::TextureUsage::Storage) {
                flags |=
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            if (usage & wgpu::TextureUsage::RenderAttachment) {
                if (format.HasDepthOrStencil()) {
                    flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    // TODO(cwallez@chromium.org): This is missing the stage where the depth and
                    // stencil values are written, but it isn't clear which one it is.
                } else {
                    flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                }
            }
            if (usage & kPresentTextureUsage) {
                // The present usage is only used internally by the swapchain and is never used in
                // combination with other usages.
                ASSERT(usage == kPresentTextureUsage);
                // The Vulkan spec has the following note:
                //
                //   When transitioning the image to VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR or
                //   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, there is no need to delay subsequent
                //   processing, or perform any visibility operations (as vkQueuePresentKHR performs
                //   automatic visibility operations). To achieve this, the dstAccessMask member of
                //   the VkImageMemoryBarrier should be set to 0, and the dstStageMask parameter
                //   should be set to VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT.
                //
                // So on the transition to Present we use the "bottom of pipe" stage. The other
                // direction doesn't matter because swapchain textures always start a new frame
                // as uninitialized.
                flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }

            // A zero value isn't a valid pipeline stage mask
            ASSERT(flags != 0);
            return flags;
        }

        VkImageMemoryBarrier BuildMemoryBarrier(const Texture* texture,
                                                wgpu::TextureUsage lastUsage,
                                                wgpu::TextureUsage usage,
                                                const SubresourceRange& range) {
            VkImageMemoryBarrier barrier;
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.srcAccessMask = VulkanAccessFlags(lastUsage, texture->GetFormat());
            barrier.dstAccessMask = VulkanAccessFlags(usage, texture->GetFormat());
            barrier.oldLayout = VulkanImageLayout(texture, lastUsage);
            barrier.newLayout = VulkanImageLayout(texture, usage);
            barrier.image = texture->GetHandle();
            barrier.subresourceRange.aspectMask = VulkanAspectMask(range.aspects);
            barrier.subresourceRange.baseMipLevel = range.baseMipLevel;
            barrier.subresourceRange.levelCount = range.levelCount;
            barrier.subresourceRange.baseArrayLayer = range.baseArrayLayer;
            barrier.subresourceRange.layerCount = range.layerCount;

            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return barrier;
        }

        void FillVulkanCreateInfoSizesAndType(const Texture& texture, VkImageCreateInfo* info) {
            const Extent3D& size = texture.GetSize();

            info->mipLevels = texture.GetNumMipLevels();
            info->samples = VulkanSampleCount(texture.GetSampleCount());

            // Fill in the image type, and paper over differences in how the array layer count is
            // specified between WebGPU and Vulkan.
            switch (texture.GetDimension()) {
                case wgpu::TextureDimension::e2D:
                    info->imageType = VK_IMAGE_TYPE_2D;
                    info->extent = {size.width, size.height, 1};
                    info->arrayLayers = size.depth;
                    break;

                case wgpu::TextureDimension::e1D:
                case wgpu::TextureDimension::e3D:
                    UNREACHABLE();
            }
        }

    }  // namespace

    // Converts Dawn texture format to Vulkan formats.
    VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8Unorm:
                return VK_FORMAT_R8_UNORM;
            case wgpu::TextureFormat::R8Snorm:
                return VK_FORMAT_R8_SNORM;
            case wgpu::TextureFormat::R8Uint:
                return VK_FORMAT_R8_UINT;
            case wgpu::TextureFormat::R8Sint:
                return VK_FORMAT_R8_SINT;

            case wgpu::TextureFormat::R16Uint:
                return VK_FORMAT_R16_UINT;
            case wgpu::TextureFormat::R16Sint:
                return VK_FORMAT_R16_SINT;
            case wgpu::TextureFormat::R16Float:
                return VK_FORMAT_R16_SFLOAT;
            case wgpu::TextureFormat::RG8Unorm:
                return VK_FORMAT_R8G8_UNORM;
            case wgpu::TextureFormat::RG8Snorm:
                return VK_FORMAT_R8G8_SNORM;
            case wgpu::TextureFormat::RG8Uint:
                return VK_FORMAT_R8G8_UINT;
            case wgpu::TextureFormat::RG8Sint:
                return VK_FORMAT_R8G8_SINT;

            case wgpu::TextureFormat::R32Uint:
                return VK_FORMAT_R32_UINT;
            case wgpu::TextureFormat::R32Sint:
                return VK_FORMAT_R32_SINT;
            case wgpu::TextureFormat::R32Float:
                return VK_FORMAT_R32_SFLOAT;
            case wgpu::TextureFormat::RG16Uint:
                return VK_FORMAT_R16G16_UINT;
            case wgpu::TextureFormat::RG16Sint:
                return VK_FORMAT_R16G16_SINT;
            case wgpu::TextureFormat::RG16Float:
                return VK_FORMAT_R16G16_SFLOAT;
            case wgpu::TextureFormat::RGBA8Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case wgpu::TextureFormat::RGBA8Snorm:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case wgpu::TextureFormat::RGBA8Uint:
                return VK_FORMAT_R8G8B8A8_UINT;
            case wgpu::TextureFormat::RGBA8Sint:
                return VK_FORMAT_R8G8B8A8_SINT;
            case wgpu::TextureFormat::BGRA8Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case wgpu::TextureFormat::RGB10A2Unorm:
                return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case wgpu::TextureFormat::RG11B10Ufloat:
                return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case wgpu::TextureFormat::RGB9E5Ufloat:
                return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

            case wgpu::TextureFormat::RG32Uint:
                return VK_FORMAT_R32G32_UINT;
            case wgpu::TextureFormat::RG32Sint:
                return VK_FORMAT_R32G32_SINT;
            case wgpu::TextureFormat::RG32Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case wgpu::TextureFormat::RGBA16Uint:
                return VK_FORMAT_R16G16B16A16_UINT;
            case wgpu::TextureFormat::RGBA16Sint:
                return VK_FORMAT_R16G16B16A16_SINT;
            case wgpu::TextureFormat::RGBA16Float:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case wgpu::TextureFormat::RGBA32Uint:
                return VK_FORMAT_R32G32B32A32_UINT;
            case wgpu::TextureFormat::RGBA32Sint:
                return VK_FORMAT_R32G32B32A32_SINT;
            case wgpu::TextureFormat::RGBA32Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case wgpu::TextureFormat::Depth32Float:
                return VK_FORMAT_D32_SFLOAT;
            case wgpu::TextureFormat::Depth24Plus:
                return VK_FORMAT_D32_SFLOAT;
            case wgpu::TextureFormat::Depth24PlusStencil8:
                // Depth24PlusStencil8 maps to either of these two formats because only requires
                // that one of the two be present. The VulkanUseD32S8 toggle combines the wish of
                // the environment, default to using D32S8, and availability information so we know
                // that the format is available.
                if (device->IsToggleEnabled(Toggle::VulkanUseD32S8)) {
                    return VK_FORMAT_D32_SFLOAT_S8_UINT;
                } else {
                    return VK_FORMAT_D24_UNORM_S8_UINT;
                }

            case wgpu::TextureFormat::BC1RGBAUnorm:
                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            case wgpu::TextureFormat::BC2RGBAUnorm:
                return VK_FORMAT_BC2_UNORM_BLOCK;
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                return VK_FORMAT_BC2_SRGB_BLOCK;
            case wgpu::TextureFormat::BC3RGBAUnorm:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                return VK_FORMAT_BC3_SRGB_BLOCK;
            case wgpu::TextureFormat::BC4RSnorm:
                return VK_FORMAT_BC4_SNORM_BLOCK;
            case wgpu::TextureFormat::BC4RUnorm:
                return VK_FORMAT_BC4_UNORM_BLOCK;
            case wgpu::TextureFormat::BC5RGSnorm:
                return VK_FORMAT_BC5_SNORM_BLOCK;
            case wgpu::TextureFormat::BC5RGUnorm:
                return VK_FORMAT_BC5_UNORM_BLOCK;
            case wgpu::TextureFormat::BC6HRGBFloat:
                return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return VK_FORMAT_BC7_UNORM_BLOCK;
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return VK_FORMAT_BC7_SRGB_BLOCK;

            case wgpu::TextureFormat::Undefined:
                UNREACHABLE();
        }
    }

    // Converts the Dawn usage flags to Vulkan usage flags. Also needs the format to choose
    // between color and depth attachment usages.
    VkImageUsageFlags VulkanImageUsage(wgpu::TextureUsage usage, const Format& format) {
        VkImageUsageFlags flags = 0;

        if (usage & wgpu::TextureUsage::CopySrc) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if (usage & wgpu::TextureUsage::CopyDst) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        if (usage & wgpu::TextureUsage::Sampled) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (usage & (wgpu::TextureUsage::Storage | kReadOnlyStorageTexture)) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if (usage & wgpu::TextureUsage::RenderAttachment) {
            if (format.HasDepthOrStencil()) {
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            } else {
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        return flags;
    }

    // Chooses which Vulkan image layout should be used for the given Dawn usage. Note that this
    // layout must match the layout given to various Vulkan operations as well as the layout given
    // to descriptor set writes.
    VkImageLayout VulkanImageLayout(const Texture* texture, wgpu::TextureUsage usage) {
        if (usage == wgpu::TextureUsage::None) {
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }

        if (!wgpu::HasZeroOrOneBits(usage)) {
            // Sampled | ReadOnlyStorage is the only possible multi-bit usage, if more appear  we
            // might need additional special-casing.
            ASSERT(usage == (wgpu::TextureUsage::Sampled | kReadOnlyStorageTexture));
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        // Usage has a single bit so we can switch on its value directly.
        switch (usage) {
            case wgpu::TextureUsage::CopyDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                // A texture that's sampled and storage may be used as both usages in the same pass.
                // When that happens, the layout must be GENERAL because that's a requirement for
                // the storage usage. We can't know at bindgroup creation time if that case will
                // happen so we must prepare for the pessimistic case and always use the GENERAL
                // layout.
            case wgpu::TextureUsage::Sampled:
                if (texture->GetUsage() & wgpu::TextureUsage::Storage) {
                    return VK_IMAGE_LAYOUT_GENERAL;
                } else {
                    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }

                // Vulkan texture copy functions require the image to be in _one_  known layout.
                // Depending on whether parts of the texture have been transitioned to only CopySrc
                // or a combination with something else, the texture could be in a combination of
                // GENERAL and TRANSFER_SRC_OPTIMAL. This would be a problem, so we make CopySrc use
                // GENERAL.
                // TODO(cwallez@chromium.org): We no longer need to transition resources all at
                // once and can instead track subresources so we should lift this limitation.
            case wgpu::TextureUsage::CopySrc:
                // Read-only and write-only storage textures must use general layout because load
                // and store operations on storage images can only be done on the images in
                // VK_IMAGE_LAYOUT_GENERAL layout.
            case wgpu::TextureUsage::Storage:
            case kReadOnlyStorageTexture:
                return VK_IMAGE_LAYOUT_GENERAL;

            case wgpu::TextureUsage::RenderAttachment:
                if (texture->GetFormat().HasDepthOrStencil()) {
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                } else {
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }

            case kPresentTextureUsage:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            case wgpu::TextureUsage::None:
                UNREACHABLE();
        }
    }

    VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount) {
        switch (sampleCount) {
            case 1:
                return VK_SAMPLE_COUNT_1_BIT;
            case 4:
                return VK_SAMPLE_COUNT_4_BIT;
            default:
                UNREACHABLE();
        }
    }

    MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase*,
                                               const TextureDescriptor* descriptor) {
        if (descriptor->dimension != wgpu::TextureDimension::e2D) {
            return DAWN_VALIDATION_ERROR("Texture must be 2D");
        }

        if (descriptor->mipLevelCount != 1) {
            return DAWN_VALIDATION_ERROR("Mip level count must be 1");
        }

        if (descriptor->size.depth != 1) {
            return DAWN_VALIDATION_ERROR("Array layer count must be 1");
        }

        if (descriptor->sampleCount != 1) {
            return DAWN_VALIDATION_ERROR("Sample count must be 1");
        }

        return {};
    }

    bool IsSampleCountSupported(const dawn_native::vulkan::Device* device,
                                const VkImageCreateInfo& imageCreateInfo) {
        ASSERT(device);

        VkPhysicalDevice physicalDevice = ToBackend(device->GetAdapter())->GetPhysicalDevice();
        VkImageFormatProperties properties;
        if (device->fn.GetPhysicalDeviceImageFormatProperties(
                physicalDevice, imageCreateInfo.format, imageCreateInfo.imageType,
                imageCreateInfo.tiling, imageCreateInfo.usage, imageCreateInfo.flags,
                &properties) != VK_SUCCESS) {
            UNREACHABLE();
        }

        return properties.sampleCounts & imageCreateInfo.samples;
    }

    // static
    ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                                const TextureDescriptor* descriptor,
                                                VkImageUsageFlags extraUsages) {
        Ref<Texture> texture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
        DAWN_TRY(texture->InitializeAsInternalTexture(extraUsages));
        return std::move(texture);
    }

    // static
    ResultOrError<Texture*> Texture::CreateFromExternal(
        Device* device,
        const ExternalImageDescriptorVk* descriptor,
        const TextureDescriptor* textureDescriptor,
        external_memory::Service* externalMemoryService) {
        Ref<Texture> texture =
            AcquireRef(new Texture(device, textureDescriptor, TextureState::OwnedInternal));
        DAWN_TRY(texture->InitializeFromExternal(descriptor, externalMemoryService));
        return texture.Detach();
    }

    // static
    Ref<Texture> Texture::CreateForSwapChain(Device* device,
                                             const TextureDescriptor* descriptor,
                                             VkImage nativeImage) {
        Ref<Texture> texture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedExternal));
        texture->InitializeForSwapChain(nativeImage);
        return texture;
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor, TextureState state)
        : TextureBase(device, descriptor, state),
          // A usage of none will make sure the texture is transitioned before its first use as
          // required by the Vulkan spec.
          mSubresourceLastUsages(ComputeAspectsForSubresourceStorage(),
                                 GetArrayLayers(),
                                 GetNumMipLevels(),
                                 wgpu::TextureUsage::None) {
    }

    MaybeError Texture::InitializeAsInternalTexture(VkImageUsageFlags extraUsages) {
        Device* device = ToBackend(GetDevice());

        // Create the Vulkan image "container". We don't need to check that the format supports the
        // combination of sample, usage etc. because validation should have been done in the Dawn
        // frontend already based on the minimum supported formats in the Vulkan spec
        VkImageCreateInfo createInfo = {};
        FillVulkanCreateInfoSizesAndType(*this, &createInfo);

        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.format = VulkanImageFormat(device, GetFormat().format);
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = VulkanImageUsage(GetUsage(), GetFormat()) | extraUsages;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        ASSERT(IsSampleCountSupported(device, createInfo));

        if (GetArrayLayers() >= 6 && GetWidth() == GetHeight()) {
            createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally beause the Vulkan images
        // that are used in vkCmdClearColorImage() must have been created with this flag, which is
        // also required for the implementation of robust resource initialization.
        createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
            "CreateImage"));

        // Create the image memory and associate it with the container
        VkMemoryRequirements requirements;
        device->fn.GetImageMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

        DAWN_TRY_ASSIGN(mMemoryAllocation, device->AllocateMemory(requirements, false));

        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(device->GetVkDevice(), mHandle,
                                       ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory(),
                                       mMemoryAllocation.GetOffset()),
            "BindImageMemory"));

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            DAWN_TRY(ClearTexture(ToBackend(GetDevice())->GetPendingRecordingContext(),
                                  GetAllSubresources(), TextureBase::ClearValue::NonZero));
        }

        return {};
    }

    // Internally managed, but imported from external handle
    MaybeError Texture::InitializeFromExternal(const ExternalImageDescriptorVk* descriptor,
                                               external_memory::Service* externalMemoryService) {
        VkFormat format = VulkanImageFormat(ToBackend(GetDevice()), GetFormat().format);
        VkImageUsageFlags usage = VulkanImageUsage(GetUsage(), GetFormat());
        if (!externalMemoryService->SupportsCreateImage(descriptor, format, usage)) {
            return DAWN_VALIDATION_ERROR("Creating an image from external memory is not supported");
        }

        mExternalState = ExternalState::PendingAcquire;

        mPendingAcquireOldLayout = descriptor->releasedOldLayout;
        mPendingAcquireNewLayout = descriptor->releasedNewLayout;

        VkImageCreateInfo baseCreateInfo = {};
        FillVulkanCreateInfoSizesAndType(*this, &baseCreateInfo);

        baseCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        baseCreateInfo.pNext = nullptr;
        baseCreateInfo.format = format;
        baseCreateInfo.usage = usage;
        baseCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        baseCreateInfo.queueFamilyIndexCount = 0;
        baseCreateInfo.pQueueFamilyIndices = nullptr;

        // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally beause the Vulkan images
        // that are used in vkCmdClearColorImage() must have been created with this flag, which is
        // also required for the implementation of robust resource initialization.
        baseCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        DAWN_TRY_ASSIGN(mHandle, externalMemoryService->CreateImage(descriptor, baseCreateInfo));
        return {};
    }

    void Texture::InitializeForSwapChain(VkImage nativeImage) {
        mHandle = nativeImage;
    }

    MaybeError Texture::BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
                                           VkSemaphore signalSemaphore,
                                           VkDeviceMemory externalMemoryAllocation,
                                           std::vector<VkSemaphore> waitSemaphores) {
        Device* device = ToBackend(GetDevice());
        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(device->GetVkDevice(), mHandle, externalMemoryAllocation, 0),
            "BindImageMemory (external)"));

        // Don't clear imported texture if already initialized
        if (descriptor->isInitialized) {
            SetIsSubresourceContentInitialized(true, GetAllSubresources());
        }

        // Success, acquire all the external objects.
        mExternalAllocation = externalMemoryAllocation;
        mSignalSemaphore = signalSemaphore;
        mWaitRequirements = std::move(waitSemaphores);
        return {};
    }

    MaybeError Texture::ExportExternalTexture(VkImageLayout desiredLayout,
                                              VkSemaphore* signalSemaphore,
                                              VkImageLayout* releasedOldLayout,
                                              VkImageLayout* releasedNewLayout) {
        Device* device = ToBackend(GetDevice());

        if (mExternalState == ExternalState::Released) {
            return DAWN_VALIDATION_ERROR("Can't export signal semaphore from signaled texture");
        }

        if (mExternalAllocation == VK_NULL_HANDLE) {
            return DAWN_VALIDATION_ERROR(
                "Can't export signal semaphore from destroyed / non-external texture");
        }

        ASSERT(mSignalSemaphore != VK_NULL_HANDLE);

        // Release the texture
        mExternalState = ExternalState::Released;

        ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);
        wgpu::TextureUsage usage = mSubresourceLastUsages.Get(Aspect::Color, 0, 0);

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.image = GetHandle();
        barrier.subresourceRange.aspectMask = VulkanAspectMask(GetFormat().aspects);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.srcAccessMask = VulkanAccessFlags(usage, GetFormat());
        barrier.dstAccessMask = 0;  // The barrier must be paired with another barrier that will
                                    // specify the dst access mask on the importing queue.

        barrier.oldLayout = VulkanImageLayout(this, usage);
        if (desiredLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            // VK_IMAGE_LAYOUT_UNDEFINED is invalid here. We use it as a
            // special value to indicate no layout transition should be done.
            barrier.newLayout = barrier.oldLayout;
        } else {
            barrier.newLayout = desiredLayout;
        }

        barrier.srcQueueFamilyIndex = device->GetGraphicsQueueFamily();
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;

        VkPipelineStageFlags srcStages = VulkanPipelineStage(usage, GetFormat());
        VkPipelineStageFlags dstStages =
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // We don't know when the importing queue will need
                                                // the texture, so pass
                                                // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT to ensure
                                                // the barrier happens-before any usage in the
                                                // importing queue.

        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        device->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                      nullptr, 0, nullptr, 1, &barrier);

        // Queue submit to signal we are done with the texture
        recordingContext->signalSemaphores.push_back(mSignalSemaphore);
        DAWN_TRY(device->SubmitPendingCommands());

        // Write out the layouts and signal semaphore
        *releasedOldLayout = barrier.oldLayout;
        *releasedNewLayout = barrier.newLayout;
        *signalSemaphore = mSignalSemaphore;

        mSignalSemaphore = VK_NULL_HANDLE;

        // Destroy the texture so it can't be used again
        DestroyInternal();
        return {};
    }

    Texture::~Texture() {
        DestroyInternal();
    }

    void Texture::DestroyImpl() {
        if (GetTextureState() == TextureState::OwnedInternal) {
            Device* device = ToBackend(GetDevice());

            // For textures created from a VkImage, the allocation if kInvalid so the Device knows
            // to skip the deallocation of the (absence of) VkDeviceMemory.
            device->DeallocateMemory(&mMemoryAllocation);

            if (mHandle != VK_NULL_HANDLE) {
                device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            }

            if (mExternalAllocation != VK_NULL_HANDLE) {
                device->GetFencedDeleter()->DeleteWhenUnused(mExternalAllocation);
            }

            mHandle = VK_NULL_HANDLE;
            mExternalAllocation = VK_NULL_HANDLE;
            // If a signal semaphore exists it should be requested before we delete the texture
            ASSERT(mSignalSemaphore == VK_NULL_HANDLE);
        }
    }

    VkImage Texture::GetHandle() const {
        return mHandle;
    }

    VkImageAspectFlags Texture::GetVkAspectMask(wgpu::TextureAspect aspect) const {
        // TODO(enga): These masks could be precomputed.
        switch (aspect) {
            case wgpu::TextureAspect::All:
                return VulkanAspectMask(GetFormat().aspects);
            case wgpu::TextureAspect::DepthOnly:
                ASSERT(GetFormat().aspects & Aspect::Depth);
                return VulkanAspectMask(Aspect::Depth);
            case wgpu::TextureAspect::StencilOnly:
                ASSERT(GetFormat().aspects & Aspect::Stencil);
                return VulkanAspectMask(Aspect::Stencil);
        }
    }

    void Texture::TweakTransitionForExternalUsage(CommandRecordingContext* recordingContext,
                                                  std::vector<VkImageMemoryBarrier>* barriers,
                                                  size_t transitionBarrierStart) {
        ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);

        // transitionBarrierStart specify the index where barriers for current transition start in
        // the vector. barriers->size() - transitionBarrierStart is the number of barriers that we
        // have already added into the vector during current transition.
        ASSERT(barriers->size() - transitionBarrierStart <= 1);

        if (mExternalState == ExternalState::PendingAcquire) {
            if (barriers->size() == transitionBarrierStart) {
                barriers->push_back(BuildMemoryBarrier(
                    this, wgpu::TextureUsage::None, wgpu::TextureUsage::None,
                    SubresourceRange::SingleMipAndLayer(0, 0, GetFormat().aspects)));
            }

            VkImageMemoryBarrier* barrier = &(*barriers)[transitionBarrierStart];
            // Transfer texture from external queue to graphics queue
            barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;
            barrier->dstQueueFamilyIndex = ToBackend(GetDevice())->GetGraphicsQueueFamily();

            // srcAccessMask means nothing when importing. Queue transfers require a barrier on
            // both the importing and exporting queues. The exporting queue should have specified
            // this.
            barrier->srcAccessMask = 0;

            // This should be the first barrier after import.
            ASSERT(barrier->oldLayout == VK_IMAGE_LAYOUT_UNDEFINED);

            // Save the desired layout. We may need to transition through an intermediate
            // |mPendingAcquireLayout| first.
            VkImageLayout desiredLayout = barrier->newLayout;

            bool isInitialized = IsSubresourceContentInitialized(GetAllSubresources());

            // We don't care about the pending old layout if the texture is uninitialized. The
            // driver is free to discard it. Likewise, we don't care about the pending new layout if
            // the texture is uninitialized. We can skip the layout transition.
            if (!isInitialized) {
                barrier->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier->newLayout = desiredLayout;
            } else {
                barrier->oldLayout = mPendingAcquireOldLayout;
                barrier->newLayout = mPendingAcquireNewLayout;
            }

            // If these are unequal, we need an another barrier to transition the layout.
            if (barrier->newLayout != desiredLayout) {
                VkImageMemoryBarrier layoutBarrier;
                layoutBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                layoutBarrier.pNext = nullptr;
                layoutBarrier.image = GetHandle();
                layoutBarrier.subresourceRange = barrier->subresourceRange;

                // Transition from the acquired new layout to the desired layout.
                layoutBarrier.oldLayout = barrier->newLayout;
                layoutBarrier.newLayout = desiredLayout;

                // We already transitioned these.
                layoutBarrier.srcAccessMask = 0;
                layoutBarrier.dstAccessMask = 0;
                layoutBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                layoutBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barriers->push_back(layoutBarrier);
            }

            mExternalState = ExternalState::Acquired;
        }

        mLastExternalState = mExternalState;

        recordingContext->waitSemaphores.insert(recordingContext->waitSemaphores.end(),
                                                mWaitRequirements.begin(), mWaitRequirements.end());
        mWaitRequirements.clear();
    }

    bool Texture::CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage, wgpu::TextureUsage usage) {
        // Reuse the texture directly and avoid encoding barriers when it isn't needed.
        bool lastReadOnly = IsSubset(lastUsage, kReadOnlyTextureUsages);
        if (lastReadOnly && lastUsage == usage && mLastExternalState == mExternalState) {
            return true;
        }
        return false;
    }

    bool Texture::ShouldCombineDepthStencilBarriers() const {
        return GetFormat().aspects == (Aspect::Depth | Aspect::Stencil);
    }

    Aspect Texture::ComputeAspectsForSubresourceStorage() const {
        if (ShouldCombineDepthStencilBarriers()) {
            return Aspect::CombinedDepthStencil;
        }
        return GetFormat().aspects;
    }

    void Texture::TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                         const PassTextureUsage& textureUsages,
                                         std::vector<VkImageMemoryBarrier>* imageBarriers,
                                         VkPipelineStageFlags* srcStages,
                                         VkPipelineStageFlags* dstStages) {
        // Base Vulkan doesn't support transitioning depth and stencil separately. We work around
        // this limitation by combining the usages in the two planes of `textureUsages` into a
        // single plane in a new SubresourceStorage<TextureUsage>. The barriers will be produced
        // for DEPTH | STENCIL since the SubresourceRange uses Aspect::CombinedDepthStencil.
        if (ShouldCombineDepthStencilBarriers()) {
            SubresourceStorage<wgpu::TextureUsage> combinedUsages(
                Aspect::CombinedDepthStencil, GetArrayLayers(), GetNumMipLevels());
            textureUsages.Iterate([&](const SubresourceRange& range, wgpu::TextureUsage usage) {
                SubresourceRange updateRange = range;
                updateRange.aspects = Aspect::CombinedDepthStencil;

                combinedUsages.Update(
                    updateRange, [&](const SubresourceRange&, wgpu::TextureUsage* combinedUsage) {
                        *combinedUsage |= usage;
                    });
            });

            TransitionUsageForPassImpl(recordingContext, combinedUsages, imageBarriers, srcStages,
                                       dstStages);
        } else {
            TransitionUsageForPassImpl(recordingContext, textureUsages, imageBarriers, srcStages,
                                       dstStages);
        }
    }

    void Texture::TransitionUsageForPassImpl(
        CommandRecordingContext* recordingContext,
        const SubresourceStorage<wgpu::TextureUsage>& subresourceUsages,
        std::vector<VkImageMemoryBarrier>* imageBarriers,
        VkPipelineStageFlags* srcStages,
        VkPipelineStageFlags* dstStages) {
        size_t transitionBarrierStart = imageBarriers->size();
        const Format& format = GetFormat();

        wgpu::TextureUsage allUsages = wgpu::TextureUsage::None;
        wgpu::TextureUsage allLastUsages = wgpu::TextureUsage::None;

        // This transitions assume it is a 2D texture
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);

        mSubresourceLastUsages.Merge(
            subresourceUsages, [&](const SubresourceRange& range, wgpu::TextureUsage* lastUsage,
                                   const wgpu::TextureUsage& newUsage) {
                if (newUsage == wgpu::TextureUsage::None ||
                    CanReuseWithoutBarrier(*lastUsage, newUsage)) {
                    return;
                }

                imageBarriers->push_back(BuildMemoryBarrier(this, *lastUsage, newUsage, range));

                allLastUsages |= *lastUsage;
                allUsages |= newUsage;

                *lastUsage = newUsage;
            });

        if (mExternalState != ExternalState::InternalOnly) {
            TweakTransitionForExternalUsage(recordingContext, imageBarriers,
                                            transitionBarrierStart);
        }

        *srcStages |= VulkanPipelineStage(allLastUsages, format);
        *dstStages |= VulkanPipelineStage(allUsages, format);
    }

    void Texture::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                     wgpu::TextureUsage usage,
                                     const SubresourceRange& range) {
        std::vector<VkImageMemoryBarrier> barriers;

        VkPipelineStageFlags srcStages = 0;
        VkPipelineStageFlags dstStages = 0;

        TransitionUsageAndGetResourceBarrier(usage, range, &barriers, &srcStages, &dstStages);

        if (mExternalState != ExternalState::InternalOnly) {
            TweakTransitionForExternalUsage(recordingContext, &barriers, 0);
        }

        if (!barriers.empty()) {
            ASSERT(srcStages != 0 && dstStages != 0);
            ToBackend(GetDevice())
                ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                        nullptr, 0, nullptr, barriers.size(), barriers.data());
        }
    }

    void Texture::TransitionUsageAndGetResourceBarrier(
        wgpu::TextureUsage usage,
        const SubresourceRange& range,
        std::vector<VkImageMemoryBarrier>* imageBarriers,
        VkPipelineStageFlags* srcStages,
        VkPipelineStageFlags* dstStages) {
        // Base Vulkan doesn't support transitioning depth and stencil separately. We work around
        // this limitation by modifying the range to be on CombinedDepthStencil. The barriers will
        // be produced for DEPTH | STENCIL since the SubresourceRange uses
        // Aspect::CombinedDepthStencil.
        if (ShouldCombineDepthStencilBarriers()) {
            SubresourceRange updatedRange = range;
            updatedRange.aspects = Aspect::CombinedDepthStencil;

            std::vector<VkImageMemoryBarrier> newBarriers;
            TransitionUsageAndGetResourceBarrierImpl(usage, updatedRange, imageBarriers, srcStages,
                                                     dstStages);
        } else {
            TransitionUsageAndGetResourceBarrierImpl(usage, range, imageBarriers, srcStages,
                                                     dstStages);
        }
    }

    void Texture::TransitionUsageAndGetResourceBarrierImpl(
        wgpu::TextureUsage usage,
        const SubresourceRange& range,
        std::vector<VkImageMemoryBarrier>* imageBarriers,
        VkPipelineStageFlags* srcStages,
        VkPipelineStageFlags* dstStages) {
        ASSERT(imageBarriers != nullptr);
        const Format& format = GetFormat();

        // This transitions assume it is a 2D texture
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);

        wgpu::TextureUsage allLastUsages = wgpu::TextureUsage::None;
        mSubresourceLastUsages.Update(
            range, [&](const SubresourceRange& range, wgpu::TextureUsage* lastUsage) {
                if (CanReuseWithoutBarrier(*lastUsage, usage)) {
                    return;
                }

                imageBarriers->push_back(BuildMemoryBarrier(this, *lastUsage, usage, range));

                allLastUsages |= *lastUsage;
                *lastUsage = usage;
            });

        *srcStages |= VulkanPipelineStage(allLastUsages, format);
        *dstStages |= VulkanPipelineStage(usage, format);
    }

    MaybeError Texture::ClearTexture(CommandRecordingContext* recordingContext,
                                     const SubresourceRange& range,
                                     TextureBase::ClearValue clearValue) {
        Device* device = ToBackend(GetDevice());

        const bool isZero = clearValue == TextureBase::ClearValue::Zero;
        uint32_t uClearColor = isZero ? 0 : 1;
        int32_t sClearColor = isZero ? 0 : 1;
        float fClearColor = isZero ? 0.f : 1.f;

        TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst, range);

        VkImageSubresourceRange imageRange = {};
        imageRange.levelCount = 1;
        imageRange.layerCount = 1;

        if (GetFormat().isCompressed) {
            if (range.aspects == Aspect::None) {
                return {};
            }
            // need to clear the texture with a copy from buffer
            ASSERT(range.aspects == Aspect::Color);
            const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(range.aspects).block;

            uint32_t bytesPerRow = Align((GetWidth() / blockInfo.width) * blockInfo.byteSize,
                                         device->GetOptimalBytesPerRowAlignment());
            uint64_t bufferSize = bytesPerRow * (GetHeight() / blockInfo.height);
            DynamicUploader* uploader = device->GetDynamicUploader();
            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle,
                            uploader->Allocate(bufferSize, device->GetPendingCommandSerial(),
                                               blockInfo.byteSize));
            memset(uploadHandle.mappedBuffer, uClearColor, bufferSize);

            std::vector<VkBufferImageCopy> regions;
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                imageRange.baseMipLevel = level;
                for (uint32_t layer = range.baseArrayLayer;
                     layer < range.baseArrayLayer + range.layerCount; ++layer) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(
                            SubresourceRange::SingleMipAndLayer(level, layer, range.aspects))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    TextureDataLayout dataLayout;
                    dataLayout.offset = uploadHandle.startOffset;
                    dataLayout.rowsPerImage = GetHeight() / blockInfo.height;
                    dataLayout.bytesPerRow = bytesPerRow;
                    TextureCopy textureCopy;
                    textureCopy.aspect = range.aspects;
                    textureCopy.mipLevel = level;
                    textureCopy.origin = {0, 0, layer};
                    textureCopy.texture = this;

                    regions.push_back(ComputeBufferImageCopyRegion(dataLayout, textureCopy,
                                                                   GetMipLevelPhysicalSize(level)));
                }
            }
            device->fn.CmdCopyBufferToImage(
                recordingContext->commandBuffer,
                ToBackend(uploadHandle.stagingBuffer)->GetBufferHandle(), GetHandle(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, regions.data());
        } else {
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                imageRange.baseMipLevel = level;
                for (uint32_t layer = range.baseArrayLayer;
                     layer < range.baseArrayLayer + range.layerCount; ++layer) {
                    Aspect aspects = Aspect::None;
                    for (Aspect aspect : IterateEnumMask(range.aspects)) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }
                        aspects |= aspect;
                    }

                    if (aspects == Aspect::None) {
                        continue;
                    }

                    imageRange.aspectMask = VulkanAspectMask(aspects);
                    imageRange.baseArrayLayer = layer;

                    if (aspects &
                        (Aspect::Depth | Aspect::Stencil | Aspect::CombinedDepthStencil)) {
                        VkClearDepthStencilValue clearDepthStencilValue[1];
                        clearDepthStencilValue[0].depth = fClearColor;
                        clearDepthStencilValue[0].stencil = uClearColor;
                        device->fn.CmdClearDepthStencilImage(
                            recordingContext->commandBuffer, GetHandle(),
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearDepthStencilValue, 1,
                            &imageRange);
                    } else {
                        ASSERT(aspects == Aspect::Color);
                        VkClearColorValue clearColorValue;
                        switch (GetFormat().GetAspectInfo(Aspect::Color).baseType) {
                            case wgpu::TextureComponentType::Float:
                                clearColorValue.float32[0] = fClearColor;
                                clearColorValue.float32[1] = fClearColor;
                                clearColorValue.float32[2] = fClearColor;
                                clearColorValue.float32[3] = fClearColor;
                                break;
                            case wgpu::TextureComponentType::Sint:
                                clearColorValue.int32[0] = sClearColor;
                                clearColorValue.int32[1] = sClearColor;
                                clearColorValue.int32[2] = sClearColor;
                                clearColorValue.int32[3] = sClearColor;
                                break;
                            case wgpu::TextureComponentType::Uint:
                                clearColorValue.uint32[0] = uClearColor;
                                clearColorValue.uint32[1] = uClearColor;
                                clearColorValue.uint32[2] = uClearColor;
                                clearColorValue.uint32[3] = uClearColor;
                                break;
                            case wgpu::TextureComponentType::DepthComparison:
                                UNREACHABLE();
                        }
                        device->fn.CmdClearColorImage(recordingContext->commandBuffer, GetHandle(),
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                      &clearColorValue, 1, &imageRange);
                    }
                }
            }
        }

        if (clearValue == TextureBase::ClearValue::Zero) {
            SetIsSubresourceContentInitialized(true, range);
            device->IncrementLazyClearCountForTesting();
        }
        return {};
    }

    void Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                      const SubresourceRange& range) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(range)) {
            // If subresource has not been initialized, clear it to black as it could contain dirty
            // bits from recycled memory
            GetDevice()->ConsumedError(
                ClearTexture(recordingContext, range, TextureBase::ClearValue::Zero));
        }
    }

    VkImageLayout Texture::GetCurrentLayoutForSwapChain() const {
        return VulkanImageLayout(this, mSubresourceLastUsages.Get(Aspect::Color, 0, 0));
    }

    // static
    ResultOrError<TextureView*> TextureView::Create(TextureBase* texture,
                                                    const TextureViewDescriptor* descriptor) {
        Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
        DAWN_TRY(view->Initialize(descriptor));
        return view.Detach();
    }

    MaybeError TextureView::Initialize(const TextureViewDescriptor* descriptor) {
        if ((GetTexture()->GetUsage() &
             ~(wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) == 0) {
            // If the texture view has no other usage than CopySrc and CopyDst, then it can't
            // actually be used as a render pass attachment or sampled/storage texture. The Vulkan
            // validation errors warn if you create such a vkImageView, so return early.
            return {};
        }

        Device* device = ToBackend(GetTexture()->GetDevice());

        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = ToBackend(GetTexture())->GetHandle();
        createInfo.viewType = VulkanImageViewType(descriptor->dimension);
        createInfo.format = VulkanImageFormat(device, descriptor->format);
        createInfo.components = VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                                                   VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        const SubresourceRange& subresources = GetSubresourceRange();
        createInfo.subresourceRange.baseMipLevel = subresources.baseMipLevel;
        createInfo.subresourceRange.levelCount = subresources.levelCount;
        createInfo.subresourceRange.baseArrayLayer = subresources.baseArrayLayer;
        createInfo.subresourceRange.layerCount = subresources.layerCount;
        createInfo.subresourceRange.aspectMask = VulkanAspectMask(subresources.aspects);

        return CheckVkSuccess(
            device->fn.CreateImageView(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
            "CreateImageView");
    }

    TextureView::~TextureView() {
        Device* device = ToBackend(GetTexture()->GetDevice());

        if (mHandle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkImageView TextureView::GetHandle() const {
        return mHandle;
    }

}}  // namespace dawn_native::vulkan
