// Copyright 2023 The Dawn Authors
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
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/PhysicalDevice.h"

namespace dawn::native {

AdapterBase::AdapterBase(Ref<PhysicalDeviceBase> physicalDevice, FeatureLevel featureLevel)
    : AdapterBase(physicalDevice,
                  featureLevel,
                  TogglesState(ToggleStage::Adapter)
                      .InheritFrom(physicalDevice->GetInstance()->GetTogglesState())) {}

AdapterBase::AdapterBase(Ref<PhysicalDeviceBase> physicalDevice,
                         FeatureLevel featureLevel,
                         const TogglesState& adapterToggles)
    : mPhysicalDevice(std::move(physicalDevice)),
      mFeatureLevel(featureLevel),
      mTogglesState(adapterToggles) {
    ASSERT(mPhysicalDevice->SupportsFeatureLevel(featureLevel));
    ASSERT(mTogglesState.GetStage() == ToggleStage::Adapter);
}

AdapterBase::~AdapterBase() = default;

void AdapterBase::SetUseTieredLimits(bool useTieredLimits) {
    mUseTieredLimits = useTieredLimits;
}

PhysicalDeviceBase* AdapterBase::GetPhysicalDevice() {
    return mPhysicalDevice.Get();
}

InstanceBase* AdapterBase::APIGetInstance() const {
    InstanceBase* instance = mPhysicalDevice->GetInstance();
    ASSERT(instance != nullptr);
    instance->APIReference();
    return instance;
}

bool AdapterBase::APIGetLimits(SupportedLimits* limits) const {
    ASSERT(limits != nullptr);
    if (limits->nextInChain != nullptr) {
        return false;
    }
    if (mUseTieredLimits) {
        limits->limits = ApplyLimitTiers(mPhysicalDevice->GetLimits().v1);
    } else {
        limits->limits = mPhysicalDevice->GetLimits().v1;
    }
    return true;
}

void AdapterBase::APIGetProperties(AdapterProperties* properties) const {
    ASSERT(properties != nullptr);

    MaybeError result = ValidateSingleSType(properties->nextInChain,
                                            wgpu::SType::DawnAdapterPropertiesPowerPreference);
    if (result.IsError()) {
        mPhysicalDevice->GetInstance()->ConsumedError(result.AcquireError());
        return;
    }

    DawnAdapterPropertiesPowerPreference* powerPreferenceDesc = nullptr;
    FindInChain(properties->nextInChain, &powerPreferenceDesc);

    if (powerPreferenceDesc != nullptr) {
        powerPreferenceDesc->powerPreference = wgpu::PowerPreference::Undefined;
    }
    properties->vendorID = mPhysicalDevice->GetVendorId();
    properties->vendorName = mPhysicalDevice->GetVendorName().c_str();
    properties->architecture = mPhysicalDevice->GetArchitectureName().c_str();
    properties->deviceID = mPhysicalDevice->GetDeviceId();
    properties->name = mPhysicalDevice->GetName().c_str();
    properties->driverDescription = mPhysicalDevice->GetDriverDescription().c_str();
    properties->adapterType = mPhysicalDevice->GetAdapterType();
    properties->backendType = mPhysicalDevice->GetBackendType();
    properties->compatibilityMode = mFeatureLevel == FeatureLevel::Compatibility;
}

bool AdapterBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mPhysicalDevice->HasFeature(feature);
}

size_t AdapterBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mPhysicalDevice->EnumerateFeatures(features);
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
    ASSERT(descriptor != nullptr);

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

    // Backend-specific forced and default device toggles
    mPhysicalDevice->SetupBackendDeviceToggles(&deviceToggles);

    // Validate all required features are supported by the adapter and suitable under given toggles.
    // Note that certain toggles in device toggles state may be overriden by user and different from
    // the adapter toggles state.
    // TODO(dawn:1495): After implementing adapter toggles, decide whether we should validate
    // supported features using adapter toggles or device toggles.
    for (uint32_t i = 0; i < descriptor->requiredFeaturesCount; ++i) {
        wgpu::FeatureName feature = descriptor->requiredFeatures[i];
        DAWN_TRY(mPhysicalDevice->ValidateFeatureSupportedWithToggles(feature, deviceToggles));
    }

    if (descriptor->requiredLimits != nullptr) {
        SupportedLimits supportedLimits;
        bool success = APIGetLimits(&supportedLimits);
        ASSERT(success);

        DAWN_TRY_CONTEXT(ValidateLimits(supportedLimits.limits, descriptor->requiredLimits->limits),
                         "validating required limits");

        DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                        "nextInChain is not nullptr.");
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
        }
    };

    std::sort(adapters.begin(), adapters.end(),
              [&](const Ref<AdapterBase>& a, const Ref<AdapterBase>& b) -> bool {
                  return std::tuple(ComputeAdapterTypeRank(a), ComputeBackendTypeRank(a)) <
                         std::tuple(ComputeAdapterTypeRank(b), ComputeBackendTypeRank(b));
              });

    return adapters;
}

}  // namespace dawn::native
