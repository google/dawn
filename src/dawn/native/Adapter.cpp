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

#include "dawn/native/Adapter.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/PhysicalDevice.h"

namespace dawn::native {

AdapterBase::AdapterBase(Ref<PhysicalDeviceBase> physicalDevice,
                         FeatureLevel featureLevel,
                         const TogglesState& requiredAdapterToggles,
                         wgpu::PowerPreference powerPreference)
    : mPhysicalDevice(std::move(physicalDevice)),
      mFeatureLevel(featureLevel),
      mTogglesState(requiredAdapterToggles),
      mPowerPreference(powerPreference) {
    DAWN_ASSERT(mPhysicalDevice->SupportsFeatureLevel(featureLevel));
    DAWN_ASSERT(mTogglesState.GetStage() == ToggleStage::Adapter);
    // Cache the supported features of this adapter. Note that with device toggles overriding, a
    // device created by this adapter may support features not in this set and vice versa.
    mSupportedFeatures = mPhysicalDevice->GetSupportedFeatures(mTogglesState);
}

AdapterBase::~AdapterBase() = default;

void AdapterBase::SetUseTieredLimits(bool useTieredLimits) {
    mUseTieredLimits = useTieredLimits;
}

FeaturesSet AdapterBase::GetSupportedFeatures() const {
    return mSupportedFeatures;
}

PhysicalDeviceBase* AdapterBase::GetPhysicalDevice() {
    return mPhysicalDevice.Get();
}

InstanceBase* AdapterBase::APIGetInstance() const {
    InstanceBase* instance = mPhysicalDevice->GetInstance();
    DAWN_ASSERT(instance != nullptr);
    instance->APIReference();
    return instance;
}

bool AdapterBase::APIGetLimits(SupportedLimits* limits) const {
    DAWN_ASSERT(limits != nullptr);
    // TODO(dawn:1955): Revisit after deciding how to improve the validation for ChainedStructOut.
    MaybeError result =
        ValidateSTypes(limits->nextInChain, {{wgpu::SType::DawnExperimentalSubgroupLimits}});
    if (mPhysicalDevice->GetInstance()->ConsumedError(std::move(result))) {
        return false;
    }
    if (mUseTieredLimits) {
        limits->limits = ApplyLimitTiers(mPhysicalDevice->GetLimits().v1);
    } else {
        limits->limits = mPhysicalDevice->GetLimits().v1;
    }
    for (auto* chain = limits->nextInChain; chain; chain = chain->nextInChain) {
        wgpu::ChainedStructOut originalChain = *chain;
        switch (chain->sType) {
            case (wgpu::SType::DawnExperimentalSubgroupLimits): {
                DawnExperimentalSubgroupLimits* subgroupLimits =
                    reinterpret_cast<DawnExperimentalSubgroupLimits*>(chain);
                if (!mTogglesState.IsEnabled(Toggle::AllowUnsafeAPIs)) {
                    // If AllowUnsafeAPIs is not enabled, return the default-initialized
                    // DawnExperimentalSubgroupLimits object, where minSubgroupSize and
                    // maxSubgroupSize are WGPU_LIMIT_U32_UNDEFINED.
                    *subgroupLimits = DawnExperimentalSubgroupLimits{};
                } else {
                    *subgroupLimits = mPhysicalDevice->GetLimits().experimentalSubgroupLimits;
                }
                break;
            }
            default:
                // ValidateSTypes ensures that all chained sTypes are known.
                DAWN_UNREACHABLE();
        }
        // Recover the original chain
        *chain = originalChain;
    }
    return true;
}

void AdapterBase::APIGetProperties(AdapterProperties* properties) const {
    DAWN_ASSERT(properties != nullptr);

    MaybeError result = ValidateSingleSType(properties->nextInChain,
                                            wgpu::SType::DawnAdapterPropertiesPowerPreference);
    if (result.IsError()) {
        mPhysicalDevice->GetInstance()->ConsumedError(result.AcquireError());
        return;
    }

    DawnAdapterPropertiesPowerPreference* powerPreferenceDesc = nullptr;
    FindInChain(properties->nextInChain, &powerPreferenceDesc);

    if (powerPreferenceDesc != nullptr) {
        powerPreferenceDesc->powerPreference = mPowerPreference;
    }
    properties->vendorID = mPhysicalDevice->GetVendorId();
    properties->deviceID = mPhysicalDevice->GetDeviceId();
    properties->adapterType = mPhysicalDevice->GetAdapterType();
    properties->backendType = mPhysicalDevice->GetBackendType();
    properties->compatibilityMode = mFeatureLevel == FeatureLevel::Compatibility;

    // Get lengths, with null terminators.
    size_t vendorNameCLen = mPhysicalDevice->GetVendorName().length() + 1;
    size_t architectureCLen = mPhysicalDevice->GetArchitectureName().length() + 1;
    size_t nameCLen = mPhysicalDevice->GetName().length() + 1;
    size_t driverDescriptionCLen = mPhysicalDevice->GetDriverDescription().length() + 1;

    // Allocate space for all strings.
    char* ptr = new char[vendorNameCLen + architectureCLen + nameCLen + driverDescriptionCLen];

    properties->vendorName = ptr;
    memcpy(ptr, mPhysicalDevice->GetVendorName().c_str(), vendorNameCLen);
    ptr += vendorNameCLen;

    properties->architecture = ptr;
    memcpy(ptr, mPhysicalDevice->GetArchitectureName().c_str(), architectureCLen);
    ptr += architectureCLen;

    properties->name = ptr;
    memcpy(ptr, mPhysicalDevice->GetName().c_str(), nameCLen);
    ptr += nameCLen;

    properties->driverDescription = ptr;
    memcpy(ptr, mPhysicalDevice->GetDriverDescription().c_str(), driverDescriptionCLen);
    ptr += driverDescriptionCLen;
}

void APIAdapterPropertiesFreeMembers(WGPUAdapterProperties properties) {
    // This single delete is enough because everything is a single allocation.
    delete[] properties.vendorName;
}

bool AdapterBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mSupportedFeatures.IsEnabled(feature);
}

