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

#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;
class TogglesState;
struct SupportedLimits;

class AdapterBase : public RefCounted {
  public:
    AdapterBase(Ref<PhysicalDeviceBase> physicalDevice, FeatureLevel featureLevel);
    AdapterBase(Ref<PhysicalDeviceBase> physicalDevice,
                FeatureLevel featureLevel,
                const TogglesState& adapterToggles);
    ~AdapterBase() override;

    // WebGPU API
    InstanceBase* APIGetInstance() const;
    bool APIGetLimits(SupportedLimits* limits) const;
    void APIGetProperties(AdapterProperties* properties) const;
    bool APIHasFeature(wgpu::FeatureName feature) const;
    size_t APIEnumerateFeatures(wgpu::FeatureName* features) const;
    void APIRequestDevice(const DeviceDescriptor* descriptor,
                          WGPURequestDeviceCallback callback,
                          void* userdata);
    DeviceBase* APICreateDevice(const DeviceDescriptor* descriptor = nullptr);
    ResultOrError<Ref<DeviceBase>> CreateDevice(const DeviceDescriptor* descriptor);

    void SetUseTieredLimits(bool useTieredLimits);

    // Return the underlying PhysicalDevice.
    PhysicalDeviceBase* GetPhysicalDevice();

    // Get the actual toggles state of the adapter.
    const TogglesState& GetTogglesState() const;

    FeatureLevel GetFeatureLevel() const;

  private:
    Ref<PhysicalDeviceBase> mPhysicalDevice;
    FeatureLevel mFeatureLevel;
    bool mUseTieredLimits = false;
    // Adapter toggles state, currently only inherited from instance toggles state.
    TogglesState mTogglesState;
};

std::vector<Ref<AdapterBase>> SortAdapters(std::vector<Ref<AdapterBase>> adapters,
                                           const RequestAdapterOptions* options);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ADAPTER_H_
