// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/TextureVk.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Error.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/CommandRecordingContext.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {
// Converts an Dawn texture dimension to a Vulkan image view type.
// Contrary to image types, image view types include arrayness and cubemapness
VkImageViewType VulkanImageViewType(wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case wgpu::TextureViewDimension::e2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case wgpu::TextureViewDimension::e2DArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case wgpu::TextureViewDimension::Cube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case wgpu::TextureViewDimension::CubeArray:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case wgpu::TextureViewDimension::e3D:
            return VK_IMAGE_VIEW_TYPE_3D;

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

// Reserved texture usages to represent mixed read-only/writable depth-stencil texture usages
// when combining the planes of depth-stencil textures. They can be combined with other in-pass
// readonly usages like wgpu::TextureUsage::TextureBinding.
// TODO(dawn:2172): Consider making a bespoke enum instead of hackily extending TextureUsage.
constexpr wgpu::TextureUsage kDepthReadOnlyStencilWritableAttachment =
    kReservedTextureUsage | static_cast<wgpu::TextureUsage>(1 << 30);
constexpr wgpu::TextureUsage kDepthWritableStencilReadOnlyAttachment =
    kReservedTextureUsage | static_cast<wgpu::TextureUsage>(1 << 29);

// Merge two usages for depth and stencil into a single combined usage that uses the reserved
// texture usages above. This is used to handle combining Aspect::Depth and Aspect::Stencil into a
// single Aspect::CombinedDepthStencil.
wgpu::TextureUsage MergeDepthStencilUsage(wgpu::TextureUsage depth, wgpu::TextureUsage stencil) {
    // Aspects that are RenderAttachment cannot be anything else at the same time. This lets us
    // check if we are in one of the RenderAttachment + (ReadOnlyAttachment|readonly usage) cases
    // and know only the aspect with the readonly attachment might contain extra usages like
    // TextureBinding.
    DAWN_ASSERT(depth == wgpu::TextureUsage::RenderAttachment ||
                IsSubset(depth, ~wgpu::TextureUsage::RenderAttachment));
    DAWN_ASSERT(stencil == wgpu::TextureUsage::RenderAttachment ||
                IsSubset(stencil, ~wgpu::TextureUsage::RenderAttachment));

    if (depth == wgpu::TextureUsage::RenderAttachment && stencil & kReadOnlyRenderAttachment) {
        return kDepthWritableStencilReadOnlyAttachment | (stencil & ~kReadOnlyRenderAttachment);
    } else if (depth & kReadOnlyRenderAttachment &&
               stencil == wgpu::TextureUsage::RenderAttachment) {
        return kDepthReadOnlyStencilWritableAttachment | (depth & ~kReadOnlyRenderAttachment);
    } else {
        // Not one of the reserved usage special cases, we can just combine the aspect's usage the
        // simple way!
        return depth | stencil;
    }
}

// Computes which vulkan access type could be required for the given Dawn usage.
// TODO(crbug.com/dawn/269): We shouldn't need any access usages for srcAccessMask when
// the previous usage is readonly because an execution dependency is sufficient.
VkAccessFlags VulkanAccessFlags(wgpu::TextureUsage usage, const Format& format) {
    if (usage & kReservedTextureUsage) {
        // Handle the special readonly usages for mixed depth-stencil.
        DAWN_ASSERT(IsSubset(kDepthReadOnlyStencilWritableAttachment, usage) ||
                    IsSubset(kDepthWritableStencilReadOnlyAttachment, usage));

        // Add any additional access flags for the non-attachment part of the usage.
        const wgpu::TextureUsage nonAttachmentUsages =
            usage &
            ~(kDepthReadOnlyStencilWritableAttachment | kDepthWritableStencilReadOnlyAttachment);
        return VulkanAccessFlags(nonAttachmentUsages, format) |
               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkAccessFlags flags = 0;
    if (usage & wgpu::TextureUsage::CopySrc) {
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (usage & wgpu::TextureUsage::CopyDst) {
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (usage & (wgpu::TextureUsage::TextureBinding | kReadOnlyStorageTexture)) {
        flags |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (usage & kWriteOnlyStorageTexture) {
        flags |= VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (usage & wgpu::TextureUsage::StorageBinding) {
        flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (usage & wgpu::TextureUsage::RenderAttachment) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        } else {
            flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
    }
    if (usage & kReadOnlyRenderAttachment) {
        flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if (usage & kPresentTextureUsage) {
        // The present usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentTextureUsage);
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
    if (usage & kReservedTextureUsage) {
        // Handle the special readonly usages for mixed depth-stencil.
        DAWN_ASSERT(IsSubset(kDepthReadOnlyStencilWritableAttachment, usage) ||
                    IsSubset(kDepthWritableStencilReadOnlyAttachment, usage));

        // Convert all the reserved attachment usages into just RenderAttachment.
        const wgpu::TextureUsage nonAttachmentUsages =
            usage &
            ~(kDepthReadOnlyStencilWritableAttachment | kDepthWritableStencilReadOnlyAttachment);
        return VulkanPipelineStage(nonAttachmentUsages | wgpu::TextureUsage::RenderAttachment,
                                   format);
    }

    VkPipelineStageFlags flags = 0;

    if (usage == wgpu::TextureUsage::None) {
        // This only happens when a texture is initially created (and for srcAccessMask) in
        // which case there is no need to wait on anything to stop accessing this texture.
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (usage & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (usage & (wgpu::TextureUsage::TextureBinding | kReadOnlyStorageTexture)) {
        // TODO(crbug.com/dawn/851): Only transition to the usage we care about to avoid
        // introducing FS -> VS dependencies that would prevent parallelization on tiler
        // GPUs
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if (usage & (wgpu::TextureUsage::StorageBinding | kWriteOnlyStorageTexture)) {
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if (usage & (wgpu::TextureUsage::RenderAttachment | kReadOnlyRenderAttachment)) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } else {
            flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
    }
    if (usage & kPresentTextureUsage) {
        // The present usage is only used internally by the swapchain and is never used in
        // combination with other usages.
        DAWN_ASSERT(usage == kPresentTextureUsage);
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
    DAWN_ASSERT(flags != 0);
    return flags;
}

VkImageMemoryBarrier BuildMemoryBarrier(const Texture* texture,
                                        wgpu::TextureUsage lastUsage,
                                        wgpu::TextureUsage usage,
                                        const SubresourceRange& range) {
    const Format& format = texture->GetFormat();

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VulkanAccessFlags(lastUsage, format);
    barrier.dstAccessMask = VulkanAccessFlags(usage, format);
    barrier.oldLayout = VulkanImageLayout(format, lastUsage);
    barrier.newLayout = VulkanImageLayout(format, usage);
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
    const Extent3D& size = texture.GetBaseSize();

    info->mipLevels = texture.GetNumMipLevels();
    info->samples = VulkanSampleCount(texture.GetSampleCount());

    // Fill in the image type, and paper over differences in how the array layer count is
    // specified between WebGPU and Vulkan.
    switch (texture.GetDimension()) {
        case wgpu::TextureDimension::e1D:
            info->imageType = VK_IMAGE_TYPE_1D;
            info->extent = {size.width, 1, 1};
            info->arrayLayers = 1;
            break;

        case wgpu::TextureDimension::e2D:
            info->imageType = VK_IMAGE_TYPE_2D;
            info->extent = {size.width, size.height, 1};
            info->arrayLayers = size.depthOrArrayLayers;
            break;

        case wgpu::TextureDimension::e3D:
            info->imageType = VK_IMAGE_TYPE_3D;
            info->extent = {size.width, size.height, size.depthOrArrayLayers};
            info->arrayLayers = 1;
            break;
    }
}

Aspect ComputeCombinedAspect(Device* device, const Format& format) {
    // In early Vulkan versions it is not possible to transition depth and stencil separetely so
    // textures with Depth|Stencil will be promoted to a single CombinedDepthStencil aspect
    // internally.
    if (format.aspects == (Aspect::Depth | Aspect::Stencil)) {
        return Aspect::CombinedDepthStencil;
    }
    // Same thing for Stencil8 if it is emulated with a depth-stencil format and not directly S8.
    if (format.format == wgpu::TextureFormat::Stencil8 &&
        !device->IsToggleEnabled(Toggle::VulkanUseS8)) {
        return Aspect::CombinedDepthStencil;
    }

    // Some multiplanar images cannot have planes transitioned separately and instead Vulkan
    // requires that the "Color" aspect be used for barriers, so Plane0|Plane1 is promoted to just
    // Color. The Vulkan spec requires: "If image has a single-plane color format or is not
    // disjoint, then the aspectMask member of subresourceRange must be VK_IMAGE_ASPECT_COLOR_BIT.".
    if (format.aspects == (Aspect::Plane0 | Aspect::Plane1)) {
        return Aspect::Color;
    }

    // No need to combine aspects.
    return Aspect::None;
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

        case wgpu::TextureFormat::R16Unorm:
            return VK_FORMAT_R16_UNORM;
        case wgpu::TextureFormat::R16Snorm:
            return VK_FORMAT_R16_SNORM;
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
        case wgpu::TextureFormat::RG16Unorm:
            return VK_FORMAT_R16G16_UNORM;
        case wgpu::TextureFormat::RG16Snorm:
            return VK_FORMAT_R16G16_SNORM;
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
        case wgpu::TextureFormat::RGB10A2Uint:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
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
        case wgpu::TextureFormat::RGBA16Unorm:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case wgpu::TextureFormat::RGBA16Snorm:
            return VK_FORMAT_R16G16B16A16_SNORM;
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

        case wgpu::TextureFormat::Depth16Unorm:
            return VK_FORMAT_D16_UNORM;
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
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case wgpu::TextureFormat::Stencil8:
            // Try to use the stencil8 format if possible, otherwise use whatever format we can
            // use that contains a stencil8 component.
            if (device->IsToggleEnabled(Toggle::VulkanUseS8)) {
                return VK_FORMAT_S8_UINT;
            } else {
                return VulkanImageFormat(device, wgpu::TextureFormat::Depth24PlusStencil8);
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

        case wgpu::TextureFormat::ETC2RGB8Unorm:
            return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
            return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
            return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
            return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
            return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case wgpu::TextureFormat::EACR11Unorm:
            return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case wgpu::TextureFormat::EACR11Snorm:
            return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case wgpu::TextureFormat::EACRG11Unorm:
            return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case wgpu::TextureFormat::EACRG11Snorm:
            return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

        case wgpu::TextureFormat::ASTC4x4Unorm:
            return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC5x4Unorm:
            return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC5x5Unorm:
            return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC6x5Unorm:
            return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC6x6Unorm:
            return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC8x5Unorm:
            return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC8x6Unorm:
            return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC8x8Unorm:
            return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC10x5Unorm:
            return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC10x6Unorm:
            return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC10x8Unorm:
            return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC10x10Unorm:
            return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC12x10Unorm:
            return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case wgpu::TextureFormat::ASTC12x12Unorm:
            return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;

        case wgpu::TextureFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
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
    if (usage & wgpu::TextureUsage::TextureBinding) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        // If the sampled texture is a depth/stencil texture, its image layout will be set
        // to DEPTH_STENCIL_READ_ONLY_OPTIMAL in order to support readonly depth/stencil
        // attachment. That layout requires DEPTH_STENCIL_ATTACHMENT_BIT image usage.
        if (format.HasDepthOrStencil() && format.isRenderable) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
    }
    if (usage & wgpu::TextureUsage::StorageBinding) {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (usage & wgpu::TextureUsage::RenderAttachment) {
        if (format.HasDepthOrStencil()) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    // Choosing Vulkan image usages should not know about kReadOnlyRenderAttachment because that's
    // a property of when the image is used, not of the creation.
    DAWN_ASSERT(!(usage & kReadOnlyRenderAttachment));

    return flags;
}

// Chooses which Vulkan image layout should be used for the given Dawn usage. Note that this
// layout must match the layout given to various Vulkan operations as well as the layout given
// to descriptor set writes.
VkImageLayout VulkanImageLayout(const Format& format, wgpu::TextureUsage usage) {
    if (usage == wgpu::TextureUsage::None) {
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (!wgpu::HasZeroOrOneBits(usage)) {
        // sampled | (some sort of readonly depth-stencil aspect) is the only possible multi-bit
        // usage, if more appear we will need additional special-casing.
        DAWN_ASSERT(IsSubset(
            usage, wgpu::TextureUsage::TextureBinding | kDepthReadOnlyStencilWritableAttachment |
                       kDepthWritableStencilReadOnlyAttachment | kReadOnlyRenderAttachment));

        if (IsSubset(kDepthReadOnlyStencilWritableAttachment, usage)) {
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (IsSubset(kDepthWritableStencilReadOnlyAttachment, usage)) {
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            DAWN_ASSERT(
                IsSubset(usage, kReadOnlyRenderAttachment | wgpu::TextureUsage::TextureBinding));
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
    }

    // Usage has a single bit so we can switch on its value directly.
    switch (usage) {
        case wgpu::TextureUsage::CopyDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            // The layout returned here is the one that will be used at bindgroup creation time.
        case wgpu::TextureUsage::TextureBinding:
            // The sampled image can be used as a readonly depth/stencil attachment at the same
            // time if it is a depth/stencil renderable format, so the image layout need to be
            // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL.
            if (format.HasDepthOrStencil() && format.isRenderable) {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // Vulkan texture copy functions require the image to be in _one_  known layout.
            // Depending on whether parts of the texture have been transitioned to only CopySrc
            // or a combination with something else, the texture could be in a combination of
            // GENERAL and TRANSFER_SRC_OPTIMAL. This would be a problem, so we make CopySrc use
            // GENERAL.
            // TODO(crbug.com/dawn/851): We no longer need to transition resources all at
            // once and can instead track subresources so we should lift this limitation.
        case wgpu::TextureUsage::CopySrc:
            // Read-only and write-only storage textures must use general layout because load
            // and store operations on storage images can only be done on the images in
            // VK_IMAGE_LAYOUT_GENERAL layout.
        case wgpu::TextureUsage::StorageBinding:
        case kReadOnlyStorageTexture:
        case kWriteOnlyStorageTexture:
            return VK_IMAGE_LAYOUT_GENERAL;

        case wgpu::TextureUsage::RenderAttachment:
            if (format.HasDepthOrStencil()) {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

        case kReadOnlyRenderAttachment:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        case kPresentTextureUsage:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        case wgpu::TextureUsage::TransientAttachment:
            // Will be covered by RenderAttachment above, as specification of
            // TransientAttachment requires that RenderAttachment also be
            // specified.
            DAWN_UNREACHABLE();
            break;

        case wgpu::TextureUsage::StorageAttachment:
            // TODO(dawn:1704): Support PLS on Vulkan.
            DAWN_UNREACHABLE();

        case wgpu::TextureUsage::None:
            break;
    }
    DAWN_UNREACHABLE();
}

VkImageLayout VulkanImageLayoutForDepthStencilAttachment(const Format& format,
                                                         bool depthReadOnly,
                                                         bool stencilReadOnly) {
    wgpu::TextureUsage depth = wgpu::TextureUsage::None;
    if (format.HasDepth()) {
        depth = depthReadOnly ? kReadOnlyRenderAttachment : wgpu::TextureUsage::RenderAttachment;
    }

    wgpu::TextureUsage stencil = wgpu::TextureUsage::None;
    if (format.HasStencil()) {
        stencil =
            stencilReadOnly ? kReadOnlyRenderAttachment : wgpu::TextureUsage::RenderAttachment;
    }

    return VulkanImageLayout(format, MergeDepthStencilUsage(depth, stencil));
}

VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount) {
    switch (sampleCount) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase*, const TextureDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    return {};
}

bool IsSampleCountSupported(const dawn::native::vulkan::Device* device,
                            const VkImageCreateInfo& imageCreateInfo) {
    DAWN_ASSERT(device);

    VkPhysicalDevice vkPhysicalDevice =
        ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice();
    VkImageFormatProperties properties;
    if (device->fn.GetPhysicalDeviceImageFormatProperties(
            vkPhysicalDevice, imageCreateInfo.format, imageCreateInfo.imageType,
            imageCreateInfo.tiling, imageCreateInfo.usage, imageCreateInfo.flags,
            &properties) != VK_SUCCESS) {
        DAWN_UNREACHABLE();
    }

    return properties.sampleCounts & imageCreateInfo.samples;
}

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                            const TextureDescriptor* descriptor,
                                            VkImageUsageFlags extraUsages) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor));
    DAWN_TRY(texture->InitializeAsInternalTexture(extraUsages));
    return std::move(texture);
}

// static
ResultOrError<Texture*> Texture::CreateFromExternal(
    Device* device,
    const ExternalImageDescriptorVk* descriptor,
    const TextureDescriptor* textureDescriptor,
    external_memory::Service* externalMemoryService) {
    Ref<Texture> texture = AcquireRef(new Texture(device, textureDescriptor));
    DAWN_TRY(texture->InitializeFromExternal(descriptor, externalMemoryService));
    return texture.Detach();
}

// static
Ref<Texture> Texture::CreateForSwapChain(Device* device,
                                         const TextureDescriptor* descriptor,
                                         VkImage nativeImage) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor));
    texture->InitializeForSwapChain(nativeImage);
    return texture;
}

Texture::Texture(Device* device, const TextureDescriptor* descriptor)
    : TextureBase(device, descriptor),
      mCombinedAspect(ComputeCombinedAspect(device, GetFormat())),
      // A usage of none will make sure the texture is transitioned before its first use as
      // required by the Vulkan spec.
      mSubresourceLastUsages(
          mCombinedAspect != Aspect::None ? mCombinedAspect : GetFormat().aspects,
          GetArrayLayers(),
          GetNumMipLevels(),
          wgpu::TextureUsage::None) {}

MaybeError Texture::InitializeAsInternalTexture(VkImageUsageFlags extraUsages) {
    Device* device = ToBackend(GetDevice());

    // If this triggers, it means it's time to add tests and implement support for readonly
    // depth-stencil attachments that are also used as readonly storage bindings in the pass.
    // Have fun! :)
    DAWN_ASSERT(
        !(GetFormat().HasDepthOrStencil() && (GetUsage() & wgpu::TextureUsage::StorageBinding)));

    // Create the Vulkan image "container". We don't need to check that the format supports the
    // combination of sample, usage etc. because validation should have been done in the Dawn
    // frontend already based on the minimum supported formats in the Vulkan spec
    VkImageCreateInfo createInfo = {};
    FillVulkanCreateInfoSizesAndType(*this, &createInfo);

    PNextChainBuilder createInfoChain(&createInfo);

    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.format = VulkanImageFormat(device, GetFormat().format);
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VulkanImageUsage(GetInternalUsage(), GetFormat()) | extraUsages;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageFormatListCreateInfo imageFormatListInfo = {};
    std::vector<VkFormat> viewFormats;
    bool requiresViewFormatsList = GetViewFormats().any();
    // As current SPIR-V SPEC doesn't support 'bgra8' as a valid image format, to support the
    // STORAGE usage of BGRA8Unorm we have to create an RGBA8Unorm image view on the BGRA8Unorm
    // storage texture and polyfill it as RGBA8Unorm in Tint. See http://crbug.com/dawn/1641 for
    // more details.
    if (createInfo.format == VK_FORMAT_B8G8R8A8_UNORM &&
        createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        viewFormats.push_back(VK_FORMAT_R8G8B8A8_UNORM);
        requiresViewFormatsList = true;
    }
    if (GetFormat().IsMultiPlanar() || requiresViewFormatsList) {
        // Multi-planar image needs to have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT in order to be able
        // to create per-plane view. See
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateFlagBits.html
        //
        // Note: we cannot include R8 & RG8 in the viewFormats list of
        // G8_B8R8_2PLANE_420_UNORM. The Vulkan validation layer will disallow that.
        createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    if (requiresViewFormatsList) {
        if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
            createInfoChain.Add(&imageFormatListInfo,
                                VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
            viewFormats.push_back(VulkanImageFormat(device, GetFormat().format));
            for (FormatIndex i : IterateBitSet(GetViewFormats())) {
                const Format& viewFormat = device->GetValidInternalFormat(i);
                viewFormats.push_back(VulkanImageFormat(device, viewFormat.format));
            }

            imageFormatListInfo.viewFormatCount = viewFormats.size();
            imageFormatListInfo.pViewFormats = viewFormats.data();
        }
    }

    DAWN_ASSERT(IsSampleCountSupported(device, createInfo));

    if (GetArrayLayers() >= 6 && GetBaseSize().width == GetBaseSize().height) {
        createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally beause the Vulkan images
    // that are used in vkCmdClearColorImage() must have been created with this flag, which is
    // also required for the implementation of robust resource initialization.
    createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateImage"));
    mOwnsHandle = true;

    // Create the image memory and associate it with the container
    VkMemoryRequirements requirements;
    device->fn.GetImageMemoryRequirements(device->GetVkDevice(), mHandle, &requirements);

    bool forceDisableSubAllocation =
        (device->IsToggleEnabled(
            Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment)) &&
        GetDimension() == wgpu::TextureDimension::e2D &&
        (GetInternalUsage() & (wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment));
    auto memoryKind = (GetInternalUsage() & wgpu::TextureUsage::TransientAttachment)
                          ? MemoryKind::LazilyAllocated
                          : MemoryKind::Opaque;
    DAWN_TRY_ASSIGN(mMemoryAllocation, device->GetResourceMemoryAllocator()->Allocate(
                                           requirements, memoryKind, forceDisableSubAllocation));

    DAWN_TRY(CheckVkSuccess(
        device->fn.BindImageMemory(device->GetVkDevice(), mHandle,
                                   ToBackend(mMemoryAllocation.GetResourceHeap())->GetMemory(),
                                   mMemoryAllocation.GetOffset()),
        "BindImageMemory"));

    // crbug.com/1361662
    // This works around an Intel Gen12 mesa bug due to CCS ambiguates stomping on each other.
    // https://gitlab.freedesktop.org/mesa/mesa/-/issues/7301#note_1826367
    if (device->IsToggleEnabled(Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation)) {
        auto format = GetFormat().format;
        bool textureIsBuggy =
            format == wgpu::TextureFormat::R8Unorm || format == wgpu::TextureFormat::R8Snorm ||
            format == wgpu::TextureFormat::R8Uint || format == wgpu::TextureFormat::R8Sint ||
            // These are flaky.
            format == wgpu::TextureFormat::RG16Sint || format == wgpu::TextureFormat::RGBA16Sint ||
            format == wgpu::TextureFormat::RGBA32Float;
        textureIsBuggy &= GetNumMipLevels() > 1;
        textureIsBuggy &= GetDimension() == wgpu::TextureDimension::e2D;
        textureIsBuggy &= IsPowerOfTwo(GetBaseSize().width) && IsPowerOfTwo(GetBaseSize().height);
        if (textureIsBuggy) {
            DAWN_TRY(ClearTexture(ToBackend(GetDevice())->GetPendingRecordingContext(),
                                  GetAllSubresources(), TextureBase::ClearValue::Zero));
        }
    }

    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(ClearTexture(ToBackend(GetDevice())->GetPendingRecordingContext(),
                              GetAllSubresources(), TextureBase::ClearValue::NonZero));
    }

    SetLabelImpl();

    return {};
}

// Internally managed, but imported from external handle
MaybeError Texture::InitializeFromExternal(const ExternalImageDescriptorVk* descriptor,
                                           external_memory::Service* externalMemoryService) {
    Device* device = ToBackend(GetDevice());
    VkFormat format = VulkanImageFormat(device, GetFormat().format);
    VkImageUsageFlags usage = VulkanImageUsage(GetInternalUsage(), GetFormat());

    bool supportsDisjoint;
    DAWN_INVALID_IF(
        !externalMemoryService->SupportsCreateImage(descriptor, format, usage, &supportsDisjoint),
        "Creating an image from external memory is not supported.");
    // The creation of mSubresourceLastUsage assumes that multi-planar are always disjoint and sets
    // the combined aspect without checking for disjoint support.
    // TODO(dawn:1548): Support multi-planar images with the DISJOINT feature and potentially allow
    // acting on planes individually? Always using Color is valid even for disjoint images.
    DAWN_UNUSED(supportsDisjoint);
    DAWN_ASSERT(!GetFormat().IsMultiPlanar() || mCombinedAspect == Aspect::Color);

    mExternalState = ExternalState::PendingAcquire;
    mExportQueueFamilyIndex = externalMemoryService->GetQueueFamilyIndex(descriptor->GetType());

    mPendingAcquireOldLayout = descriptor->releasedOldLayout;
    mPendingAcquireNewLayout = descriptor->releasedNewLayout;

    VkImageCreateInfo baseCreateInfo = {};
    FillVulkanCreateInfoSizesAndType(*this, &baseCreateInfo);

    PNextChainBuilder createInfoChain(&baseCreateInfo);

    baseCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    baseCreateInfo.format = format;
    baseCreateInfo.usage = usage;
    baseCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    baseCreateInfo.queueFamilyIndexCount = 0;
    baseCreateInfo.pQueueFamilyIndices = nullptr;

    // We always set VK_IMAGE_USAGE_TRANSFER_DST_BIT unconditionally beause the Vulkan images
    // that are used in vkCmdClearColorImage() must have been created with this flag, which is
    // also required for the implementation of robust resource initialization.
    baseCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageFormatListCreateInfo imageFormatListInfo = {};
    std::vector<VkFormat> viewFormats;
    if (GetViewFormats().any()) {
        baseCreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
            createInfoChain.Add(&imageFormatListInfo,
                                VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
            for (FormatIndex i : IterateBitSet(GetViewFormats())) {
                const Format& viewFormat = device->GetValidInternalFormat(i);
                viewFormats.push_back(VulkanImageFormat(device, viewFormat.format));
            }

            imageFormatListInfo.viewFormatCount = viewFormats.size();
            imageFormatListInfo.pViewFormats = viewFormats.data();
        }
    }

    DAWN_TRY_ASSIGN(mHandle, externalMemoryService->CreateImage(descriptor, baseCreateInfo));
    mOwnsHandle = true;

    SetLabelHelper("Dawn_ExternalTexture");

    return {};
}

void Texture::InitializeForSwapChain(VkImage nativeImage) {
    mHandle = nativeImage;
    SetLabelHelper("Dawn_SwapChainTexture");
}

MaybeError Texture::BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
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
    mWaitRequirements = std::move(waitSemaphores);
    return {};
}

void Texture::TransitionEagerlyForExport(CommandRecordingContext* recordingContext) {
    mExternalState = ExternalState::EagerlyTransitioned;

    // Get any usage, ideally the last one to do nothing
    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);
    SubresourceRange range = {GetDisjointVulkanAspects(), {0, 1}, {0, 1}};

    wgpu::TextureUsage usage = mSubresourceLastUsages.Get(range.aspects, 0, 0);

    std::vector<VkImageMemoryBarrier> barriers;
    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    // Same usage as last.
    TransitionUsageAndGetResourceBarrier(usage, range, &barriers, &srcStages, &dstStages);

    DAWN_ASSERT(barriers.size() == 1);
    VkImageMemoryBarrier& barrier = barriers[0];
    // The barrier must be paired with another barrier that will specify the dst access mask on the
    // importing queue.
    barrier.dstAccessMask = 0;

    if (mDesiredExportLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        barrier.newLayout = mDesiredExportLayout;
    }

    Device* device = ToBackend(GetDevice());
    barrier.srcQueueFamilyIndex = device->GetGraphicsQueueFamily();
    barrier.dstQueueFamilyIndex = mExportQueueFamilyIndex;

    // We don't know when the importing queue will need the texture, so pass
    // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT to ensure the barrier happens-before any usage in the
    // importing queue.
    dstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    device->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                  nullptr, 0, nullptr, 1, &barrier);
}

std::vector<VkSemaphore> Texture::AcquireWaitRequirements() {
    return std::move(mWaitRequirements);
}

MaybeError Texture::ExportExternalTexture(VkImageLayout desiredLayout,
                                          ExternalSemaphoreHandle* handle,
                                          VkImageLayout* releasedOldLayout,
                                          VkImageLayout* releasedNewLayout) {
    DAWN_INVALID_IF(mExternalState == ExternalState::Released,
                    "Can't export a signal semaphore from signaled texture %s.", this);

    DAWN_INVALID_IF(mExternalAllocation == VK_NULL_HANDLE,
                    "Can't export a signal semaphore from destroyed or non-external texture %s.",
                    this);

    DAWN_INVALID_IF(desiredLayout != VK_IMAGE_LAYOUT_UNDEFINED,
                    "desiredLayout (%d) was not VK_IMAGE_LAYOUT_UNDEFINED", desiredLayout);

    // Release the texture
    mExternalState = ExternalState::Released;

    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);
    wgpu::TextureUsage usage = mSubresourceLastUsages.Get(GetDisjointVulkanAspects(), 0, 0);

    // Compute the layouts for the queue transition for export. desiredLayout == UNDEFINED is a tag
    // value used to export with whatever the current layout is. However queue transitioning to the
    // UNDEFINED layout is disallowed so we handle the case where currentLayout is UNDEFINED by
    // promoting to GENERAL.
    VkImageLayout currentLayout = VulkanImageLayout(GetFormat(), usage);
    VkImageLayout targetLayout;
    if (currentLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        targetLayout = currentLayout;
    } else {
        targetLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    // We have to manually trigger a transition if the texture hasn't been actually used or if we
    // need a layout transition.
    // TODO(dawn:1509): Avoid the empty submit.
    if (mExternalSemaphoreHandle == kNullExternalSemaphoreHandle || targetLayout != currentLayout) {
        mDesiredExportLayout = targetLayout;

        Device* device = ToBackend(GetDevice());
        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        recordingContext->externalTexturesForEagerTransition.insert(this);
        DAWN_TRY(device->SubmitPendingCommands());

        currentLayout = targetLayout;
    }
    DAWN_ASSERT(mExternalSemaphoreHandle != kNullExternalSemaphoreHandle);

    // Write out the layouts and signal semaphore
    *releasedOldLayout = currentLayout;
    *releasedNewLayout = targetLayout;
    *handle = mExternalSemaphoreHandle;
    mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;

    // Destroy the texture so it can't be used again
    Destroy();
    return {};
}

Texture::~Texture() {
    if (mExternalSemaphoreHandle != kNullExternalSemaphoreHandle) {
        ToBackend(GetDevice())
            ->GetExternalSemaphoreService()
            ->CloseHandle(mExternalSemaphoreHandle);
    }
    mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;
}

void Texture::SetLabelHelper(const char* prefix) {
    SetDebugName(ToBackend(GetDevice()), mHandle, prefix, GetLabel());
}

void Texture::SetLabelImpl() {
    SetLabelHelper("Dawn_InternalTexture");
}

void Texture::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    Device* device = ToBackend(GetDevice());

    if (mOwnsHandle) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
    }

    // For textures created from a VkImage, the allocation is kInvalid so the Device knows
    // to skip the deallocation of the (absence of) VkDeviceMemory.
    device->GetResourceMemoryAllocator()->Deallocate(&mMemoryAllocation);

    if (mExternalAllocation != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mExternalAllocation);
    }

    mHandle = VK_NULL_HANDLE;
    mExternalAllocation = VK_NULL_HANDLE;

    // For Vulkan, we currently run the base destruction code after the internal changes because
    // of the dependency on the texture state which the base code overwrites too early.
    TextureBase::DestroyImpl();
}

VkImage Texture::GetHandle() const {
    return mHandle;
}

void Texture::TweakTransitionForExternalUsage(CommandRecordingContext* recordingContext,
                                              std::vector<VkImageMemoryBarrier>* barriers,
                                              size_t transitionBarrierStart) {
    DAWN_ASSERT(GetNumMipLevels() == 1 && GetArrayLayers() == 1);

    // transitionBarrierStart specify the index where barriers for current transition start in
    // the vector. barriers->size() - transitionBarrierStart is the number of barriers that we
    // have already added into the vector during current transition.
    DAWN_ASSERT(barriers->size() - transitionBarrierStart <= 1);

    if (mExternalState == ExternalState::PendingAcquire ||
        mExternalState == ExternalState::EagerlyTransitioned) {
        recordingContext->externalTexturesForEagerTransition.insert(this);
        if (barriers->size() == transitionBarrierStart) {
            barriers->push_back(BuildMemoryBarrier(
                this, wgpu::TextureUsage::None, wgpu::TextureUsage::None,
                SubresourceRange::SingleMipAndLayer(0, 0, GetDisjointVulkanAspects())));
        }

        VkImageMemoryBarrier* barrier = &(*barriers)[transitionBarrierStart];
        // Transfer texture from external queue to graphics queue
        barrier->srcQueueFamilyIndex = mExportQueueFamilyIndex;
        barrier->dstQueueFamilyIndex = ToBackend(GetDevice())->GetGraphicsQueueFamily();

        // srcAccessMask means nothing when importing. Queue transfers require a barrier on
        // both the importing and exporting queues. The exporting queue should have specified
        // this.
        barrier->srcAccessMask = 0;

        // Save the desired layout. We may need to transition through an intermediate
        // |mPendingAcquireLayout| first.
        VkImageLayout desiredLayout = barrier->newLayout;

        if (mExternalState == ExternalState::PendingAcquire) {
            bool isInitialized = IsSubresourceContentInitialized(GetAllSubresources());

            // We don't care about the pending old layout if the texture is uninitialized. The
            // driver is free to discard it. Also it is invalid to transition to layout UNDEFINED or
            // PREINITIALIZED. If the embedder provided no new layout, or we don't care about the
            // previous contents, we can skip the layout transition.
            // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-VkImageMemoryBarrier-newLayout-01198
            if (!isInitialized || mPendingAcquireNewLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
                mPendingAcquireNewLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                barrier->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier->newLayout = desiredLayout;
            } else {
                barrier->oldLayout = mPendingAcquireOldLayout;
                barrier->newLayout = mPendingAcquireNewLayout;
            }
        } else {
            // In case of ExternalState::EagerlyTransitioned, the layouts of the texture's queue
            // release were always same. So we exactly match that here for the queue acquire.
            // The spec text:
            // If the transfer is via an image memory barrier, and an image layout transition is
            // desired, then the values of oldLayout and newLayout in the release operation's memory
            // barrier must be equal to values of oldLayout and newLayout in the acquire operation's
            // memory barrier.
            barrier->newLayout = barrier->oldLayout;
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

            layoutBarrier.srcAccessMask = 0;
            layoutBarrier.dstAccessMask = barrier->dstAccessMask;
            layoutBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barriers->push_back(layoutBarrier);
        }

        mExternalState = ExternalState::Acquired;
    }

    mLastExternalState = mExternalState;
}

bool Texture::CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage, wgpu::TextureUsage usage) {
    // Reuse the texture directly and avoid encoding barriers when it isn't needed.
    bool lastReadOnly = IsSubset(lastUsage, kReadOnlyTextureUsages);
    if (lastReadOnly && lastUsage == usage && mLastExternalState == mExternalState) {
        return true;
    }
    return false;
}

void Texture::TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                     const TextureSubresourceUsage& textureUsages,
                                     std::vector<VkImageMemoryBarrier>* imageBarriers,
                                     VkPipelineStageFlags* srcStages,
                                     VkPipelineStageFlags* dstStages) {
    if (!UseCombinedAspects()) {
        TransitionUsageForPassImpl(recordingContext, textureUsages, imageBarriers, srcStages,
                                   dstStages);
        return;
    }

    // We need to combine aspects for the transition, use a new subresource storage that will
    // contain the combined usages for the aspects.
    SubresourceStorage<wgpu::TextureUsage> combinedUsages(mCombinedAspect, GetArrayLayers(),
                                                          GetNumMipLevels());

    if (mCombinedAspect == Aspect::CombinedDepthStencil) {
        // For depth-stencil we can't just combine the aspect with an | operation because there
        // needs to be special handling for readonly aspects. Instead figure out which aspect is
        // currently being added (and which one is already present) and call the custom merging
        // function for depth-stencil.
        textureUsages.Iterate([&](const SubresourceRange& range, wgpu::TextureUsage usage) {
            SubresourceRange updateRange = range;
            updateRange.aspects = mCombinedAspect;
            Aspect aspectsToMerge = range.aspects;

            combinedUsages.Update(
                updateRange, [&](const SubresourceRange&, wgpu::TextureUsage* combinedUsage) {
                    if (aspectsToMerge == Aspect::Depth) {
                        *combinedUsage = MergeDepthStencilUsage(usage, *combinedUsage);
                    } else if (aspectsToMerge == Aspect::Stencil) {
                        *combinedUsage = MergeDepthStencilUsage(*combinedUsage, usage);
                    } else {
                        DAWN_ASSERT(aspectsToMerge == (Aspect::Depth | Aspect::Stencil));
                        *combinedUsage = usage;
                    }
                });
        });
    } else {
        // Combine aspect's usages with the | operation.
        textureUsages.Iterate([&](const SubresourceRange& range, wgpu::TextureUsage usage) {
            SubresourceRange updateRange = range;
            updateRange.aspects = mCombinedAspect;

            combinedUsages.Update(updateRange,
                                  [&](const SubresourceRange&, wgpu::TextureUsage* combinedUsage) {
                                      *combinedUsage |= usage;
                                  });
        });
    }

    TransitionUsageForPassImpl(recordingContext, combinedUsages, imageBarriers, srcStages,
                               dstStages);
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

    mSubresourceLastUsages.Merge(subresourceUsages, [&](const SubresourceRange& range,
                                                        wgpu::TextureUsage* lastUsage,
                                                        const wgpu::TextureUsage& newUsage) {
        if (newUsage == wgpu::TextureUsage::None || CanReuseWithoutBarrier(*lastUsage, newUsage)) {
            return;
        }

        imageBarriers->push_back(BuildMemoryBarrier(this, *lastUsage, newUsage, range));

        allLastUsages |= *lastUsage;
        allUsages |= newUsage;

        *lastUsage = newUsage;
    });

    if (mExternalState != ExternalState::InternalOnly) {
        TweakTransitionForExternalUsage(recordingContext, imageBarriers, transitionBarrierStart);
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
        DAWN_ASSERT(srcStages != 0 && dstStages != 0);
        ToBackend(GetDevice())
            ->fn.CmdPipelineBarrier(recordingContext->commandBuffer, srcStages, dstStages, 0, 0,
                                    nullptr, 0, nullptr, barriers.size(), barriers.data());
    }
}

void Texture::TransitionUsageAndGetResourceBarrier(wgpu::TextureUsage usage,
                                                   const SubresourceRange& range,
                                                   std::vector<VkImageMemoryBarrier>* imageBarriers,
                                                   VkPipelineStageFlags* srcStages,
                                                   VkPipelineStageFlags* dstStages) {
    if (UseCombinedAspects()) {
        SubresourceRange updatedRange = range;
        updatedRange.aspects = mCombinedAspect;
        TransitionUsageAndGetResourceBarrierImpl(usage, updatedRange, imageBarriers, srcStages,
                                                 dstStages);
    } else {
        TransitionUsageAndGetResourceBarrierImpl(usage, range, imageBarriers, srcStages, dstStages);
    }
}

void Texture::TransitionUsageAndGetResourceBarrierImpl(
    wgpu::TextureUsage usage,
    const SubresourceRange& range,
    std::vector<VkImageMemoryBarrier>* imageBarriers,
    VkPipelineStageFlags* srcStages,
    VkPipelineStageFlags* dstStages) {
    DAWN_ASSERT(imageBarriers != nullptr);
    const Format& format = GetFormat();

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

    if (GetFormat().isCompressed || GetFormat().IsMultiPlanar()) {
        if (range.aspects == Aspect::None) {
            return {};
        }
        // need to clear the texture with a copy from buffer
        DAWN_ASSERT(range.aspects == Aspect::Color || range.aspects == Aspect::Plane0 ||
                    range.aspects == Aspect::Plane1);
        const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(range.aspects).block;

        Extent3D largestMipSize =
            GetMipLevelSingleSubresourcePhysicalSize(range.baseMipLevel, range.aspects);

        uint32_t bytesPerRow = Align((largestMipSize.width / blockInfo.width) * blockInfo.byteSize,
                                     device->GetOptimalBytesPerRowAlignment());
        uint64_t bufferSize = bytesPerRow * (largestMipSize.height / blockInfo.height) *
                              largestMipSize.depthOrArrayLayers;
        DynamicUploader* uploader = device->GetDynamicUploader();
        UploadHandle uploadHandle;
        DAWN_TRY_ASSIGN(
            uploadHandle,
            uploader->Allocate(bufferSize, device->GetPendingCommandSerial(), blockInfo.byteSize));
        memset(uploadHandle.mappedBuffer, uClearColor, bufferSize);

        std::vector<VkBufferImageCopy> regions;
        for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
             ++level) {
            Extent3D copySize = GetMipLevelSingleSubresourcePhysicalSize(level, range.aspects);
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
                dataLayout.rowsPerImage = copySize.height / blockInfo.height;
                dataLayout.bytesPerRow = bytesPerRow;
                TextureCopy textureCopy;
                textureCopy.aspect = range.aspects;
                textureCopy.mipLevel = level;
                textureCopy.origin = {0, 0, layer};
                textureCopy.texture = this;

                regions.push_back(ComputeBufferImageCopyRegion(dataLayout, textureCopy, copySize));
            }
        }
        device->fn.CmdCopyBufferToImage(
            recordingContext->commandBuffer, ToBackend(uploadHandle.stagingBuffer)->GetHandle(),
            GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
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

                if (aspects & (Aspect::Depth | Aspect::Stencil | Aspect::CombinedDepthStencil)) {
                    VkClearDepthStencilValue clearDepthStencilValue[1];
                    clearDepthStencilValue[0].depth = fClearColor;
                    clearDepthStencilValue[0].stencil = uClearColor;
                    device->fn.CmdClearDepthStencilImage(recordingContext->commandBuffer,
                                                         GetHandle(),
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         clearDepthStencilValue, 1, &imageRange);
                } else {
                    DAWN_ASSERT(aspects == Aspect::Color);
                    VkClearColorValue clearColorValue;
                    switch (GetFormat().GetAspectInfo(Aspect::Color).baseType) {
                        case TextureComponentType::Float:
                            clearColorValue.float32[0] = fClearColor;
                            clearColorValue.float32[1] = fClearColor;
                            clearColorValue.float32[2] = fClearColor;
                            clearColorValue.float32[3] = fClearColor;
                            break;
                        case TextureComponentType::Sint:
                            clearColorValue.int32[0] = sClearColor;
                            clearColorValue.int32[1] = sClearColor;
                            clearColorValue.int32[2] = sClearColor;
                            clearColorValue.int32[3] = sClearColor;
                            break;
                        case TextureComponentType::Uint:
                            clearColorValue.uint32[0] = uClearColor;
                            clearColorValue.uint32[1] = uClearColor;
                            clearColorValue.uint32[2] = uClearColor;
                            clearColorValue.uint32[3] = uClearColor;
                            break;
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

MaybeError Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                        const SubresourceRange& range) {
    if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        return {};
    }
    if (!IsSubresourceContentInitialized(range)) {
        // If subresource has not been initialized, clear it to black as it could contain dirty
        // bits from recycled memory
        DAWN_TRY(ClearTexture(recordingContext, range, TextureBase::ClearValue::Zero));
    }
    return {};
}

void Texture::UpdateExternalSemaphoreHandle(ExternalSemaphoreHandle handle) {
    if (mExternalSemaphoreHandle != kNullExternalSemaphoreHandle) {
        ToBackend(GetDevice())
            ->GetExternalSemaphoreService()
            ->CloseHandle(mExternalSemaphoreHandle);
    }
    mExternalSemaphoreHandle = handle;
}

VkImageLayout Texture::GetCurrentLayoutForSwapChain() const {
    DAWN_ASSERT(GetFormat().aspects == Aspect::Color);
    return VulkanImageLayout(GetFormat(), mSubresourceLastUsages.Get(Aspect::Color, 0, 0));
}

bool Texture::UseCombinedAspects() const {
    return mCombinedAspect != Aspect::None;
}

Aspect Texture::GetDisjointVulkanAspects() const {
    if (UseCombinedAspects()) {
        return mCombinedAspect;
    }
    return GetFormat().aspects;
}

// static
ResultOrError<Ref<TextureView>> TextureView::Create(TextureBase* texture,
                                                    const TextureViewDescriptor* descriptor) {
    Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
    DAWN_TRY(view->Initialize(descriptor));
    return view;
}

MaybeError TextureView::Initialize(const TextureViewDescriptor* descriptor) {
    if ((GetTexture()->GetInternalUsage() &
         ~(wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst)) == 0) {
        // If the texture view has no other usage than CopySrc and CopyDst, then it can't
        // actually be used as a render pass attachment or sampled/storage texture. The Vulkan
        // validation errors warn if you create such a vkImageView, so return early.
        return {};
    }

    // Texture could be destroyed by the time we make a view.
    if (GetTexture()->IsDestroyed()) {
        return {};
    }

    Device* device = ToBackend(GetTexture()->GetDevice());

    VkImageViewCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.image = ToBackend(GetTexture())->GetHandle();
    createInfo.viewType = VulkanImageViewType(descriptor->dimension);

    const Format& textureFormat = GetTexture()->GetFormat();
    if (textureFormat.HasStencil() &&
        (textureFormat.HasDepth() || !device->IsToggleEnabled(Toggle::VulkanUseS8))) {
        // Unlike multi-planar formats, depth-stencil formats have multiple aspects but are not
        // created with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.
        // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkImageViewCreateInfo.html#VUID-VkImageViewCreateInfo-image-01762
        // Without, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, the view format must match the texture
        // format.
        createInfo.format = VulkanImageFormat(device, textureFormat.format);
    } else {
        createInfo.format = VulkanImageFormat(device, descriptor->format);
    }

    createInfo.components = VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                                               VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    const SubresourceRange& subresources = GetSubresourceRange();
    createInfo.subresourceRange.baseMipLevel = subresources.baseMipLevel;
    createInfo.subresourceRange.levelCount = subresources.levelCount;
    createInfo.subresourceRange.baseArrayLayer = subresources.baseArrayLayer;
    createInfo.subresourceRange.layerCount = subresources.layerCount;
    createInfo.subresourceRange.aspectMask = VulkanAspectMask(subresources.aspects);

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateImageView(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateImageView"));

    // We should create an image view with format RGBA8Unorm on the BGRA8Unorm texture when the
    // texture is used as storage texture. See http://crbug.com/dawn/1641 for more details.
    if (createInfo.format == VK_FORMAT_B8G8R8A8_UNORM &&
        (GetTexture()->GetInternalUsage() & wgpu::TextureUsage::StorageBinding)) {
        createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        DAWN_TRY(CheckVkSuccess(device->fn.CreateImageView(device->GetVkDevice(), &createInfo,
                                                           nullptr, &*mHandleForBGRA8UnormStorage),
                                "CreateImageView for BGRA8Unorm storage"));
    }

    SetLabelImpl();

    return {};
}

TextureView::~TextureView() {}

void TextureView::DestroyImpl() {
    Device* device = ToBackend(GetTexture()->GetDevice());

    if (mHandle != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }

    if (mHandleForBGRA8UnormStorage != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandleForBGRA8UnormStorage);
        mHandleForBGRA8UnormStorage = VK_NULL_HANDLE;
    }
}

VkImageView TextureView::GetHandle() const {
    return mHandle;
}

VkImageView TextureView::GetHandleForBGRA8UnormStorage() const {
    return mHandleForBGRA8UnormStorage;
}

void TextureView::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_TextureView", GetLabel());
}

}  // namespace dawn::native::vulkan
