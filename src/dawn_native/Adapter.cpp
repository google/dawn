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

#include "dawn_native/Adapter.h"

#include "common/Constants.h"
#include "dawn_native/Instance.h"

namespace dawn_native {

    AdapterBase::AdapterBase(InstanceBase* instance, wgpu::BackendType backend)
        : mInstance(instance), mBackend(backend) {
        mSupportedFeatures.EnableFeature(Feature::DawnInternalUsages);
    }

    MaybeError AdapterBase::Initialize() {
        DAWN_TRY_CONTEXT(InitializeImpl(), "initializing adapter (backend=%s)", mBackend);
        DAWN_TRY_CONTEXT(
            InitializeSupportedFeaturesImpl(),
            "gathering supported features for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
            "backend=%s type=%s)",
            mPCIInfo.name, mDriverDescription, mPCIInfo.vendorId, mPCIInfo.deviceId, mBackend,
            mAdapterType);
        DAWN_TRY_CONTEXT(
            InitializeSupportedLimitsImpl(&mLimits),
            "gathering supported limits for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
            "backend=%s type=%s)",
            mPCIInfo.name, mDriverDescription, mPCIInfo.vendorId, mPCIInfo.deviceId, mBackend,
            mAdapterType);

        // Enforce internal Dawn constants.
        mLimits.v1.maxVertexBufferArrayStride =
            std::min(mLimits.v1.maxVertexBufferArrayStride, kMaxVertexBufferArrayStride);
        mLimits.v1.maxBindGroups = std::min(mLimits.v1.maxBindGroups, kMaxBindGroups);
        mLimits.v1.maxVertexAttributes =
            std::min(mLimits.v1.maxVertexAttributes, uint32_t(kMaxVertexAttributes));
        mLimits.v1.maxVertexBuffers =
            std::min(mLimits.v1.maxVertexBuffers, uint32_t(kMaxVertexBuffers));
        mLimits.v1.maxInterStageShaderComponents =
            std::min(mLimits.v1.maxInterStageShaderComponents, kMaxInterStageShaderComponents);
        mLimits.v1.maxSampledTexturesPerShaderStage = std::min(
            mLimits.v1.maxSampledTexturesPerShaderStage, kMaxSampledTexturesPerShaderStage);
        mLimits.v1.maxSamplersPerShaderStage =
            std::min(mLimits.v1.maxSamplersPerShaderStage, kMaxSamplersPerShaderStage);
        mLimits.v1.maxStorageBuffersPerShaderStage =
            std::min(mLimits.v1.maxStorageBuffersPerShaderStage, kMaxStorageBuffersPerShaderStage);
        mLimits.v1.maxStorageTexturesPerShaderStage = std::min(
            mLimits.v1.maxStorageTexturesPerShaderStage, kMaxStorageTexturesPerShaderStage);
        mLimits.v1.maxUniformBuffersPerShaderStage =
            std::min(mLimits.v1.maxUniformBuffersPerShaderStage, kMaxUniformBuffersPerShaderStage);
        mLimits.v1.maxDynamicUniformBuffersPerPipelineLayout =
            std::min(mLimits.v1.maxDynamicUniformBuffersPerPipelineLayout,
                     kMaxDynamicUniformBuffersPerPipelineLayout);
        mLimits.v1.maxDynamicStorageBuffersPerPipelineLayout =
            std::min(mLimits.v1.maxDynamicStorageBuffersPerPipelineLayout,
                     kMaxDynamicStorageBuffersPerPipelineLayout);

        return {};
    }

    wgpu::BackendType AdapterBase::GetBackendType() const {
        return mBackend;
    }

    wgpu::AdapterType AdapterBase::GetAdapterType() const {
        return mAdapterType;
    }

    const std::string& AdapterBase::GetDriverDescription() const {
        return mDriverDescription;
    }

    const PCIInfo& AdapterBase::GetPCIInfo() const {
        return mPCIInfo;
    }

    InstanceBase* AdapterBase::GetInstance() const {
        return mInstance;
    }

    FeaturesSet AdapterBase::GetSupportedFeatures() const {
        return mSupportedFeatures;
    }

