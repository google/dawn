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

#include "src/dawn/native/vulkan/SharedTextureMemoryVk.h"

#include <algorithm>
#include <array>
#include <bit>
#include <utility>

#include "dawn/native/wgpu_structs_autogen.h"
#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/SystemHandle.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/vulkan/DeviceVk.h"
#include "src/dawn/native/vulkan/PhysicalDeviceVk.h"
#include "src/dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "src/dawn/native/vulkan/SharedFenceVk.h"
#include "src/dawn/native/vulkan/TextureVk.h"
#include "src/dawn/native/vulkan/UtilsVulkan.h"
#include "src/dawn/native/vulkan/VulkanError.h"
#include "src/utils/compiler.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/hardware_buffer.h>

#include "src/dawn/native/AHBFunctions.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn::native::vulkan {

namespace {

// Encoding from <drm/drm_fourcc.h>
constexpr uint32_t DrmFourccCode(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return a | (b << 8) | (c << 16) | (d << 24);
}

constexpr uint32_t DrmFourccCode(char a, char b, char c, char d) {
    return DrmFourccCode(static_cast<uint32_t>(a), static_cast<uint32_t>(b),
                         static_cast<uint32_t>(c), static_cast<uint32_t>(d));
}

constexpr auto kDrmFormatR8 = DrmFourccCode('R', '8', ' ', ' '); /* [7:0] R */
constexpr auto kDrmFormatGR88 =
    DrmFourccCode('G', 'R', '8', '8'); /* [15:0] G:R 8:8 little endian */
constexpr auto kDrmFormatXRGB8888 =
    DrmFourccCode('X', 'R', '2', '4'); /* [15:0] x:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatXBGR8888 =
    DrmFourccCode('X', 'B', '2', '4'); /* [15:0] x:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatARGB8888 =
    DrmFourccCode('A', 'R', '2', '4'); /* [31:0] A:R:G:B 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR8888 =
    DrmFourccCode('A', 'B', '2', '4'); /* [31:0] A:B:G:R 8:8:8:8 little endian */
constexpr auto kDrmFormatABGR2101010 =
    DrmFourccCode('A', 'B', '3', '0'); /* [31:0] A:B:G:R 2:10:10:10 little endian */
constexpr auto kDrmFormatABGR16161616F =
    DrmFourccCode('A', 'B', '4', 'H'); /* [63:0] A:B:G:R 16:16:16:16 little endian */
constexpr auto kDrmFormatNV12 = DrmFourccCode('N', 'V', '1', '2'); /* 2x2 subsampled Cr:Cb plane */

[[maybe_unused]] ResultOrError<wgpu::TextureFormat> FormatFromDrmFormat(uint32_t drmFormat) {
    switch (drmFormat) {
        case kDrmFormatR8:
            return wgpu::TextureFormat::R8Unorm;
        case kDrmFormatGR88:
            return wgpu::TextureFormat::RG8Unorm;
        case kDrmFormatXRGB8888:
        case kDrmFormatARGB8888:
            return wgpu::TextureFormat::BGRA8Unorm;
        case kDrmFormatXBGR8888:
        case kDrmFormatABGR8888:
            return wgpu::TextureFormat::RGBA8Unorm;
        case kDrmFormatABGR2101010:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case kDrmFormatABGR16161616F:
            return wgpu::TextureFormat::RGBA16Float;
        case kDrmFormatNV12:
            return wgpu::TextureFormat::R8BG8Biplanar420Unorm;
        default:
            return DAWN_VALIDATION_ERROR("Unsupported drm format %x.", drmFormat);
    }
}

[[maybe_unused]] ResultOrError<uint32_t> FindImportMemoryType(
    Device* device,
    const VkMemoryRequirements& requirements,
    bool allowHostCached) {
    auto& allocator = device->GetResourceMemoryAllocator();
    auto index = allocator->FindBestTypeIndex(requirements, MemoryKind::DeviceLocal);
    if (!index.has_value() && allowHostCached) {
        index = allocator->FindBestTypeIndex(requirements, MemoryKind::HostCached);
    }
    DAWN_INVALID_IF(!index.has_value(), "Unable to find an appropriate memory type for import.");
    return index.value();
}

// Construct the common image parameters after the STM properties have been reified.
[[maybe_unused]] VkImageCreateInfo MakeImageCreateInfo(
    Device* device,
    const SharedTextureMemoryProperties& properties,
    const Format& format,
    VkFormat vkFormat,
    VkImageTiling tiling) {
    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.flags = VulkanImageCreateFlags(device, properties.usage, format, /*sampleCount=*/1);
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = vkFormat;
    createInfo.extent = {properties.size.width, properties.size.height, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = properties.size.depthOrArrayLayers;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = tiling;
    createInfo.usage = VulkanImageUsage(device, properties.usage, format);
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return createInfo;
}

struct ViewFormatRequirements {
    bool needsMutable;
    bool needsBGRA8UnormStoragePolyfill;
};

[[maybe_unused]] ViewFormatRequirements GetViewFormatRequirements(
    Device* device,
    const SharedTextureMemoryProperties& properties,
    const Format& format) {
    constexpr wgpu::TextureUsage kUsageRequiringView = wgpu::TextureUsage::RenderAttachment |
                                                       wgpu::TextureUsage::TextureBinding |
                                                       wgpu::TextureUsage::StorageBinding;
    const bool needsBGRA8UnormStoragePolyfill =
        properties.format == wgpu::TextureFormat::BGRA8Unorm &&
        (properties.usage & wgpu::TextureUsage::StorageBinding) != 0;
    const bool viewMayReinterpretFormat = (properties.usage & kUsageRequiringView) != 0 &&
                                          !device->GetCompatibleViewFormats(format).empty();
    // Creating per-plane views requires mutable format.
    const bool isMultiplanar = format.IsMultiPlanar();
    // DRM modifier tiling alone does not require mutable format.
    // When mutable format is needed with DRM modifier tiling, a non-empty
    // image format list must be provided by the caller.
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageCreateInfo.html#VUID-VkImageCreateInfo-tiling-02353
    return {
        .needsMutable = needsBGRA8UnormStoragePolyfill || viewMayReinterpretFormat || isMultiplanar,
        .needsBGRA8UnormStoragePolyfill = needsBGRA8UnormStoragePolyfill,
    };
}

// Append chains to local copies. With no additional chains, preserve the caller's pNext
// chain unchanged (as required for opaque FD imports).
template <typename... AdditionalChains>
ResultOrError<Ref<RefCountedVkHandle<VkImage>>>
CreateVkImage(Device* device, VkImageCreateInfo createInfo, AdditionalChains... additionalChains) {
    if constexpr (sizeof...(AdditionalChains) > 0) {
        DAWN_ASSERT(createInfo.pNext == nullptr);
        PNextChainBuilder createInfoChain(&createInfo);
        (createInfoChain.Add(&additionalChains), ...);
    }
    VkImage vkImage;
    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.CreateImage(device->GetVkDevice(), &createInfo, nullptr, &*vkImage),
        "vkCreateImage"));
    return AcquireRef(new RefCountedVkHandle<VkImage>(device, vkImage));
}

