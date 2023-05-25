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

// D3D11Backend.cpp: contains the definition of symbols exported by D3D11Backend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/D3D11Backend.h"

#include <utility>

#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"

namespace dawn::native::d3d11 {

PhysicalDeviceDiscoveryOptions::PhysicalDeviceDiscoveryOptions()
    : PhysicalDeviceDiscoveryOptions(nullptr) {}

PhysicalDeviceDiscoveryOptions::PhysicalDeviceDiscoveryOptions(ComPtr<IDXGIAdapter> adapter)
    : d3d::PhysicalDeviceDiscoveryOptions(WGPUBackendType_D3D11, std::move(adapter)) {}

Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device(WGPUDevice device) {
    return ToBackend(FromAPI(device))->GetD3D11Device();
}

}  // namespace dawn::native::d3d11