    bool AdapterBase::SupportsAllRequestedFeatures(
        const std::vector<const char*>& requestedFeatures) const {
        for (const char* featureStr : requestedFeatures) {
            Feature featureEnum = mInstance->FeatureNameToEnum(featureStr);
            if (featureEnum == Feature::InvalidEnum) {
                return false;
            }
            if (!mSupportedFeatures.IsEnabled(featureEnum)) {
                return false;
            }
        }
        return true;
    }

    WGPUDeviceProperties AdapterBase::GetAdapterProperties() const {
        WGPUDeviceProperties adapterProperties = {};
        adapterProperties.deviceID = mPCIInfo.deviceId;
        adapterProperties.vendorID = mPCIInfo.vendorId;
        adapterProperties.adapterType = static_cast<WGPUAdapterType>(mAdapterType);

        mSupportedFeatures.InitializeDeviceProperties(&adapterProperties);
        // This is OK for now because there are no limit feature structs.
        // If we add additional structs, the caller will need to provide memory
        // to store them (ex. by calling GetLimits directly instead). Currently,
        // we keep this function as it's only used internally in Chromium to
        // send the adapter properties across the wire.
        GetLimits(FromAPI(&adapterProperties.limits));
        return adapterProperties;
    }

    bool AdapterBase::GetLimits(SupportedLimits* limits) const {
        ASSERT(limits != nullptr);
        if (limits->nextInChain != nullptr) {
            return false;
        }
        if (mUseTieredLimits) {
            limits->limits = ApplyLimitTiers(mLimits.v1);
        } else {
            limits->limits = mLimits.v1;
        }
        return true;
    }

    DeviceBase* AdapterBase::CreateDevice(const DawnDeviceDescriptor* descriptor) {
        DeviceBase* result = nullptr;

        if (mInstance->ConsumedError(CreateDeviceInternal(&result, descriptor))) {
            return nullptr;
        }

        return result;
    }

    void AdapterBase::RequestDevice(const DawnDeviceDescriptor* descriptor,
                                    WGPURequestDeviceCallback callback,
                                    void* userdata) {
        DeviceBase* device = nullptr;
        MaybeError err = CreateDeviceInternal(&device, descriptor);

        if (err.IsError()) {
            std::unique_ptr<ErrorData> errorData = err.AcquireError();
            callback(WGPURequestDeviceStatus_Error, ToAPI(device),
                     errorData->GetFormattedMessage().c_str(), userdata);
            return;
        }
        WGPURequestDeviceStatus status =
            device == nullptr ? WGPURequestDeviceStatus_Unknown : WGPURequestDeviceStatus_Success;
        callback(status, ToAPI(device), nullptr, userdata);
    }

    MaybeError AdapterBase::CreateDeviceInternal(DeviceBase** result,
                                                 const DawnDeviceDescriptor* descriptor) {
        if (descriptor != nullptr) {
            for (const char* featureStr : descriptor->requiredFeatures) {
                Feature featureEnum = mInstance->FeatureNameToEnum(featureStr);
                DAWN_INVALID_IF(featureEnum == Feature::InvalidEnum,
                                "Requested feature %s is unknown.", featureStr);
                DAWN_INVALID_IF(!mSupportedFeatures.IsEnabled(featureEnum),
                                "Requested feature %s is disabled.", featureStr);
            }
        }

        if (descriptor != nullptr && descriptor->requiredLimits != nullptr) {
            DAWN_TRY_CONTEXT(
                ValidateLimits(mUseTieredLimits ? ApplyLimitTiers(mLimits.v1) : mLimits.v1,
                               FromAPI(descriptor->requiredLimits)->limits),
                "validating required limits");

            DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                            "nextInChain is not nullptr.");
        }

        DAWN_TRY_ASSIGN(*result, CreateDeviceImpl(descriptor));
        return {};
    }

    void AdapterBase::SetUseTieredLimits(bool useTieredLimits) {
        mUseTieredLimits = useTieredLimits;
    }

    void AdapterBase::ResetInternalDeviceForTesting() {
        mInstance->ConsumedError(ResetInternalDeviceForTestingImpl());
    }

    MaybeError AdapterBase::ResetInternalDeviceForTestingImpl() {
        return DAWN_INTERNAL_ERROR(
            "ResetInternalDeviceForTesting should only be used with the D3D12 backend.");
    }

}  // namespace dawn_native