template <typename Chain>
void AddImageFormatQueryChain(PNextChainBuilder& builder, Chain& chain) {
    builder.Add(&chain);
}

// An empty list does not restrict view formats and need not be included in the query.
[[maybe_unused]] void AddImageFormatQueryChain(PNextChainBuilder& builder,
                                               VkImageFormatListCreateInfo& chain) {
    if (chain.viewFormatCount > 0) {
        builder.Add(&chain);
    }
}

// Query import support and validate the image-format limits available before creation.
// Query-only chains are kept local; the caller's creation chain is never modified.
template <typename... AdditionalChains>
MaybeError CheckExternalImageFormatSupport(Device* device,
                                           const SharedTextureMemoryProperties& properties,
                                           const VkImageCreateInfo& createInfo,
                                           AdditionalChains... additionalChains) {
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.format = createInfo.format;
    imageFormatInfo.type = createInfo.imageType;
    imageFormatInfo.tiling = createInfo.tiling;
    imageFormatInfo.usage = createInfo.usage;
    imageFormatInfo.flags = createInfo.flags;

    PNextChainBuilder imageFormatInfoChain(&imageFormatInfo);
    (AddImageFormatQueryChain(imageFormatInfoChain, additionalChains), ...);

    VkImageFormatProperties2 imageFormatProps = {};
    imageFormatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

    VkExternalImageFormatProperties externalImageFormatProps = {};

    PNextChainBuilder imageFormatPropsChain(&imageFormatProps);
    imageFormatPropsChain.Add(&externalImageFormatProps,
                              VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES);

    DAWN_TRY_CONTEXT(
        CheckVkSuccess(device->fn.GetPhysicalDeviceImageFormatProperties2(
                           ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice(),
                           &imageFormatInfo, &imageFormatProps),
                       "vkGetPhysicalDeviceImageFormatProperties2"),
        "checking external image import support with %s %s", properties.format, properties.usage);

    VkExternalMemoryFeatureFlags featureFlags =
        externalImageFormatProps.externalMemoryProperties.externalMemoryFeatures;
    DAWN_INVALID_IF(!(featureFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT),
                    "Vulkan memory is not importable.");

    const VkImageFormatProperties& limits = imageFormatProps.imageFormatProperties;
    DAWN_INVALID_IF(createInfo.extent.width == 0 || createInfo.extent.height == 0 ||
                        createInfo.extent.depth == 0 ||
                        createInfo.extent.width > limits.maxExtent.width ||
                        createInfo.extent.height > limits.maxExtent.height ||
                        createInfo.extent.depth > limits.maxExtent.depth,
                    "Image extent (%u, %u, %u) contains zero or exceeds maxExtent (%u, %u, %u).",
                    createInfo.extent.width, createInfo.extent.height, createInfo.extent.depth,
                    limits.maxExtent.width, limits.maxExtent.height, limits.maxExtent.depth);
    DAWN_INVALID_IF(createInfo.mipLevels == 0 || createInfo.mipLevels > limits.maxMipLevels,
                    "Image mip level count (%u) is zero or exceeds maxMipLevels (%u).",
                    createInfo.mipLevels, limits.maxMipLevels);
    DAWN_INVALID_IF(createInfo.arrayLayers == 0 || createInfo.arrayLayers > limits.maxArrayLayers,
                    "Image array layer count (%u) is zero or exceeds maxArrayLayers (%u).",
                    createInfo.arrayLayers, limits.maxArrayLayers);
    DAWN_INVALID_IF(!std::has_single_bit(static_cast<uint32_t>(createInfo.samples)) ||
                        (createInfo.samples & limits.sampleCounts) == 0,
                    "Image sample count (%u) must be a single bit supported by sampleCounts (%u).",
                    createInfo.samples, limits.sampleCounts);

    // The image size cannot be queried before creation to compare against maxResourceSize.
    // Vulkan requires vkCreateImage to fail with VK_ERROR_OUT_OF_DEVICE_MEMORY if that
    // limit is exceeded. Rely on that guarantee and propagate OOM in CreateVkImage.
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageFormatProperties.html
    return {};
}

// Import memory with an optional dedicated image. Copy the import structure so no
// caller-owned pNext pointers are modified.
template <typename ImportInfo>
ResultOrError<Ref<RefCountedVkHandle<VkDeviceMemory>>> AllocateDeviceMemory(
    Device* device,
    VkDeviceSize allocationSize,
    VkImage dedicatedImage,
    uint32_t memoryTypeIndex,
    ImportInfo importInfo) {
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = allocationSize;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkMemoryDedicatedAllocateInfo dedicatedInfo = {};
    dedicatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedInfo.image = dedicatedImage;

    PNextChainBuilder allocateInfoChain(&allocateInfo);
    if (dedicatedImage != VkImage{}) {
        allocateInfoChain.Add(&dedicatedInfo);
    }
    allocateInfoChain.Add(&importInfo);

    VkDeviceMemory vkDeviceMemory;
    DAWN_TRY(CheckVkOOMThenSuccess(
        device->fn.AllocateMemory(device->GetVkDevice(), &allocateInfo, nullptr, &*vkDeviceMemory),
        "vkAllocateMemory"));
    return AcquireRef(new RefCountedVkHandle<VkDeviceMemory>(device, vkDeviceMemory));
}

