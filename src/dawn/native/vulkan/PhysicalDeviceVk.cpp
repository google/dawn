// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/PhysicalDeviceVk.h"

#include <algorithm>
#include <string>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Limits.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native::vulkan {

namespace {

gpu_info::DriverVersion DecodeVulkanDriverVersion(uint32_t vendorID, uint32_t versionRaw) {
    gpu_info::DriverVersion driverVersion;
    switch (vendorID) {
        case gpu_info::kVendorID_Nvidia:
            driverVersion = {static_cast<uint16_t>((versionRaw >> 22) & 0x3FF),
                             static_cast<uint16_t>((versionRaw >> 14) & 0x0FF),
                             static_cast<uint16_t>((versionRaw >> 6) & 0x0FF),
                             static_cast<uint16_t>(versionRaw & 0x003F)};
            break;
        case gpu_info::kVendorID_Intel:
#if DAWN_PLATFORM_IS(WINDOWS)
            // Windows Vulkan driver releases together with D3D driver, so they share the same
            // version. But only CCC.DDDD is encoded in 32-bit driverVersion.
            driverVersion = {static_cast<uint16_t>(versionRaw >> 14),
                             static_cast<uint16_t>(versionRaw & 0x3FFF)};
            break;
#endif
        default:
            // Use Vulkan driver conversions for other vendors
            driverVersion = {static_cast<uint16_t>(versionRaw >> 22),
                             static_cast<uint16_t>((versionRaw >> 12) & 0x3FF),
                             static_cast<uint16_t>(versionRaw & 0xFFF)};
            break;
    }

    return driverVersion;
}

}  // anonymous namespace

PhysicalDevice::PhysicalDevice(InstanceBase* instance,
                               VulkanInstance* vulkanInstance,
                               VkPhysicalDevice physicalDevice)
    : PhysicalDeviceBase(instance, wgpu::BackendType::Vulkan),
      mVkPhysicalDevice(physicalDevice),
      mVulkanInstance(vulkanInstance) {}

PhysicalDevice::~PhysicalDevice() = default;

const VulkanDeviceInfo& PhysicalDevice::GetDeviceInfo() const {
    return mDeviceInfo;
}

VkPhysicalDevice PhysicalDevice::GetVkPhysicalDevice() const {
    return mVkPhysicalDevice;
}

VulkanInstance* PhysicalDevice::GetVulkanInstance() const {
    return mVulkanInstance.Get();
}

