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

#include "dawn/native/Adapter.h"

#include <algorithm>
#include <memory>

#include "dawn/common/Constants.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

AdapterBase::AdapterBase(InstanceBase* instance, wgpu::BackendType backend)
    : mInstance(instance), mBackend(backend) {
    mSupportedFeatures.EnableFeature(Feature::DawnNative);
    mSupportedFeatures.EnableFeature(Feature::DawnInternalUsages);
}

AdapterBase::~AdapterBase() = default;

MaybeError AdapterBase::Initialize() {
    DAWN_TRY_CONTEXT(InitializeImpl(), "initializing adapter (backend=%s)", mBackend);
    InitializeVendorArchitectureImpl();

    InitializeSupportedFeaturesImpl();

    DAWN_TRY_CONTEXT(
        InitializeSupportedLimitsImpl(&mLimits),
        "gathering supported limits for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
        "backend=%s type=%s)",
        mName, mDriverDescription, mVendorId, mDeviceId, mBackend, mAdapterType);

    // Enforce internal Dawn constants for some limits to ensure they don't go over fixed-size
    // arrays in Dawn's internal code.
    mLimits.v1.maxVertexBufferArrayStride =
        std::min(mLimits.v1.maxVertexBufferArrayStride, kMaxVertexBufferArrayStride);
    mLimits.v1.maxColorAttachments =
        std::min(mLimits.v1.maxColorAttachments, uint32_t(kMaxColorAttachments));
    mLimits.v1.maxBindGroups = std::min(mLimits.v1.maxBindGroups, kMaxBindGroups);
    mLimits.v1.maxVertexAttributes =
        std::min(mLimits.v1.maxVertexAttributes, uint32_t(kMaxVertexAttributes));
    mLimits.v1.maxVertexBuffers =
        std::min(mLimits.v1.maxVertexBuffers, uint32_t(kMaxVertexBuffers));
    mLimits.v1.maxInterStageShaderComponents =
        std::min(mLimits.v1.maxInterStageShaderComponents, kMaxInterStageShaderComponents);
    mLimits.v1.maxSampledTexturesPerShaderStage =
        std::min(mLimits.v1.maxSampledTexturesPerShaderStage, kMaxSampledTexturesPerShaderStage);
    mLimits.v1.maxSamplersPerShaderStage =
        std::min(mLimits.v1.maxSamplersPerShaderStage, kMaxSamplersPerShaderStage);
    mLimits.v1.maxStorageBuffersPerShaderStage =
        std::min(mLimits.v1.maxStorageBuffersPerShaderStage, kMaxStorageBuffersPerShaderStage);
    mLimits.v1.maxStorageTexturesPerShaderStage =
        std::min(mLimits.v1.maxStorageTexturesPerShaderStage, kMaxStorageTexturesPerShaderStage);
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

bool AdapterBase::APIGetLimits(SupportedLimits* limits) const {
    return GetLimits(limits);
}

void AdapterBase::APIGetProperties(AdapterProperties* properties) const {
    MaybeError result = ValidateSingleSType(properties->nextInChain,
                                            wgpu::SType::DawnAdapterPropertiesPowerPreference);
    if (result.IsError()) {
        mInstance->ConsumedError(result.AcquireError());
        return;
    }

    DawnAdapterPropertiesPowerPreference* powerPreferenceDesc = nullptr;
    FindInChain(properties->nextInChain, &powerPreferenceDesc);
    if (powerPreferenceDesc != nullptr) {
        powerPreferenceDesc->powerPreference = wgpu::PowerPreference::Undefined;
    }

    properties->vendorID = mVendorId;
    properties->vendorName = mVendorName.c_str();
    properties->architecture = mArchitectureName.c_str();
    properties->deviceID = mDeviceId;
    properties->name = mName.c_str();
    properties->driverDescription = mDriverDescription.c_str();
    properties->adapterType = mAdapterType;
    properties->backendType = mBackend;
}

bool AdapterBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mSupportedFeatures.IsEnabled(feature);
}

size_t AdapterBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mSupportedFeatures.EnumerateFeatures(features);
}

DeviceBase* AdapterBase::APICreateDevice(const DeviceDescriptor* descriptor) {
    DeviceDescriptor defaultDesc = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDesc;
    }
    auto result = CreateDeviceInternal(descriptor);
    if (result.IsError()) {
        mInstance->ConsumedError(result.AcquireError());
        return nullptr;
    }
    return result.AcquireSuccess().Detach();
}

void AdapterBase::APIRequestDevice(const DeviceDescriptor* descriptor,
                                   WGPURequestDeviceCallback callback,
                                   void* userdata) {
    static constexpr DeviceDescriptor kDefaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDescriptor;
    }
    auto result = CreateDeviceInternal(descriptor);

    if (result.IsError()) {
        std::unique_ptr<ErrorData> errorData = result.AcquireError();
        // TODO(crbug.com/dawn/1122): Call callbacks only on wgpuInstanceProcessEvents
        callback(WGPURequestDeviceStatus_Error, nullptr, errorData->GetFormattedMessage().c_str(),
                 userdata);
        return;
    }

    Ref<DeviceBase> device = result.AcquireSuccess();

    WGPURequestDeviceStatus status =
        device == nullptr ? WGPURequestDeviceStatus_Unknown : WGPURequestDeviceStatus_Success;
    // TODO(crbug.com/dawn/1122): Call callbacks only on wgpuInstanceProcessEvents
    callback(status, ToAPI(device.Detach()), nullptr, userdata);
}

void AdapterBase::InitializeVendorArchitectureImpl() {
    mVendorName = gpu_info::GetVendorName(mVendorId);
    mArchitectureName = gpu_info::GetArchitectureName(mVendorId, mDeviceId);
}

