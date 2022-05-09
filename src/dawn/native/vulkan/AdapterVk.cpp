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

#include "dawn/native/vulkan/AdapterVk.h"

#include <algorithm>
#include <string>

#include "dawn/native/Limits.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"

#include "dawn/common/GPUInfo.h"

namespace dawn::native::vulkan {

Adapter::Adapter(InstanceBase* instance,
                 VulkanInstance* vulkanInstance,
                 VkPhysicalDevice physicalDevice)
    : AdapterBase(instance, wgpu::BackendType::Vulkan),
      mPhysicalDevice(physicalDevice),
      mVulkanInstance(vulkanInstance) {}

Adapter::~Adapter() = default;

const VulkanDeviceInfo& Adapter::GetDeviceInfo() const {
    return mDeviceInfo;
}

VkPhysicalDevice Adapter::GetPhysicalDevice() const {
    return mPhysicalDevice;
}

VulkanInstance* Adapter::GetVulkanInstance() const {
    return mVulkanInstance.Get();
}

bool Adapter::IsDepthStencilFormatSupported(VkFormat format) {
    ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

    VkFormatProperties properties;
    mVulkanInstance->GetFunctions().GetPhysicalDeviceFormatProperties(mPhysicalDevice, format,
                                                                      &properties);
    return properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

MaybeError Adapter::InitializeImpl() {
    DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));

    if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
        mDriverDescription = mDeviceInfo.driverProperties.driverName;
        if (mDeviceInfo.driverProperties.driverInfo[0] != '\0') {
            mDriverDescription += std::string(": ") + mDeviceInfo.driverProperties.driverInfo;
        }
    } else {
        mDriverDescription =
            "Vulkan driver version: " + std::to_string(mDeviceInfo.properties.driverVersion);
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

    return {};
}

MaybeError Adapter::InitializeSupportedFeaturesImpl() {
    // Needed for viewport Y-flip.
    if (!mDeviceInfo.HasExt(DeviceExt::Maintenance1)) {
        return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance1 required.");
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

    // Initialize supported extensions
    if (mDeviceInfo.features.textureCompressionBC == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::TextureCompressionBC);
    }

    if (mDeviceInfo.features.textureCompressionETC2 == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::TextureCompressionETC2);
    }

    if (mDeviceInfo.features.textureCompressionASTC_LDR == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::TextureCompressionASTC);
    }

    if (mDeviceInfo.features.pipelineStatisticsQuery == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::PipelineStatisticsQuery);
    }

    if (mDeviceInfo.features.depthClamp == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::DepthClamping);
    }

    if (mDeviceInfo.properties.limits.timestampComputeAndGraphics == VK_TRUE) {
        mSupportedFeatures.EnableFeature(Feature::TimestampQuery);
    }

    if (IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT)) {
        mSupportedFeatures.EnableFeature(Feature::Depth24UnormStencil8);
    }

    if (IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT)) {
        mSupportedFeatures.EnableFeature(Feature::Depth32FloatStencil8);
    }

#if defined(DAWN_USE_SYNC_FDS)
    // TODO(chromium:1258986): Precisely enable the feature by querying the device's format
    // features.
    mSupportedFeatures.EnableFeature(Feature::MultiPlanarFormats);
#endif

    return {};
}

MaybeError Adapter::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimits(&limits->v1);
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

    if (vkLimits.maxColorAttachments < kMaxColorAttachments) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxColorAttachments");
    }
    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferColorSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferColorSampleCounts");
    }
    if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                  vkLimits.framebufferDepthSampleCounts)) {
        return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for framebufferDepthSampleCounts");
    }

    // Only check maxFragmentCombinedOutputResources on mobile GPUs. Desktop GPUs drivers seem
    // to put incorrect values for this limit with things like 8 or 16 when they can do bindless
    // storage buffers. Mesa llvmpipe driver also puts 8 here.
    uint32_t vendorId = mDeviceInfo.properties.vendorID;
    if (!gpu_info::IsAMD(vendorId) && !gpu_info::IsIntel(vendorId) && !gpu_info::IsMesa(vendorId) &&
        !gpu_info::IsNvidia(vendorId)) {
        if (vkLimits.maxFragmentCombinedOutputResources <
            kMaxColorAttachments + baseLimits.v1.maxStorageTexturesPerShaderStage +
                baseLimits.v1.maxStorageBuffersPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan maxFragmentCombinedOutputResources limit");
        }

        uint32_t maxFragmentCombinedOutputResources = kMaxColorAttachments +
                                                      limits->v1.maxStorageTexturesPerShaderStage +
                                                      limits->v1.maxStorageBuffersPerShaderStage;

        if (maxFragmentCombinedOutputResources > vkLimits.maxFragmentCombinedOutputResources) {
            // WebGPU's maxFragmentCombinedOutputResources exceeds the Vulkan limit.
            // Decrease |maxStorageTexturesPerShaderStage| and |maxStorageBuffersPerShaderStage|
            // to fit within the Vulkan limit.
            uint32_t countOverLimit =
                maxFragmentCombinedOutputResources - vkLimits.maxFragmentCombinedOutputResources;

            uint32_t maxStorageTexturesOverBase = limits->v1.maxStorageTexturesPerShaderStage -
                                                  baseLimits.v1.maxStorageTexturesPerShaderStage;
            uint32_t maxStorageBuffersOverBase = limits->v1.maxStorageBuffersPerShaderStage -
                                                 baseLimits.v1.maxStorageBuffersPerShaderStage;

            // Reduce the number of resources by half the overage count, but clamp to
            // to ensure we don't go below the base limits.
            uint32_t numFewerStorageTextures =
                std::min(countOverLimit / 2, maxStorageTexturesOverBase);
            uint32_t numFewerStorageBuffers =
                std::min((countOverLimit + 1) / 2, maxStorageBuffersOverBase);

            if (numFewerStorageTextures == maxStorageTexturesOverBase) {
                // If |numFewerStorageTextures| was clamped, subtract the remaining
                // from the storage buffers.
                numFewerStorageBuffers = countOverLimit - numFewerStorageTextures;
                ASSERT(numFewerStorageBuffers <= maxStorageBuffersOverBase);
            } else if (numFewerStorageBuffers == maxStorageBuffersOverBase) {
                // If |numFewerStorageBuffers| was clamped, subtract the remaining
                // from the storage textures.
                numFewerStorageTextures = countOverLimit - numFewerStorageBuffers;
                ASSERT(numFewerStorageTextures <= maxStorageTexturesOverBase);
            }
            limits->v1.maxStorageTexturesPerShaderStage -= numFewerStorageTextures;
            limits->v1.maxStorageBuffersPerShaderStage -= numFewerStorageBuffers;
        }
    }

    return {};
}

bool Adapter::SupportsExternalImages() const {
    // Via dawn::native::vulkan::WrapVulkanImage
    return external_memory::Service::CheckSupport(mDeviceInfo) &&
           external_semaphore::Service::CheckSupport(mDeviceInfo, mPhysicalDevice,
                                                     mVulkanInstance->GetFunctions());
}

ResultOrError<Ref<DeviceBase>> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor) {
    return Device::Create(this, descriptor);
}

}  // namespace dawn::native::vulkan
