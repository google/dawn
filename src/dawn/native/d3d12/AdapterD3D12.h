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

#ifndef SRC_DAWN_NATIVE_D3D12_ADAPTERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_ADAPTERD3D12_H_

#include "dawn/native/Adapter.h"

#include "dawn/native/d3d12/D3D12Info.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Backend;

class Adapter : public AdapterBase {
  public:
    Adapter(Backend* backend, ComPtr<IDXGIAdapter3> hardwareAdapter);
    ~Adapter() override;

    // AdapterBase Implementation
    bool SupportsExternalImages() const override;

    const D3D12DeviceInfo& GetDeviceInfo() const;
    IDXGIAdapter3* GetHardwareAdapter() const;
    Backend* GetBackend() const;
    ComPtr<ID3D12Device> GetDevice() const;

  private:
    void SetupBackendDeviceToggles(TogglesState* deviceToggles) const override;

    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                    const TogglesState& deviceToggles) override;

    MaybeError ResetInternalDeviceForTestingImpl() override;

    bool AreTimestampQueriesSupported() const;

    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    MaybeError ValidateFeatureSupportedWithDeviceTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& deviceTogglesState) override;

    MaybeError InitializeDebugLayerFilters();
    void CleanUpDebugLayerFilters();

    ComPtr<IDXGIAdapter3> mHardwareAdapter;
    ComPtr<ID3D12Device> mD3d12Device;

    Backend* mBackend;
    D3D12DeviceInfo mDeviceInfo = {};
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_ADAPTERD3D12_H_