bool PhysicalDevice::IsDepthStencilFormatSupported(VkFormat format) const {
    DAWN_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
                format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

    VkFormatProperties properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(mVkPhysicalDevice, format,
                                                                      &properties);
    return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    mDriverVersion = DecodeVulkanDriverVersion(mDeviceInfo.properties.vendorID,
                                               mDeviceInfo.properties.driverVersion);
    const std::string driverVersionStr = mDriverVersion.ToString();

#if DAWN_PLATFORM_IS(WINDOWS)
    // Disable Vulkan adapter on Windows Intel driver < 30.0.101.2111 due to flaky
    // issues.
    const gpu_info::DriverVersion kDriverVersion({30, 0, 101, 2111});
    if (gpu_info::IsIntel(mDeviceInfo.properties.vendorID) &&
        gpu_info::CompareWindowsDriverVersion(mDeviceInfo.properties.vendorID, mDriverVersion,
                                              kDriverVersion) == -1) {
        return DAWN_FORMAT_INTERNAL_ERROR(
            "Disable Intel Vulkan adapter on Windows driver version %s. See "
            "https://crbug.com/1338622.",
            driverVersionStr);
    }
#endif

    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        mDriverDescription = mDeviceInfo.driverProperties.driverName;
        if (mDeviceInfo.driverProperties.driverInfo[0] != '\0') {
            mDriverDescription += std::string(": ") + mDeviceInfo.driverProperties.driverInfo;
        }
        // There may be no driver version in driverInfo.
        if (mDriverDescription.find(driverVersionStr) == std::string::npos) {
            mDriverDescription += std::string(" ") + driverVersionStr;
        }
    } else {
        mDriverDescription = std::string("Vulkan driver version ") + driverVersionStr;
    }

    mDeviceId = mDeviceInfo.properties.deviceID;
    mVendorId = mDeviceInfo.properties.vendorID;
    mName = mDeviceInfo.properties.deviceName;

    switch (mDeviceInfo.properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            mAdapterType = wgpu::AdapterType::IntegratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            mAdapterType = wgpu::AdapterType::DiscreteGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            mAdapterType = wgpu::AdapterType::CPU;
            break;
        default:
            mAdapterType = wgpu::AdapterType::Unknown;
            break;
    }

    // Check for essential Vulkan extensions and features
    // Needed for viewport Y-flip.
    if (!mDeviceInfo.HasExt(DeviceExt::Maintenance1)) {
        return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance1 required.");
    }

    // Needed for separate depth/stencilReadOnly
    if (!mDeviceInfo.HasExt(DeviceExt::Maintenance2)) {
        return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance2 required.");
    }

    // Needed for security
    if (!mDeviceInfo.features.robustBufferAccess) {
        return DAWN_INTERNAL_ERROR("Vulkan robustBufferAccess feature required.");
    }

    if (!mDeviceInfo.features.textureCompressionBC &&
        !(mDeviceInfo.features.textureCompressionETC2 &&
          mDeviceInfo.features.textureCompressionASTC_LDR)) {
        return DAWN_INTERNAL_ERROR(
            "Vulkan textureCompressionBC feature required or both textureCompressionETC2 and "
            "textureCompressionASTC required.");
    }

    // Needed for the respective WebGPU features.
    if (!mDeviceInfo.features.depthBiasClamp) {
        return DAWN_INTERNAL_ERROR("Vulkan depthBiasClamp feature required.");
    }
    if (!mDeviceInfo.features.fragmentStoresAndAtomics) {
        return DAWN_INTERNAL_ERROR("Vulkan fragmentStoresAndAtomics feature required.");
    }
    if (!mDeviceInfo.features.fullDrawIndexUint32) {
        return DAWN_INTERNAL_ERROR("Vulkan fullDrawIndexUint32 feature required.");
    }
    if (!mDeviceInfo.features.imageCubeArray) {
        return DAWN_INTERNAL_ERROR("Vulkan imageCubeArray feature required.");
    }
    if (!mDeviceInfo.features.independentBlend) {
        return DAWN_INTERNAL_ERROR("Vulkan independentBlend feature required.");
    }
    if (!mDeviceInfo.features.sampleRateShading) {
        return DAWN_INTERNAL_ERROR("Vulkan sampleRateShading feature required.");
    }

    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    // Initialize supported extensions
    if (mDeviceInfo.features.textureCompressionBC == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionBC);
    }

    if (mDeviceInfo.features.textureCompressionETC2 == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionETC2);
    }

    if (mDeviceInfo.features.textureCompressionASTC_LDR == VK_TRUE) {
        EnableFeature(Feature::TextureCompressionASTC);
    }

    // TODO(dawn:1559) Resolving timestamp queries after a render pass is failing on Qualcomm-based
    // Android devices.
    if (mDeviceInfo.properties.limits.timestampComputeAndGraphics == VK_TRUE &&
        !IsAndroidQualcomm()) {
        EnableFeature(Feature::TimestampQuery);
        EnableFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses);
    }

    if (IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT)) {
        EnableFeature(Feature::Depth32FloatStencil8);
    }

    if (mDeviceInfo.features.drawIndirectFirstInstance == VK_TRUE) {
        EnableFeature(Feature::IndirectFirstInstance);
    }

    if (mDeviceInfo.features.dualSrcBlend == VK_TRUE) {
        EnableFeature(Feature::DualSourceBlending);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ShaderFloat16Int8) &&
        mDeviceInfo.HasExt(DeviceExt::_16BitStorage) &&
        mDeviceInfo.shaderFloat16Int8Features.shaderFloat16 == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.storageInputOutput16 == VK_TRUE &&
        mDeviceInfo._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_TRUE) {
        EnableFeature(Feature::ShaderF16);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ShaderIntegerDotProduct) &&
        mDeviceInfo.shaderIntegerDotProductFeatures.shaderIntegerDotProduct == VK_TRUE &&
        mDeviceInfo.shaderIntegerDotProductProperties
                .integerDotProduct4x8BitPackedSignedAccelerated == VK_TRUE &&
        mDeviceInfo.shaderIntegerDotProductProperties
                .integerDotProduct4x8BitPackedUnsignedAccelerated == VK_TRUE) {
        EnableFeature(Feature::ChromiumExperimentalDp4a);
    }

    // unclippedDepth=true translates to depthClamp=true, which implicitly disables clipping.
    if (mDeviceInfo.features.depthClamp == VK_TRUE) {
        EnableFeature(Feature::DepthClipControl);
    }

    VkFormatProperties rg11b10Properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_B10G11R11_UFLOAT_PACK32, &rg11b10Properties);

    if (IsSubset(static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                                   VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
                 rg11b10Properties.optimalTilingFeatures)) {
        EnableFeature(Feature::RG11B10UfloatRenderable);
    }

    VkFormatProperties bgra8unormProperties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_B8G8R8A8_UNORM, &bgra8unormProperties);
    if (bgra8unormProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        EnableFeature(Feature::BGRA8UnormStorage);
    }

    bool norm16TextureFormatsSupported = true;
    for (const auto& norm16Format :
         {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16A16_UNORM,
          VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16A16_SNORM}) {
        VkFormatProperties norm16Properties;
        mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
            mVkPhysicalDevice, norm16Format, &norm16Properties);
        norm16TextureFormatsSupported &= IsSubset(
            static_cast<VkFormatFeatureFlags>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                              VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
            norm16Properties.optimalTilingFeatures);
    }
    if (norm16TextureFormatsSupported) {
        EnableFeature(Feature::Norm16TextureFormats);
    }

    // 32 bit float channel formats.
    VkFormatProperties r32Properties;
    VkFormatProperties rg32Properties;
    VkFormatProperties rgba32Properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32_SFLOAT, &r32Properties);
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32G32_SFLOAT, &rg32Properties);
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
        mVkPhysicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &rgba32Properties);
    if ((r32Properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
        (rg32Properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) &&
        (rgba32Properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        EnableFeature(Feature::Float32Filterable);
    }

    // Multiplanar formats.
    constexpr VkFormat multiplanarFormats[] = {
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    };

    bool allMultiplanarFormatsSupported = true;
    for (const auto multiplanarFormat : multiplanarFormats) {
        VkFormatProperties multiplanarProps;
        mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(
            mVkPhysicalDevice, multiplanarFormat, &multiplanarProps);

        if (!IsSubset(static_cast<VkFormatFeatureFlagBits>(
                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                          VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT),
                      multiplanarProps.optimalTilingFeatures)) {
            allMultiplanarFormatsSupported = false;
        }
    }

    if (allMultiplanarFormatsSupported) {
        EnableFeature(Feature::DawnMultiPlanarFormats);
        EnableFeature(Feature::MultiPlanarFormatExtendedUsages);
    }

    EnableFeature(Feature::SurfaceCapabilities);
    EnableFeature(Feature::TransientAttachments);

    // Enable ChromiumExperimentalSubgroups feature if:
    // 1. Vulkan API version is 1.1 or later, and
    // 2. subgroupSupportedStages includes compute stage bit, and
    // 3. subgroupSupportedOperations includes basic and ballot bits, and
    // 4. VK_EXT_subgroup_size_control extension is valid, and both subgroupSizeControl
    //    and computeFullSubgroups is TRUE in VkPhysicalDeviceSubgroupSizeControlFeaturesEXT.
    if ((mDeviceInfo.properties.apiVersion >= VK_API_VERSION_1_1) &&
        (mDeviceInfo.subgroupProperties.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) &&
        (mDeviceInfo.subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) &&
        (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) &&
        (mDeviceInfo.subgroupSizeControlFeatures.subgroupSizeControl == VK_TRUE) &&
        (mDeviceInfo.subgroupSizeControlFeatures.computeFullSubgroups == VK_TRUE)) {
        EnableFeature(Feature::ChromiumExperimentalSubgroups);
    }
    // Enable ChromiumExperimentalSubgroupUniformControlFlow if
    // VK_KHR_shader_subgroup_uniform_control_flow is supported.
    if (mDeviceInfo.HasExt(DeviceExt::ShaderSubgroupUniformControlFlow) &&
        (mDeviceInfo.shaderSubgroupUniformControlFlowFeatures.shaderSubgroupUniformControlFlow ==
         VK_TRUE)) {
        EnableFeature(Feature::ChromiumExperimentalSubgroupUniformControlFlow);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ExternalMemoryHost) &&
        mDeviceInfo.externalMemoryHostProperties.minImportedHostPointerAlignment <= 4096) {
        // TODO(crbug.com/dawn/2018): properly surface the limit.
        // Linux nearly always exposes 4096.
        // https://vulkan.gpuinfo.org/displayextensionproperty.php?platform=linux&extensionname=VK_EXT_external_memory_host&extensionproperty=minImportedHostPointerAlignment
        EnableFeature(Feature::HostMappedPointer);
    }
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimitsForSupportedFeatureLevel(&limits->v1);
    CombinedLimits baseLimits = *limits;

    const VkPhysicalDeviceLimits& vkLimits = mDeviceInfo.properties.limits;

#define CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, compareOp, msgSegment)   \
    do {                                                                             \
        if (vkLimits.vulkanName compareOp baseLimits.v1.webgpuName) {                \
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for " #webgpuName \
                                       "."                                           \
                                       " VkPhysicalDeviceLimits::" #vulkanName       \
                                       " must be at " msgSegment " " +               \
                                       std::to_string(baseLimits.v1.webgpuName));    \
        }                                                                            \
        limits->v1.webgpuName = vkLimits.vulkanName;                                 \
    } while (false)

#define CHECK_AND_SET_V1_MAX_LIMIT(vulkanName, webgpuName) \
    CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, <, "least")
#define CHECK_AND_SET_V1_MIN_LIMIT(vulkanName, webgpuName) \
    CHECK_AND_SET_V1_LIMIT_IMPL(vulkanName, webgpuName, >, "most")

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension1D, maxTextureDimension1D);

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension2D, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimensionCube, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxFramebufferWidth, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxFramebufferHeight, maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxViewportDimensions[0], maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxViewportDimensions[1], maxTextureDimension2D);
    CHECK_AND_SET_V1_MAX_LIMIT(viewportBoundsRange[1], maxTextureDimension2D);
    limits->v1.maxTextureDimension2D = std::min({
        static_cast<uint32_t>(vkLimits.maxImageDimension2D),
        static_cast<uint32_t>(vkLimits.maxImageDimensionCube),
        static_cast<uint32_t>(vkLimits.maxFramebufferWidth),
        static_cast<uint32_t>(vkLimits.maxFramebufferHeight),
        static_cast<uint32_t>(vkLimits.maxViewportDimensions[0]),
        static_cast<uint32_t>(vkLimits.maxViewportDimensions[1]),
        static_cast<uint32_t>(vkLimits.viewportBoundsRange[1]),
    });

    CHECK_AND_SET_V1_MAX_LIMIT(maxImageDimension3D, maxTextureDimension3D);
    CHECK_AND_SET_V1_MAX_LIMIT(maxImageArrayLayers, maxTextureArrayLayers);
    CHECK_AND_SET_V1_MAX_LIMIT(maxBoundDescriptorSets, maxBindGroups);
    CHECK_AND_SET_V1_MAX_LIMIT(maxDescriptorSetUniformBuffersDynamic,
                               maxDynamicUniformBuffersPerPipelineLayout);
    CHECK_AND_SET_V1_MAX_LIMIT(maxDescriptorSetStorageBuffersDynamic,
                               maxDynamicStorageBuffersPerPipelineLayout);

    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorSampledImages,
                               maxSampledTexturesPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorSamplers, maxSamplersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorStorageBuffers,
                               maxStorageBuffersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorStorageImages,
                               maxStorageTexturesPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxPerStageDescriptorUniformBuffers,
                               maxUniformBuffersPerShaderStage);
    CHECK_AND_SET_V1_MAX_LIMIT(maxUniformBufferRange, maxUniformBufferBindingSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxStorageBufferRange, maxStorageBufferBindingSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxColorAttachments, maxColorAttachments);

    // Validate against maxFragmentCombinedOutputResources, tightening the limits when necessary.
    const uint32_t minFragmentCombinedOutputResources =
        baseLimits.v1.maxStorageBuffersPerShaderStage +
        baseLimits.v1.maxStorageTexturesPerShaderStage + baseLimits.v1.maxColorAttachments;
    const uint64_t maxFragmentCombinedOutputResources =
        limits->v1.maxStorageBuffersPerShaderStage + limits->v1.maxStorageTexturesPerShaderStage +
        limits->v1.maxColorAttachments;
    // Only re-adjust the limits when the limit makes sense w.r.t to the required WebGPU limits.
    // Otherwise, we ignore the maxFragmentCombinedOutputResources since it is known to yield
    // incorrect values on desktop drivers.
    bool readjustFragmentCombinedOutputResources =
        vkLimits.maxFragmentCombinedOutputResources > minFragmentCombinedOutputResources &&
        uint64_t(vkLimits.maxFragmentCombinedOutputResources) < maxFragmentCombinedOutputResources;
    if (readjustFragmentCombinedOutputResources) {
        // Split extra resources across the three other limits instead of using the default values
        // since it would overflow.
        uint32_t extraResources =
            vkLimits.maxFragmentCombinedOutputResources - minFragmentCombinedOutputResources;
        limits->v1.maxColorAttachments = std::min(
            baseLimits.v1.maxColorAttachments + (extraResources / 3), vkLimits.maxColorAttachments);
        extraResources -= limits->v1.maxColorAttachments - baseLimits.v1.maxColorAttachments;
        limits->v1.maxStorageTexturesPerShaderStage =
            std::min(baseLimits.v1.maxStorageTexturesPerShaderStage + (extraResources / 2),
                     vkLimits.maxPerStageDescriptorStorageImages);
        extraResources -= limits->v1.maxStorageTexturesPerShaderStage -
                          baseLimits.v1.maxStorageTexturesPerShaderStage;
        limits->v1.maxStorageBuffersPerShaderStage =
            std::min(baseLimits.v1.maxStorageBuffersPerShaderStage + extraResources,
                     vkLimits.maxPerStageDescriptorStorageBuffers);
    }

    CHECK_AND_SET_V1_MIN_LIMIT(minUniformBufferOffsetAlignment, minUniformBufferOffsetAlignment);
    CHECK_AND_SET_V1_MIN_LIMIT(minStorageBufferOffsetAlignment, minStorageBufferOffsetAlignment);

    CHECK_AND_SET_V1_MAX_LIMIT(maxVertexInputBindings, maxVertexBuffers);
    CHECK_AND_SET_V1_MAX_LIMIT(maxVertexInputAttributes, maxVertexAttributes);

    if (vkLimits.maxVertexInputBindingStride < baseLimits.v1.maxVertexBufferArrayStride ||
        vkLimits.maxVertexInputAttributeOffset < baseLimits.v1.maxVertexBufferArrayStride - 1) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBufferArrayStride");
    }
    limits->v1.maxVertexBufferArrayStride =
        std::min(vkLimits.maxVertexInputBindingStride, vkLimits.maxVertexInputAttributeOffset + 1);

    if (vkLimits.maxVertexOutputComponents < baseLimits.v1.maxInterStageShaderComponents ||
        vkLimits.maxFragmentInputComponents < baseLimits.v1.maxInterStageShaderComponents) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxInterStageShaderComponents");
    }
    limits->v1.maxInterStageShaderComponents =
        std::min(vkLimits.maxVertexOutputComponents, vkLimits.maxFragmentInputComponents);

    // Reserve 4 components for the SPIR-V builtin `position`.
    // See the discussions in https://github.com/gpuweb/gpuweb/issues/1962 for more details.
    limits->v1.maxInterStageShaderComponents =
        std::min(vkLimits.maxVertexOutputComponents, vkLimits.maxFragmentInputComponents) - 4;
    limits->v1.maxInterStageShaderVariables = limits->v1.maxInterStageShaderComponents / 4;

    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeSharedMemorySize, maxComputeWorkgroupStorageSize);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupInvocations, maxComputeInvocationsPerWorkgroup);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[0], maxComputeWorkgroupSizeX);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[1], maxComputeWorkgroupSizeY);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupSize[2], maxComputeWorkgroupSizeZ);

    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[0], maxComputeWorkgroupsPerDimension);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[1], maxComputeWorkgroupsPerDimension);
    CHECK_AND_SET_V1_MAX_LIMIT(maxComputeWorkGroupCount[2], maxComputeWorkgroupsPerDimension);
    limits->v1.maxComputeWorkgroupsPerDimension = std::min({
        vkLimits.maxComputeWorkGroupCount[0],
        vkLimits.maxComputeWorkGroupCount[1],
        vkLimits.maxComputeWorkGroupCount[2],
    });

    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferColorSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferColorSampleCounts");
    }
    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferDepthSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferDepthSampleCounts");
    }

    limits->v1.maxBufferSize = kAssumedMaxBufferSize;
    if (mDeviceInfo.HasExt(DeviceExt::Maintenance4)) {
        limits->v1.maxBufferSize = mDeviceInfo.propertiesMaintenance4.maxBufferSize;
    } else if (mDeviceInfo.HasExt(DeviceExt::Maintenance3)) {
        limits->v1.maxBufferSize = mDeviceInfo.propertiesMaintenance3.maxMemoryAllocationSize;
    }
    if (limits->v1.maxBufferSize < baseLimits.v1.maxBufferSize) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan maxBufferSize limit");
    }

    if (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        mDefaultComputeSubgroupSize = FindDefaultComputeSubgroupSize();
        if (mDefaultComputeSubgroupSize > 0) {
            // According to VK_EXT_subgroup_size_control, for compute shaders we must ensure
            // computeInvocationsPerWorkgroup <= maxComputeWorkgroupSubgroups x computeSubgroupSize
            limits->v1.maxComputeInvocationsPerWorkgroup =
                std::min(limits->v1.maxComputeInvocationsPerWorkgroup,
                         mDeviceInfo.subgroupSizeControlProperties.maxComputeWorkgroupSubgroups *
                             mDefaultComputeSubgroupSize);
        }
    }

    // Experimental limits for subgroups
    limits->experimentalSubgroupLimits.minSubgroupSize =
        mDeviceInfo.subgroupSizeControlProperties.minSubgroupSize;
    limits->experimentalSubgroupLimits.maxSubgroupSize =
        mDeviceInfo.subgroupSizeControlProperties.maxSubgroupSize;

    return {};
}