#if DAWN_PLATFORM_IS(POSIX)
ResultOrError<Ref<RefCountedVkHandle<VkDeviceMemory>>> ImportMemoryFD(
    Device* device,
    int fd,
    VkExternalMemoryHandleTypeFlagBits handleType,
    VkDeviceSize allocationSize,
    uint32_t memoryTypeIndex,
    VkImage dedicatedImage = {}) {
    SystemHandle memoryFD = SystemHandle::Duplicate(fd);

    VkImportMemoryFdInfoKHR importInfo = {};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
    importInfo.handleType = handleType;
    importInfo.fd = memoryFD.Get();

    Ref<RefCountedVkHandle<VkDeviceMemory>> memory;
    DAWN_TRY_ASSIGN(memory, AllocateDeviceMemory(device, allocationSize, dedicatedImage,
                                                 memoryTypeIndex, importInfo));
    memoryFD.Detach();  // A successful import transfers FD ownership to the Vulkan implementation.
    return memory;
}
#endif  // DAWN_PLATFORM_IS(POSIX)

}  // namespace

// TODO(crbug.com/536831387): Separate the common import sequence from per-handle logic:
// validate the descriptor and derive properties, initialize STM, describe/validate view formats,
// query support, create the image, gather memory requirements, import and bind memory, then
// perform any post-bind validation. Per-handle importer methods could enforce this ordering;
// AHB memory requirements must remain in the post-bind step.

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryDmaBufDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(LINUX)
    VkDevice vkDevice = device->GetVkDevice();
    VkPhysicalDevice vkPhysicalDevice =
        ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice();

    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

    const CombinedLimits& limits = device->GetLimits();
    DAWN_INVALID_IF(
        descriptor->size.width == 0 || descriptor->size.width > limits.v1.maxTextureDimension2D,
        "Resource width (%u) is zero or exceeds maxTextureDimension2D (%u).",
        descriptor->size.width, limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(
        descriptor->size.height == 0 || descriptor->size.height > limits.v1.maxTextureDimension2D,
        "Resource height (%u) is zero or exceeds maxTextureDimension2D (%u).",
        descriptor->size.height, limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "depthOrArrayLayers was not 1.");

    SharedTextureMemoryProperties properties;
    properties.size = {descriptor->size.width, descriptor->size.height,
                       descriptor->size.depthOrArrayLayers};

    DAWN_TRY_ASSIGN(properties.format, FormatFromDrmFormat(descriptor->drmFormat));

    properties.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                       wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding |
                       wgpu::TextureUsage::RenderAttachment;

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory = SharedTextureMemory::CreateAndReifyProperties(
        device, label, &properties, VK_QUEUE_FAMILY_EXTERNAL_KHR);

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    VkFormat vkFormat = VulkanImageFormat(device, properties.format);

    // Number of memory planes in the image which will be queried from the DRM modifier.
    uint32_t memoryPlaneCount;

    // Share the base image parameters between the support query and image creation.
    // The view-format workaround below may extend the format list after the query.
    VkImageCreateInfo createInfo = MakeImageCreateInfo(
        device, properties, *internalFormat, vkFormat, VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);
    // View formats allowed for the image.
    std::array<VkFormat, 3> viewFormats{};
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;

    // Including an sRGB view format in the support query can fail for storage usage because
    // sRGB formats do not support storage. Keep the existing workaround: query without the
    // additional sRGB view format, then append it below when backend validation is disabled.
    // See https://github.com/gpuweb/gpuweb/issues/4426.
    // TODO(crbug.com/dawn/2304): Follow up with the Vulkan spec and try to lift this.
    bool addViewFormats = false;

    // Validate that the import is valid.
    {
        // Verify plane count for the modifier.
        // https://docs.vulkan.org/refpages/latest/refpages/source/VkDrmFormatModifierPropertiesEXT.html#_description
        VkDrmFormatModifierPropertiesEXT drmModifierProps;
        DAWN_TRY_ASSIGN(drmModifierProps,
                        GetFormatModifierProps(device->fn, vkPhysicalDevice, vkFormat,
                                               descriptor->drmModifier));
        memoryPlaneCount = drmModifierProps.drmFormatModifierPlaneCount;
        if (drmModifierProps.drmFormatModifier == 0 /* DRM_FORMAT_MOD_LINEAR */) {
            uint32_t formatPlaneCount = GetAspectCount(internalFormat->aspects);
            DAWN_INVALID_IF(memoryPlaneCount != formatPlaneCount,
                            "DRM format plane count (%u) must match the format plane count (%u) if "
                            "drmModifier is DRM_FORMAT_MOD_LINEAR.",
                            memoryPlaneCount, formatPlaneCount);
        }
        DAWN_INVALID_IF(
            memoryPlaneCount != descriptor->planes.size(),
            "Memory plane count (%x) for drm format (%u) and modifier (%u) specify a plane "
            "count of %u which "
            "does not match the provided plane count (%u)",
            vkFormat, descriptor->drmFormat, descriptor->drmModifier, memoryPlaneCount,
            descriptor->planes.size());
        DAWN_INVALID_IF(memoryPlaneCount == 0, "Memory plane count must not be 0");
        DAWN_INVALID_IF(memoryPlaneCount > kMaxPlanesPerFormat,
                        "Memory plane count (%u) must not exceed %u.", memoryPlaneCount,
                        kMaxPlanesPerFormat);

        const auto viewRequirements =
            GetViewFormatRequirements(device, properties, *internalFormat);
        if (viewRequirements.needsMutable) {
            // Allow format reinterpretation and per-plane views.
            createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

            // Append the list of view formats the image must be compatible with.
            if (device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
                if (internalFormat->IsMultiPlanar()) {
                    viewFormats = {
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane0).format),
                        VulkanImageFormat(device,
                                          internalFormat->GetAspectInfo(Aspect::Plane1).format)};
                    imageFormatListInfo.viewFormatCount = 2;
                } else {
                    // Start with the base format and add the BGRA storage polyfill format if
                    // needed. Mutable images with DRM modifier tiling require a non-empty list.
                    const bool needsBGRA8UnormStoragePolyfill =
                        viewRequirements.needsBGRA8UnormStoragePolyfill;
                    viewFormats[imageFormatListInfo.viewFormatCount++] = vkFormat;
                    DAWN_ASSERT(!compatibleViewFormats.empty() || !needsBGRA8UnormStoragePolyfill);
                    if (needsBGRA8UnormStoragePolyfill) {
                        viewFormats[imageFormatListInfo.viewFormatCount++] =
                            VK_FORMAT_R8G8B8A8_UNORM;
                    }
                    addViewFormats = !compatibleViewFormats.empty();
                }
                imageFormatListInfo.pViewFormats = viewFormats.data();
            }
        }
    }

    // Validate that there is a single FD. If there is more than one FD, Dawn will need to validate
    // the format has VK_FORMAT_FEATURE_DISJOINT_BIT, create the VkImage with
    // VK_IMAGE_CREATE_DISJOINT_BIT, and separately bind the image planes to memory. Dawn doesn't
    // support use of VK_IMAGE_CREATE_DISJOINT_BIT currently. See crbug.com/42240514.
    int fd = descriptor->planes[0].fd;
    for (auto [i, plane] : Enumerate(descriptor->planes.subspan(1))) {
        DAWN_INVALID_IF(plane.fd != fd,
                        "descriptor->planes[%u].fd (%i) does not match other plane fd (%i). All "
                        "fds must be the same.",
                        i + 1, plane.fd, fd);
    }

    // Query support for the requested format and DRM modifier.
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drmModifierInfo = {};
    drmModifierInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
    drmModifierInfo.drmFormatModifier = descriptor->drmModifier;
    drmModifierInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
    externalImageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    externalImageFormatInfo.handleType = handleType;

    DAWN_TRY(CheckExternalImageFormatSupport(device, properties, createInfo,
                                             externalImageFormatInfo, imageFormatListInfo,
                                             drmModifierInfo));

    // Don't add the view format if backend validation is enabled, otherwise most image creations
    // will fail with VVL. This view format is only needed for sRGB reinterpretation.
    // TODO(crbug.com/dawn/2304): Investigate if this is a bug in VVL.
    if (addViewFormats && !device->GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
        DAWN_ASSERT(compatibleViewFormats.size() == 1u);
        viewFormats[imageFormatListInfo.viewFormatCount++] =
            VulkanImageFormat(device, compatibleViewFormats[0]->format);
    }

    // Create the VkImage for the import.
    {
        // Zero initialization also supplies the required zero size and unused array/depth pitches.
        std::array<VkSubresourceLayout, kMaxPlanesPerFormat> planeLayouts{};
        for (uint32_t plane = 0u; plane < memoryPlaneCount; ++plane) {
            planeLayouts[plane].offset = descriptor->planes[plane].offset;
            planeLayouts[plane].rowPitch = descriptor->planes[plane].stride;
        }

        VkImageDrmFormatModifierExplicitCreateInfoEXT explicitCreateInfo = {};
        explicitCreateInfo.sType =
            VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
        explicitCreateInfo.drmFormatModifier = descriptor->drmModifier;
        explicitCreateInfo.drmFormatModifierPlaneCount = memoryPlaneCount;
        explicitCreateInfo.pPlaneLayouts = planeLayouts.data();

        VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
        externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        externalMemoryImageCreateInfo.handleTypes = handleType;

        DAWN_TRY_ASSIGN(sharedTextureMemory->mVkImage,
                        CreateVkImage(device, createInfo, externalMemoryImageCreateInfo,
                                      imageFormatListInfo, explicitCreateInfo));
    }

    // Import the memory plane(s) as VkDeviceMemory and bind to the VkImage.
    VkMemoryFdPropertiesKHR fdProperties;
    fdProperties.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
    fdProperties.pNext = nullptr;

    // Get the valid memory types that the external memory can be imported as.
    DAWN_TRY(
        CheckVkSuccess(device->fn.GetMemoryFdPropertiesKHR(vkDevice, handleType, fd, &fdProperties),
                       "vkGetMemoryFdPropertiesKHR"));

    // Get the valid memory types for the VkImage.
    VkMemoryRequirements memoryRequirements;
    device->fn.GetImageMemoryRequirements(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                          &memoryRequirements);

    // Choose the best memory type that satisfies both the image's constraint and the
    // import's constraint.
    memoryRequirements.memoryTypeBits &= fdProperties.memoryTypeBits;
    // Some dma-buf imports may lack a compatible device-local memory type. Allow a host-cached
    // fallback for this case, observed on AMD with imports thought to originate from a camera.
    // See crbug.com/422128949.
    uint32_t memoryTypeIndex;
    DAWN_TRY_ASSIGN(memoryTypeIndex,
                    FindImportMemoryType(device, memoryRequirements, /*allowHostCached=*/true));

    DAWN_TRY_ASSIGN(
        sharedTextureMemory->mVkDeviceMemory,
        ImportMemoryFD(device, fd, handleType, memoryRequirements.size, memoryTypeIndex));
    DAWN_TRY(sharedTextureMemory->BindImageMemory());
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(LINUX)
}

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryAHardwareBufferDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(ANDROID)
    const auto* ahbFunctions =
        ToBackend(device->GetAdapter()->GetPhysicalDevice())->GetOrLoadAHBFunctions();
    VkDevice vkDevice = device->GetVkDevice();

    auto* aHardwareBuffer = static_cast<struct AHardwareBuffer*>(descriptor->handle);

    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Reflect the properties of the AHardwareBuffer.
    AHBSharedTextureMemoryProperties ahbProperties =
        GetAHBSharedTextureMemoryProperties(ahbFunctions, aHardwareBuffer);

    // Reject protected AHBs.
    // Dawn doesn't enable VkPhysicalDeviceProtectedMemoryFeatures::protectedMemory and never sets
    // VK_IMAGE_CREATE_PROTECTED_BIT, so an AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT buffer would
    // produce an invalid vkAllocateMemory (VUID-VkMemoryAllocateInfo-None-01872) and
    // vkBindImageMemory (VUID-vkBindImageMemory-None-01902).
    DAWN_INVALID_IF(ahbProperties.isProtected,
                    "Unsupported AHardwareBuffer usage AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT.");

    SharedTextureMemoryProperties& properties = ahbProperties.properties;

    const CombinedLimits& limits = device->GetLimits();
    DAWN_INVALID_IF(properties.size.width > limits.v1.maxTextureDimension2D,
                    "Resource width (%u) exceeds maxTextureDimension2D (%u).",
                    properties.size.width, limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(properties.size.height > limits.v1.maxTextureDimension2D,
                    "Resource weight (%u) exceeds maxTextureDimension2D (%u).",
                    properties.size.height, limits.v1.maxTextureDimension2D);
    DAWN_INVALID_IF(properties.size.depthOrArrayLayers > limits.v1.maxTextureArrayLayers,
                    "Resource layers (%d) exceeds maxTextureArrayLayers (%u).",
                    properties.size.depthOrArrayLayers, limits.v1.maxTextureArrayLayers);

    bool usesExternalFormat = properties.format == wgpu::TextureFormat::OpaqueYCbCrAndroid;
    if (usesExternalFormat) {
        // Multi-layer YCbCr AHBs shouldn't be possible to create but validate it just in case.
        DAWN_INVALID_IF(properties.size.depthOrArrayLayers != 1,
                        "Resource layers (%d) is not 1 for YCbCr formats.",
                        properties.size.depthOrArrayLayers);

        // When using the opaque YUV texture formats, only the TextureBinding usage is valid.
        properties.usage &= wgpu::TextureUsage::TextureBinding;
    }

    VkFormat vkFormat;
    YCbCrVkDescriptor yCbCrAHBInfo;
    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };
    VkExternalFormatANDROID externalFormatAndroid = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
    };

    // Query the properties to find the appropriate VkFormat and memory type.
    {
        VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
        PNextChainBuilder bufferPropertiesChain(&bufferProperties);
        bufferPropertiesChain.Add(
            &bufferFormatProperties,
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

        DAWN_TRY(CheckVkSuccess(device->fn.GetAndroidHardwareBufferPropertiesANDROID(
                                    vkDevice, aHardwareBuffer, &bufferProperties),
                                "vkGetAndroidHardwareBufferPropertiesANDROID"));

        // TODO(crbug.com/dawn/2476): Validate more as per
        // https://docs.vulkan.org/refpages/latest/refpages/source/VkImageCreateInfo.html
        if (usesExternalFormat) {
            DAWN_INVALID_IF(
                bufferFormatProperties.externalFormat == 0,
                "AHardwareBuffer with external sampler must have non-zero external format.");
            vkFormat = VK_FORMAT_UNDEFINED;
            externalFormatAndroid.externalFormat = bufferFormatProperties.externalFormat;
        } else {
            vkFormat = bufferFormatProperties.format;
            DAWN_TRY_ASSIGN(properties.format, FormatFromVkFormat(device, vkFormat));
        }

        // Populate the YCbCr info.
        yCbCrAHBInfo.externalFormat = externalFormatAndroid.externalFormat;
        yCbCrAHBInfo.vkFormat = vkFormat;
        yCbCrAHBInfo.vkYCbCrModel = bufferFormatProperties.suggestedYcbcrModel;
        yCbCrAHBInfo.vkYCbCrRange = bufferFormatProperties.suggestedYcbcrRange;
        yCbCrAHBInfo.vkComponentSwizzleRed =
            bufferFormatProperties.samplerYcbcrConversionComponents.r;
        yCbCrAHBInfo.vkComponentSwizzleGreen =
            bufferFormatProperties.samplerYcbcrConversionComponents.g;
        yCbCrAHBInfo.vkComponentSwizzleBlue =
            bufferFormatProperties.samplerYcbcrConversionComponents.b;
        yCbCrAHBInfo.vkComponentSwizzleAlpha =
            bufferFormatProperties.samplerYcbcrConversionComponents.a;
        yCbCrAHBInfo.vkXChromaOffset = bufferFormatProperties.suggestedXChromaOffset;
        yCbCrAHBInfo.vkYChromaOffset = bufferFormatProperties.suggestedYChromaOffset;

        uint32_t formatFeatures = bufferFormatProperties.formatFeatures;
        if (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) {
            yCbCrAHBInfo.vkChromaFilter = wgpu::FilterMode::Linear;
        } else {
            yCbCrAHBInfo.vkChromaFilter = wgpu::FilterMode::Nearest;
        }
        yCbCrAHBInfo.forceExplicitReconstruction =
            formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT;
    }

    const Format* internalFormat = nullptr;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    DAWN_INVALID_IF(internalFormat->IsMultiPlanar(),
                    "Multi-planar AHardwareBuffer not supported yet.");

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory = SharedTextureMemory::CreateAndReifyProperties(
        device, label, &properties, VK_QUEUE_FAMILY_FOREIGN_EXT, yCbCrAHBInfo);

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    // Use the same parameters to validate import support and create the image.
    VkImageCreateInfo createInfo =
        MakeImageCreateInfo(device, properties, *internalFormat, vkFormat, VK_IMAGE_TILING_OPTIMAL);
    // View formats allowed for the image.
    std::array<VkFormat, 2> viewFormats{};
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;

    // External Android formats use VK_FORMAT_UNDEFINED and cannot use this format query.
    if (!usesExternalFormat) {
        const auto viewRequirements =
            GetViewFormatRequirements(device, properties, *internalFormat);
        if (viewRequirements.needsMutable) {
            // Allow format reinterpretation.
            createInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

            // Leave the view-format list empty for storage usage to avoid querying sRGB view
            // formats with unsupported storage usage. The empty list is omitted from the query
            // but is still chained during image creation. Mutable images with DRM modifier
            // tiling require a non-empty list, so the dma-buf path uses a different workaround.
            // TODO(crbug.com/dawn/2304): Apply a common fix when the format-list issue is resolved.
            if ((properties.usage & wgpu::TextureUsage::StorageBinding) == 0 &&
                device->GetDeviceInfo().HasExt(DeviceExt::ImageFormatList)) {
                // Set the list of view formats the image can be compatible with.
                DAWN_ASSERT(compatibleViewFormats.size() == 1u);
                viewFormats[0] = vkFormat;
                viewFormats[1] = VulkanImageFormat(device, compatibleViewFormats[0]->format);
                imageFormatListInfo.viewFormatCount = 2;
                imageFormatListInfo.pViewFormats = viewFormats.data();
            }
        }

        VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
        externalImageFormatInfo.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
        externalImageFormatInfo.handleType = handleType;

        DAWN_TRY(CheckExternalImageFormatSupport(device, properties, createInfo,
                                                 externalImageFormatInfo, imageFormatListInfo));
    }
    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.handleTypes = handleType;

    DAWN_TRY_ASSIGN(sharedTextureMemory->mVkImage,
                    CreateVkImage(device, createInfo, externalMemoryImageCreateInfo,
                                  imageFormatListInfo, externalFormatAndroid));

    // Import the memory as VkDeviceMemory and bind to the VkImage.
    {
        // Choose the best memory type that satisfies the import's constraint.
        VkMemoryRequirements memoryRequirements = {};
        memoryRequirements.memoryTypeBits = bufferProperties.memoryTypeBits;
        uint32_t memoryTypeIndex;
        DAWN_TRY_ASSIGN(memoryTypeIndex, FindImportMemoryType(device, memoryRequirements,
                                                              /*allowHostCached=*/false));

        VkImportAndroidHardwareBufferInfoANDROID importMemoryAHBInfo = {};
        importMemoryAHBInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
        importMemoryAHBInfo.buffer = aHardwareBuffer;

        // AHardwareBuffer image imports must use dedicated allocations.
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#memory-external-android-hardware-buffer-image-resources
        DAWN_TRY_ASSIGN(sharedTextureMemory->mVkDeviceMemory,
                        AllocateDeviceMemory(device, bufferProperties.allocationSize,
                                             sharedTextureMemory->mVkImage->Get(), memoryTypeIndex,
                                             importMemoryAHBInfo));
        DAWN_TRY(sharedTextureMemory->BindImageMemory());

        // Unlike ordinary images, AHB images must be bound before querying memory requirements
        // (VUID-vkGetImageMemoryRequirements-image-04004). Verify they fit the AHB constraints.
        // https://docs.vulkan.org/refpages/latest/refpages/source/vkGetImageMemoryRequirements.html#VUID-vkGetImageMemoryRequirements-image-04004
        device->fn.GetImageMemoryRequirements(vkDevice, sharedTextureMemory->mVkImage->Get(),
                                              &memoryRequirements);

        DAWN_INVALID_IF((memoryRequirements.memoryTypeBits & bufferProperties.memoryTypeBits) == 0,
                        "Required memory type bits (%u) do not overlap with AHardwareBuffer memory "
                        "type bits (%u).",
                        memoryRequirements.memoryTypeBits, bufferProperties.memoryTypeBits);

        if (!device->IsToggleEnabled(Toggle::IgnoreImportedAHardwareBufferVulkanImageSize)) {
            DAWN_INVALID_IF(memoryRequirements.size > bufferProperties.allocationSize,
                            "Required texture memory size (%u) is larger than the AHardwareBuffer "
                            "allocation size (%u).",
                            memoryRequirements.size, bufferProperties.allocationSize);
        }
    }
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(ANDROID)
}

