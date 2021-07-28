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

        InitializeSupportedExtensions();

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
        // Needed for viewport Y-flip.
        if (!mDeviceInfo.HasExt(DeviceExt::Maintenance1)) {
            return DAWN_INTERNAL_ERROR("Vulkan 1.1 or Vulkan 1.0 with KHR_Maintenance1 required.");
        }

        // Needed for security
        if (!mDeviceInfo.features.robustBufferAccess) {
            return DAWN_INTERNAL_ERROR("Vulkan robustBufferAccess feature required.");
        }

        // TODO(crbug.com/dawn/955): Require BC || (ETC && ASTC) instead.
        if (!mDeviceInfo.features.textureCompressionBC) {
            return DAWN_INTERNAL_ERROR("Vulkan textureCompressionBC feature required.");
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
        if (limits.maxImageDimension1D < kMaxTextureDimension1D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension1D");
        }
        if (limits.maxImageDimension2D < kMaxTextureDimension2D ||
            limits.maxImageDimensionCube < kMaxTextureDimension2D ||
            limits.maxFramebufferWidth < kMaxTextureDimension2D ||
            limits.maxFramebufferHeight < kMaxTextureDimension2D ||
            limits.maxViewportDimensions[0] < kMaxTextureDimension2D ||
            limits.maxViewportDimensions[1] < kMaxTextureDimension2D ||
            limits.viewportBoundsRange[1] < kMaxTextureDimension2D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension2D");
        }
        if (limits.maxImageDimension3D < kMaxTextureDimension3D) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureDimension3D");
        }
        if (limits.maxImageArrayLayers < kMaxTextureArrayLayers) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxTextureArrayLayers");
        }
        if (limits.maxBoundDescriptorSets < kMaxBindGroups) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxBindGroups");
        }
        if (limits.maxDescriptorSetUniformBuffersDynamic <
            kMaxDynamicUniformBuffersPerPipelineLayout) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxDynamicUniformBuffersPerPipelineLayout");
        }
        if (limits.maxDescriptorSetStorageBuffersDynamic <
            kMaxDynamicStorageBuffersPerPipelineLayout) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxDynamicStorageBuffersPerPipelineLayout");
        }
        if (limits.maxPerStageDescriptorSampledImages < kMaxSampledTexturesPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxDynamicStorageBuffersPerPipelineLayout");
        }
        if (limits.maxPerStageDescriptorSampledImages < kMaxSampledTexturesPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxSampledTexturesPerShaderStage");
        }
        if (limits.maxPerStageDescriptorSamplers < kMaxSamplersPerShaderStage) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxSamplersPerShaderStage");
        }
        if (limits.maxPerStageDescriptorStorageBuffers < kMaxStorageBuffersPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageBuffersPerShaderStage");
        }
        if (limits.maxPerStageDescriptorStorageImages < kMaxStorageTexturesPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageTexturesPerShaderStage");
        }
        if (limits.maxPerStageDescriptorUniformBuffers < kMaxUniformBuffersPerShaderStage) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxUniformBuffersPerShaderStage");
        }
        if (limits.maxUniformBufferRange < kMaxUniformBufferBindingSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxUniformBufferBindingSize");
        }
        if (limits.maxStorageBufferRange < kMaxStorageBufferBindingSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxStorageBufferBindingSize");
        }
        if (limits.minUniformBufferOffsetAlignment > kMinUniformBufferOffsetAlignment) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for minUniformBufferOffsetAlignment");
        }
        if (limits.minStorageBufferOffsetAlignment > kMinStorageBufferOffsetAlignment) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for minStorageBufferOffsetAlignment");
        }
        if (limits.maxVertexInputBindings < kMaxVertexBuffers) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBuffers");
        }
        if (limits.maxVertexInputAttributes < kMaxVertexAttributes) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexAttributes");
        }
        if (limits.maxVertexInputBindingStride < kMaxVertexBufferArrayStride ||
            limits.maxVertexInputAttributeOffset < kMaxVertexBufferArrayStride - 1) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxVertexBufferArrayStride");
        }
        if (limits.maxVertexOutputComponents < kMaxInterStageShaderComponents ||
            limits.maxFragmentInputComponents < kMaxInterStageShaderComponents) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxInterStageShaderComponents");
        }
        if (limits.maxComputeSharedMemorySize < kMaxComputeWorkgroupStorageSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupStorageSize");
        }
        if (limits.maxComputeWorkGroupInvocations < kMaxComputeWorkgroupInvocations) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupInvocations");
        }
        if (limits.maxComputeWorkGroupSize[0] < kMaxComputeWorkgroupSizeX ||
            limits.maxComputeWorkGroupSize[1] < kMaxComputeWorkgroupSizeY ||
            limits.maxComputeWorkGroupSize[2] < kMaxComputeWorkgroupSizeZ) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputeWorkgroupSize");
        }
        if (limits.maxComputeWorkGroupCount[0] < kMaxComputePerDimensionDispatchSize ||
            limits.maxComputeWorkGroupCount[1] < kMaxComputePerDimensionDispatchSize ||
            limits.maxComputeWorkGroupCount[2] < kMaxComputePerDimensionDispatchSize) {
            return DAWN_INTERNAL_ERROR(
                "Insufficient Vulkan limits for maxComputePerDimensionDispatchSize");
        }
        if (limits.maxColorAttachments < kMaxColorAttachments) {
            return DAWN_INTERNAL_ERROR("Insufficient Vulkan limits for maxColorAttachments");
        }

        // Only check maxFragmentCombinedOutputResources on mobile GPUs. Desktop GPUs drivers seem
        // to put incorrect values for this limit with things like 8 or 16 when they can do bindless
        // storage buffers.
        uint32_t vendorId = mDeviceInfo.properties.vendorID;
        if (!gpu_info::IsAMD(vendorId) && !gpu_info::IsIntel(vendorId) &&
            !gpu_info::IsNvidia(vendorId)) {
            if (limits.maxFragmentCombinedOutputResources < kMaxColorAttachments +
                                                                kMaxStorageTexturesPerShaderStage +
                                                                kMaxStorageBuffersPerShaderStage) {
                return DAWN_INTERNAL_ERROR(
                    "Insufficient Vulkan maxFragmentCombinedOutputResources limit");
            }
        }

        return {};
    }

    void Adapter::InitializeSupportedExtensions() {
        if (mDeviceInfo.features.textureCompressionBC == VK_TRUE) {
            mSupportedExtensions.EnableExtension(Extension::TextureCompressionBC);
        }

        if (mDeviceInfo.features.pipelineStatisticsQuery == VK_TRUE) {
            mSupportedExtensions.EnableExtension(Extension::PipelineStatisticsQuery);
        }

        if (mDeviceInfo.properties.limits.timestampComputeAndGraphics == VK_TRUE) {
            mSupportedExtensions.EnableExtension(Extension::TimestampQuery);
        }
    }

    ResultOrError<DeviceBase*> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor) {
        return Device::Create(this, descriptor);
    }

}}  // namespace dawn_native::vulkan