bool PhysicalDevice::SupportsExternalImages() const {
    // Via dawn::native::vulkan::WrapVulkanImage
    return external_memory::Service::CheckSupport(mDeviceInfo) &&
           external_semaphore::Service::CheckSupport(mDeviceInfo, mVkPhysicalDevice,
                                                     mVulkanInstance->GetFunctions());
}

bool PhysicalDevice::SupportsFeatureLevel(FeatureLevel) const {
    return true;
}

void PhysicalDevice::SetupBackendAdapterToggles(TogglesState* adpterToggles) const {}

void PhysicalDevice::SetupBackendDeviceToggles(TogglesState* deviceToggles) const {
    // TODO(crbug.com/dawn/857): tighten this workaround when this issue is fixed in both
    // Vulkan SPEC and drivers.
    deviceToggles->Default(Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy, true);

#if DAWN_PLATFORM_IS(ANDROID)
    // Default to the IR backend on Android.
    deviceToggles->Default(Toggle::UseTintIR, true);
#else
    // All other platforms default to the value corresponding to the feature flag.
    deviceToggles->Default(Toggle::UseTintIR, GetInstance()->GetPlatform()->IsFeatureEnabled(
                                                  platform::Features::kWebGPUUseTintIR));
#endif

    if (IsAndroidQualcomm()) {
        // dawn:1564, dawn:1897: Recording a compute pass after a render pass in the same command
        // buffer frequently causes a crash on Qualcomm GPUs. To work around that bug, split the
        // command buffer any time we are about to record a compute pass when a render pass has
        // already been recorded.
        deviceToggles->Default(Toggle::VulkanSplitCommandBufferOnComputePassAfterRenderPass, true);

        // dawn:1569: Qualcomm devices have a bug resolving into a non-zero level of an array
        // texture. Work around it by resolving into a single level texture and then copying into
        // the intended layer.
        deviceToggles->Default(Toggle::AlwaysResolveIntoZeroLevelAndLayer, true);
    }

    if (IsAndroidARM()) {
        // dawn:1550: Resolving multiple color targets in a single pass fails on ARM GPUs. To
        // work around the issue, passes that resolve to multiple color targets will instead be
        // forced to store the multisampled targets and do the resolves as separate passes injected
        // after the original one.
        deviceToggles->Default(Toggle::ResolveMultipleAttachmentInSeparatePasses, true);
    }

    if (IsIntelMesa() && gpu_info::IsIntelGen12LP(GetVendorId(), GetDeviceId())) {
        // dawn:1688: Intel Mesa driver has a bug about reusing the VkDeviceMemory that was
        // previously bound to a 2D VkImage. To work around that bug we have to disable the resource
        // sub-allocation for 2D textures with CopyDst or RenderAttachment usage.
        const gpu_info::DriverVersion kBuggyDriverVersion = {21, 3, 6, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kBuggyDriverVersion) >= 0) {
            deviceToggles->Default(
                Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment, true);
        }

        // chromium:1361662: Mesa driver has a bug clearing R8 mip-leveled textures on Intel Gen12
        // GPUs. Work around it by clearing the whole texture as soon as they are created.
        const gpu_info::DriverVersion kFixedDriverVersion = {23, 1, 0, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kFixedDriverVersion) < 0) {
            deviceToggles->Default(Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation, true);
        }
    }

    if (IsIntelMesa() && (gpu_info::IsIntelGen12LP(GetVendorId(), GetDeviceId()) ||
                          gpu_info::IsIntelGen12HP(GetVendorId(), GetDeviceId()))) {
        // Intel Mesa driver has a bug where vkCmdCopyQueryPoolResults fails to write overlapping
        // queries to a same buffer after the buffer is accessed by a compute shader with correct
        // resource barriers, which may caused by flush and memory coherency issue on Intel Gen12
        // GPUs. Workaround for it to clear the buffer before vkCmdCopyQueryPoolResults on Mesa
        // driver version < 23.1.3.
        const gpu_info::DriverVersion kBuggyDriverVersion = {21, 2, 0, 0};
        const gpu_info::DriverVersion kFixedDriverVersion = {23, 1, 3, 0};
        if (gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kBuggyDriverVersion) >= 0 &&
            gpu_info::CompareIntelMesaDriverVersion(GetDriverVersion(), kFixedDriverVersion) < 0) {
            deviceToggles->Default(Toggle::ClearBufferBeforeResolveQueries, true);
        }
    }

    // The environment can request to various options for depth-stencil formats that could be
    // unavailable. Override the decision if it is not applicable.
    bool supportsD32s8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
    bool supportsD24s8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
    bool supportsS8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

    DAWN_ASSERT(supportsD32s8 || supportsD24s8);

    if (!supportsD24s8) {
        deviceToggles->ForceSet(Toggle::VulkanUseD32S8, true);
    }
    if (!supportsD32s8) {
        deviceToggles->ForceSet(Toggle::VulkanUseD32S8, false);
    }
    // By default try to use D32S8 for Depth24PlusStencil8
    deviceToggles->Default(Toggle::VulkanUseD32S8, true);

    if (!supportsS8) {
        deviceToggles->ForceSet(Toggle::VulkanUseS8, false);
    }
    // By default try to use S8 if available.
    deviceToggles->Default(Toggle::VulkanUseS8, true);

    // The environment can only request to use VK_KHR_zero_initialize_workgroup_memory when the
    // extension is available. Override the decision if it is not applicable or
    // zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory == VK_FALSE.
    if (!GetDeviceInfo().HasExt(DeviceExt::ZeroInitializeWorkgroupMemory) ||
        GetDeviceInfo().zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory ==
            VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, false);
    }
    // By default try to initialize workgroup memory with OpConstantNull according to the Vulkan
    // extension VK_KHR_zero_initialize_workgroup_memory.
    deviceToggles->Default(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, true);

    // Inject fragment shaders in all vertex-only pipelines.
    // TODO(crbug.com/dawn/1698): relax this requirement where the Vulkan spec allows.
    // In particular, enable rasterizer discard if the depth-stencil stage is a no-op, and skip
    // insertion of the placeholder fragment shader.
    deviceToggles->Default(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline, true);

    // The environment can only request to use VK_EXT_robustness2 when the extension is available.
    // Override the decision if it is not applicable or robustImageAccess2 is false.
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustImageAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseImageRobustAccess2, false);
    }
    // By default try to skip robustness transform on textures according to the Vulkan extension
    // VK_EXT_robustness2.
    deviceToggles->Default(Toggle::VulkanUseImageRobustAccess2, true);
    // The environment can only request to use VK_EXT_robustness2 when the extension is available.
    // Override the decision if it is not applicable or robustBufferAccess2 is false.
    if (!GetDeviceInfo().HasExt(DeviceExt::Robustness2) ||
        GetDeviceInfo().robustness2Features.robustBufferAccess2 == VK_FALSE) {
        deviceToggles->ForceSet(Toggle::VulkanUseBufferRobustAccess2, false);
    }
    // By default try to disable index clamping on the runtime-sized arrays on storage buffers in
    // Tint robustness transform according to the Vulkan extension VK_EXT_robustness2.
    deviceToggles->Default(Toggle::VulkanUseBufferRobustAccess2, true);
}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(AdapterBase* adapter,
                                                                const DeviceDescriptor* descriptor,
                                                                const TogglesState& deviceToggles) {
    return Device::Create(adapter, descriptor, deviceToggles);
}

