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
        // Converts an Dawn texture dimension to a Vulkan image type.
        // Note that in Vulkan dimensionality is only 1D, 2D, 3D. Arrays and cube maps are expressed
        // via the array size and a "cubemap compatible" flag.
        VkImageType VulkanImageType(wgpu::TextureDimension dimension) {
            switch (dimension) {
                case wgpu::TextureDimension::e2D:
                    return VK_IMAGE_TYPE_2D;
                default:
                    UNREACHABLE();
            }
        }

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
                default:
                    UNREACHABLE();
            }
        }

        // Computes which vulkan access type could be required for the given Dawn usage.
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
            if (usage & wgpu::TextureUsage::OutputAttachment) {
                if (format.HasDepthOrStencil()) {
                    flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                } else {
                    flags |=
                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                }
            }
            if (usage & wgpu::TextureUsage::Present) {
                // There is no access flag for present because the VK_KHR_SWAPCHAIN extension says
                // that vkQueuePresentKHR makes the memory of the image visible to the presentation
                // engine. There's also a note explicitly saying dstAccessMask should be 0. On the
                // other side srcAccessMask can also be 0 because synchronization is required to
                // happen with a semaphore instead.
                flags |= 0;
            }

            return flags;
        }

        // Chooses which Vulkan image layout should be used for the given Dawn usage
        VkImageLayout VulkanImageLayout(wgpu::TextureUsage usage, const Format& format) {
            if (usage == wgpu::TextureUsage::None) {
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }

            if (!wgpu::HasZeroOrOneBits(usage)) {
                return VK_IMAGE_LAYOUT_GENERAL;
            }

            // Usage has a single bit so we can switch on its value directly.
            switch (usage) {
                case wgpu::TextureUsage::CopyDst:
                    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                case wgpu::TextureUsage::Sampled:
                    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                // Vulkan texture copy functions require the image to be in _one_  known layout.
                // Depending on whether parts of the texture have been transitioned to only
                // CopySrc or a combination with something else, the texture could be in a
                // combination of GENERAL and TRANSFER_SRC_OPTIMAL. This would be a problem, so we
                // make CopySrc use GENERAL.
                case wgpu::TextureUsage::CopySrc:
                // Writable storage textures must use general. If we could know the texture is read
                // only we could use SHADER_READ_ONLY_OPTIMAL
                case wgpu::TextureUsage::Storage:
                    return VK_IMAGE_LAYOUT_GENERAL;
                case wgpu::TextureUsage::OutputAttachment:
                    if (format.HasDepthOrStencil()) {
                        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    } else {
                        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    }
                case wgpu::TextureUsage::Present:
                    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                default:
                    UNREACHABLE();
            }
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
            if (usage & (wgpu::TextureUsage::Sampled | wgpu::TextureUsage::Storage)) {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            if (usage & wgpu::TextureUsage::OutputAttachment) {
                if (format.HasDepthOrStencil()) {
                    flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    // TODO(cwallez@chromium.org): This is missing the stage where the depth and
                    // stencil values are written, but it isn't clear which one it is.
                } else {
                    flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                }
            }
            if (usage & wgpu::TextureUsage::Present) {
                // There is no pipeline stage for present but a pipeline stage is required so we use
                // "bottom of pipe" to block as little as possible and vkQueuePresentKHR will make
                // the memory visible to the presentation engine. The spec explicitly mentions that
                // "bottom of pipe" is ok. On the other direction, synchronization happens with a
                // semaphore so bottom of pipe is ok too (but maybe it could be "top of pipe" to
                // block less?)
                flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }

            // A zero value isn't a valid pipeline stage mask
            ASSERT(flags != 0);
            return flags;
        }

        // Computes which Vulkan texture aspects are relevant for the given Dawn format
        VkImageAspectFlags VulkanAspectMask(const Format& format) {
            switch (format.aspect) {
                case Format::Aspect::Color:
                    return VK_IMAGE_ASPECT_COLOR_BIT;
                case Format::Aspect::Depth:
                    return VK_IMAGE_ASPECT_DEPTH_BIT;
                case Format::Aspect::Stencil:
                    return VK_IMAGE_ASPECT_STENCIL_BIT;
                case Format::Aspect::DepthStencil:
                    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                default:
                    UNREACHABLE();
                    return 0;
            }
        }

        VkExtent3D VulkanExtent3D(const Extent3D& extent) {
            return {extent.width, extent.height, extent.depth};
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
            case wgpu::TextureFormat::RG11B10Float:
                return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

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
            case wgpu::TextureFormat::BC6HRGBSfloat:
                return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return VK_FORMAT_BC7_UNORM_BLOCK;
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return VK_FORMAT_BC7_SRGB_BLOCK;

            default:
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
        if (usage & wgpu::TextureUsage::Storage) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if (usage & wgpu::TextureUsage::OutputAttachment) {
            if (format.HasDepthOrStencil()) {
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            } else {
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        return flags;
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

        if (descriptor->arrayLayerCount != 1) {
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
    ResultOrError<Texture*> Texture::Create(Device* device, const TextureDescriptor* descriptor) {
        std::unique_ptr<Texture> texture =
            std::make_unique<Texture>(device, descriptor, TextureState::OwnedInternal);
        DAWN_TRY(texture->InitializeAsInternalTexture());
        return texture.release();
    }

    // static
    ResultOrError<Texture*> Texture::CreateFromExternal(
        Device* device,
        const ExternalImageDescriptor* descriptor,
        const TextureDescriptor* textureDescriptor,
        external_memory::Service* externalMemoryService) {
        std::unique_ptr<Texture> texture =
            std::make_unique<Texture>(device, textureDescriptor, TextureState::OwnedInternal);
        DAWN_TRY(texture->InitializeFromExternal(descriptor, externalMemoryService));
        return texture.release();
    }

    MaybeError Texture::InitializeAsInternalTexture() {
        Device* device = ToBackend(GetDevice());

        // Create the Vulkan image "container". We don't need to check that the format supports the
        // combination of sample, usage etc. because validation should have been done in the Dawn
        // frontend already based on the minimum supported formats in the Vulkan spec
        VkImageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.imageType = VulkanImageType(GetDimension());
        createInfo.format = VulkanImageFormat(device, GetFormat().format);
        createInfo.extent = VulkanExtent3D(GetSize());
        createInfo.mipLevels = GetNumMipLevels();
        createInfo.arrayLayers = GetArrayLayers();
        createInfo.samples = VulkanSampleCount(GetSampleCount());
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = VulkanImageUsage(GetUsage(), GetFormat());
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        ASSERT(IsSampleCountSupported(device, createInfo));

        if (GetArrayLayers() >= 6 && GetSize().width == GetSize().height) {
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
            DAWN_TRY(ClearTexture(ToBackend(GetDevice())->GetPendingRecordingContext(), 0,
                                  GetNumMipLevels(), 0, GetArrayLayers(),
                                  TextureBase::ClearValue::NonZero));
        }

        return {};
    }

    // With this constructor, the lifetime of the resource is externally managed.
    Texture::Texture(Device* device, const TextureDescriptor* descriptor, VkImage nativeImage)
        : TextureBase(device, descriptor, TextureState::OwnedExternal), mHandle(nativeImage) {
    }

    // Internally managed, but imported from external handle
    MaybeError Texture::InitializeFromExternal(const ExternalImageDescriptor* descriptor,
                                               external_memory::Service* externalMemoryService) {
        VkFormat format = VulkanImageFormat(ToBackend(GetDevice()), GetFormat().format);
        VkImageUsageFlags usage = VulkanImageUsage(GetUsage(), GetFormat());
        if (!externalMemoryService->SupportsCreateImage(descriptor, format, usage)) {
            return DAWN_VALIDATION_ERROR("Creating an image from external memory is not supported");
        }

        mExternalState = ExternalState::PendingAcquire;
        VkImageCreateInfo baseCreateInfo = {};
        baseCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        baseCreateInfo.pNext = nullptr;
        baseCreateInfo.imageType = VulkanImageType(GetDimension());
        baseCreateInfo.format = format;
        baseCreateInfo.extent = VulkanExtent3D(GetSize());
        baseCreateInfo.mipLevels = GetNumMipLevels();
        baseCreateInfo.arrayLayers = GetArrayLayers();
        baseCreateInfo.samples = VulkanSampleCount(GetSampleCount());
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

    MaybeError Texture::BindExternalMemory(const ExternalImageDescriptor* descriptor,
                                           VkSemaphore signalSemaphore,
                                           VkDeviceMemory externalMemoryAllocation,
                                           std::vector<VkSemaphore> waitSemaphores) {
        Device* device = ToBackend(GetDevice());
        DAWN_TRY(CheckVkSuccess(
            device->fn.BindImageMemory(device->GetVkDevice(), mHandle, externalMemoryAllocation, 0),
            "BindImageMemory (external)"));

        // Don't clear imported texture if already cleared
        if (descriptor->isCleared) {
            SetIsSubresourceContentInitialized(true, 0, 1, 0, 1);
        }

        // Success, acquire all the external objects.
        mExternalAllocation = externalMemoryAllocation;
        mSignalSemaphore = signalSemaphore;
        mWaitRequirements = std::move(waitSemaphores);
        return {};
    }

    MaybeError Texture::SignalAndDestroy(VkSemaphore* outSignalSemaphore) {
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
        mExternalState = ExternalState::PendingRelease;
        TransitionUsageNow(device->GetPendingRecordingContext(), wgpu::TextureUsage::None);

        // Queue submit to signal we are done with the texture
        device->GetPendingRecordingContext()->signalSemaphores.push_back(mSignalSemaphore);
        DAWN_TRY(device->SubmitPendingCommands());

        // Write out the signal semaphore
        *outSignalSemaphore = mSignalSemaphore;
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

    VkImageAspectFlags Texture::GetVkAspectMask() const {
        return VulkanAspectMask(GetFormat());
    }

    void Texture::TransitionUsageNow(CommandRecordingContext* recordingContext,
                                     wgpu::TextureUsage usage) {
        // Avoid encoding barriers when it isn't needed.
        bool lastReadOnly = (mLastUsage & kReadOnlyTextureUsages) == mLastUsage;
        if (lastReadOnly && mLastUsage == usage && mLastExternalState == mExternalState) {
            return;
        }

        const Format& format = GetFormat();

        VkPipelineStageFlags srcStages = VulkanPipelineStage(mLastUsage, format);
        VkPipelineStageFlags dstStages = VulkanPipelineStage(usage, format);

        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VulkanAccessFlags(mLastUsage, format);
        barrier.dstAccessMask = VulkanAccessFlags(usage, format);
        barrier.oldLayout = VulkanImageLayout(mLastUsage, format);
        barrier.newLayout = VulkanImageLayout(usage, format);
        barrier.image = mHandle;
        // This transitions the whole resource but assumes it is a 2D texture
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
        barrier.subresourceRange.aspectMask = VulkanAspectMask(format);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = GetNumMipLevels();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = GetArrayLayers();

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        if (mExternalState == ExternalState::PendingAcquire) {
            // Transfer texture from external queue to graphics queue
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;
            barrier.dstQueueFamilyIndex = ToBackend(GetDevice())->GetGraphicsQueueFamily();
            // Don't override oldLayout to leave it as VK_IMAGE_LAYOUT_UNDEFINED
            // TODO(http://crbug.com/dawn/200)
            mExternalState = ExternalState::Acquired;

        } else if (mExternalState == ExternalState::PendingRelease) {
            // Transfer texture from graphics queue to external queue
            barrier.srcQueueFamilyIndex = ToBackend(GetDevice())->GetGraphicsQueueFamily();
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHR;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            mExternalState = ExternalState::Released;
        }

        // Move required semaphores into waitSemaphores
        recordingContext->waitSemaphores.insert(recordingContext->waitSemaphores.end(),
                                                mWaitRequirements.begin(), mWaitRequirements.end());
        mWaitRequirements.clear();

        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                    nullptr, 0, nullptr, 1, &barrier);

        mLastUsage = usage;
        mLastExternalState = mExternalState;
    }

    MaybeError Texture::ClearTexture(CommandRecordingContext* recordingContext,
                                     uint32_t baseMipLevel,
                                     uint32_t levelCount,
                                     uint32_t baseArrayLayer,
                                     uint32_t layerCount,
                                     TextureBase::ClearValue clearValue) {
        Device* device = ToBackend(GetDevice());

        uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
        float fClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.f : 1.f;

        TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst);
        if (GetFormat().isRenderable) {
            VkImageSubresourceRange range = {};
            range.aspectMask = GetVkAspectMask();
            range.levelCount = 1;
            range.layerCount = 1;

            for (uint32_t level = baseMipLevel; level < baseMipLevel + levelCount; ++level) {
                range.baseMipLevel = level;
                for (uint32_t layer = baseArrayLayer; layer < baseArrayLayer + layerCount;
                     ++layer) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(level, 1, layer, 1)) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    range.baseArrayLayer = layer;

                    if (GetFormat().HasDepthOrStencil()) {
                        VkClearDepthStencilValue clearDepthStencilValue[1];
                        clearDepthStencilValue[0].depth = fClearColor;
                        clearDepthStencilValue[0].stencil = clearColor;
                        device->fn.CmdClearDepthStencilImage(recordingContext->commandBuffer,
                                                             GetHandle(),
                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                             clearDepthStencilValue, 1, &range);
                    } else {
                        VkClearColorValue clearColorValue = {
                            {fClearColor, fClearColor, fClearColor, fClearColor}};
                        device->fn.CmdClearColorImage(recordingContext->commandBuffer, GetHandle(),
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                      &clearColorValue, 1, &range);
                    }
                }
            }
        } else {
            // TODO(natlee@microsoft.com): test compressed textures are cleared
            // create temp buffer with clear color to copy to the texture image
            uint32_t rowPitch =
                Align((GetSize().width / GetFormat().blockWidth) * GetFormat().blockByteSize,
                      kTextureRowPitchAlignment);
            uint64_t bufferSize64 = rowPitch * (GetSize().height / GetFormat().blockHeight);
            if (bufferSize64 > std::numeric_limits<uint32_t>::max()) {
                return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
            }
            uint32_t bufferSize = static_cast<uint32_t>(bufferSize64);
            DynamicUploader* uploader = device->GetDynamicUploader();
            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle,
                            uploader->Allocate(bufferSize, device->GetPendingCommandSerial()));
            memset(uploadHandle.mappedBuffer, clearColor, bufferSize);

            // compute the buffer image copy to set the clear region of entire texture
            dawn_native::BufferCopy bufferCopy;
            bufferCopy.imageHeight = 0;
            bufferCopy.offset = uploadHandle.startOffset;
            bufferCopy.rowPitch = rowPitch;

            for (uint32_t level = baseMipLevel; level < baseMipLevel + levelCount; ++level) {
                Extent3D copySize = GetMipLevelVirtualSize(level);

                for (uint32_t layer = baseArrayLayer; layer < baseArrayLayer + layerCount;
                     ++layer) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(level, 1, layer, 1)) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    dawn_native::TextureCopy textureCopy;
                    textureCopy.texture = this;
                    textureCopy.origin = {0, 0, 0};
                    textureCopy.mipLevel = level;
                    textureCopy.arrayLayer = layer;

                    VkBufferImageCopy region =
                        ComputeBufferImageCopyRegion(bufferCopy, textureCopy, copySize);

                    // copy the clear buffer to the texture image
                    device->fn.CmdCopyBufferToImage(
                        recordingContext->commandBuffer,
                        ToBackend(uploadHandle.stagingBuffer)->GetBufferHandle(), GetHandle(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                }
            }
        }
        if (clearValue == TextureBase::ClearValue::Zero) {
            SetIsSubresourceContentInitialized(true, baseMipLevel, levelCount, baseArrayLayer,
                                               layerCount);
            device->IncrementLazyClearCountForTesting();
        }
        return {};
    }

    void Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                      uint32_t baseMipLevel,
                                                      uint32_t levelCount,
                                                      uint32_t baseArrayLayer,
                                                      uint32_t layerCount) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(baseMipLevel, levelCount, baseArrayLayer,
                                             layerCount)) {
            // TODO(jiawei.shao@intel.com): initialize textures in BC formats with Buffer-to-Texture
            // copies.
            if (GetFormat().isCompressed) {
                return;
            }

            // If subresource has not been initialized, clear it to black as it could contain dirty
            // bits from recycled memory
            GetDevice()->ConsumedError(ClearTexture(recordingContext, baseMipLevel, levelCount,
                                                    baseArrayLayer, layerCount,
                                                    TextureBase::ClearValue::Zero));
        }
    }

    // static
    ResultOrError<TextureView*> TextureView::Create(TextureBase* texture,
                                                    const TextureViewDescriptor* descriptor) {
        std::unique_ptr<TextureView> view = std::make_unique<TextureView>(texture, descriptor);
        DAWN_TRY(view->Initialize(descriptor));
        return view.release();
    }

    MaybeError TextureView::Initialize(const TextureViewDescriptor* descriptor) {
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
        createInfo.subresourceRange.aspectMask = VulkanAspectMask(GetFormat());
        createInfo.subresourceRange.baseMipLevel = descriptor->baseMipLevel;
        createInfo.subresourceRange.levelCount = descriptor->mipLevelCount;
        createInfo.subresourceRange.baseArrayLayer = descriptor->baseArrayLayer;
        createInfo.subresourceRange.layerCount = descriptor->arrayLayerCount;

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