// static
ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemory::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryOpaqueFDDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(POSIX)
    const VkExternalMemoryHandleTypeFlagBits handleType =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    const VkImageCreateInfo* createInfo =
        static_cast<const VkImageCreateInfo*>(descriptor->vkImageCreateInfo);
    DAWN_INVALID_IF(
        createInfo->sType != VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        "descriptor->vkImageCreateInfo.sType was not VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO.");

    // Validate the createInfo chain.
    const VkExternalMemoryImageCreateInfo* externalMemoryImageCreateInfo = nullptr;
    VkImageFormatListCreateInfo imageFormatListInfo = {};
    {
        const VkBaseInStructure* current = static_cast<const VkBaseInStructure*>(createInfo->pNext);
        while (current != nullptr) {
            switch (current->sType) {
                case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
                    // TODO(crbug.com/dawn/1745): Use this to inform supported types of WebGPU
                    // format reinterpretation (srgb).
                    imageFormatListInfo =
                        *reinterpret_cast<const VkImageFormatListCreateInfo*>(current);
                    break;
                case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
                    externalMemoryImageCreateInfo =
                        reinterpret_cast<const VkExternalMemoryImageCreateInfo*>(current);
                    DAWN_INVALID_IF((externalMemoryImageCreateInfo->handleTypes & handleType) == 0,
                                    "VkExternalMemoryImageCreateInfo::handleTypes (chained on "
                                    "descriptor->vkImageCreateInfo) did not have "
                                    "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT.");
                    break;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Unsupported descriptor->vkImageCreateInfo chain with sType 0x%x",
                        current->sType);
            }
            current = current->pNext;
        }
    }

    DAWN_INVALID_IF(
        externalMemoryImageCreateInfo == nullptr,
        "descriptor->vkImageCreateInfo did not have chain with VkExternalMemoryImageCreateInfo");

    DAWN_INVALID_IF(
        (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0,
        "descriptor->vkImageCreateInfo.usage did not have VK_IMAGE_USAGE_TRANSFER_DST_BIT");

    const bool isBGRA8UnormStorage = createInfo->format == VK_FORMAT_B8G8R8A8_UNORM &&
                                     (createInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
    DAWN_INVALID_IF(
        isBGRA8UnormStorage && (createInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) == 0,
        "descriptor->vkImageCreateInfo.flags did not have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT when "
        "usage has VK_IMAGE_USAGE_STORAGE_BIT and format is VK_FORMAT_B8G8R8A8_UNORM");

    // Populate the properties from the VkImageCreateInfo
    SharedTextureMemoryProperties properties{};
    properties.size = {
        createInfo->extent.width,
        createInfo->extent.height,
        std::max(createInfo->arrayLayers, createInfo->extent.depth),
    };
    DAWN_TRY_ASSIGN(properties.format, FormatFromVkFormat(device, createInfo->format));
    if (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        properties.usage |= wgpu::TextureUsage::CopySrc;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        properties.usage |= wgpu::TextureUsage::CopyDst;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        properties.usage |= wgpu::TextureUsage::TextureBinding;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        properties.usage |= wgpu::TextureUsage::StorageBinding;
    }
    if (createInfo->usage &
        (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        properties.usage |= wgpu::TextureUsage::RenderAttachment;
    }
    if (createInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        properties.usage |= wgpu::TextureUsage::TransientAttachment;
    }

    const Format* internalFormat;
    DAWN_TRY_ASSIGN(internalFormat, device->GetInternalFormat(properties.format));

    const auto& compatibleViewFormats = device->GetCompatibleViewFormats(*internalFormat);

    // Create the SharedTextureMemory object.
    Ref<SharedTextureMemory> sharedTextureMemory = SharedTextureMemory::CreateAndReifyProperties(
        device, label, &properties, VK_QUEUE_FAMILY_EXTERNAL_KHR);

    const bool needsMutable =
        GetViewFormatRequirements(device, properties, *internalFormat).needsMutable;

    DAWN_INVALID_IF(needsMutable && !(createInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT),
                    "VkImageCreateInfo::flags did not have VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT "
                    "which is required for view format reinterpretation or per-plane views.");

    if (imageFormatListInfo.sType == VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO &&
        needsMutable && !compatibleViewFormats.empty()) {
        // SAFETY: Vulkan requires that pViewFormats point at viewFormatCount valid formats and the
        // application giving us its VkImageCreateInfo needs to conform to that.
        Span<const VkFormat> viewFormats = DAWN_UNSAFE_BUFFERS(
            {imageFormatListInfo.pViewFormats, imageFormatListInfo.viewFormatCount});
        VkFormat baseVkFormat = VulkanImageFormat(device, properties.format);

        DAWN_INVALID_IF(
            std::ranges::find(viewFormats, baseVkFormat) == viewFormats.end(),
            "VkImageFormatCreateInfo did not contain VkFormat 0x%x which may be required to "
            "create a texture view with %s.",
            baseVkFormat, properties.format);

        for (const auto* f : compatibleViewFormats) {
            VkFormat vkFormat = VulkanImageFormat(device, f->format);
            DAWN_INVALID_IF(std::ranges::find(viewFormats, vkFormat) == viewFormats.end(),
                            "VkImageFormatCreateInfo did not contain VkFormat 0x%x which may be "
                            "required to create a texture view with %s.",
                            vkFormat, f->format);
        }
    }

    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo = {};
    externalImageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    externalImageFormatInfo.handleType = handleType;

    DAWN_TRY(CheckExternalImageFormatSupport(device, properties, *createInfo,
                                             externalImageFormatInfo, imageFormatListInfo));
    DAWN_TRY_ASSIGN(sharedTextureMemory->mVkImage, CreateVkImage(device, *createInfo));

    // Import the memoryFD as VkDeviceMemory and bind to the VkImage.
    {
        VkMemoryRequirements requirements;
        device->fn.GetImageMemoryRequirements(device->GetVkDevice(),
                                              sharedTextureMemory->mVkImage->Get(), &requirements);
        DAWN_INVALID_IF(requirements.size > descriptor->allocationSize,
                        "Required texture memory size (%u) is larger than the memory fd "
                        "allocation size (%u).",
                        requirements.size, descriptor->allocationSize);

        DAWN_TRY_ASSIGN(
            sharedTextureMemory->mVkDeviceMemory,
            ImportMemoryFD(device, descriptor->memoryFD, handleType, descriptor->allocationSize,
                           descriptor->memoryTypeIndex,
                           descriptor->dedicatedAllocation ? sharedTextureMemory->mVkImage->Get()
                                                           : VkImage{}));
        DAWN_TRY(sharedTextureMemory->BindImageMemory());
    }
    return sharedTextureMemory;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(POSIX)
}

// static
Ref<SharedTextureMemory> SharedTextureMemory::CreateAndReifyProperties(
    Device* device,
    StringView label,
    SharedTextureMemoryProperties* properties,
    uint32_t queueFamilyIndex,
    const YCbCrVkDescriptor& yCbCrVkDesc) {
    Ref<SharedTextureMemory> sharedTextureMemory = AcquireRef(
        new SharedTextureMemory(device, label, *properties, queueFamilyIndex, yCbCrVkDesc));
    sharedTextureMemory->Initialize();
    // Copy the supported STM properties back to the caller.
    sharedTextureMemory->APIGetProperties(properties);
    return sharedTextureMemory;
}

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         StringView label,
                                         const SharedTextureMemoryProperties& properties,
                                         uint32_t queueFamilyIndex,
                                         const YCbCrVkDescriptor& yCbCrVkDesc)
    : SharedTextureMemoryBase(device, label, properties),
      mQueueFamilyIndex(queueFamilyIndex),
      mYCbCrVkDesc(yCbCrVkDesc) {}

RefCountedVkHandle<VkDeviceMemory>* SharedTextureMemory::GetVkDeviceMemory() const {
    return mVkDeviceMemory.Get();
}

RefCountedVkHandle<VkImage>* SharedTextureMemory::GetVkImage() const {
    return mVkImage.Get();
}

uint32_t SharedTextureMemory::GetQueueFamilyIndex() const {
    return mQueueFamilyIndex;
}

MaybeError SharedTextureMemory::BindImageMemory() {
    Device* device = ToBackend(GetDevice());
    return CheckVkOOMThenSuccess(device->fn.BindImageMemory(device->GetVkDevice(), mVkImage->Get(),
                                                            mVkDeviceMemory->Get(), 0),
                                 "vkBindImageMemory");
}

void SharedTextureMemory::DestroyImpl(DestroyReason reason) {
    mVkImage = nullptr;
    mVkDeviceMemory = nullptr;
}

Ref<SharedResourceMemoryContents> SharedTextureMemory::CreateContents() {
    return AcquireRef(new SharedTextureMemoryContentsVk(GetWeakRef(this), mYCbCrVkDesc));
}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return SharedTexture::Create(this, descriptor);
}