size_t AdapterBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mSupportedFeatures.EnumerateFeatures(features);
}

DeviceBase* AdapterBase::APICreateDevice(const DeviceDescriptor* descriptor) {
    constexpr DeviceDescriptor kDefaultDesc = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDesc;
    }

    auto result = CreateDevice(descriptor);
    if (result.IsError()) {
        mPhysicalDevice->GetInstance()->ConsumedError(result.AcquireError());
        return nullptr;
    }
    return result.AcquireSuccess().Detach();
}

ResultOrError<Ref<DeviceBase>> AdapterBase::CreateDevice(const DeviceDescriptor* descriptor) {
    DAWN_ASSERT(descriptor != nullptr);

    // Create device toggles state from required toggles descriptor and inherited adapter toggles
    // state.
    const DawnTogglesDescriptor* deviceTogglesDesc = nullptr;
    FindInChain(descriptor->nextInChain, &deviceTogglesDesc);

    // Create device toggles state.
    TogglesState deviceToggles =
        TogglesState::CreateFromTogglesDescriptor(deviceTogglesDesc, ToggleStage::Device);
    deviceToggles.InheritFrom(mTogglesState);
    // Default toggles for all backend
    deviceToggles.Default(Toggle::LazyClearResourceOnFirstUse, true);
    deviceToggles.Default(Toggle::TimestampQuantization, true);

    // Backend-specific forced and default device toggles
    mPhysicalDevice->SetupBackendDeviceToggles(&deviceToggles);

    // Validate all required features are supported by the adapter and suitable under device
    // toggles. Note that certain toggles in device toggles state may be overriden by user and
    // different from the adapter toggles state, and in this case a device may support features
    // that not supported by the adapter. We allow such toggles overriding for the convinience e.g.
    // creating a deivce for internal usage with AllowUnsafeAPI enabled from an adapter that
    // disabled AllowUnsafeAPIS.
    for (uint32_t i = 0; i < descriptor->requiredFeatureCount; ++i) {
        wgpu::FeatureName feature = descriptor->requiredFeatures[i];
        DAWN_TRY(mPhysicalDevice->ValidateFeatureSupportedWithToggles(feature, deviceToggles));
    }

    if (descriptor->requiredLimits != nullptr) {
        // Only consider limits in RequiredLimits structure, and currently no chained structure
        // supported.
        DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                        "can not chain after requiredLimits.");

        SupportedLimits supportedLimits;
        bool success = APIGetLimits(&supportedLimits);
        DAWN_ASSERT(success);

        DAWN_TRY_CONTEXT(ValidateLimits(supportedLimits.limits, descriptor->requiredLimits->limits),
                         "validating required limits");
    }

    return mPhysicalDevice->CreateDevice(this, descriptor, deviceToggles);
}

void AdapterBase::APIRequestDevice(const DeviceDescriptor* descriptor,
                                   WGPURequestDeviceCallback callback,
                                   void* userdata) {
    constexpr DeviceDescriptor kDefaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDescriptor;
    }
    auto result = CreateDevice(descriptor);
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

const TogglesState& AdapterBase::GetTogglesState() const {
    return mTogglesState;
}

FeatureLevel AdapterBase::GetFeatureLevel() const {
    return mFeatureLevel;
}

std::vector<Ref<AdapterBase>> SortAdapters(std::vector<Ref<AdapterBase>> adapters,
                                           const RequestAdapterOptions* options) {
    const bool highPerformance =
        options != nullptr && options->powerPreference == wgpu::PowerPreference::HighPerformance;

    const auto ComputeAdapterTypeRank = [&](const Ref<AdapterBase>& a) {
        switch (a->GetPhysicalDevice()->GetAdapterType()) {
            case wgpu::AdapterType::DiscreteGPU:
                return highPerformance ? 0 : 1;
            case wgpu::AdapterType::IntegratedGPU:
                return highPerformance ? 1 : 0;
            case wgpu::AdapterType::CPU:
                return 2;
            case wgpu::AdapterType::Unknown:
                return 3;
        }
        DAWN_UNREACHABLE();
    };

    const auto ComputeBackendTypeRank = [](const Ref<AdapterBase>& a) {
        switch (a->GetPhysicalDevice()->GetBackendType()) {
            // Sort backends generally in order of Core -> Compat -> Testing,
            // while preferring OS-specific backends like Metal/D3D.
            case wgpu::BackendType::Metal:
            case wgpu::BackendType::D3D12:
                return 0;
            case wgpu::BackendType::Vulkan:
                return 1;
            case wgpu::BackendType::D3D11:
                return 2;
            case wgpu::BackendType::OpenGLES:
                return 3;
            case wgpu::BackendType::OpenGL:
                return 4;
            case wgpu::BackendType::WebGPU:
                return 5;
            case wgpu::BackendType::Null:
                return 6;
            case wgpu::BackendType::Undefined:
                break;
        }
        DAWN_UNREACHABLE();
    };

    std::sort(adapters.begin(), adapters.end(),
              [&](const Ref<AdapterBase>& a, const Ref<AdapterBase>& b) -> bool {
                  return std::tuple(ComputeAdapterTypeRank(a), ComputeBackendTypeRank(a)) <
                         std::tuple(ComputeAdapterTypeRank(b), ComputeBackendTypeRank(b));
              });

    return adapters;
}

}  // namespace dawn::native
