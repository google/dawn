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

#include "dawn/native/PhysicalDevice.h"

#include <algorithm>
#include <memory>

#include "dawn/common/Constants.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

PhysicalDeviceBase::PhysicalDeviceBase(InstanceBase* instance, wgpu::BackendType backend)
    : mInstance(instance), mBackend(backend) {}

PhysicalDeviceBase::~PhysicalDeviceBase() = default;

MaybeError PhysicalDeviceBase::Initialize() {
    DAWN_TRY_CONTEXT(InitializeImpl(), "initializing adapter (backend=%s)", mBackend);
    InitializeVendorArchitectureImpl();

    EnableFeature(Feature::DawnNative);
    EnableFeature(Feature::DawnInternalUsages);
    EnableFeature(Feature::ImplicitDeviceSynchronization);
    EnableFeature(Feature::ChromiumExperimentalReadWriteStorageTexture);
    InitializeSupportedFeaturesImpl();

    DAWN_TRY_CONTEXT(
        InitializeSupportedLimitsImpl(&mLimits),
        "gathering supported limits for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
        "backend=%s type=%s)",
        mName, mDriverDescription, mVendorId, mDeviceId, mBackend, mAdapterType);

    NormalizeLimits(&mLimits.v1);

    return {};
}

ResultOrError<Ref<DeviceBase>> PhysicalDeviceBase::CreateDevice(AdapterBase* adapter,
                                                                const DeviceDescriptor* descriptor,
                                                                const TogglesState& deviceToggles) {
    return CreateDeviceImpl(adapter, descriptor, deviceToggles);
}

void PhysicalDeviceBase::InitializeVendorArchitectureImpl() {
    mVendorName = gpu_info::GetVendorName(mVendorId);
    mArchitectureName = gpu_info::GetArchitectureName(mVendorId, mDeviceId);
}

uint32_t PhysicalDeviceBase::GetVendorId() const {
    return mVendorId;
}

uint32_t PhysicalDeviceBase::GetDeviceId() const {
    return mDeviceId;
}

const std::string& PhysicalDeviceBase::GetVendorName() const {
    return mVendorName;
}

const std::string& PhysicalDeviceBase::GetArchitectureName() const {
    return mArchitectureName;
}

const std::string& PhysicalDeviceBase::GetName() const {
    return mName;
}

const gpu_info::DriverVersion& PhysicalDeviceBase::GetDriverVersion() const {
    return mDriverVersion;
}

const std::string& PhysicalDeviceBase::GetDriverDescription() const {
    return mDriverDescription;
}

wgpu::AdapterType PhysicalDeviceBase::GetAdapterType() const {
    return mAdapterType;
}

wgpu::BackendType PhysicalDeviceBase::GetBackendType() const {
    return mBackend;
}

InstanceBase* PhysicalDeviceBase::GetInstance() const {
    return mInstance.Get();
}

bool PhysicalDeviceBase::IsFeatureSupportedWithToggles(wgpu::FeatureName feature,
                                                       const TogglesState& toggles) const {
    MaybeError validateResult = ValidateFeatureSupportedWithToggles(feature, toggles);
    if (validateResult.IsError()) {
        validateResult.AcquireError();
        return false;
    } else {
        return true;
    }
}

void PhysicalDeviceBase::GetDefaultLimitsForSupportedFeatureLevel(Limits* limits) const {
    // If the physical device does not support core then the defaults are compat defaults.
    GetDefaultLimits(limits, SupportsFeatureLevel(FeatureLevel::Core)
                                 ? FeatureLevel::Core
                                 : FeatureLevel::Compatibility);
}

FeaturesSet PhysicalDeviceBase::GetSupportedFeatures(const TogglesState& toggles) const {
    FeaturesSet supportedFeaturesWithToggles;
    // Iterate each PhysicalDevice's supported feature and check if it is supported with given
    // toggles
    for (Feature feature : IterateBitSet(mSupportedFeatures.featuresBitSet)) {
        if (IsFeatureSupportedWithToggles(ToAPI(feature), toggles)) {
            supportedFeaturesWithToggles.EnableFeature(feature);
        }
    }
    return supportedFeaturesWithToggles;
}

bool PhysicalDeviceBase::SupportsAllRequiredFeatures(
    const ityp::span<size_t, const wgpu::FeatureName>& features,
    const TogglesState& toggles) const {
    for (wgpu::FeatureName f : features) {
        if (!IsFeatureSupportedWithToggles(f, toggles)) {
            return false;
        }
    }
    return true;
}

const CombinedLimits& PhysicalDeviceBase::GetLimits() const {
    return mLimits;
}

void PhysicalDeviceBase::EnableFeature(Feature feature) {
    mSupportedFeatures.EnableFeature(feature);
}

MaybeError PhysicalDeviceBase::ValidateFeatureSupportedWithToggles(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    DAWN_TRY(ValidateFeatureName(feature));
    DAWN_INVALID_IF(!mSupportedFeatures.IsEnabled(feature),
                    "Requested feature %s is not supported.", feature);

    const FeatureInfo* featureInfo = GetInstance()->GetFeatureInfo(feature);
    // Experimental features are guarded by the AllowUnsafeAPIs toggle.
    if (featureInfo->featureState == FeatureInfo::FeatureState::Experimental) {
        // AllowUnsafeAPIs toggle is by default disabled if not explicitly enabled.
        DAWN_INVALID_IF(!toggles.IsEnabled(Toggle::AllowUnsafeAPIs),
                        "Feature %s is guarded by toggle allow_unsafe_apis.", featureInfo->name);
    }

    // Do backend-specific validation.
    return ValidateFeatureSupportedWithTogglesImpl(feature, toggles);
}

void PhysicalDeviceBase::SetSupportedFeaturesForTesting(
    const std::vector<wgpu::FeatureName>& requiredFeatures) {
    mSupportedFeatures = {};
    for (wgpu::FeatureName f : requiredFeatures) {
        mSupportedFeatures.EnableFeature(f);
    }
}

void PhysicalDeviceBase::ResetInternalDeviceForTesting() {
    mInstance->ConsumedError(ResetInternalDeviceForTestingImpl());
}

MaybeError PhysicalDeviceBase::ResetInternalDeviceForTestingImpl() {
    return DAWN_INTERNAL_ERROR(
        "ResetInternalDeviceForTesting should only be used with the D3D12 backend.");
}

}  // namespace dawn::native