MaybeError PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

// Android devices with Qualcomm GPUs have a myriad of known issues. (dawn:1549)
bool PhysicalDevice::IsAndroidQualcomm() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsQualcomm(GetVendorId());
#else
    return false;
#endif
}

// Android devices with ARM GPUs have known issues. (dawn:1550)
bool PhysicalDevice::IsAndroidARM() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return gpu_info::IsARM(GetVendorId());
#else
    return false;
#endif
}

bool PhysicalDevice::IsIntelMesa() const {
    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        return mDeviceInfo.driverProperties.driverID == VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA_KHR;
    }
    return false;
}

uint32_t PhysicalDevice::FindDefaultComputeSubgroupSize() const {
    if (!mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        return 0;
    }

    const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& ext =
        mDeviceInfo.subgroupSizeControlProperties;

    if (ext.minSubgroupSize == ext.maxSubgroupSize) {
        return 0;
    }

    // At the moment, only Intel devices support varying subgroup sizes and 16, which is the
    // next value after the minimum of 8, is the sweet spot according to [1]. Hence the
    // following heuristics, which may need to be adjusted in the future for other
    // architectures, or if a specific API is added to let client code select the size.
    //
    // [1] https://bugs.freedesktop.org/show_bug.cgi?id=108875
    uint32_t subgroupSize = ext.minSubgroupSize * 2;
    if (subgroupSize <= ext.maxSubgroupSize) {
        return subgroupSize;
    } else {
        return ext.minSubgroupSize;
    }
}

uint32_t PhysicalDevice::GetDefaultComputeSubgroupSize() const {
    return mDefaultComputeSubgroupSize;
}

}  // namespace dawn::native::vulkan