uint32_t AdapterBase::GetVendorId() const {
    return mVendorId;
}

uint32_t AdapterBase::GetDeviceId() const {
    return mDeviceId;
}

const gpu_info::DriverVersion& AdapterBase::GetDriverVersion() const {
    return mDriverVersion;
}

wgpu::BackendType AdapterBase::GetBackendType() const {
    return mBackend;
}

InstanceBase* AdapterBase::GetInstance() const {
    return mInstance.Get();
}

FeaturesSet AdapterBase::GetSupportedFeatures() const {
    return mSupportedFeatures;
}

bool AdapterBase::SupportsAllRequiredFeatures(
    const ityp::span<size_t, const wgpu::FeatureName>& features) const {
    for (wgpu::FeatureName f : features) {
        if (!mSupportedFeatures.IsEnabled(f)) {
            return false;
        }
    }
    return true;
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

MaybeError AdapterBase::ValidateFeatureSupportedWithDeviceToggles(
    wgpu::FeatureName feature,
    const TogglesState& deviceTogglesState) {
    DAWN_TRY(ValidateFeatureName(feature));
    DAWN_INVALID_IF(!mSupportedFeatures.IsEnabled(feature),
                    "Requested feature %s is not supported.", feature);

    const FeatureInfo* featureInfo = GetInstance()->GetFeatureInfo(feature);
    // Experimental features are guarded by toggle DisallowUnsafeAPIs.
    if (featureInfo->featureState == FeatureInfo::FeatureState::Experimental) {
        // DisallowUnsafeAPIs toggle is by default enabled if not explicitly disabled.
        DAWN_INVALID_IF(deviceTogglesState.IsEnabled(Toggle::DisallowUnsafeAPIs),
                        "Feature %s is guarded by toggle disallow_unsafe_apis.", featureInfo->name);
    }

    // Do backend-specific validation.
    return ValidateFeatureSupportedWithDeviceTogglesImpl(feature, deviceTogglesState);
}

ResultOrError<Ref<DeviceBase>> AdapterBase::CreateDeviceInternal(
    const DeviceDescriptor* descriptor) {
    ASSERT(descriptor != nullptr);

    // Create device toggles state from required toggles descriptor.
    // TODO(dawn:1495): After implementing adapter toggles, also inherite adapter toggles state.
    const DawnTogglesDescriptor* deviceTogglesDesc = nullptr;
    FindInChain(descriptor->nextInChain, &deviceTogglesDesc);

    // Handle the deprecated DawnTogglesDeviceDescriptor
    // TODO(dawn:1495): Remove this fallback once Chromium is changed to use DawnToggleDescriptor
    // and DawnTogglesDeviceDescriptor is removed.
    const DawnTogglesDeviceDescriptor* deprecatedTogglesDeviceDesc = nullptr;
    DawnTogglesDescriptor convertedDeviceTogglesDesc = {};
    FindInChain(descriptor->nextInChain, &deprecatedTogglesDeviceDesc);

    if (deprecatedTogglesDeviceDesc) {
        // Emit the deprecation warning.
        dawn::WarningLog()
            << "DawnTogglesDeviceDescriptor is deprecated and replaced by DawnTogglesDescriptor.";
        // Ensure that at most one toggles descriptor is used.
        DAWN_INVALID_IF(
            deviceTogglesDesc && deprecatedTogglesDeviceDesc,
            "DawnTogglesDeviceDescriptor should not be used together with DawnTogglesDescriptor.");

        convertedDeviceTogglesDesc.enabledToggles =
            deprecatedTogglesDeviceDesc->forceEnabledToggles;
        convertedDeviceTogglesDesc.enabledTogglesCount =
            deprecatedTogglesDeviceDesc->forceEnabledTogglesCount;
        convertedDeviceTogglesDesc.disabledToggles =
            deprecatedTogglesDeviceDesc->forceDisabledToggles;
        convertedDeviceTogglesDesc.disabledTogglesCount =
            deprecatedTogglesDeviceDesc->forceDisabledTogglesCount;
        deviceTogglesDesc = &convertedDeviceTogglesDesc;
    }

    // Create device toggles state from user-given toggles descriptor, and set up forced and default
    // toggles.
    // TODO(dawn:1495): After implementing adapter toggles, device toggles state should also inherit
    // from adapter toggles state.
    TogglesState deviceToggles =
        TogglesState::CreateFromTogglesDescriptor(deviceTogglesDesc, ToggleStage::Device);
    // Default toggles for all backend
    deviceToggles.Default(Toggle::LazyClearResourceOnFirstUse, true);
    deviceToggles.Default(Toggle::DisallowUnsafeAPIs, true);
    // Backend-specific forced and default device toggles
    SetupBackendDeviceToggles(&deviceToggles);

    // Validate all required features are supported by the adapter and suitable under given toggles.
    // TODO(dawn:1495): After implementing adapter toggles, validate supported features using
    // adapter toggles instead of device toggles.
    for (uint32_t i = 0; i < descriptor->requiredFeaturesCount; ++i) {
        wgpu::FeatureName feature = descriptor->requiredFeatures[i];
        DAWN_TRY(ValidateFeatureSupportedWithDeviceToggles(feature, deviceToggles));
    }

    if (descriptor->requiredLimits != nullptr) {
        DAWN_TRY_CONTEXT(ValidateLimits(mUseTieredLimits ? ApplyLimitTiers(mLimits.v1) : mLimits.v1,
                                        descriptor->requiredLimits->limits),
                         "validating required limits");

        DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                        "nextInChain is not nullptr.");
    }
    return CreateDeviceImpl(descriptor, deviceToggles);
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

}  // namespace dawn::native
