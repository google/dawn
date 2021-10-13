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

#include "dawn_native/vulkan/AdapterVk.h"

#include "dawn_native/Limits.h"
#include "dawn_native/vulkan/BackendVk.h"
#include "dawn_native/vulkan/DeviceVk.h"

#include "common/GPUInfo.h"

namespace dawn_native { namespace vulkan {

    Adapter::Adapter(Backend* backend, VkPhysicalDevice physicalDevice)
        : AdapterBase(backend->GetInstance(), wgpu::BackendType::Vulkan),
          mPhysicalDevice(physicalDevice),
          mBackend(backend) {
    }

    const VulkanDeviceInfo& Adapter::GetDeviceInfo() const {
        return mDeviceInfo;
    }

    VkPhysicalDevice Adapter::GetPhysicalDevice() const {
        return mPhysicalDevice;
    }

    Backend* Adapter::GetBackend() const {
        return mBackend;
    }

    MaybeError Adapter::Initialize() {
        DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this));
        DAWN_TRY(CheckCoreWebGPUSupport());

        if (mDeviceInfo.HasExt(DeviceExt::DriverProperties)) {
            mDriverDescription = mDeviceInfo.driverProperties.driverName;
            if (mDeviceInfo.driverProperties.driverInfo[0] != '\0') {
                mDriverDescription += std::string(": ") + mDeviceInfo.driverProperties.driverInfo;
            }
        } else {
            mDriverDescription =
                "Vulkan driver version: " + std::to_string(mDeviceInfo.properties.driverVersion);
        }

        InitializeSupportedFeatures();

        mPCIInfo.deviceId = mDeviceInfo.properties.deviceID;
        mPCIInfo.vendorId = mDeviceInfo.properties.vendorID;
        mPCIInfo.name = mDeviceInfo.properties.deviceName;

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

