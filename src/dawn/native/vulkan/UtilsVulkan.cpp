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

#include "dawn/native/vulkan/UtilsVulkan.h"

#include "dawn/common/Assert.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Format.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/Forward.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

constexpr char kDeviceDebugPrefix[] = "DawnDbg=";
constexpr char kDeviceDebugSeparator[] = ";";

#define VK_OBJECT_TYPE_GETTER(object, objectType)         \
    template <>                                           \
    VkObjectType GetVkObjectType<object>(object handle) { \
        return objectType;                                \
    }

VK_OBJECT_TYPE_GETTER(VkBuffer, VK_OBJECT_TYPE_BUFFER)
VK_OBJECT_TYPE_GETTER(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_OBJECT_TYPE_GETTER(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_OBJECT_TYPE_GETTER(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_OBJECT_TYPE_GETTER(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_OBJECT_TYPE_GETTER(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
VK_OBJECT_TYPE_GETTER(VkSampler, VK_OBJECT_TYPE_SAMPLER)
VK_OBJECT_TYPE_GETTER(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
VK_OBJECT_TYPE_GETTER(VkImage, VK_OBJECT_TYPE_IMAGE)
VK_OBJECT_TYPE_GETTER(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)

#undef VK_OBJECT_TYPE_GETTER

VkCompareOp ToVulkanCompareOp(wgpu::CompareFunction op) {
    switch (op) {
        case wgpu::CompareFunction::Never:
            return VK_COMPARE_OP_NEVER;
        case wgpu::CompareFunction::Less:
            return VK_COMPARE_OP_LESS;
        case wgpu::CompareFunction::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case wgpu::CompareFunction::Greater:
            return VK_COMPARE_OP_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case wgpu::CompareFunction::Equal:
            return VK_COMPARE_OP_EQUAL;
        case wgpu::CompareFunction::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case wgpu::CompareFunction::Always:
            return VK_COMPARE_OP_ALWAYS;

        case wgpu::CompareFunction::Undefined:
            break;
    }
    UNREACHABLE();
}

// Convert Dawn texture aspects to  Vulkan texture aspect flags
VkImageAspectFlags VulkanAspectMask(const Aspect& aspects) {
    VkImageAspectFlags flags = 0;
    for (Aspect aspect : IterateEnumMask(aspects)) {
        switch (aspect) {
            case Aspect::Color:
                flags |= VK_IMAGE_ASPECT_COLOR_BIT;
                break;
            case Aspect::Depth:
                flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            case Aspect::Stencil:
                flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
                break;

            case Aspect::CombinedDepthStencil:
                flags |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;

            case Aspect::Plane0:
                flags |= VK_IMAGE_ASPECT_PLANE_0_BIT;
                break;
            case Aspect::Plane1:
                flags |= VK_IMAGE_ASPECT_PLANE_1_BIT;
                break;

            case Aspect::None:
                UNREACHABLE();
        }
    }
    return flags;
}

// Vulkan SPEC requires the source/destination region specified by each element of
// pRegions must be a region that is contained within srcImage/dstImage. Here the size of
// the image refers to the virtual size, while Dawn validates texture copy extent with the
// physical size, so we need to re-calculate the texture copy extent to ensure it should fit
// in the virtual size of the subresource.
Extent3D ComputeTextureCopyExtent(const TextureCopy& textureCopy, const Extent3D& copySize) {
    Extent3D validTextureCopyExtent = copySize;
    const TextureBase* texture = textureCopy.texture.Get();
    Extent3D virtualSizeAtLevel =
        texture->GetMipLevelSingleSubresourceVirtualSize(textureCopy.mipLevel);
    ASSERT(textureCopy.origin.x <= virtualSizeAtLevel.width);
    ASSERT(textureCopy.origin.y <= virtualSizeAtLevel.height);
    if (copySize.width > virtualSizeAtLevel.width - textureCopy.origin.x) {
        ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.width = virtualSizeAtLevel.width - textureCopy.origin.x;
    }
    if (copySize.height > virtualSizeAtLevel.height - textureCopy.origin.y) {
        ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.height = virtualSizeAtLevel.height - textureCopy.origin.y;
    }

    return validTextureCopyExtent;
}

VkBufferImageCopy ComputeBufferImageCopyRegion(const BufferCopy& bufferCopy,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize) {
    TextureDataLayout passDataLayout;
    passDataLayout.offset = bufferCopy.offset;
    passDataLayout.rowsPerImage = bufferCopy.rowsPerImage;
    passDataLayout.bytesPerRow = bufferCopy.bytesPerRow;
    return ComputeBufferImageCopyRegion(passDataLayout, textureCopy, copySize);
}

VkBufferImageCopy ComputeBufferImageCopyRegion(const TextureDataLayout& dataLayout,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize) {
    const Texture* texture = ToBackend(textureCopy.texture.Get());

    VkBufferImageCopy region;

    region.bufferOffset = dataLayout.offset;
    // In Vulkan the row length is in texels while it is in bytes for Dawn
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;
    ASSERT(dataLayout.bytesPerRow % blockInfo.byteSize == 0);
    region.bufferRowLength = dataLayout.bytesPerRow / blockInfo.byteSize * blockInfo.width;
    region.bufferImageHeight = dataLayout.rowsPerImage * blockInfo.height;

    region.imageSubresource.aspectMask = VulkanAspectMask(textureCopy.aspect);
    region.imageSubresource.mipLevel = textureCopy.mipLevel;

    switch (textureCopy.texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            ASSERT(textureCopy.origin.z == 0 && copySize.depthOrArrayLayers == 1);
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = 0;
            region.imageOffset.z = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            ASSERT(!textureCopy.texture->GetFormat().isCompressed);
            region.imageExtent.width = copySize.width;
            region.imageExtent.height = 1;
            region.imageExtent.depth = 1;
            break;

        case wgpu::TextureDimension::e2D: {
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = textureCopy.origin.y;
            region.imageOffset.z = 0;
            region.imageSubresource.baseArrayLayer = textureCopy.origin.z;
            region.imageSubresource.layerCount = copySize.depthOrArrayLayers;

            Extent3D imageExtent = ComputeTextureCopyExtent(textureCopy, copySize);
            region.imageExtent.width = imageExtent.width;
            region.imageExtent.height = imageExtent.height;
            region.imageExtent.depth = 1;
            break;
        }

        case wgpu::TextureDimension::e3D: {
            region.imageOffset.x = textureCopy.origin.x;
            region.imageOffset.y = textureCopy.origin.y;
            region.imageOffset.z = textureCopy.origin.z;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            ASSERT(!textureCopy.texture->GetFormat().isCompressed);
            region.imageExtent.width = copySize.width;
            region.imageExtent.height = copySize.height;
            region.imageExtent.depth = copySize.depthOrArrayLayers;
            break;
        }
    }

    return region;
}

void SetDebugNameInternal(Device* device,
                          VkObjectType objectType,
                          uint64_t objectHandle,
                          const char* prefix,
                          std::string label) {
    if (!objectHandle) {
        return;
    }

    if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
        VkDebugUtilsObjectNameInfoEXT objectNameInfo;
        objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        objectNameInfo.pNext = nullptr;
        objectNameInfo.objectType = objectType;
        objectNameInfo.objectHandle = objectHandle;

        std::ostringstream objectNameStream;
        // Prefix with the device's message ID so that if this label appears in a validation
        // message it can be parsed out and the message can be associated with the right device.
        objectNameStream << device->GetDebugPrefix() << kDeviceDebugSeparator << prefix;
        if (!label.empty() && device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
            objectNameStream << "_" << label;
        }
        std::string objectName = objectNameStream.str();
        objectNameInfo.pObjectName = objectName.c_str();
        device->fn.SetDebugUtilsObjectNameEXT(device->GetVkDevice(), &objectNameInfo);
    }
}

std::string GetNextDeviceDebugPrefix() {
    static uint64_t nextDeviceDebugId = 1;
    std::ostringstream objectName;
    objectName << kDeviceDebugPrefix << nextDeviceDebugId++;
    return objectName.str();
}

std::string GetDeviceDebugPrefixFromDebugName(const char* debugName) {
    if (debugName == nullptr) {
        return {};
    }

    if (strncmp(debugName, kDeviceDebugPrefix, sizeof(kDeviceDebugPrefix) - 1) != 0) {
        return {};
    }

    const char* separator = strstr(debugName + sizeof(kDeviceDebugPrefix), kDeviceDebugSeparator);
    if (separator == nullptr) {
        return {};
    }

    size_t length = separator - debugName;
    return std::string(debugName, length);
}

}  // namespace dawn::native::vulkan