MaybeError SharedTextureMemory::BeginAccessImpl(
    TextureBase* texture,
    const UnpackedPtr<BeginAccessDescriptor>& descriptor) {
    // TODO(dawn/2276): support concurrent read access.
    DAWN_INVALID_IF(descriptor->concurrentRead, "Vulkan backend doesn't support concurrent read.");
    DAWN_INVALID_IF(texture->GetFormat().format == wgpu::TextureFormat::OpaqueYCbCrAndroid &&
                        !descriptor->initialized,
                    "BeginAccess with Texture format (%s) must be initialized",
                    texture->GetFormat().format);

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (descriptor.ValidateBranches<Branch<SharedTextureMemoryVkImageLayoutBeginState>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedTextureMemoryVkImageLayoutBeginState);

    auto vkLayoutBeginState = descriptor.Get<SharedTextureMemoryVkImageLayoutBeginState>();
    DAWN_ASSERT(vkLayoutBeginState != nullptr);

    for (auto [i, fence] : Enumerate(descriptor->fences)) {
        // All fences are backed by binary semaphores.
        DAWN_INVALID_IF(descriptor->signaledValues[i] != 1, "%s signaled value (%u) was not 1.",
                        fence, descriptor->signaledValues[i]);
    }
    static_cast<SharedTexture*>(texture)->SetPendingAcquire(
        static_cast<VkImageLayout>(vkLayoutBeginState->oldLayout),
        static_cast<VkImageLayout>(vkLayoutBeginState->newLayout));

    // TODO(crbug.com/449708316): Better identify textures used as a swapchain.
    ToBackend(texture)->SetIsExternalSwapchainTexture(true);

    return {};
}

