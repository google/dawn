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

#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Limits.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

class AdapterBase : public RefCounted {
  public:
    AdapterBase(InstanceBase* instance, wgpu::BackendType backend);
    virtual ~AdapterBase() = default;

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
    wgpu::BackendType GetBackendType() const;
    InstanceBase* GetInstance() const;

    void ResetInternalDeviceForTesting();

    FeaturesSet GetSupportedFeatures() const;
    bool SupportsAllRequiredFeatures(
        const ityp::span<size_t, const wgpu::FeatureName>& features) const;
    WGPUDeviceProperties GetAdapterProperties() const;

    bool GetLimits(SupportedLimits* limits) const;

    void SetUseTieredLimits(bool useTieredLimits);

    virtual bool SupportsExternalImages() const = 0;

  protected:
    uint32_t mVendorId = 0xFFFFFFFF;
    uint32_t mDeviceId = 0xFFFFFFFF;
    std::string mName;
    wgpu::AdapterType mAdapterType = wgpu::AdapterType::Unknown;
    std::string mDriverDescription;
    FeaturesSet mSupportedFeatures;

  private:
    virtual ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor) = 0;

    virtual MaybeError InitializeImpl() = 0;

    // Check base WebGPU features and discover supported featurees.
    virtual MaybeError InitializeSupportedFeaturesImpl() = 0;

    // Check base WebGPU limits and populate supported limits.
    virtual MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) = 0;

    ResultOrError<Ref<DeviceBase>> CreateDeviceInternal(const DeviceDescriptor* descriptor);

    virtual MaybeError ResetInternalDeviceForTestingImpl();
    InstanceBase* mInstance = nullptr;
    wgpu::BackendType mBackend;
    CombinedLimits mLimits;
    bool mUseTieredLimits = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ADAPTER_H_
