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

#ifndef SRC_DAWN_NATIVE_ADAPTER_H_
#define SRC_DAWN_NATIVE_ADAPTER_H_

#include <string>

#include "dawn/native/DawnNative.h"

#include "dawn/common/GPUInfo.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Limits.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

class AdapterBase : public RefCounted {
  public:
    AdapterBase(InstanceBase* instance, wgpu::BackendType backend);
    ~AdapterBase() override;

    MaybeError Initialize();

    // WebGPU API
    bool APIGetLimits(SupportedLimits* limits) const;
    void APIGetProperties(AdapterProperties* properties) const;
    bool APIHasFeature(wgpu::FeatureName feature) const;
    size_t APIEnumerateFeatures(wgpu::FeatureName* features) const;
    void APIRequestDevice(const DeviceDescriptor* descriptor,
                          WGPURequestDeviceCallback callback,
                          void* userdata);
    DeviceBase* APICreateDevice(const DeviceDescriptor* descriptor = nullptr);

    uint32_t GetVendorId() const;
    uint32_t GetDeviceId() const;
    const gpu_info::DriverVersion& GetDriverVersion() const;
    wgpu::BackendType GetBackendType() const;
    InstanceBase* GetInstance() const;

    void ResetInternalDeviceForTesting();

    FeaturesSet GetSupportedFeatures() const;
    bool SupportsAllRequiredFeatures(
        const ityp::span<size_t, const wgpu::FeatureName>& features) const;

    bool GetLimits(SupportedLimits* limits) const;

    void SetUseTieredLimits(bool useTieredLimits);

    virtual bool SupportsExternalImages() const = 0;

  protected:
    uint32_t mVendorId = 0xFFFFFFFF;
    std::string mVendorName;
    std::string mArchitectureName;
    uint32_t mDeviceId = 0xFFFFFFFF;
    std::string mName;
    wgpu::AdapterType mAdapterType = wgpu::AdapterType::Unknown;
    gpu_info::DriverVersion mDriverVersion;
    std::string mDriverDescription;

    // Features set that CAN be supported by devices of this adapter. Some features in this set may
    // be guarded by toggles, and creating a device with these features required may result in a
    // validation error if proper toggles are not enabled/disabled.
    FeaturesSet mSupportedFeatures;
    // Check if a feature os supported by this adapter AND suitable with given toggles.
    // TODO(dawn:1495): After implementing adapter toggles, remove this and use adapter toggles
    // instead of device toggles to validate supported features.
    MaybeError ValidateFeatureSupportedWithDeviceToggles(wgpu::FeatureName feature,
                                                         const TogglesState& deviceTogglesState);

  private:
    // Backend-specific force-setting and defaulting device toggles
    virtual void SetupBackendDeviceToggles(TogglesState* deviceToggles) const = 0;

    virtual ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                            const TogglesState& deviceToggles) = 0;

    virtual MaybeError InitializeImpl() = 0;

    // Check base WebGPU features and discover supported features.
    virtual void InitializeSupportedFeaturesImpl() = 0;

    // Check base WebGPU limits and populate supported limits.
    virtual MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) = 0;

    virtual void InitializeVendorArchitectureImpl();

    virtual MaybeError ValidateFeatureSupportedWithDeviceTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& deviceTogglesState) = 0;

    ResultOrError<Ref<DeviceBase>> CreateDeviceInternal(const DeviceDescriptor* descriptor);

    virtual MaybeError ResetInternalDeviceForTestingImpl();
    Ref<InstanceBase> mInstance;
    wgpu::BackendType mBackend;
    CombinedLimits mLimits;
    bool mUseTieredLimits = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ADAPTER_H_