#if DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)
ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (state.ValidateBranches<Branch<SharedTextureMemoryVkImageLayoutEndState>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedTextureMemoryVkImageLayoutEndState);

    auto vkLayoutEndState = state.Get<SharedTextureMemoryVkImageLayoutEndState>();
    DAWN_ASSERT(vkLayoutEndState != nullptr);

#if DAWN_PLATFORM_IS(FUCHSIA)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreZirconHandle),
                    "Required feature (%s) for %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreZirconHandle,
                    wgpu::SharedFenceType::VkSemaphoreZirconHandle);
#elif DAWN_PLATFORM_IS(LINUX)
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceSyncFD) &&
                        !GetDevice()->HasFeature(Feature::SharedFenceVkSemaphoreOpaqueFD),
                    "Required feature (%s or %s) for %s or %s is missing.",
                    wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD,
                    wgpu::FeatureName::SharedFenceSyncFD,
                    wgpu::SharedFenceType::VkSemaphoreOpaqueFD, wgpu::SharedFenceType::SyncFD);
#endif

    SystemHandle handle;
    {
        ExternalSemaphoreHandle semaphoreHandle;
        VkImageLayout releasedOldLayout;
        VkImageLayout releasedNewLayout;
        DAWN_TRY(static_cast<SharedTexture*>(texture)->EndAccess(
            &semaphoreHandle, &releasedOldLayout, &releasedNewLayout));
        // Handle is acquired from the texture so we need to make sure to close it.
        // TODO(dawn:1745): Consider using one event per submit that is tracked by the
        // CommandRecordingContext so that we don't need to create one handle per texture,
        // and so we don't need to acquire it here to close it.
        handle = SystemHandle::Acquire(semaphoreHandle);
        vkLayoutEndState->oldLayout = releasedOldLayout;
        vkLayoutEndState->newLayout = releasedNewLayout;
    }

    Ref<SharedFence> fence;

