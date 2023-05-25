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

// D3D12Backend.cpp: contains the definition of symbols exported by D3D12Backend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/D3D12Backend.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Math.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

uint64_t SetExternalMemoryReservation(WGPUDevice device,
                                      uint64_t requestedReservationSize,
                                      MemorySegment memorySegment) {
    Device* backendDevice = ToBackend(FromAPI(device));

    auto deviceLock(backendDevice->GetScopedLock());

    return backendDevice->GetResidencyManager()->SetExternalMemoryReservation(
        memorySegment, requestedReservationSize);
}

PhysicalDeviceDiscoveryOptions::PhysicalDeviceDiscoveryOptions()
    : PhysicalDeviceDiscoveryOptions(nullptr) {}

PhysicalDeviceDiscoveryOptions::PhysicalDeviceDiscoveryOptions(ComPtr<IDXGIAdapter> adapter)
    : d3d::PhysicalDeviceDiscoveryOptions(WGPUBackendType_D3D12, std::move(adapter)) {}

}  // namespace dawn::native::d3d12