    MaybeError Adapter::CheckCoreWebGPUSupport() {
        Limits baseLimits;
        GetDefaultLimits(&baseLimits);

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

        // Check base WebGPU limits are supported.
        const VkPhysicalDeviceLimits& limits = mDeviceInfo.properties.limits;
        if (limits.maxImageDimension1D < baseLimits.maxTextureDimension1D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension1D");
        }
        if (limits.maxImageDimension2D < baseLimits.maxTextureDimension2D ||
            limits.maxImageDimensionCube < baseLimits.maxTextureDimension2D ||
            limits.maxFramebufferWidth < baseLimits.maxTextureDimension2D ||
            limits.maxFramebufferHeight < baseLimits.maxTextureDimension2D ||
            limits.maxViewportDimensions[0] < baseLimits.maxTextureDimension2D ||
            limits.maxViewportDimensions[1] < baseLimits.maxTextureDimension2D ||
            limits.viewportBoundsRange[1] < baseLimits.maxTextureDimension2D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension2D");
        }
        if (limits.maxImageDimension3D < baseLimits.maxTextureDimension3D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension3D");
        }
        if (limits.maxImageArrayLayers < baseLimits.maxTextureArrayLayers) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureArrayLayers");
        }
        if (limits.maxBoundDescriptorSets < baseLimits.maxBindGroups) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxBindGroups");
        }
        if (limits.maxDescriptorSetUniformBuffersDynamic <
            baseLimits.maxDynamicUniformBuffersPerPipelineLayout) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxDynamicUniformBuffersPerPipelineLayout");
        }
        if (limits.maxDescriptorSetStorageBuffersDynamic <
            baseLimits.maxDynamicStorageBuffersPerPipelineLayout) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxDynamicStorageBuffersPerPipelineLayout");
        }
        if (limits.maxPerStageDescriptorSampledImages <
            baseLimits.maxSampledTexturesPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxSampledTexturesPerShaderStage");
        }
        if (limits.maxPerStageDescriptorSamplers < baseLimits.maxSamplersPerShaderStage) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxSamplersPerShaderStage");
        }
        if (limits.maxPerStageDescriptorStorageBuffers <
            baseLimits.maxStorageBuffersPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageBuffersPerShaderStage");
        }
        if (limits.maxPerStageDescriptorStorageImages <
            baseLimits.maxStorageTexturesPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageTexturesPerShaderStage");
        }
        if (limits.maxPerStageDescriptorUniformBuffers <
            baseLimits.maxUniformBuffersPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxUniformBuffersPerShaderStage");
        }
        if (limits.maxUniformBufferRange < baseLimits.maxUniformBufferBindingSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxUniformBufferBindingSize");
        }
        if (limits.maxStorageBufferRange < baseLimits.maxStorageBufferBindingSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageBufferBindingSize");
        }
        if (limits.minUniformBufferOffsetAlignment > baseLimits.minUniformBufferOffsetAlignment) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for minUniformBufferOffsetAlignment");
        }
        if (limits.minStorageBufferOffsetAlignment > baseLimits.minStorageBufferOffsetAlignment) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for minStorageBufferOffsetAlignment");
        }
        if (limits.maxVertexInputBindings < baseLimits.maxVertexBuffers) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBuffers");
        }
        if (limits.maxVertexInputAttributes < baseLimits.maxVertexAttributes) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexAttributes");
        }
        if (limits.maxVertexInputBindingStride < baseLimits.maxVertexBufferArrayStride ||
            limits.maxVertexInputAttributeOffset < baseLimits.maxVertexBufferArrayStride - 1) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBufferArrayStride");
        }
        if (limits.maxVertexOutputComponents < baseLimits.maxInterStageShaderComponents ||
            limits.maxFragmentInputComponents < baseLimits.maxInterStageShaderComponents) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxInterStageShaderComponents");
        }
        if (limits.maxComputeSharedMemorySize < baseLimits.maxComputeWorkgroupStorageSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupStorageSize");
        }
        if (limits.maxComputeWorkGroupInvocations < baseLimits.maxComputeInvocationsPerWorkgroup) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeInvocationsPerWorkgroup");
        }
        if (limits.maxComputeWorkGroupSize[0] < baseLimits.maxComputeWorkgroupSizeX ||
            limits.maxComputeWorkGroupSize[1] < baseLimits.maxComputeWorkgroupSizeY ||
            limits.maxComputeWorkGroupSize[2] < baseLimits.maxComputeWorkgroupSizeZ) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupSize");
        }
        if (limits.maxComputeWorkGroupCount[0] < baseLimits.maxComputeWorkgroupsPerDimension ||
            limits.maxComputeWorkGroupCount[1] < baseLimits.maxComputeWorkgroupsPerDimension ||
            limits.maxComputeWorkGroupCount[2] < baseLimits.maxComputeWorkgroupsPerDimension) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupsPerDimension");
        }
        if (limits.maxColorAttachments < kMaxColorAttachments) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxColorAttachments");
        }
        if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                      limits.framebufferColorSampleCounts)) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for framebufferColorSampleCounts");
        }
        if (!IsSubset(VkSampleCountFlags(VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT),
                      limits.framebufferDepthSampleCounts)) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for framebufferDepthSampleCounts");
        }

        // Only check maxFragmentCombinedOutputResources on mobile GPUs. Desktop GPUs drivers seem
        // to put incorrect values for this limit with things like 8 or 16 when they can do bindless
        // storage buffers.
        uint32_t vendorId = mDeviceInfo.properties.vendorID;
        if (!gpu_info::IsAMD(vendorId) && !gpu_info::IsIntel(vendorId) &&
            !gpu_info::IsNvidia(vendorId)) {
            if (limits.maxFragmentCombinedOutputResources <
                kMaxColorAttachments + baseLimits.maxStorageTexturesPerShaderStage +
                    baseLimits.maxStorageBuffersPerShaderStage) {
                return DAWN_INTERNAL_ERROR(
                    "Insufficient Vulkan maxFragmentCombinedOutputResources limit");
            }
        }

        return {};
    }

    bool Adapter::SupportsExternalImages() const {
        // Via dawn_native::vulkan::WrapVulkanImage
        return external_memory::Service::CheckSupport(mDeviceInfo) &&
               external_semaphore::Service::CheckSupport(mDeviceInfo, mPhysicalDevice,
                                                         mBackend->GetFunctions());
    }

    void Adapter::InitializeSupportedFeatures() {
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
    }

    ResultOrError<DeviceBase*> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor) {
        return Device::Create(this, descriptor);
    }

}}  // namespace dawn_native::vulkan