#if DAWN_PLATFORM_IS(FUCHSIA)
    SharedFenceVkSemaphoreZirconHandleDescriptor desc;
    desc.handle = handle.Get();

    DAWN_TRY_ASSIGN(fence,
                    SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
#elif DAWN_PLATFORM_IS(LINUX)
    if (GetDevice()->HasFeature(Feature::SharedFenceSyncFD)) {
        SharedFenceSyncFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    } else {
        SharedFenceVkSemaphoreOpaqueFDDescriptor desc;
        desc.handle = handle.Get();

        DAWN_TRY_ASSIGN(fence,
                        SharedFence::Create(ToBackend(GetDevice()), "Internal VkSemaphore", &desc));
    }
#endif
    ToBackend(texture)->NotifySwapChainPresent();

    // All semaphores are binary semaphores.
    return FenceAndSignalValue{std::move(fence), 1};
}

#else  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    return DAWN_VALIDATION_ERROR("No shared fence features supported.");
}

#endif  // DAWN_PLATFORM_IS(FUCHSIA) || DAWN_PLATFORM_IS(LINUX)

MaybeError SharedTextureMemory::GetChainedProperties(
    UnpackedPtr<SharedTextureMemoryProperties>& properties) const {
    auto ahbProperties = properties.Get<SharedTextureMemoryAHardwareBufferProperties>();

    if (!ahbProperties) {
        return {};
    }

    if (ahbProperties->yCbCrInfo.nextInChain) {
        return DAWN_VALIDATION_ERROR(
            "yCBCrInfo field of SharedTextureMemoryAHardwareBufferProperties has a chained "
            "struct.");
    }

    ahbProperties->yCbCrInfo = mYCbCrVkDesc;

    return {};
}

// SharedTextureMemoryContentsVk

SharedTextureMemoryContentsVk::SharedTextureMemoryContentsVk(
    WeakRef<SharedTextureMemoryBase> sharedTextureMemory,
    YCbCrVkDescriptor ycbcrVkDesc)
    : SharedTextureMemoryContents(std::move(sharedTextureMemory)), mYCbCrVkDesc(ycbcrVkDesc) {}

bool SharedTextureMemoryContentsVk::IsYCbCrFilterable() const {
    return mYCbCrVkDesc.vkChromaFilter != wgpu::FilterMode::Nearest;
}

const YCbCrVkDescriptor& SharedTextureMemoryContentsVk::GetYCbCrVkDesc() const {
    return mYCbCrVkDesc;
}

}  // namespace dawn::native::vulkan
